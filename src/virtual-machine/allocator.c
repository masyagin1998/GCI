#include "allocator.h"

#include "utils.h"

/*
  Allocator is a dynamic array divided into blocks.
  Each block consists of data and two boundary descriptors:
  [flag][len][prev][next]...[len][flag], where
  [flag] takes values 0 (block is free) and 1 (block is busy) and uses 1 byte of memory,
  [len] is size of block (including boundary descriptors) in bytes and uses 8 bytes of memory,
  [prev] is pointer to previous block (to its left [flag]) and uses 8 bytes of memory,
  [next] is pointer to next block and (to its left [flag]) also uses 8 bytes of memory,
  so every block has 34 bytes size overhead.
*/

#define BLOCK_OVERHEAD (2 * (sizeof(char) + sizeof(size_t) + sizeof(void*)))
#define MIN_BLOCK_LEN (BLOCK_OVERHEAD + sizeof(char))

/*      -position-                 -type-          -l flag-         -l len-        -list prev-      -list next-      -data-                              -r len-                          -r flag-       -arr prev/next- */
#define BLOCK_L_FLAG(block)     (*((char*)   (((char*) (block)))))
#define BLOCK_L_DATA_LEN(block) (*((size_t*) (((char*) (block)) + sizeof(char))))

#define BLOCK_LIST_PREV(block)  (*((void**)  (((char*) (block)) + sizeof(char) + sizeof(size_t))))
#define BLOCK_LIST_NEXT(block)  (*((void**)  (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*))))

#define BLOCK_DATA(block)       ((void*)     (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*)))

#define BLOCK_R_DATA_LEN(block) (*((size_t*) (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*) + ((BLOCK_L_DATA_LEN((char*) (block))) * sizeof(char)))))
#define BLOCK_R_FLAG(block)     (*((char*)   (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*) + ((BLOCK_L_DATA_LEN((char*) (block))) * sizeof(char)) + sizeof(size_t))))

#define BLOCK_ARR_NEXT(block)   ((void*)     (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*) + ((BLOCK_L_DATA_LEN((char*) (block))) * sizeof(char)) + sizeof(size_t) + sizeof(char)))

/*      -position-                   -type-          -cur-            -r flag-       -r len- */
#define BLOCK_ARR_PREV_INNER(block) ((char*) (((char*) (block)) - sizeof(char) - sizeof(size_t)))
/*      -position-                    -type-          -r len-                                           -data-                             -list next-     -list prev-       -l len-         -l flag-    */
#define BLOCK_ARR_PREV(block)       ((void*) (BLOCK_ARR_PREV_INNER(block) - ((*((size_t*) BLOCK_ARR_PREV_INNER(block))) * sizeof(char)) - sizeof(void*) - sizeof(void*) - sizeof(size_t) - sizeof(char)))

#define FREE_BLOCK 0
#define BUSY_BLOCK 1
#define ANY_BLOCK  2

struct ALLOCATOR
{
    char*mem;
    size_t sizemem;

    /* free blocks. */
    void*first, *last;
    
};

allocator_type_t create_allocator()
{
    struct ALLOCATOR*allocator;
    SAFE_MALLOC(allocator, 1);
    return allocator;
}

void allocator_conf(allocator_type_t a, size_t sizemem_start)
{
    unsigned len;

    if (sizemem_start < MIN_BLOCK_LEN) {
        fprintf(stderr, "not enough memory for allocator:\n");
        fprintf(stderr, "got:         %ld bytes\n", sizemem_start);
        fprintf(stderr, "minimum:     %ld bytes;\n", MIN_BLOCK_LEN);
        fprintf(stderr, "recommended: %ld bytes;\n", MIN_BLOCK_LEN * 1024 * 1024);
        
        exit(EXIT_FAILURE);
    }
    
    a->sizemem = sizemem_start;
    SAFE_MALLOC(a->mem, a->sizemem);

    len = a->sizemem - BLOCK_OVERHEAD;

    BLOCK_L_FLAG(a->mem) = FREE_BLOCK;
    BLOCK_L_DATA_LEN(a->mem) = len;
    BLOCK_LIST_PREV(a->mem) = NULL;
    BLOCK_LIST_NEXT(a->mem) = NULL;
    BLOCK_R_DATA_LEN(a->mem) = len;
    BLOCK_R_FLAG(a->mem) = FREE_BLOCK;

    a->first = a->mem;
    a->last = a->mem;
}

static void allocator_list_remove_elem(allocator_type_t a, void*elem)
{
    void*prev = BLOCK_LIST_PREV(elem);
    void*next = BLOCK_LIST_NEXT(elem);

    if (prev != NULL) {
        BLOCK_LIST_NEXT(prev) = next;
    }
    if (next != NULL) {
        BLOCK_LIST_PREV(next) = prev;
    }

    if (prev == NULL) {
        a->first = next;
            
    }
    if (next == NULL) {
        a->last = prev;
    }
}

static void*allocator_list_search_by_sizemem(allocator_type_t a, size_t sizemem)
{
    void*cur = a->first;

    while (cur != NULL) {
        if (BLOCK_L_DATA_LEN(cur) >= sizemem) {
            return cur;
        }

        cur = BLOCK_LIST_NEXT(cur);
    }

    return NULL;
}

static void allocator_list_push_front(allocator_type_t a, void*elem)
{
    BLOCK_LIST_NEXT(elem) = a->first;
    BLOCK_LIST_PREV(elem) = NULL;

    if (a->first != NULL) {
        BLOCK_LIST_PREV(a->first) = elem;
    }
    
    a->first = elem;

    if (a->last == NULL) {
        a->last = elem;
    }
}

static void allocator_list_push_back(allocator_type_t a, void*elem)
{
    BLOCK_LIST_NEXT(elem) = NULL;
    BLOCK_LIST_PREV(elem) = a->last;
    
    if (a->last != NULL) {
        BLOCK_LIST_NEXT(a->last) = elem;
    }
    
    a->last = elem;
    
    if (a->first == NULL) {
        a->first = elem;
    }
}

static void allocator_list_insert_elem(allocator_type_t a, void*elem)
{
    size_t len = BLOCK_L_DATA_LEN(elem);
    void*next_elem = allocator_list_search_by_sizemem(a, len);

    if (next_elem == NULL) {
        allocator_list_push_back(a, elem);
    } else {
        void*prev_elem;
        
        if (BLOCK_LIST_PREV(next_elem) == NULL) {
            allocator_list_push_front(a, elem);
            return;
        }

        BLOCK_LIST_PREV(elem) = BLOCK_LIST_PREV(next_elem);
        prev_elem = BLOCK_LIST_PREV(next_elem);
        BLOCK_LIST_NEXT(prev_elem) = elem;
        BLOCK_LIST_NEXT(elem) = next_elem;
        BLOCK_LIST_PREV(next_elem) = elem;
    }
}

void*allocator_malloc_mem(allocator_type_t a, size_t sizemem)
{
    void*cur = allocator_list_search_by_sizemem(a, sizemem);

    if (cur != NULL) {
        /* remove block from list. */
        allocator_list_remove_elem(a, cur);
        
        if (BLOCK_L_DATA_LEN(cur) < (sizemem + MIN_BLOCK_LEN)) {
            printf("kek\n");
            /* don't need to divide block. */
            BLOCK_L_FLAG(cur) = BUSY_BLOCK;
            BLOCK_R_FLAG(cur) = BUSY_BLOCK;
            return BLOCK_DATA(cur);
        } else {
            unsigned len = BLOCK_L_DATA_LEN(cur) - sizemem - BLOCK_OVERHEAD;
            void*tmp_block;
            /* have to divide block. */

            /* split block into two smaller blocks (one of them will contain data) */
            BLOCK_L_FLAG(cur) = BUSY_BLOCK;
            BLOCK_L_DATA_LEN(cur) = sizemem;
            BLOCK_LIST_PREV(cur) = NULL;
            BLOCK_LIST_NEXT(cur) = NULL;
            BLOCK_R_DATA_LEN(cur) = sizemem;
            BLOCK_R_FLAG(cur) = BUSY_BLOCK;

            tmp_block = BLOCK_ARR_NEXT(cur);
            BLOCK_L_FLAG(tmp_block) = FREE_BLOCK;
            BLOCK_L_DATA_LEN(tmp_block) = len;
            BLOCK_LIST_PREV(tmp_block) = NULL;
            BLOCK_LIST_NEXT(tmp_block) = NULL;
            BLOCK_R_DATA_LEN(tmp_block) = len;
            BLOCK_R_FLAG(tmp_block) = FREE_BLOCK;

            allocator_list_insert_elem(a, tmp_block);

            return BLOCK_DATA(cur);
        }
    } else {
        // TODO.
        printf("malloc with reallocation is not implemented\n");
        exit(1);
    }
}

void*allocator_realloc_mem(allocator_type_t a, void*ptrmem, size_t sizemem)
{
    // TODO.
    printf("realloc is not implemented\n");
    exit(1);
}

void allocator_free_mem(allocator_type_t a, void*ptrmem)
{
    // TODO.
    printf("free is not implemented\n");
    exit(1);
}

void allocator_free(allocator_type_t a)
{
    SAFE_FREE(a->mem);
    SAFE_FREE(a);
}

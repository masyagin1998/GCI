#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

#include <string.h>

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

#define BLOCK_L_OVERHEAD (sizeof(char) + sizeof(size_t) + 2 * sizeof(void*))
#define BLOCK_R_OVERHEAD (sizeof(size_t) + sizeof(char))
#define BLOCK_OVERHEAD (BLOCK_L_OVERHEAD + BLOCK_R_OVERHEAD)
#define MIN_BLOCK_LEN (BLOCK_OVERHEAD + sizeof(char))

/*      -position-                 -type-          -l flag-         -l len-        -list prev-      -list next-      -data-                              -r len-                          -r flag-         -arr next-   */
#define BLOCK_L_FLAG(block)     (*((char*)   (((char*) (block)))))
#define BLOCK_L_DATA_LEN(block) (*((size_t*) (((char*) (block)) + sizeof(char))))

#define BLOCK_LIST_PREV(block)  (*((void**)  (((char*) (block)) + sizeof(char) + sizeof(size_t))))
#define BLOCK_LIST_NEXT(block)  (*((void**)  (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*))))

#define BLOCK_DATA(block)       ((void*)     (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*)))

#define BLOCK_R_DATA_LEN(block) (*((size_t*) (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*) + ((BLOCK_L_DATA_LEN((char*) (block))) * sizeof(char)))))
#define BLOCK_R_FLAG(block)     (*((char*)   (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*) + ((BLOCK_L_DATA_LEN((char*) (block))) * sizeof(char)) + sizeof(size_t))))

#define BLOCK_ARR_NEXT(block)   ((void*)     (((char*) (block)) + sizeof(char) + sizeof(size_t) + sizeof(void*) + sizeof(void*) + ((BLOCK_L_DATA_LEN((char*) (block))) * sizeof(char)) + sizeof(size_t) + sizeof(char)))

/*      -position-                   -type-         -cur-         -list next-     -list prev-       -l len-         -l flag-    */
#define BLOCK_PTR_FROM_DATA(block) ((void*) (((char*) (block)) - sizeof(void*) - sizeof(void*) - sizeof(size_t) - sizeof(char)))

/*      -position-                   -type-          -cur-            -r flag-       -r len- */
#define BLOCK_ARR_PREV_INNER(block) ((char*) (((char*) (block)) - sizeof(char) - sizeof(size_t)))
/*      -position-                    -type-          -r len-                                           -data-                             -list next-     -list prev-       -l len-         -l flag-    */
#define BLOCK_ARR_PREV(block)       ((void*) (BLOCK_ARR_PREV_INNER(block) - ((*((size_t*) BLOCK_ARR_PREV_INNER(block))) * sizeof(char)) - sizeof(void*) - sizeof(void*) - sizeof(size_t) - sizeof(char)))

#define FREE_BLOCK 0
#define BUSY_BLOCK 1
#define ANY_BLOCK  2

struct ALLOCATOR_LIST
{
    void*first, *last;
    size_t count;
    size_t sizemem;
};

struct ALLOCATOR
{
    char*mem;
    size_t sizemem;

    /* free blocks. */
    struct ALLOCATOR_LIST free_list;

    /* busy blocks. */
    struct ALLOCATOR_LIST busy_list;
};

typedef struct ALLOCATOR* allocator_type_t;

void allocator_malloc_pool(allocator_type_t a, size_t sizemem);
void allocator_free_pool(allocator_type_t a);

void allocator_clean_pool(allocator_type_t a);

void*allocator_malloc_block(allocator_type_t a, size_t sizemem);
void*allocator_realloc_block(allocator_type_t a, void*ptrmem, size_t sizemem);
void allocator_free_block(allocator_type_t a, void*ptrmem);

#endif  /* ALLOCATOR_H_INCLUDED */

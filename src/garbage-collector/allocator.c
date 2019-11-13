#include "allocator.h"

#include "utils.h"

void allocator_malloc_pool(allocator_type_t a, size_t sizemem)
{
    unsigned len;

    if (sizemem < MIN_BLOCK_LEN) {
        fprintf(stderr, "not enough memory for allocator:\n");
        fprintf(stderr, "got:         %ld bytes\n", sizemem);
        fprintf(stderr, "minimum:     %ld bytes;\n", MIN_BLOCK_LEN);
        fprintf(stderr, "recommended: %ld bytes;\n", MIN_BLOCK_LEN * 1024 * 1024);
        
        exit(EXIT_FAILURE);
    }
    
    a->sizemem = sizemem;
    SAFE_MALLOC(a->mem, a->sizemem);

    len = a->sizemem - BLOCK_OVERHEAD;

    BLOCK_L_FLAG(a->mem) = FREE_BLOCK;
    BLOCK_L_DATA_LEN(a->mem) = len;
    BLOCK_LIST_PREV(a->mem) = NULL;
    BLOCK_LIST_NEXT(a->mem) = NULL;
    BLOCK_R_DATA_LEN(a->mem) = len;
    BLOCK_R_FLAG(a->mem) = FREE_BLOCK;

    a->free_lst.first = a->mem;
    a->free_lst.last = a->mem;


}

void allocator_realloc_pool(allocator_type_t a, size_t sizemem)
{
    
}

void allocator_free_pool(allocator_type_t a)
{
    SAFE_FREE(a->mem);
}

static void allocator_list_remove_elem(struct ALLOCATOR_LIST*lst, void*elem)
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
        lst->first = next;
            
    }
    if (next == NULL) {
        lst->last = prev;
    }

    lst->count--;
}

static void*allocator_list_search_by_sizemem(struct ALLOCATOR_LIST*lst, size_t sizemem)
{
    void*cur = lst->first;

    while (cur != NULL) {
        if (BLOCK_L_DATA_LEN(cur) >= sizemem) {
            return cur;
        }

        cur = BLOCK_LIST_NEXT(cur);
    }

    return NULL;
}

static void allocator_list_push_front(struct ALLOCATOR_LIST*lst, void*elem)
{
    BLOCK_LIST_NEXT(elem) = lst->first;
    BLOCK_LIST_PREV(elem) = NULL;

    if (lst->first != NULL) {
        BLOCK_LIST_PREV(lst->first) = elem;
    }
    
    lst->first = elem;

    if (lst->last == NULL) {
        lst->last = elem;
    }

    lst->count++;    
}

static void allocator_list_push_back(struct ALLOCATOR_LIST*lst, void*elem)
{
    BLOCK_LIST_NEXT(elem) = NULL;
    BLOCK_LIST_PREV(elem) = lst->last;
    
    if (lst->last != NULL) {
        BLOCK_LIST_NEXT(lst->last) = elem;
    }
    
    lst->last = elem;
    
    if (lst->first == NULL) {
        lst->first = elem;
    }

    lst->count++;
}

static void allocator_list_insert_elem(struct ALLOCATOR_LIST*lst, void*elem)
{
    size_t len = BLOCK_L_DATA_LEN(elem);
    void*next_elem = allocator_list_search_by_sizemem(lst, len);

    if (next_elem == NULL) {
        allocator_list_push_back(lst, elem);
    } else {
        void*prev_elem;
        
        if (BLOCK_LIST_PREV(next_elem) == NULL) {
            allocator_list_push_front(lst, elem);
            return;
        }

        BLOCK_LIST_PREV(elem) = BLOCK_LIST_PREV(next_elem);
        prev_elem = BLOCK_LIST_PREV(next_elem);
        BLOCK_LIST_NEXT(prev_elem) = elem;
        BLOCK_LIST_NEXT(elem) = next_elem;
        BLOCK_LIST_PREV(next_elem) = elem;

        lst->count++;
    }
}

void*allocator_malloc_block(allocator_type_t a, size_t sizemem)
{
    void*cur;

    if ((BLOCK_L_DATA_LEN(a->free_lst.last) + BLOCK_OVERHEAD) < sizemem) {
        return NULL;
    }

    cur = allocator_list_search_by_sizemem(&(a->free_lst), sizemem);

    /* remove block from list. */
    allocator_list_remove_elem(&(a->free_lst), cur);
    
    if (BLOCK_L_DATA_LEN(cur) < (sizemem + MIN_BLOCK_LEN)) {
        /* don't need to divide block. */
        BLOCK_L_FLAG(cur) = BUSY_BLOCK;
        BLOCK_R_FLAG(cur) = BUSY_BLOCK;
        allocator_list_push_back(&(a->busy_lst), cur);        
        return BLOCK_DATA(cur);
    }

    unsigned len = BLOCK_L_DATA_LEN(cur) - sizemem - BLOCK_OVERHEAD;
    void*tmp_block;
    /* have to divide block. */
    
    /* split block into two smaller blocks (one of them will contain data). */
    
    /* put busy block to busy list. */
    BLOCK_L_FLAG(cur) = BUSY_BLOCK;
    BLOCK_L_DATA_LEN(cur) = sizemem;
    BLOCK_LIST_PREV(cur) = NULL;
    BLOCK_LIST_NEXT(cur) = NULL;
    BLOCK_R_DATA_LEN(cur) = sizemem;
    BLOCK_R_FLAG(cur) = BUSY_BLOCK;
    allocator_list_push_back(&(a->busy_lst), cur);
    
    /* put free block to free list. */
    tmp_block = BLOCK_ARR_NEXT(cur);
    BLOCK_L_FLAG(tmp_block) = FREE_BLOCK;
    BLOCK_L_DATA_LEN(tmp_block) = len;
    BLOCK_LIST_PREV(tmp_block) = NULL;
    BLOCK_LIST_NEXT(tmp_block) = NULL;
    BLOCK_R_DATA_LEN(tmp_block) = len;
    BLOCK_R_FLAG(tmp_block) = FREE_BLOCK;
    allocator_list_insert_elem(&(a->free_lst), tmp_block);
    
    return BLOCK_DATA(cur);
}

void*allocator_realloc_block(allocator_type_t a, void*ptrmem, size_t sizemem)
{
    void*orig;
    void*cur;

    if ((BLOCK_L_DATA_LEN(a->free_lst.last) + BLOCK_OVERHEAD) < sizemem) {
        return NULL;
    }    

    orig = BLOCK_PTR_FROM_DATA(ptrmem);

    cur = allocator_list_search_by_sizemem(&(a->free_lst), sizemem);

    /* remove block from list. */
    allocator_list_remove_elem(&(a->free_lst), cur);

    if (BLOCK_L_DATA_LEN(cur) < (sizemem + MIN_BLOCK_LEN)) {
        /* don't need to divide block. */
        BLOCK_L_FLAG(cur) = BUSY_BLOCK;
        BLOCK_R_FLAG(cur) = BUSY_BLOCK;
        allocator_list_push_back(&(a->busy_lst), cur);
        memcpy(BLOCK_DATA(cur), BLOCK_DATA(orig), BLOCK_L_DATA_LEN(orig));
        return BLOCK_DATA(cur);
    }

    unsigned len = BLOCK_L_DATA_LEN(cur) - sizemem - BLOCK_OVERHEAD;
    void*tmp_block;
    /* have to divide block. */
    
    /* split block into two smaller blocks (one of them will contain data). */
    
    /* put busy block to busy list. */
    BLOCK_L_FLAG(cur) = BUSY_BLOCK;
    BLOCK_L_DATA_LEN(cur) = sizemem;
    BLOCK_LIST_PREV(cur) = NULL;
    BLOCK_LIST_NEXT(cur) = NULL;
    BLOCK_R_DATA_LEN(cur) = sizemem;
    BLOCK_R_FLAG(cur) = BUSY_BLOCK;
    allocator_list_push_back(&(a->busy_lst), cur);
    memcpy(BLOCK_DATA(cur), BLOCK_DATA(orig), BLOCK_L_DATA_LEN(orig));
    
    /* put free block to free list. */
    tmp_block = BLOCK_ARR_NEXT(cur);
    BLOCK_L_FLAG(tmp_block) = FREE_BLOCK;
    BLOCK_L_DATA_LEN(tmp_block) = len;
    BLOCK_LIST_PREV(tmp_block) = NULL;
    BLOCK_LIST_NEXT(tmp_block) = NULL;
    BLOCK_R_DATA_LEN(tmp_block) = len;
    BLOCK_R_FLAG(tmp_block) = FREE_BLOCK;
    allocator_list_insert_elem(&(a->free_lst), tmp_block);
    
    return BLOCK_DATA(cur);
}

void allocator_free_block(allocator_type_t a, void*ptrmem)
{
    // TODO.
    printf("free is not implemented\n");
    exit(1);
}


#include "allocator.h"

#include "utils.h"

void allocator_malloc_pool(allocator_type_t a, size_t sizemem)
{
    if (sizemem < MIN_BLOCK_LEN) {
        fprintf(stderr, "not enough memory for allocator:\n");
        fprintf(stderr, "got:         %ld bytes\n", sizemem);
        fprintf(stderr, "minimum:     %ld bytes;\n", MIN_BLOCK_LEN);
        fprintf(stderr, "recommended: %ld bytes;\n", MIN_BLOCK_LEN * 1024 * 1024);
        
        exit(EXIT_FAILURE);
    }
    
    a->sizemem = sizemem;
    SAFE_MALLOC(a->mem, a->sizemem);

    allocator_clean_pool(a);
}

void allocator_free_pool(allocator_type_t a)
{
    SAFE_FREE(a->mem);
}

void allocator_clean_pool(allocator_type_t a)
{
    size_t len = a->sizemem - BLOCK_OVERHEAD;

    BLOCK_L_FLAG(a->mem) = FREE_BLOCK;
    BLOCK_L_DATA_LEN(a->mem) = len;
    BLOCK_LIST_PREV(a->mem) = NULL;
    BLOCK_LIST_NEXT(a->mem) = NULL;
    BLOCK_R_DATA_LEN(a->mem) = len;
    BLOCK_R_FLAG(a->mem) = FREE_BLOCK;

    a->free_list.first = a->mem;
    a->free_list.last = a->mem;
    a->free_list.count = 1;
    a->busy_list.sizemem = a->sizemem;

    a->busy_list.first = NULL;
    a->busy_list.last = NULL;
    a->busy_list.count = 0;
    a->busy_list.sizemem = 0;
}

static void allocator_list_remove_elem(struct ALLOCATOR_LIST*list, void*elem)
{
    void*list_prev = BLOCK_LIST_PREV(elem);
    void*list_next = BLOCK_LIST_NEXT(elem);

    if (list_prev != NULL) {
        BLOCK_LIST_NEXT(list_prev) = list_next;
    }
    if (list_next != NULL) {
        BLOCK_LIST_PREV(list_next) = list_prev;
    }

    if (list_prev == NULL) {
        list->first = list_next;
            
    }
    if (list_next == NULL) {
        list->last = list_prev;
    }

    list->count--;
    list->sizemem -= BLOCK_L_DATA_LEN(elem);
}

static void*allocator_list_search_by_sizemem(struct ALLOCATOR_LIST*list, size_t sizemem)
{
    void*cur = list->first;

    while (cur != NULL) {
        if (BLOCK_L_DATA_LEN(cur) >= sizemem) {
            return cur;
        }

        cur = BLOCK_LIST_NEXT(cur);
    }

    return NULL;
}

static void allocator_list_push_front(struct ALLOCATOR_LIST*list, void*elem)
{
    BLOCK_LIST_NEXT(elem) = list->first;
    BLOCK_LIST_PREV(elem) = NULL;

    if (list->first != NULL) {
        BLOCK_LIST_PREV(list->first) = elem;
    }
    
    list->first = elem;

    if (list->last == NULL) {
        list->last = elem;
    }

    list->count++;
    list->sizemem += BLOCK_L_DATA_LEN(elem);    
}

static void allocator_list_push_back(struct ALLOCATOR_LIST*list, void*elem)
{
    BLOCK_LIST_NEXT(elem) = NULL;
    BLOCK_LIST_PREV(elem) = list->last;
    
    if (list->last != NULL) {
        BLOCK_LIST_NEXT(list->last) = elem;
    }
    
    list->last = elem;
    
    if (list->first == NULL) {
        list->first = elem;
    }

    list->count++;
    list->sizemem += BLOCK_L_DATA_LEN(elem);
}

static void allocator_list_insert_elem(struct ALLOCATOR_LIST*list, void*elem)
{
    size_t len = BLOCK_L_DATA_LEN(elem);
    void*list_next = allocator_list_search_by_sizemem(list, len);

    if (list_next == NULL) {
        allocator_list_push_back(list, elem);
    } else {
        void*prev_elem;
        
        if (BLOCK_LIST_PREV(list_next) == NULL) {
            allocator_list_push_front(list, elem);
            return;
        }

        BLOCK_LIST_PREV(elem) = BLOCK_LIST_PREV(list_next);
        prev_elem = BLOCK_LIST_PREV(list_next);
        BLOCK_LIST_NEXT(prev_elem) = elem;
        BLOCK_LIST_NEXT(elem) = list_next;
        BLOCK_LIST_PREV(list_next) = elem;

        list->count++;
        list->sizemem += BLOCK_L_DATA_LEN(elem);
    }
}

void*allocator_malloc_block(allocator_type_t a, size_t sizemem)
{
    void*cur;

    if ((BLOCK_L_DATA_LEN(a->free_list.last) + BLOCK_OVERHEAD) < sizemem) {
        return NULL;
    }

    cur = allocator_list_search_by_sizemem(&(a->free_list), sizemem);

    /* remove block from list. */
    allocator_list_remove_elem(&(a->free_list), cur);
    
    if (BLOCK_L_DATA_LEN(cur) < (sizemem + MIN_BLOCK_LEN)) {
        /* don't need to divide block. */
        BLOCK_L_FLAG(cur) = BUSY_BLOCK;
        BLOCK_R_FLAG(cur) = BUSY_BLOCK;
        allocator_list_push_back(&(a->busy_list), cur);
        return BLOCK_DATA(cur);
    }

    size_t len = BLOCK_L_DATA_LEN(cur) - sizemem - BLOCK_OVERHEAD;
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
    allocator_list_push_back(&(a->busy_list), cur);
    
    /* put free block to free list. */
    tmp_block = BLOCK_ARR_NEXT(cur);
    BLOCK_L_FLAG(tmp_block) = FREE_BLOCK;
    BLOCK_L_DATA_LEN(tmp_block) = len;
    BLOCK_LIST_PREV(tmp_block) = NULL;
    BLOCK_LIST_NEXT(tmp_block) = NULL;
    BLOCK_R_DATA_LEN(tmp_block) = len;
    BLOCK_R_FLAG(tmp_block) = FREE_BLOCK;
    allocator_list_insert_elem(&(a->free_list), tmp_block);
    
    return BLOCK_DATA(cur);
}

void*allocator_realloc_block(allocator_type_t a, void*ptrmem, size_t sizemem)
{
    void*orig;
    void*cur;

    if ((BLOCK_L_DATA_LEN(a->free_list.last) + BLOCK_OVERHEAD) < sizemem) {
        return NULL;
    }

    orig = BLOCK_PTR_FROM_DATA(ptrmem);

    cur = allocator_list_search_by_sizemem(&(a->free_list), sizemem);

    /* remove block from list. */
    allocator_list_remove_elem(&(a->free_list), cur);

    if (BLOCK_L_DATA_LEN(cur) < (sizemem + MIN_BLOCK_LEN)) {
        /* don't need to divide block. */
        BLOCK_L_FLAG(cur) = BUSY_BLOCK;
        BLOCK_R_FLAG(cur) = BUSY_BLOCK;
        allocator_list_push_back(&(a->busy_list), cur);
        memcpy(BLOCK_DATA(cur), BLOCK_DATA(orig), BLOCK_L_DATA_LEN(orig));
        allocator_free_block(a, ptrmem);
        return BLOCK_DATA(cur);
    }

    size_t len = BLOCK_L_DATA_LEN(cur) - sizemem - BLOCK_OVERHEAD;
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
    allocator_list_push_back(&(a->busy_list), cur);
    memcpy(BLOCK_DATA(cur), BLOCK_DATA(orig), BLOCK_L_DATA_LEN(orig));
    
    /* put free block to free list. */
    tmp_block = BLOCK_ARR_NEXT(cur);
    BLOCK_L_FLAG(tmp_block) = FREE_BLOCK;
    BLOCK_L_DATA_LEN(tmp_block) = len;
    BLOCK_LIST_PREV(tmp_block) = NULL;
    BLOCK_LIST_NEXT(tmp_block) = NULL;
    BLOCK_R_DATA_LEN(tmp_block) = len;
    BLOCK_R_FLAG(tmp_block) = FREE_BLOCK;
    allocator_list_insert_elem(&(a->free_list), tmp_block);
    allocator_free_block(a, ptrmem);

    return BLOCK_DATA(cur);
}

void allocator_free_block(allocator_type_t a, void*ptrmem)
{
    void*cur = BLOCK_PTR_FROM_DATA(ptrmem);
    void*arr_prev = NULL;
    void*arr_next = NULL;
    if ((cur == a->mem) ||
        (BLOCK_L_FLAG(BLOCK_ARR_PREV(cur)) == BUSY_BLOCK)) {
        arr_prev = NULL;
    } else {
        arr_prev = BLOCK_ARR_PREV(cur);
    }

    if (((cur + BLOCK_L_DATA_LEN(cur) + BLOCK_OVERHEAD) >= (void*) (a->mem + a->sizemem)) ||
        (BLOCK_L_FLAG(BLOCK_ARR_NEXT(cur)) == BUSY_BLOCK)) {
        arr_next = NULL;
    } else {
        arr_next = BLOCK_ARR_NEXT(cur);
    }

    allocator_list_remove_elem(&(a->busy_list), cur);
    
    if ((arr_prev != NULL) && (arr_next == NULL)) {
        /* concatenate previous free array block with current block. */
        size_t len = BLOCK_L_DATA_LEN(arr_prev) + BLOCK_R_OVERHEAD + BLOCK_L_OVERHEAD + BLOCK_L_DATA_LEN(cur);
        allocator_list_remove_elem(&(a->free_list), arr_prev);
        BLOCK_L_DATA_LEN(arr_prev) = len;
        BLOCK_R_DATA_LEN(cur) = len;
        allocator_list_insert_elem(&(a->free_list), arr_prev);
    } else if ((arr_prev == NULL) && (arr_next != NULL)) {
        /* concatenate current block with next free array block. */
        size_t len = BLOCK_L_DATA_LEN(cur) + BLOCK_R_OVERHEAD + BLOCK_L_OVERHEAD + BLOCK_L_DATA_LEN(arr_next);
        allocator_list_remove_elem(&(a->free_list), arr_next);
        BLOCK_L_DATA_LEN(cur) = len;
        BLOCK_R_DATA_LEN(arr_next) = len;
        allocator_list_insert_elem(&(a->free_list), cur);
    } else if ((arr_prev != NULL) && (arr_next != NULL)) {
        /* concatenate previous free array block with current block with next free array block. */
        size_t len =
            BLOCK_L_DATA_LEN(arr_prev) + BLOCK_R_OVERHEAD +
            BLOCK_L_OVERHEAD + BLOCK_L_DATA_LEN(cur) + BLOCK_R_OVERHEAD +
            BLOCK_L_OVERHEAD + BLOCK_L_DATA_LEN(arr_next);
        allocator_list_remove_elem(&(a->free_list), arr_prev);
        allocator_list_remove_elem(&(a->free_list), arr_next);
        BLOCK_L_DATA_LEN(arr_prev) = len;
        BLOCK_R_DATA_LEN(arr_next) = len;
        allocator_list_insert_elem(&(a->free_list), arr_prev);
    } else {
        BLOCK_L_FLAG(cur) = FREE_BLOCK;
        BLOCK_R_FLAG(cur) = FREE_BLOCK;
        allocator_list_insert_elem(&(a->free_list), cur);
    }
}


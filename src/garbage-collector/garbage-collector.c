#include "garbage-collector.h"

#include "allocator.h"
#include "utils.h"
#include "ptrs-map.h"

struct GARBAGE_COLLECTOR
{
    struct VALUE**stack;
    struct VALUE**stack_top;

    struct ALLOCATOR a;
    struct ALLOCATOR b;

    struct PTRS_MAP ptrs_map;
};

garbage_collector_type_t create_garbage_collector()
{
    struct GARBAGE_COLLECTOR*gc;
    SAFE_CALLOC(gc, 1);
    return gc;
}

void garbage_collector_conf(garbage_collector_type_t gc, size_t sizemem_start, struct VALUE**stack, struct VALUE**stack_top)
{
    gc->stack = stack;
    gc->stack_top = stack_top;
    allocator_malloc_pool(&(gc->a), sizemem_start);
    allocator_malloc_pool(&(gc->b), sizemem_start);
}

static void*lookup_new_location(garbage_collector_type_t gc, void*ptr)
{
    void*o = BLOCK_PTR_FROM_DATA((((char*) ptr) - 1));
    
    if (ptrs_map_get(&(gc->ptrs_map), o) == NULL) {
        ptrs_map_set(&(gc->ptrs_map), o, gc->b.free_list.first);

        allocator_malloc_block(&(gc->b), BLOCK_L_DATA_LEN(o));
        memcpy(BLOCK_DATA(gc->b.busy_list.last), BLOCK_DATA(o), BLOCK_L_DATA_LEN(o));
    }

    return (((char*) BLOCK_DATA(ptrs_map_get(&(gc->ptrs_map), o))) + 1);
}

static void run_gc_inner(garbage_collector_type_t gc, void**ptr, size_t ptr_sizemem)
{
    struct VALUE*stack = (*(gc->stack));
    struct VALUE*stack_top = (*(gc->stack_top));
    
    struct VALUE*val;
    void*unscanned_ptr;

    allocator_clean_pool(&(gc->b));

    /* Cheneys GC from Aho-Ullman. */

    /* 2) for (All o in FROM) NewLocation(o) = NULL; */
    ptrs_map_conf(&(gc->ptrs_map), gc->a.busy_list.count);

    /* 3) unscanned = free = start TO address. */
    /* Nothing to do */

    /* 4-5) for (every link R in root subset) R = LookupNewLocation(r) */
    for (val = stack; val != stack_top; val++) {
        if (val->type == VALUE_TYPE_OBJ) {
            val->obj_val = lookup_new_location(gc, val->obj_val);
        } else if (val->type == VALUE_TYPE_ARR) {
            val->arr_val = lookup_new_location(gc, val->arr_val);
        }
    }

    /* 6-7-8-9-10) */
    unscanned_ptr = gc->b.busy_list.first;
    while (unscanned_ptr != NULL) {
        char*ptr = BLOCK_DATA(unscanned_ptr);
        if (ptr[0] == VALUE_TYPE_OBJ) {
            size_t i;
            struct OBJECT*obj = (struct OBJECT*) (ptr + 1);
            for (i = 0; i < obj->properties_len; i++) {
                if (obj->properties[i].val.type == VALUE_TYPE_OBJ) {
                    obj->properties[i].val.obj_val = lookup_new_location(gc, obj->properties[i].val.obj_val);
                } else if (obj->properties[i].val.type == VALUE_TYPE_ARR) {
                    obj->properties[i].val.arr_val = lookup_new_location(gc, obj->properties[i].val.arr_val);
                }
            }
        } else if (ptr[1] == VALUE_TYPE_ARR) {
            size_t i;
            struct ARRAY*arr = (struct ARRAY*) (ptr + 1);
            for (i = 0; i < arr->len; i++) {
                if (arr->values[i].type == VALUE_TYPE_OBJ) {
                    arr->values[i].obj_val = lookup_new_location(gc, arr->values[i].obj_val);
                } else if (arr->values[i].type == VALUE_TYPE_ARR) {
                    arr->values[i].arr_val = lookup_new_location(gc, arr->values[i].arr_val);
                }
            }
        }
        unscanned_ptr = BLOCK_LIST_NEXT(unscanned_ptr);
    }    
}

static void run_gc(garbage_collector_type_t gc, void**ptr, size_t ptr_sizemem)
{
    struct ALLOCATOR tmp;

    int need_realloc;

    run_gc_inner(gc, ptr, ptr_sizemem);

    /* check if realloc is needed. */
    need_realloc = ((gc->b.busy_list.sizemem + gc->b.busy_list.count * BLOCK_OVERHEAD) +
                    (ptr_sizemem + BLOCK_OVERHEAD)) >= gc->b.sizemem / 2;

    if (need_realloc) {
        size_t new_sizemem = (gc->b.sizemem + (ptr_sizemem + BLOCK_OVERHEAD)) * 2;

        tmp = gc->a;
        gc->a = gc->b;
        gc->b = tmp;
        
        allocator_free_pool(&(gc->b));
        allocator_malloc_pool(&(gc->b), new_sizemem);

        run_gc_inner(gc, ptr, ptr_sizemem);

        allocator_free_pool(&(gc->a));
        allocator_malloc_pool(&(gc->a), new_sizemem);
    }

    tmp = gc->a;
    gc->a = gc->b;
    gc->b = tmp;
}

static struct OBJECT*malloc_obj_force(garbage_collector_type_t gc, size_t start_properties_cap, size_t sizemem)
{
    struct OBJECT*obj;
    char*ptr = allocator_malloc_block(&(gc->a), sizemem);

    ptr[0] = VALUE_TYPE_OBJ;
    obj = (struct OBJECT*) (ptr + 1);
    obj->properties_len = 0;
    obj->properties_cap = start_properties_cap;
    
    return obj;
}

struct OBJECT*garbage_collector_malloc_obj(garbage_collector_type_t gc, size_t start_properties_num)
{
    size_t start_properties_cap = start_properties_num * 2;
    size_t sizemem = sizeof(char) + sizeof(struct OBJECT) + sizeof(struct PROPERTY) * start_properties_cap;
    
    if ((gc->a.free_list.last != NULL) && (BLOCK_L_DATA_LEN(gc->a.free_list.last) >= (sizemem + BLOCK_OVERHEAD))) {
        return malloc_obj_force(gc, start_properties_cap, sizemem);
    }

    /* need garbage collection. */
    run_gc(gc, NULL, sizemem);

    return malloc_obj_force(gc, start_properties_cap, sizemem);
}

static void change_vals_ptr(struct VALUE*val, char*prev_ptr, char*new_ptr)
{
    if ((val->type == VALUE_TYPE_OBJ) &&
        (val->obj_val == (struct OBJECT*) (prev_ptr + 1))) {
        val->obj_val = (struct OBJECT*) (new_ptr + 1);
    } else if ((val->type == VALUE_TYPE_ARR) &&
               (val->arr_val == (struct ARRAY*) (prev_ptr + 1))) {
        val->arr_val = (struct ARRAY*) (new_ptr + 1);
    }
}

static void change_one_ptr(garbage_collector_type_t gc, char*prev_ptr, char*new_ptr)
{
    struct VALUE*stack = (*(gc->stack));
    struct VALUE*stack_top = (*(gc->stack_top));

    struct VALUE*val;
    
    void*cur;

    for (val = stack; val != stack_top; val++) {
        change_vals_ptr(val, prev_ptr, new_ptr);
    }

    cur = gc->a.busy_list.first;

    while (cur != NULL) {
        size_t i;
        char*ptr = BLOCK_DATA(cur);
        if (ptr[0] == VALUE_TYPE_OBJ) {
            struct OBJECT*obj = (struct OBJECT*) (ptr + 1);
            for (i = 0; i < obj->properties_len; i++) {
                val = &(obj->properties[i].val);
                change_vals_ptr(val, prev_ptr, new_ptr);
            }
        } else if (ptr[0] == VALUE_TYPE_ARR) {
            size_t i;
            struct ARRAY*arr = (struct ARRAY*) (ptr + 1);
            for (i = 0; i < arr->len; i++) {
                val = &(arr->values[i]);
                change_vals_ptr(val, prev_ptr, new_ptr);
            }
        }
        cur = BLOCK_LIST_NEXT(cur);
    }
}

static struct OBJECT*realloc_obj_force(garbage_collector_type_t gc,
                                       struct OBJECT*obj,
                                       size_t new_properties_cap, size_t sizemem)
{
    char*prev_ptr = ((char*) obj) - 1;
    char*new_ptr = allocator_realloc_block(&(gc->a), prev_ptr, sizemem);

    change_one_ptr(gc, prev_ptr, new_ptr);
    obj = (struct OBJECT*) (new_ptr + 1);
    obj->properties_cap = new_properties_cap;

    return obj;
}

struct OBJECT*garbage_collector_realloc_obj(garbage_collector_type_t gc, struct OBJECT*obj, size_t new_properties_num)
{
    size_t new_properties_cap = new_properties_num * 2;
    size_t sizemem = sizeof(char) + sizeof(struct OBJECT) + sizeof(struct PROPERTY) * new_properties_cap;

    if ((gc->a.free_list.last != NULL) && (BLOCK_L_DATA_LEN(gc->a.free_list.last) >= (sizemem + BLOCK_OVERHEAD))) {
        return realloc_obj_force(gc, obj, new_properties_cap, sizemem);
    }

    /* need garbage collection. */
    run_gc(gc, (void**) &obj, sizemem);

    return realloc_obj_force(gc, obj, new_properties_cap, sizemem);
}

void free_garbage_collector(garbage_collector_type_t gc)
{
    allocator_free_pool(&(gc->a));
    allocator_free_pool(&(gc->b));
    SAFE_FREE(gc);
}

static struct ARRAY*malloc_arr_force(garbage_collector_type_t gc, size_t start_cap, size_t sizemem)
{
    struct ARRAY*arr;
    char*ptr = allocator_malloc_block(&(gc->a), sizemem);
    ptr[0] = VALUE_TYPE_ARR;
    arr = (struct ARRAY*) (ptr + 1);
    arr->len = 0;
    arr->cap = start_cap;
    
    return arr;
}

struct ARRAY*garbage_collector_malloc_arr(garbage_collector_type_t gc, size_t arr_len)
{
    size_t start_arr_cap = arr_len * 2;
    size_t sizemem = sizeof(char) + sizeof(struct ARRAY) + sizeof(struct VALUE) * start_arr_cap;
    
    if ((gc->a.free_list.last != NULL) && (BLOCK_L_DATA_LEN(gc->a.free_list.last) >= (sizemem + BLOCK_OVERHEAD))) {
        return malloc_arr_force(gc, start_arr_cap, sizemem);
    }

    /* need garbage collection. */
    run_gc(gc, NULL, sizemem);

    return malloc_arr_force(gc, start_arr_cap, sizemem);
}

static struct ARRAY*realloc_arr_force(garbage_collector_type_t gc,
                                       struct ARRAY*arr,
                                       size_t new_arr_cap, size_t sizemem)
{
    char*prev_ptr = ((char*) arr) - 1;
    char*new_ptr = allocator_realloc_block(&(gc->a), prev_ptr, sizemem);

    change_one_ptr(gc, prev_ptr, new_ptr);
    arr = (struct ARRAY*) (new_ptr + 1);
    arr->cap = new_arr_cap;
    
    return arr;
}

struct ARRAY*garbage_collector_realloc_arr(garbage_collector_type_t gc, struct ARRAY*arr, size_t new_arr_len)
{
    size_t new_arr_cap = new_arr_len * 2;
    size_t sizemem = sizeof(struct ARRAY) + sizeof(struct VALUE) * new_arr_cap;

    if ((gc->a.free_list.last != NULL) && (BLOCK_L_DATA_LEN(gc->a.free_list.last) >= (sizemem + BLOCK_OVERHEAD))) {
        return realloc_arr_force(gc, arr, new_arr_cap, sizemem);
    }

    /* need garbage collection. */
    run_gc(gc, (void**) &arr, sizemem);

    return realloc_arr_force(gc, arr, new_arr_cap, sizemem);
}

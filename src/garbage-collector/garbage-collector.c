#include "garbage-collector.h"

#include "allocator.h"
#include "utils.h"

struct GARBAGE_COLLECTOR
{
    struct VALUE**stack;
    struct VALUE**stack_top;

    struct ALLOCATOR a;
    struct ALLOCATOR b;    
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
}

struct OBJECT*garbage_collector_malloc_obj(garbage_collector_type_t gc, unsigned start_properties_num)
{
    struct OBJECT*obj;
    size_t start_properties_cap = start_properties_num * 2;
    size_t sizemem = sizeof(struct OBJECT) + sizeof(struct PROPERTY) * start_properties_cap;
    
    if ((BLOCK_L_DATA_LEN(gc->a.free_list.last) + BLOCK_OVERHEAD) >= sizemem) {
        obj = allocator_malloc_block(&(gc->a), sizemem);
        obj->properties_len = 0;
        obj->properties_cap = start_properties_cap;

        return obj;
    }

    /* TODO */

    return NULL;
}

static void change_one_ptr(garbage_collector_type_t gc, struct OBJECT*prev_obj_ptr, struct OBJECT*new_obj_ptr)
{
    struct VALUE*stack = (*(gc->stack));
    struct VALUE*stack_top = (*(gc->stack_top));

    struct VALUE*val;
    struct OBJECT*obj;
    
    void*cur;

    unsigned i;

    for (val = stack; val != stack_top; val++) {
        if ((val->type == VALUE_TYPE_OBJ) &&
            (val->obj_val == prev_obj_ptr)) {
            val->obj_val = new_obj_ptr;
        }
    }

    cur = gc->a.busy_list.first;

    while (cur != NULL) {
        obj = BLOCK_DATA(cur);
        for (i = 0; i < obj->properties_len; i++) {
            if ((obj->properties[i].val.type == VALUE_TYPE_OBJ) &&
                (obj->properties[i].val.obj_val == prev_obj_ptr)) {
                obj->properties[i].val.obj_val = new_obj_ptr;
            }
        }
        cur = BLOCK_LIST_NEXT(cur);
    }
}

struct OBJECT*garbage_collector_realloc_obj(garbage_collector_type_t gc, struct OBJECT*obj, unsigned new_properties_num)
{
    struct OBJECT*prev_obj_ptr = obj;
    struct OBJECT*new_obj_ptr;
    size_t new_properties_cap = new_properties_num * 2;
    size_t sizemem = sizeof(struct OBJECT) + sizeof(struct PROPERTY) * new_properties_cap;

    if ((BLOCK_L_DATA_LEN(gc->a.free_list.last) + BLOCK_OVERHEAD) >= sizemem) {
        new_obj_ptr = allocator_realloc_block(&(gc->a), obj, sizemem);
        change_one_ptr(gc, prev_obj_ptr, new_obj_ptr);
        new_obj_ptr->properties_cap = new_properties_cap;
        return new_obj_ptr;
    }

    /* TODO */

    return NULL;
}

void free_garbage_collector(garbage_collector_type_t gc)
{
    allocator_free_pool(&(gc->a));
    SAFE_FREE(gc);
}

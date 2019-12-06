#ifndef GARBAGE_COLLECTOR_H_INCLUDED
#define GARBAGE_COLLECTOR_H_INCLUDED

#include "data-types.h"

#include "allocator.h"
#include "ptrs-map.h"

#include <string.h>

struct GARBAGE_COLLECTOR
{
    struct VALUE**stack;
    struct VALUE**stack_top;

    struct ALLOCATOR a;
    struct ALLOCATOR b;

    struct PTRS_MAP ptrs_map;

    int trace;
};

typedef struct GARBAGE_COLLECTOR* garbage_collector_type_t;

garbage_collector_type_t create_garbage_collector();

void garbage_collector_conf(garbage_collector_type_t gc, size_t sizemem_start, struct VALUE**stack, struct VALUE**stack_top, int trace);

struct OBJECT*garbage_collector_malloc_obj(garbage_collector_type_t gc, size_t start_properties_num);
struct OBJECT*garbage_collector_realloc_obj(garbage_collector_type_t gc, struct OBJECT*obj, size_t new_properties_num);

struct ARRAY*garbage_collector_malloc_arr(garbage_collector_type_t gc, size_t arr_len);
struct ARRAY*garbage_collector_realloc_arr(garbage_collector_type_t gc, struct ARRAY*arr, size_t new_arr_len);

void garbage_collector_free(garbage_collector_type_t gc);

#endif  /* GARBAGE_COLLECTOR_H_INCLUDED */

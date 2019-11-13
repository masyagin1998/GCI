#ifndef GARBAGE_COLLECTOR_H_INCLUDED
#define GARBAGE_COLLECTOR_H_INCLUDED

#include "data-types.h"

#include <string.h>

struct GARBAGE_COLLECTOR;

typedef struct GARBAGE_COLLECTOR* garbage_collector_type_t;

garbage_collector_type_t create_garbage_collector();

void garbage_collector_conf(garbage_collector_type_t gc, size_t sizemem_start, struct VALUE**stack, struct VALUE**stack_top);

struct OBJECT*garbage_collector_malloc_obj(garbage_collector_type_t gc, unsigned start_properties_num);
struct OBJECT*garbage_collector_realloc_obj(garbage_collector_type_t gc, struct OBJECT*obj, unsigned new_properties_num);

void free_garbage_collector(garbage_collector_type_t gc);

#endif  /* GARBAGE_COLLECTOR_H_INCLUDED */

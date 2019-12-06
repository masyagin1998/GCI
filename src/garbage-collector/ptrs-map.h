#ifndef PTRS_MAP_H_INCLUDED
#define PTRS_MAP_H_INCLUDED

#include <string.h>

/*
  Very simple PTRS_MAP,
  based on key-value array.
 */

struct PTRS_MAP_ELEM
{
    void*key;
    void*val;
};

struct PTRS_MAP
{
    struct PTRS_MAP_ELEM*elems;
    size_t len;
    size_t cap;
};

typedef struct PTRS_MAP* ptrs_map_type_t;

void ptrs_map_conf(ptrs_map_type_t pm, size_t cap);

void ptrs_map_set(ptrs_map_type_t pm, void*key, void*value);
void*ptrs_map_get(ptrs_map_type_t pm, void*key);

void ptrs_map_free(ptrs_map_type_t pm);

#endif  /* PTRS_MAP_H_INCLUDED */


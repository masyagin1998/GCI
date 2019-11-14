#include "ptrs-map.h"

#include "utils.h"

void ptrs_map_conf(ptrs_map_type_t pm, size_t cap)
{
    pm->len = 0;
    pm->cap = cap;
    pm->elems = (struct PTRS_MAP_ELEM*) malloc(cap * sizeof(struct PTRS_MAP_ELEM));
}

void ptrs_map_set(ptrs_map_type_t pm, void*key, void*val)
{
    size_t i;

    if (key == NULL) {
        fprintf(stderr, "insertion null-key in map is prohibited\n");
        exit(1);
    }

    for (i = 0; i < pm->len; i++) {
        if (pm->elems[i].key == key) {
            pm->elems[i].val = val;
            return;
        }
    }

    if (pm->len == pm->cap) {
        fprintf(stderr, "non enough memory in map to handle new value\n");
        exit(1);
    }

    pm->elems[pm->len].key = key;
    pm->elems[pm->len].val = val;
    pm->len++;
}

void*ptrs_map_get(ptrs_map_type_t pm, void*key)
{
    size_t i;

    if (key == NULL) {
        fprintf(stderr, "selection by null-key from map is prohibited\n");
        exit(1);
    }

    for (i = 0; i < pm->len; i++) {
        if (pm->elems[i].key == key) {
            return pm->elems[i].val;
        }
    }

    return NULL;
}

void ptrs_map_free(ptrs_map_type_t pm)
{
    SAFE_FREE(pm->elems);
}

#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

#include <string.h>

struct ALLOCATOR;

typedef struct ALLOCATOR* allocator_type_t;

allocator_type_t create_allocator();

void allocator_conf(allocator_type_t a, size_t sizemem_start);

void*allocator_malloc_mem(allocator_type_t a, size_t sizemem);
void*allocator_realloc_mem(allocator_type_t a, void*ptrmem, size_t sizemem);
void allocator_free_mem(allocator_type_t a, void*ptrmem);

void allocator_free(allocator_type_t a);

#endif  /* ALLOCATOR_H_INCLUDED */

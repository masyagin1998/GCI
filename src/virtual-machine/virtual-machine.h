#ifndef VIRTUAL_MACHINE_H_INCLUDED
#define VIRTUAL_MACHINE_H_INCLUDED

#include "bytecode-generator.h"

struct VIRTUAL_MACHINE;

typedef struct VIRTUAL_MACHINE* virtual_machine_type_t;

virtual_machine_type_t create_virtual_machine();

void virtual_machine_conf(virtual_machine_type_t vm, bytecode_type_t bc, size_t stack_size, size_t heap_size_b);

long long virtual_machine_run(virtual_machine_type_t vm);

void virtual_machine_free(virtual_machine_type_t vm);

#endif  /* VIRTUAL_MACHINE_H_INCLUDED */

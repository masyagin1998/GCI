#ifndef VIRTUAL_MACHINE_H_INCLUDED
#define VIRTUAL_MACHINE_H_INCLUDED

#include "bytecode-generator.h"

struct VIRTUAL_MACHINE;

typedef struct VIRTUAL_MACHINE* virtual_machine_type_t;

virtual_machine_type_t create_virtual_machine();

void virtual_machine_conf(bytecode_type_t bc);

void virtual_machine_run();

void virtual_machine_free(virtual_machine_type_t vm);

#endif  /* VIRTUAL_MACHINE_H_INCLUDED */

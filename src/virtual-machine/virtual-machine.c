#include "virtual-machine.h"

enum VALUE_TYPE
{
    STACK_VALUE_TYPE_INTEGER,
    STACK_VALUE_TYPE_DOUBLE,
    STACK_VALUE_TYPE_OBJ,
};

struct VALUE
{
    union
    {
        long long int_val;
        double double_valt;
    };

    enum VALUE_TYPE type;
};

struct PROPERTY
{
    unsigned key;
    struct VALUE val;
};

struct OBJECT
{
    struct OBJECT*prev, *next;

    struct PROPERTY*properties;
    unsigned properties_len;
    unsigned properties_cap;

    unsigned obj_len;
};

struct OBJECTS_POOL
{
    struct OBJECT*first, *last;

    unsigned count;
};

struct VIRTUAL_MACHINE
{
    struct VALUE*stack;
    unsigned stack_len;
    unsigned stack_cap;

    char*heap;
    unsigned heap_len;

    struct OBJECTS_POOL objects_pool;
};

virtual_machine_type_t create_virtual_machine()
{
    
}

void virtual_machine_conf(bytecode_type_t bc)
{
    
}

void virtual_machine_run()
{
    
}

void virtual_machine_free(virtual_machine_type_t vm)
{
    
}

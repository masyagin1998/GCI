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

struct VIRTUAL_MACHINE
{
    bytecode_type_t bc;
    unsigned*ip;
    
    struct VALUE*stack;
    struct VALUE*stack_top;
    unsigned stack_cap;

    char*heap;
    unsigned heap_cap;

    struct OBJECT*first, *last;
    unsigned count;
};

virtual_machine_type_t create_virtual_machine()
{
    struct VIRTUAL_MACHINE*vm;
    SAFE_CALLOC(vm, 1);
    return vm;
}

void virtual_machine_conf(virtual_machine_type_t vm, bytecode_type_t bc, unsigned stack_size, unsigned heap_size_b)
{
    vm->bc = bc;
    vm->ip = bc->op_codes;
    
    vm->stack_cap = stack_size;
    SAFE_MALLOC(vm->stack, stack_size);
    vm->stack_top = vm->stack;

    vm->heap_cap = heap_size_b;
    SAFE_MALLOC(vm->heap, vm->heap_cap);
}

static void virtual_machine_stack_push(virtual_machine_type_t vm, struct VALUE val)
{
    *vm->stack_top = val;
    vm->stack_top++;
}

static struct VALUE virtual_machine_stack_pop(virtual_machine_type_t vm)
{
    vm->stack_top--;
    return *vm->stack_top;
}

#define READ_BYTE() (*(vm->ip++))

void virtual_machine_run(virtual_machine_type_t vm)
{
    while (1) {
        unsigned instruction;

        switch (instruction = READ_BYTE()) {
        case BC_OP_POP:
            break;

        case BC_OP_CONSTANT:
            break;

        case BC_OP_SET_LOCAL:
            break;
        case BC_OP_GET_LOCAL:
            break;

        case BC_OP_CREATE_OBJ:
            break;
        case BC_OP_INIT_OBJ_PROP:
            break;

        case BC_OP_GET_HEAP:
            break;
        case BC_OP_SET_HEAP:
            break;

        case BC_OP_LOGICAL_OR:
            break;
        case BC_OP_LOGICAL_AND:
            break;

        case BC_OP_EQ_EQEQ:
            break;
        case BC_OP_EQ_NEQ:
            break;

        case BC_OP_REL_LT:
            break;
        case BC_OP_REL_GT:
            break;
        case BC_OP_REL_LE:
            break;
        case BC_OP_REL_GE:
            break;

        case BC_OP_ADDITIVE_PLUS:
            break;
        case BC_OP_ADDITIVE_MINUS:
            break;

        case BC_OP_MULTIPLICATIVE_MUL:
            break;
        case BC_OP_MULTIPLICATIVE_DIV:
            break;
        case BC_OP_MULTIPLICATIVE_MOD:
            break;

        case BC_OP_NEGATE:
            break;

        case BC_OP_RETURN:
            return;
            break;
        }
    }
}

void virtual_machine_free(virtual_machine_type_t vm)
{
    SAFE_FREE(vm->stack);
    /* clear list. */
    SAFE_FREE(vm->heap);
    SAFE_FREE(vm);
}

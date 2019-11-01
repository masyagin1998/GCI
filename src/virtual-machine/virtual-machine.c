#include "virtual-machine.h"

enum VALUE_TYPE
{
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_OBJ,
};

struct VALUE
{
    union
    {
        long long int_val;
        double double_val;
        struct OBJECT*obj_val;
    };

    enum VALUE_TYPE type;
};

static struct VALUE create_value_from_int(long long int_val)
{
    struct VALUE val;
    val.type = VALUE_TYPE_INTEGER;
    val.int_val = int_val;
    return val;
}
    
static struct VALUE create_value_from_double(double double_val)
{
    struct VALUE val;
    val.type = VALUE_TYPE_DOUBLE;
    val.double_val = double_val;
    return val;    
}

static struct VALUE create_value_from_obj(struct OBJECT*obj_val)
{
    struct VALUE val;
    val.obj_val = obj_val;
    val.type = VALUE_TYPE_OBJ;
    return val;
}

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
        case BC_OP_POP: {
            virtual_machine_stack_pop(vm);
            break;
        }

        case BC_OP_CONSTANT: {
            struct CONSTANT cnst = vm->bc->constant_pool[READ_BYTE()];
            struct VALUE val = create_value_from_int(cnst.int_cnst);
            virtual_machine_stack_push(vm, val);
        }

        case BC_OP_SET_LOCAL: {
            unsigned idx = READ_BYTE();
            vm->stack[idx] = virtual_machine_stack_pop(vm);
            break;
        }
        case BC_OP_GET_LOCAL: {
            unsigned idx = READ_BYTE();
            virtual_machine_stack_push(vm, vm->stack[idx]);
            break;
        }

        case BC_OP_CREATE_OBJ: 
            break;
        case BC_OP_INIT_OBJ_PROP:
            break;

        case BC_OP_GET_HEAP:
            break;
        case BC_OP_SET_HEAP:
            break;

        case BC_OP_LOGICAL_OR: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val || val2.int_val);
            virtual_machine_stack_push(vm, res);                        
            break;
        }
        case BC_OP_LOGICAL_AND: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val && val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_EQ_EQEQ: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val == val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_EQ_NEQ: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val != val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_REL_LT: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val < val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_REL_GT: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val > val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_REL_LE: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val <= val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_REL_GE: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val >= val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_ADDITIVE_PLUS: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val + val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_ADDITIVE_MINUS: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val - val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_MULTIPLICATIVE_MUL: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val * val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_MULTIPLICATIVE_DIV: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val / val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_MULTIPLICATIVE_MOD: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val1.int_val % val2.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_NEGATE: {
            struct VALUE val = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(-val.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

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

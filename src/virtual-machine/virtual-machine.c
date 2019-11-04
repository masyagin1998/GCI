#include "virtual-machine.h"

#include "allocator.h"

enum VALUE_TYPE
{
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_OBJ,
};

struct VALUE
{
    unsigned mark;
    
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

struct VIRTUAL_MACHINE
{
    bytecode_type_t bc;
    unsigned*ip;
    
    struct VALUE*stack;
    struct VALUE*stack_top;
    unsigned stack_cap;

    allocator_type_t allocator;
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
    SAFE_MALLOC(vm->stack, vm->stack_cap);
    vm->stack_top = vm->stack;

    vm->allocator = create_allocator();
    allocator_conf(vm->allocator, heap_size_b);
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

struct PROPERTY
{
    unsigned long long key;
    struct VALUE val;
};

struct OBJECT
{
    unsigned properties_len;
    unsigned properties_cap;
    struct PROPERTY properties[];
};

struct OBJECT*create_obj(virtual_machine_type_t vm)
{
    unsigned i;

    unsigned properties_num = READ_BYTE();
    
    struct OBJECT*obj = allocator_malloc_mem(vm->allocator, sizeof(struct OBJECT) + sizeof(struct PROPERTY) * properties_num * 2);
    
    for (i = 0; i < properties_num; i++) {
        struct VALUE key = virtual_machine_stack_pop(vm);
        struct VALUE val = virtual_machine_stack_pop(vm);
        
        obj->properties[i].key = key.int_val;
        obj->properties[i].val = val;
    }
    
    obj->properties_len = properties_num;
    obj->properties_cap = properties_num * 2;

    return obj;
}

void virtual_machine_run(virtual_machine_type_t vm)
{
    while (1) {
        unsigned instruction = READ_BYTE();
        switch (instruction) {
        case BC_OP_POP: {
            virtual_machine_stack_pop(vm);
            break;
        }

        case BC_OP_CONSTANT: {
            struct CONSTANT cnst = vm->bc->constant_pool[READ_BYTE()];
            struct VALUE val = create_value_from_int(cnst.int_cnst);
            virtual_machine_stack_push(vm, val);
            break;
        }

        case BC_OP_SET_LOCAL: {
            unsigned idx = READ_BYTE();
            vm->stack[idx] = *(vm->stack_top - 1);
            break;
        }
        case BC_OP_GET_LOCAL: {
            unsigned idx = READ_BYTE();
            virtual_machine_stack_push(vm, vm->stack[idx]);
            break;
        }

        case BC_OP_CREATE_OBJ: {
            struct OBJECT*obj = create_obj(vm);
            struct VALUE val = create_value_from_obj(obj);
            virtual_machine_stack_push(vm, val);
            break;
        }
        case BC_OP_INIT_OBJ_PROP: {
            struct VALUE val = create_value_from_int(READ_BYTE());
            virtual_machine_stack_push(vm, val);
            break;
        }
        case BC_OP_SET_HEAP: {
            unsigned i, j;
            unsigned idx = READ_BYTE();
            unsigned len = READ_BYTE();
            struct VALUE val = vm->stack[idx];
            for (i = 0; i < len; i++) {
                unsigned key = READ_BYTE();
                unsigned found = 0;
                for (j = 0; j < val.obj_val->properties_len; j++) {
                    if (val.obj_val->properties[j].key == key) {
                        if (i < len - 1) {
                            val = val.obj_val->properties[j].val;
                            found = 1;
                            break;
                        } else if (i == len - 1) {
                            val.obj_val->properties[j].val = *(vm->stack_top - 1);
                            found = 1;
                            break;                            
                        }
                    }
                }
                if (!found) {
                    if (i < len - 1) {
                        printf("need to create to many fields\n");
                        exit(1);
                    } else {
                        if (val.obj_val->properties_len == val.obj_val->properties_cap) {
                            printf("TODO: %d %d\n", val.obj_val->properties_len, val.obj_val->properties_cap);
                            exit(1);
                        } else {
                            val.obj_val->properties_len++;
                            val.obj_val->properties[j].key = key;
                            val.obj_val->properties[j].val = *(vm->stack_top - 1);
                        }
                    }
                }
            }
            break;
        }
        case BC_OP_GET_HEAP: {
            unsigned i, j;
            unsigned idx = READ_BYTE();
            unsigned len = READ_BYTE();
            struct VALUE val = vm->stack[idx];
            for (i = 0; i < len; i++) {
                unsigned key = READ_BYTE();
                unsigned found = 0;
                for (j = 0; j < val.obj_val->properties_len; j++) {
                    if (val.obj_val->properties[j].key == key) {
                        val = val.obj_val->properties[j].val;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    printf("unknown fieldref: %d\n", key);
                    exit(1);
                }                
            }
            virtual_machine_stack_push(vm, val);
            break;
        }

        case BC_OP_LOGICAL_OR: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val || val1.int_val);
            virtual_machine_stack_push(vm, res);                        
            break;
        }
        case BC_OP_LOGICAL_AND: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val && val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_EQ_EQEQ: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val == val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_EQ_NEQ: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val != val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_REL_LT: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val < val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_REL_GT: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val > val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_REL_LE: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val <= val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_REL_GE: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val >= val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_ADDITIVE_PLUS: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val + val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_ADDITIVE_MINUS: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val - val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_MULTIPLICATIVE_MUL: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val * val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_MULTIPLICATIVE_DIV: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val / val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }
        case BC_OP_MULTIPLICATIVE_MOD: {
            struct VALUE val1 = virtual_machine_stack_pop(vm);
            struct VALUE val2 = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(val2.int_val % val1.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_NEGATE: {
            struct VALUE val = virtual_machine_stack_pop(vm);
            struct VALUE res = create_value_from_int(-val.int_val);
            virtual_machine_stack_push(vm, res);
            break;
        }

        case BC_OP_RETURN: {
            struct VALUE val = virtual_machine_stack_pop(vm);
            printf("result: %lld\n", val.int_val);
            return;
            break;
        }
        }
    }
}

void virtual_machine_free(virtual_machine_type_t vm)
{
    SAFE_FREE(vm->stack);
    allocator_free(vm->allocator);
    SAFE_FREE(vm);
}

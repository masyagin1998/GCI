#include "virtual-machine.h"

#include "data-types.h"

#include "garbage-collector.h"

#include "utils.h"

static struct VALUE create_value_from_int(long long int_val)
{
    struct VALUE val;
    val.type = VALUE_TYPE_INTEGER;
    val.int_val = int_val;
    return val;
}

/*
static struct VALUE create_value_from_double(double double_val)
{
    struct VALUE val;
    val.type = VALUE_TYPE_DOUBLE;
    val.double_val = double_val;
    return val;    
}
*/

static struct VALUE create_value_from_obj(struct OBJECT*obj_val)
{
    struct VALUE val;
    val.obj_val = obj_val;
    val.type = VALUE_TYPE_OBJ;
    return val;
}

static struct VALUE create_value_from_arr(struct ARRAY*arr_val)
{
    struct VALUE val;
    val.arr_val = arr_val;
    val.type = VALUE_TYPE_ARR;
    return val;
}

struct VIRTUAL_MACHINE
{
    bytecode_type_t bc;
    size_t*ip;
    
    struct VALUE*stack;
    struct VALUE*stack_top;
    size_t stack_cap;

    garbage_collector_type_t gc;
};

virtual_machine_type_t create_virtual_machine()
{
    struct VIRTUAL_MACHINE*vm;
    SAFE_CALLOC(vm, 1);
    return vm;
}

void virtual_machine_conf(virtual_machine_type_t vm, bytecode_type_t bc, size_t stack_size, size_t start_heap_size_b)
{
    vm->bc = bc;
    vm->ip = bc->op_codes;
    
    vm->stack_cap = stack_size;
    SAFE_MALLOC(vm->stack, vm->stack_cap);
    vm->stack_top = vm->stack;

    vm->gc = create_garbage_collector();
    garbage_collector_conf(vm->gc, start_heap_size_b, &(vm->stack), &(vm->stack_top));
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

struct OBJECT*create_obj(virtual_machine_type_t vm)
{
    size_t i;

    size_t properties_num = READ_BYTE();
    
    struct OBJECT*obj = garbage_collector_malloc_obj(vm->gc, properties_num);
    
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

struct ARRAY*create_arr(virtual_machine_type_t vm)
{
    size_t i;

    size_t arr_len = READ_BYTE();

    struct ARRAY*arr = garbage_collector_malloc_arr(vm->gc, arr_len);

    for (i = 0; i < arr_len; i++) {
        struct VALUE val = virtual_machine_stack_pop(vm);
        arr->values[i] = val;
    }
    arr->len = arr_len;

    return arr;
}

void virtual_machine_run(virtual_machine_type_t vm)
{
    while (1) {
        size_t instruction = READ_BYTE();
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
            size_t idx = READ_BYTE();
            vm->stack[idx] = *(vm->stack_top - 1);
            if (idx != (size_t) (vm->stack_top - vm->stack - 1)) {
                virtual_machine_stack_pop(vm);
            }
            break;
        }
        case BC_OP_GET_LOCAL: {
            size_t idx = READ_BYTE();
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

        case BC_OP_CREATE_ARR: {
            struct ARRAY*arr = create_arr(vm);
            struct VALUE val = create_value_from_arr(arr);
            virtual_machine_stack_push(vm, val);
            break;            
        }
            
        case BC_OP_SET_HEAP: {
            size_t i, j, k;
            size_t idx = READ_BYTE();
            size_t len = READ_BYTE();
            size_t pops = 0;
            struct VALUE val = vm->stack[idx];
            for (i = 0; i < len; i++) {
                enum BC_HEAP_OP hop = READ_BYTE();
                if (hop == BC_ARRAY_INDEX) {
                    pops++;
                    size_t offset = READ_BYTE();
                    struct VALUE index = *(vm->stack_top - offset - 1);
                    if (index.int_val < 0) {
                        printf("invalid array index: %lld\n", index.int_val);
                        exit(1);
                    }
                    if ((size_t) index.int_val > val.arr_val->len) {
                        printf("array index to unitialized data: %lld\n", index.int_val);
                        exit(1);
                    }
                    if (i < len - 1) {
                        val = val.arr_val->values[index.int_val];
                    } else if (i == len - 1) {
                        for (k = 0; k < pops; k++) {
                            virtual_machine_stack_pop(vm);
                        }

                        val.arr_val->values[index.int_val] = virtual_machine_stack_pop(vm);
                    }
                } else if (hop == BC_OBJECT_FIELD) {
                    size_t key = READ_BYTE();
                    size_t found = 0;
                    for (j = 0; j < val.obj_val->properties_len; j++) {
                        if (val.obj_val->properties[j].key == key) {
                            if (i < len - 1) {
                                val = val.obj_val->properties[j].val;
                                found = 1;
                                break;
                            } else if (i == len - 1) {
                                for (k = 0; k < pops; k++) {
                                    virtual_machine_stack_pop(vm);
                                }
                                
                                val.obj_val->properties[j].val = virtual_machine_stack_pop(vm);
                                found = 1;
                                break;                            
                            }
                        }
                    }
                    if (!found) {
                        for (k = 0; k < pops; k++) {
                            virtual_machine_stack_pop(vm);
                        }
                        
                        if (i < len - 1) {
                            printf("need to create to many fields\n");
                            exit(1);
                        } else {
                            if (val.obj_val->properties_len == val.obj_val->properties_cap) {
                                val.obj_val = garbage_collector_realloc_obj(vm->gc, val.obj_val, val.obj_val->properties_len + 1);
                                val.obj_val->properties_len++;
                                val.obj_val->properties[j].key = key;
                                val.obj_val->properties[j].val = virtual_machine_stack_pop(vm);
                            } else {
                                val.obj_val->properties_len++;
                                val.obj_val->properties[j].key = key;
                                val.obj_val->properties[j].val = virtual_machine_stack_pop(vm);
                            }
                        }
                    }
                }
            }
            break;
        }
        case BC_OP_GET_HEAP: {
            size_t i, j;
            size_t idx = READ_BYTE();
            size_t len = READ_BYTE();
            size_t pops = 0;
            struct VALUE val = vm->stack[idx];
            for (i = 0; i < len; i++) {
                enum BC_HEAP_OP hop = READ_BYTE();
                if (hop == BC_ARRAY_INDEX) {
                    pops++;
                    size_t offset = READ_BYTE();
                    struct VALUE index = *(vm->stack_top - offset - 1);
                    if (index.int_val < 0) {
                        printf("invalid array index: %lld\n", index.int_val);
                        exit(1);
                    }
                    if ((size_t) index.int_val > val.arr_val->len) {
                        printf("array index to unitialized data: %lld\n", index.int_val);
                        exit(1);
                    }
                    val = val.arr_val->values[index.int_val];
                } else if (hop == BC_OBJECT_FIELD) {
                    size_t key = READ_BYTE();
                    size_t found = 0;
                    for (j = 0; j < val.obj_val->properties_len; j++) {
                        if (val.obj_val->properties[j].key == key) {
                            val = val.obj_val->properties[j].val;
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        printf("unknown fieldref: %zu\n", key);
                        exit(1);
                    }
                }
            }

            for (i = 0; i < pops; i++) {
                virtual_machine_stack_pop(vm);
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

        case BC_OP_JUMP_IF_FALSE: {
            int offset = (int) READ_BYTE();
            struct VALUE val = *(vm->stack_top - 1);
            if (!val.int_val) {
                vm->ip += offset;
            }
            break;
        }
        case BC_OP_JUMP: {
            int offset = (int) READ_BYTE();
            vm->ip += offset;
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
    free_garbage_collector(vm->gc);
    SAFE_FREE(vm);
}

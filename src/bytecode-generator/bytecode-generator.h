#ifndef BYTECODE_GENERATOR_H_INCLUDED
#define BYTECODE_GENERATOR_H_INCLUDED

#include "parser.h"

enum BYTECODE_GENERATOR_CODES
{
    BYTECODE_GENERATOR_OK = 0,
};

struct BYTECODE_GENERATOR;

typedef struct BYTECODE_GENERATOR* bytecode_generator_type_t;

bytecode_generator_type_t create_bytecode_generator();

void bytecode_generator_conf(bytecode_generator_type_t bc_gen, struct UNIT_AST*ast);

struct BYTECODE;

enum BYTECODE_GENERATOR_CODES bytecode_generator_generate(bytecode_generator_type_t bc_gen, struct BYTECODE**bc);

void bytecode_generator_free(bytecode_generator_type_t bc);

enum BC_OP_CODES
{
    BC_OP_POP, /* remove value from stack */
 
    BC_OP_CONSTANT, /* push constant to stack */

    BC_OP_GET_LOCAL, /* get local variable value */
    BC_OP_SET_LOCAL, /* set local variable value */

    BC_OP_GET_HEAP, /* get heap variable value */ 
    BC_OP_SET_HEAP, /* set heap variable value */

    BC_OP_LOGICAL_OR,
    BC_OP_LOGICAL_AND,

    BC_OP_EQ_EQEQ,
    BC_OP_EQ_NEQ,

    BC_OP_REL_LT,
    BC_OP_REL_GT,
    BC_OP_REL_LE,
    BC_OP_REL_GE,

    BC_OP_ADDITIVE_PLUS,
    BC_OP_ADDITIVE_MINUS,

    BC_OP_MULTIPLICATIVE_MUL,
    BC_OP_MULTIPLICATIVE_DIV,
    BC_OP_MULTIPLICATIVE_MOD,

    BC_OP_NEGATE,
};

enum VALUE_TYPE
{
    VALUE_TYPE_INT,
    VALUE_TYPE_DOUBLE,
};

struct VALUE
{
    union
    {
        long long int_val;
        double double_val;
    };
    
    enum VALUE_TYPE type;
};

struct VALUE create_value_from_int(long long int_val);

struct VALUE create_value_from_double(double double_val);

struct LOCAL_VARIABLE
{
    char name[32];
    unsigned depth;
};

struct LOCAL_VARIABLE create_local_variable(const char*name, unsigned depth);

struct BYTECODE
{
    long long*op_codes;
    unsigned op_codes_len;
    unsigned op_codes_cap;

    struct VALUE*values;
    unsigned values_len;
    unsigned values_cap;

    struct LOCAL_VARIABLE*locals;
    unsigned locals_len;
    unsigned locals_cap;

    unsigned scope_depth;
};

typedef struct BYTECODE* bytecode_type_t;

bytecode_type_t create_bytecode();

void free_bytecode(bytecode_type_t bc);

#endif  /* BYTECODE_GENERATRO_H_INCLUDED */

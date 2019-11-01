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

    BC_OP_CREATE_OBJ,    /* create object in heap         */
    BC_OP_INIT_OBJ_PROP, /* initialize object's property. */

    BC_OP_GET_HEAP,   /* get heap variable value */
    BC_OP_SET_HEAP,   /* set heap variable value */

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

    BC_OP_RETURN,
};

enum CONSTANT_TYPE
{
    CONSTANT_TYPE_INTEGER,
    CONSTANT_TYPE_DOUBLE,

    CONSTANT_TYPE_FIELDREF,

    CONSTANT_TYPE_FUNCTIONREF,
};

struct CONSTANT
{
    union
    {
        long long int_cnst;
        double double_cnst;
        char str_cnst[32];
    };
    
    enum CONSTANT_TYPE type;
};

struct CONSTANT create_constant_from_int(long long int_cnst);
struct CONSTANT create_constant_from_double(double double_cnst);
struct CONSTANT create_constant_from_fieldred(const char*str_cnst);
struct CONSTANT create_constant_from_functionref(const char*str_cnst);

struct BYTECODE
{
    unsigned*op_codes;
    unsigned op_codes_len;
    unsigned op_codes_cap;

    struct CONSTANT*constant_pool;
    unsigned constant_pool_len;
    unsigned constant_pool_cap;
};

typedef struct BYTECODE* bytecode_type_t;

bytecode_type_t create_bytecode();

void dump_bytecode_to_xml_file(FILE*f, const bytecode_type_t bc);

void bytecode_free(bytecode_type_t bc);

#endif  /* BYTECODE_GENERATRO_H_INCLUDED */

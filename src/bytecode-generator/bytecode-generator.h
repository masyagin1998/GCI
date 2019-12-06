#ifndef BYTECODE_GENERATOR_H_INCLUDED
#define BYTECODE_GENERATOR_H_INCLUDED

#include "parser.h"

enum BYTECODE_GENERATOR_CODES
{
    BYTECODE_GENERATOR_OK                          =  0,
    BYTECODE_GENERATOR_NO_LOCAL_VARIABLE           = -1,
    BYTECODE_GENERATOR_ALREADY_HAVE_LOCAL_VARIABLE = -2,
    BYTECODE_GENERATOR_INVALID_BREAK               = -3,
    BYTECODE_GENERATOR_INVALID_CONTINUE            = -4,
};

struct BYTECODE_ERROR
{
    struct POS pos;
    enum BYTECODE_GENERATOR_CODES code;
};

struct BYTECODE_GENERATOR;

typedef struct BYTECODE_GENERATOR* bytecode_generator_type_t;

bytecode_generator_type_t create_bytecode_generator();

void bytecode_generator_conf(bytecode_generator_type_t bc_gen, struct UNIT_AST*ast);

struct BYTECODE;

enum BYTECODE_GENERATOR_CODES bytecode_generator_generate(bytecode_generator_type_t bc_gen, struct BYTECODE**bc);

struct BYTECODE_ERROR bytecode_generator_get_error(const bytecode_generator_type_t bc_gen);

void print_bytecode_error(const struct BYTECODE_ERROR*be);

void bytecode_generator_free(bytecode_generator_type_t bc);

enum BC_HEAP_OP
{
    BC_ARRAY_INDEX,  /* get value by array index.   */
    BC_OBJECT_FIELD, /* get value by objects field. */
};

enum BC_OP_CODES
{
    BC_OP_POP, /* remove value from stack */
 
    BC_OP_CONSTANT, /* push constant to stack */

    BC_OP_CREATE_LOCAL, /* create local variable value. */
    BC_OP_GET_LOCAL,    /* get local variable value.    */
    BC_OP_SET_LOCAL,    /* set local variable value.    */

    BC_OP_CREATE_OBJ,    /* create object in heap.        */
    BC_OP_INIT_OBJ_PROP, /* initialize object's property. */

    BC_OP_CREATE_ARR, /* create array in heap.         */

    BC_OP_GET_HEAP, /* get heap variable value.      */
    BC_OP_SET_HEAP, /* set heap variable value.      */

    BC_OP_APPEND, /* push back variable to array. */
    BC_OP_DELETE, /* delete property from obj/delete from array by index. */

    BC_OP_LOGICAL_OR,  /* || */
    BC_OP_LOGICAL_AND, /* && */

    BC_OP_EQ_EQEQ, /* == */
    BC_OP_EQ_NEQ,  /* != */

    BC_OP_REL_LT, /* <  */
    BC_OP_REL_GT, /* >  */
    BC_OP_REL_LE, /* <= */
    BC_OP_REL_GE, /* >= */

    BC_OP_ADDITIVE_PLUS,  /* + */
    BC_OP_ADDITIVE_MINUS, /* - */

    BC_OP_MULTIPLICATIVE_MUL, /* * */
    BC_OP_MULTIPLICATIVE_DIV, /* / */
    BC_OP_MULTIPLICATIVE_MOD, /* % */

    BC_OP_NEGATE, /* - */

    BC_OP_LEN,          /* get len of array; (-1) - not array.              */
    BC_OP_HAS_PROPERTY, /* check if property exists in obj; (-1) - not obj. */    

    BC_OP_JUMP_IF_FALSE, /* conditional jump.   */
    BC_OP_JUMP,          /* unconditional jump. */

    BC_OP_RETURN, /* return from function */
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
    size_t*op_codes;
    size_t op_codes_len;
    size_t op_codes_cap;

    struct CONSTANT*constant_pool;
    size_t constant_pool_len;
    size_t constant_pool_cap;

    struct BYTECODE_POS*poss;
};

typedef struct BYTECODE* bytecode_type_t;

bytecode_type_t create_bytecode();

void dump_bytecode_to_xml_file(FILE*f, const bytecode_type_t bc);

void bytecode_free(bytecode_type_t bc);

#endif  /* BYTECODE_GENERATOR_H_INCLUDED */

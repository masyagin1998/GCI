#include "bytecode-generator.h"

#include "utils.h"

#include <string.h>

struct BYTECODE_GENERATOR
{
    struct UNIT_AST*ast;
    struct BYTECODE*bc;
};

bytecode_generator_type_t create_bytecode_generator()
{
    struct BYTECODE_GENERATOR*bc_gen;
    SAFE_CALLOC(bc_gen, 1);
    return bc_gen;
}

void bytecode_generator_conf(bytecode_generator_type_t bc_gen, struct UNIT_AST*ast)
{
    bc_gen->ast = ast;
    bc_gen->bc = create_bytecode();
}

struct VALUE create_value_from_int(long long int_val)
{
    struct VALUE val;
    val.int_val = int_val;
    val.type = VALUE_TYPE_INT;
    return val;
}

struct VALUE create_value_from_double(double double_val)
{
    struct VALUE val;
    val.double_val = double_val;
    val.type = VALUE_TYPE_DOUBLE;
    return val;
}

struct LOCAL_VARIABLE create_local_variable(const char*name, unsigned depth)
{
    struct LOCAL_VARIABLE lv;
    strncpy(lv.name, name, sizeof(lv.name));
    lv.depth = depth;
    return lv;
}

static int local_variable_index(bytecode_type_t bc, const char*var_name)
{
    int i;
    
    for (i = bc->locals_len - 1; i >= 0; i--) {
        if (strcmp(bc->locals[i].name, var_name) == 0) {
            return i;
        }
    }

    return -1;
}

enum BYTECODE_GENERATOR_CODES number_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct NUMBER_AST*ast)
{
    struct VALUE v;
    
    v = create_value_from_int(ast->number);
    PUSH_BACK(bc_gen->bc->values, v);

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_CONSTANT);
    PUSH_BACK(bc_gen->bc->op_codes, bc_gen->bc->values_len - 1);

    return BYTECODE_GENERATOR_OK;
}

enum BYTECODE_GENERATOR_CODES variable_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct VARIABLE_AST*ast)
{
    int idx = local_variable_index(bc_gen->bc, ast->idents[0]->ident);

    if (idx == -1) {

    }    
    
    if (ast->idents_len == 1) {
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_GET_LOCAL);
        PUSH_BACK(bc_gen->bc->op_codes, idx);
    } else {
        
    }
    
    return BYTECODE_GENERATOR_OK;
}

enum BYTECODE_GENERATOR_CODES function_call_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct FUNCTION_CALL_AST*ast)
{
    return BYTECODE_GENERATOR_OK;
}

enum BYTECODE_GENERATOR_CODES logical_or_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct LOGICAL_OR_EXPR_AST*ast);

enum BYTECODE_GENERATOR_CODES primary_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct PRIMARY_EXPR_AST*ast)
{
    switch (ast->type) {
    case AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL:
        function_call_expr_ast_bytecode_generate(bc_gen, ast->function_call);
        break;
    case AST_PRIMARY_EXPR_TYPE_VARIABLE:
        variable_ast_bytecode_generate(bc_gen, ast->var_name);
        break;
    case AST_PRIMARY_EXPR_TYPE_NUMBER:
        number_ast_bytecode_generate(bc_gen, ast->number);
        break;
    case AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR:
        logical_or_expr_ast_bytecode_generate(bc_gen, ast->logical_or_expr);
        break;
    default:
        fprintf(stderr, "Invalid PRIMARY_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);        
        break;
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES left_unary_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct LEFT_UNARY_EXPR_AST*ast)
{

    primary_expr_ast_bytecode_generate(bc_gen, ast->expr);
    if (ast->op != AST_LEFT_UNARY_OP_PLUS) {
        if (ast->op == AST_LEFT_UNARY_OP_MINUS) {
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_NEGATE);
        }
    }

    return BYTECODE_GENERATOR_OK;
}

enum BYTECODE_GENERATOR_CODES multiplicative_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct MULTIPLICATIVE_EXPR_AST*ast)
{
    unsigned i;

    left_unary_expr_ast_bytecode_generate(bc_gen, ast->lues[0]);

    for (i = 1; i < ast->lues_len; i++) {
        left_unary_expr_ast_bytecode_generate(bc_gen, ast->lues[i]);
        switch (ast->ops[i - 1]) {
        case AST_MULTIPLICATIVE_OP_MUL:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_ADDITIVE_PLUS);
            break;
        case AST_MULTIPLICATIVE_OP_DIV:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_ADDITIVE_PLUS);
            break;
        case AST_MULTIPLICATIVE_OP_MOD:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_ADDITIVE_PLUS);
            break;
        default:
            fprintf(stderr, "Invalid AST_MULTIPLICATIVE_OP type: %d\n", ast->ops[i - 1]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES additive_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct ADDITIVE_EXPR_AST*ast)
{
    unsigned i;

    multiplicative_expr_ast_bytecode_generate(bc_gen, ast->muls[0]);

    for (i = 1; i < ast->muls_len; i++) {
        multiplicative_expr_ast_bytecode_generate(bc_gen, ast->muls[i]);
        if (ast->ops[i - 1] == AST_ADDITIVE_OP_PLUS) {
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_ADDITIVE_PLUS);
        } else if (ast->ops[i - 1] == AST_ADDITIVE_OP_MINUS) {
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_ADDITIVE_MINUS);
        } else {
            fprintf(stderr, "Invalid AST_ADDITIVE_OP type: %d\n", ast->ops[i - 1]);
            exit(EXIT_FAILURE);
        }
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES relational_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct RELATIONAL_EXPR_AST*ast)
{
    additive_expr_ast_bytecode_generate(bc_gen, ast->left);
    if (ast->right != NULL) {
        additive_expr_ast_bytecode_generate(bc_gen, ast->right);
        switch (ast->rel_op) {
        case AST_REL_OP_LT:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_REL_LT);
            break;
        case AST_REL_OP_GT:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_REL_GT);
            break;
        case AST_REL_OP_LE:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_REL_LE);
            break;
        case AST_REL_OP_GE:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_REL_GE);
            break;
        default:
            fprintf(stderr, "Invalid AST_REL_OP type: %d\n", ast->rel_op);
            exit(EXIT_FAILURE);            
            break;
        }
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES eq_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct EQ_EXPR_AST*ast)
{
    relational_expr_ast_bytecode_generate(bc_gen, ast->left);
    if (ast->right != NULL) {
        relational_expr_ast_bytecode_generate(bc_gen, ast->right);
        if (ast->eq_op == AST_EQ_OP_EQEQ) {
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_EQ_EQEQ);
        } else if (ast->eq_op == AST_EQ_OP_NEQ) {
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_EQ_NEQ);
        } else {
            fprintf(stderr, "Invalid AST_EQ_OP type: %d\n", ast->eq_op);
            exit(EXIT_FAILURE);            
        }
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES logical_and_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct LOGICAL_AND_EXPR_AST*ast)
{
    unsigned i;

    eq_expr_ast_bytecode_generate(bc_gen, ast->eq_exprs[0]);
    for (i = 1; i < ast->eq_exprs_len; i++) {
        eq_expr_ast_bytecode_generate(bc_gen, ast->eq_exprs[i]);
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_LOGICAL_AND);
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES logical_or_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct LOGICAL_OR_EXPR_AST*ast)
{
    unsigned i;

    logical_and_expr_ast_bytecode_generate(bc_gen, ast->and_exprs[0]);
    for (i = 1; i < ast->and_exprs_len; i++) {
        logical_and_expr_ast_bytecode_generate(bc_gen, ast->and_exprs[i]);
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_LOGICAL_OR);
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES assignment_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct ASSIGNMENT_EXPR_AST*ast)
{
    switch (ast->type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR:
        logical_or_expr_ast_bytecode_generate(bc_gen, ast->logical_or_expr);
        break;
    default:
        fprintf(stderr, "Invalid ASSIGNMENT_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES decl_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct DECL_STMT_AST*ast)
{
    int idx;
    
    assignment_expr_ast_bytecode_generate(bc_gen, ast->assignment);

    idx = local_variable_index(bc_gen->bc, ast->new_var_name->ident);

    if ((idx != -1) && (bc_gen->bc->locals[idx].depth == bc_gen->bc->scope_depth)) {
        
    }

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_SET_LOCAL);
    PUSH_BACK(bc_gen->bc->op_codes, idx);

    return BYTECODE_GENERATOR_OK;
}

enum BYTECODE_GENERATOR_CODES assign_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct ASSIGN_STMT_AST*ast)
{
    int idx;
    
    assignment_expr_ast_bytecode_generate(bc_gen, ast->assignment);

    idx = local_variable_index(bc_gen->bc, ast->var_name->idents[0]->ident);

    if (idx == -1) {
        
    }

    if (ast->var_name->idents_len == 1) {
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_SET_LOCAL);
        PUSH_BACK(bc_gen->bc->op_codes, idx);
    } else {
        
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES function_call_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct FUNCTION_CALL_STMT_AST*ast)
{
    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES if_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct IF_STMT_AST*ast)
{
    return BYTECODE_GENERATOR_OK;
}

enum BYTECODE_GENERATOR_CODES while_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct WHILE_STMT_AST*ast)
{
    return BYTECODE_GENERATOR_OK;
}

enum BYTECODE_GENERATOR_CODES return_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct RETURN_STMT_AST*ast)
{
    assignment_expr_ast_bytecode_generate(bc_gen, ast->result);

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct STMT_AST*ast)
{
    switch (ast->type) {
    case AST_STMT_TYPE_DECL:
        decl_stmt_ast_bytecode_generate(bc_gen, ast->decl_stmt);
        break;
    case AST_STMT_TYPE_ASSIGN:
        assign_stmt_ast_bytecode_generate(bc_gen, ast->assign_stmt);
        break;
    case AST_STMT_TYPE_FUNCTION_CALL:
        function_call_stmt_ast_bytecode_generate(bc_gen, ast->function_call_stmt);
        break;
    case AST_STMT_TYPE_IF:
        if_stmt_ast_bytecode_generate(bc_gen, ast->if_stmt);
        break;
    case AST_STMT_TYPE_WHILE:
        while_stmt_ast_bytecode_generate(bc_gen, ast->while_stmt);
        break;
    case AST_STMT_TYPE_RETURN:
        return_stmt_ast_bytecode_generate(bc_gen, ast->return_stmt);
        break;
    default:
        fprintf(stderr, "Invalid STMT_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES body_ast_bytecode_generate(bytecode_generator_type_t bc_gen, struct BODY_AST*ast)
{
    unsigned i;

    bc_gen->bc->scope_depth++;

    for (i = 0; i < ast->stmts_len; i++) {
        stmt_ast_bytecode_generate(bc_gen, ast->stmts[i]);
    }

    bc_gen->bc->scope_depth--;

    while ((bc_gen->bc->locals_len > 0) &&
           (bc_gen->bc->locals[bc_gen->bc->locals_len - 1].depth >
            bc_gen->bc->scope_depth)) {
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_POP);
       bc_gen-> bc->locals_len--;
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES bytecode_generator_generate(bytecode_generator_type_t bc_gen, struct BYTECODE**bc)
{
    /* now only one function without arguments is supported. */
    struct FUNCTION_DECL_AST*f = bc_gen->ast->functions[0];
    body_ast_bytecode_generate(bc_gen, f->body);

    return BYTECODE_GENERATOR_OK;    
}

void bytecode_generator_free(bytecode_generator_type_t bc)
{
    SAFE_FREE(bc);
}

bytecode_type_t create_bytecode()
{
    struct BYTECODE*bc;
    SAFE_CALLOC(bc, 1);
    return bc;
}

void free_bytecode(bytecode_type_t bc)
{
    SAFE_FREE(bc->op_codes);
    SAFE_FREE(bc->values);
    SAFE_FREE(bc->locals);
    SAFE_FREE(bc);
}

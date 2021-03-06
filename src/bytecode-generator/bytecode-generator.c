#include "bytecode-generator.h"

#include "utils.h"

#include <string.h>

struct BYTECODE_GENERATOR
{
    struct UNIT_AST*ast;
    struct BYTECODE*bc;

    struct LOCAL_VARIABLE*locals;
    size_t locals_len;
    size_t locals_cap;

    size_t scope_depth;

    struct BYTECODE_ERROR err;
};

/* constant-pool functions. */

struct CONSTANT create_constant_from_int(long long int_cnst)
{
    struct CONSTANT cnst;
    memset(&cnst, 0, sizeof(cnst));
    cnst.int_cnst = int_cnst;
    cnst.type = CONSTANT_TYPE_INTEGER;
    return cnst;
}

struct CONSTANT create_constant_from_double(double double_cnst)
{
    struct CONSTANT cnst;
    memset(&cnst, 0, sizeof(cnst));
    cnst.double_cnst = double_cnst;
    cnst.type = CONSTANT_TYPE_DOUBLE;
    return cnst;
}

struct CONSTANT create_constant_from_fieldref(const char*str_cnst)
{
    struct CONSTANT cnst;
    memset(&cnst, 0, sizeof(cnst));
    strncpy(cnst.str_cnst, str_cnst, sizeof(cnst.str_cnst));
    cnst.type = CONSTANT_TYPE_FIELDREF;
    return cnst;
}

struct CONSTANT create_constant_from_functionref(const char*str_cnst)
{
    struct CONSTANT cnst;
    memset(&cnst, 0, sizeof(cnst));
    strncpy(cnst.str_cnst, str_cnst, sizeof(cnst.str_cnst));
    cnst.type = CONSTANT_TYPE_FUNCTIONREF;
    return cnst;
}

static int constant_pool_push_back(bytecode_type_t bc, struct CONSTANT cnst)
{
    size_t i;

    for (i = 0; i < bc->constant_pool_len; i++) {
        if (memcmp((char*) &cnst, (char*) (bc->constant_pool + i), sizeof(struct CONSTANT)) == 0) {
            return i;
        }
    }

    PUSH_BACK(bc->constant_pool, cnst);

    return bc->constant_pool_len - 1;
}

/* local variables functions. */

struct LOCAL_VARIABLE
{
    char name[32];
    size_t depth;
};

static struct LOCAL_VARIABLE create_local_variable(const char*name, size_t depth)
{
    struct LOCAL_VARIABLE lv;
    strncpy(lv.name, name, sizeof(lv.name));
    lv.depth = depth;
    return lv;
}

static int local_variable_index(bytecode_generator_type_t bc_gen, const char*var_name)
{
    int i;
    
    for (i = bc_gen->locals_len - 1; i >= 0; i--) {
        if (strcmp(bc_gen->locals[i].name, var_name) == 0) {
            return i;
        }
    }

    return -1;
}

/* bytecode generator functions. */

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

static void set_bytecode_generator_error(bytecode_generator_type_t bc_gen, size_t line, size_t pos, enum BYTECODE_GENERATOR_CODES code)
{
    bc_gen->err.pos.line = line;
    bc_gen->err.pos.pos = pos;
    bc_gen->err.code = code;
}

static enum BYTECODE_GENERATOR_CODES assignment_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct ASSIGNMENT_EXPR_AST*ast);

static enum BYTECODE_GENERATOR_CODES property_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct PROPERTY_AST*ast) {
    struct CONSTANT cnst;
    
    enum BYTECODE_GENERATOR_CODES r = assignment_expr_ast_bytecode_generate(bc_gen, ast->value);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_INIT_OBJ_PROP);
    cnst = create_constant_from_fieldref(ast->key->ident);
    PUSH_BACK(bc_gen->bc->op_codes, constant_pool_push_back(bc_gen->bc, cnst));

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES object_literal_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct OBJECT_LITERAL_AST*ast)
{
    size_t i;

    for (i = 0; i < ast->properties_len; i++) {
        enum BYTECODE_GENERATOR_CODES r = property_ast_bytecode_generate(bc_gen, ast->properties[i]);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
    }

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_CREATE_OBJ);
    PUSH_BACK(bc_gen->bc->op_codes, ast->properties_len);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES array_literal_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct ARRAY_LITERAL_AST*ast)
{
    int i;

    if (ast->args_list != NULL) {
        for (i = ast->args_list->assignment_exprs_len - 1; i >= 0; i--) {
            enum BYTECODE_GENERATOR_CODES r = assignment_expr_ast_bytecode_generate(bc_gen, ast->args_list->assignment_exprs[i]);
            if (r != BYTECODE_GENERATOR_OK) {
                return r;
            }
        }
    }

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_CREATE_ARR);

    if (ast->args_list != NULL) {
        PUSH_BACK(bc_gen->bc->op_codes, ast->args_list->assignment_exprs_len);
    } else {
        PUSH_BACK(bc_gen->bc->op_codes, 0);
    }

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES number_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct NUMBER_AST*ast)
{
    struct CONSTANT cnst = create_constant_from_int(ast->number);
    size_t index = constant_pool_push_back(bc_gen->bc, cnst);

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_CONSTANT);
    PUSH_BACK(bc_gen->bc->op_codes, index);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES logical_or_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LOGICAL_OR_EXPR_AST*ast);

static enum BYTECODE_GENERATOR_CODES variable_ast_bytecode_generate_inner(bytecode_generator_type_t bc_gen, const struct VARIABLE_AST*ast, int is_set_op)
{
    int idx = local_variable_index(bc_gen, ast->ident->ident);

    if (idx == -1) {
        set_bytecode_generator_error(bc_gen, ast->line, ast->pos, BYTECODE_GENERATOR_NO_LOCAL_VARIABLE);
        return BYTECODE_GENERATOR_NO_LOCAL_VARIABLE;
    }

    if (ast->parts_len == 0) {
        PUSH_BACK(bc_gen->bc->op_codes, is_set_op ? BC_OP_SET_LOCAL : BC_OP_GET_LOCAL);
        PUSH_BACK(bc_gen->bc->op_codes, idx);        
    } else {
        size_t i;
        size_t count = 0;

        /* Generate code for array indexes. */
        for (i = 0; i < ast->parts_len; i++) {
            if (ast->parts[i]->type == AST_VARIABLE_PART_TYPE_INDEX) {
                count++;
                logical_or_expr_ast_bytecode_generate(bc_gen, ast->parts[i]->index);
            }
        }
        
        PUSH_BACK(bc_gen->bc->op_codes, is_set_op ? BC_OP_SET_HEAP : BC_OP_GET_HEAP);
        PUSH_BACK(bc_gen->bc->op_codes, idx);
        PUSH_BACK(bc_gen->bc->op_codes, ast->parts_len);

        for (i = 0; i < ast->parts_len; i++) {
            if (ast->parts[i]->type == AST_VARIABLE_PART_TYPE_FIELD) {
                struct CONSTANT cnst = create_constant_from_fieldref(ast->parts[i]->field->ident);
                PUSH_BACK(bc_gen->bc->op_codes, BC_OBJECT_FIELD);
                PUSH_BACK(bc_gen->bc->op_codes, constant_pool_push_back(bc_gen->bc, cnst));   
            } else if (ast->parts[i]->type == AST_VARIABLE_PART_TYPE_INDEX) {
                PUSH_BACK(bc_gen->bc->op_codes, BC_ARRAY_INDEX);
                count--;
                PUSH_BACK(bc_gen->bc->op_codes, count);
            } else {
                fprintf(stderr, "Invalid VARIALE_PART_TYPE: %d\n", ast->parts[i]->type);
                exit(EXIT_FAILURE);        
            }
        }   
    }
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES variable_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct VARIABLE_AST*ast)
{
    return variable_ast_bytecode_generate_inner(bc_gen, ast, 0);
}

static enum BYTECODE_GENERATOR_CODES has_property_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct HAS_PROPERTY_EXPR_AST*ast)
{
    struct CONSTANT cnst;
    enum BYTECODE_GENERATOR_CODES r = variable_ast_bytecode_generate_inner(bc_gen, ast->obj, 0);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    
    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_HAS_PROPERTY);
    cnst = create_constant_from_fieldref(ast->ident->ident);
    PUSH_BACK(bc_gen->bc->op_codes, constant_pool_push_back(bc_gen->bc, cnst));    

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES len_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LEN_EXPR_AST*ast)
{
    enum BYTECODE_GENERATOR_CODES r = variable_ast_bytecode_generate_inner(bc_gen, ast->arr, 0);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }
    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_LEN);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES function_call_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct FUNCTION_CALL_AST*ast)
{
    /* TODO */
    PREFIX_UNUSED(bc_gen);
    PREFIX_UNUSED(ast);
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES primary_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct PRIMARY_EXPR_AST*ast)
{
    enum BYTECODE_GENERATOR_CODES r;
    
    switch (ast->type) {
    case AST_PRIMARY_EXPR_TYPE_HAS_PROPERTY:
        r = has_property_expr_ast_bytecode_generate(bc_gen, ast->has_property_expr);
        break;
    case AST_PRIMARY_EXPR_TYPE_LEN:
        r = len_expr_ast_bytecode_generate(bc_gen, ast->len_expr);
        break;
    case AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL:
        r = function_call_expr_ast_bytecode_generate(bc_gen, ast->function_call);
        break;
    case AST_PRIMARY_EXPR_TYPE_VARIABLE:
        r = variable_ast_bytecode_generate(bc_gen, ast->var_name);
        break;
    case AST_PRIMARY_EXPR_TYPE_NUMBER:
        r = number_ast_bytecode_generate(bc_gen, ast->number);
        break;
    case AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR:
        r = logical_or_expr_ast_bytecode_generate(bc_gen, ast->logical_or_expr);
        break;
    default:
        fprintf(stderr, "Invalid PRIMARY_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);        
        break;
    }

    return r;
}

static enum BYTECODE_GENERATOR_CODES left_unary_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LEFT_UNARY_EXPR_AST*ast)
{

    enum BYTECODE_GENERATOR_CODES r = primary_expr_ast_bytecode_generate(bc_gen, ast->expr);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }
    if (ast->op != AST_LEFT_UNARY_OP_PLUS) {
        if (ast->op == AST_LEFT_UNARY_OP_MINUS) {
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_NEGATE);
        }
    }

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES multiplicative_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct MULTIPLICATIVE_EXPR_AST*ast)
{
    size_t i;

    enum BYTECODE_GENERATOR_CODES r = left_unary_expr_ast_bytecode_generate(bc_gen, ast->lues[0]);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }
    for (i = 1; i < ast->lues_len; i++) {
        r = left_unary_expr_ast_bytecode_generate(bc_gen, ast->lues[i]);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
        switch (ast->ops[i - 1]) {
        case AST_MULTIPLICATIVE_OP_MUL:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_MULTIPLICATIVE_MUL);
            break;
        case AST_MULTIPLICATIVE_OP_DIV:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_MULTIPLICATIVE_DIV);
            break;
        case AST_MULTIPLICATIVE_OP_MOD:
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_MULTIPLICATIVE_MOD);
            break;
        default:
            fprintf(stderr, "Invalid AST_MULTIPLICATIVE_OP type: %d\n", ast->ops[i - 1]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES additive_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct ADDITIVE_EXPR_AST*ast)
{
    size_t i;

    enum BYTECODE_GENERATOR_CODES r = multiplicative_expr_ast_bytecode_generate(bc_gen, ast->muls[0]);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }
    for (i = 1; i < ast->muls_len; i++) {
        r = multiplicative_expr_ast_bytecode_generate(bc_gen, ast->muls[i]);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
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

static enum BYTECODE_GENERATOR_CODES relational_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct RELATIONAL_EXPR_AST*ast)
{
    enum BYTECODE_GENERATOR_CODES r = additive_expr_ast_bytecode_generate(bc_gen, ast->left);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }
    if (ast->right != NULL) {
        r = additive_expr_ast_bytecode_generate(bc_gen, ast->right);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
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

static enum BYTECODE_GENERATOR_CODES eq_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct EQ_EXPR_AST*ast)
{
    enum BYTECODE_GENERATOR_CODES r = relational_expr_ast_bytecode_generate(bc_gen, ast->left);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    
    if (ast->right != NULL) {
        r = relational_expr_ast_bytecode_generate(bc_gen, ast->right);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
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

static enum BYTECODE_GENERATOR_CODES logical_and_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LOGICAL_AND_EXPR_AST*ast)
{
    size_t i;

    enum BYTECODE_GENERATOR_CODES r = eq_expr_ast_bytecode_generate(bc_gen, ast->eq_exprs[0]);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    
    for (i = 1; i < ast->eq_exprs_len; i++) {
        r = eq_expr_ast_bytecode_generate(bc_gen, ast->eq_exprs[i]);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_LOGICAL_AND);
    }

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES logical_or_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LOGICAL_OR_EXPR_AST*ast)
{
    size_t i;

    enum BYTECODE_GENERATOR_CODES r = logical_and_expr_ast_bytecode_generate(bc_gen, ast->and_exprs[0]);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    
    
    for (i = 1; i < ast->and_exprs_len; i++) {
        r = logical_and_expr_ast_bytecode_generate(bc_gen, ast->and_exprs[i]);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_LOGICAL_OR);
    }

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES assignment_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct ASSIGNMENT_EXPR_AST*ast)
{
    enum BYTECODE_GENERATOR_CODES r;
    
    switch (ast->type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        r = object_literal_ast_bytecode_generate(bc_gen, ast->object_literal);
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_ARRAY_LITERAL:
        r = array_literal_ast_bytecode_generate(bc_gen, ast->array_literal);
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR:
        r = logical_or_expr_ast_bytecode_generate(bc_gen, ast->logical_or_expr);
        break;
    default:
        fprintf(stderr, "Invalid ASSIGNMENT_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;
    }

    return r;
}

static enum BYTECODE_GENERATOR_CODES decl_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct DECL_STMT_AST*ast)
{
    int idx;

    struct LOCAL_VARIABLE lv;
    
    enum BYTECODE_GENERATOR_CODES r = assignment_expr_ast_bytecode_generate(bc_gen, ast->assignment);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }

    idx = local_variable_index(bc_gen, ast->new_var_name->ident);

    if ((idx != -1) && (bc_gen->locals[idx].depth == bc_gen->scope_depth)) {
        set_bytecode_generator_error(bc_gen, ast->line, ast->pos, BYTECODE_GENERATOR_ALREADY_HAVE_LOCAL_VARIABLE);
        return BYTECODE_GENERATOR_ALREADY_HAVE_LOCAL_VARIABLE;
    }

    lv = create_local_variable(ast->new_var_name->ident, bc_gen->scope_depth);
    PUSH_BACK(bc_gen->locals, lv);
    idx = bc_gen->locals_len - 1;

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_SET_LOCAL);
    PUSH_BACK(bc_gen->bc->op_codes, idx);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES assign_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct ASSIGN_STMT_AST*ast)
{
    enum BYTECODE_GENERATOR_CODES r = assignment_expr_ast_bytecode_generate(bc_gen, ast->assignment);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    

    return variable_ast_bytecode_generate_inner(bc_gen, ast->var_name, 1);
}

enum BYTECODE_GENERATOR_CODES function_call_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct FUNCTION_CALL_STMT_AST*ast)
{
    /* TODO */
    PREFIX_UNUSED(bc_gen);
    PREFIX_UNUSED(ast);
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES body_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct BODY_AST*ast, int*loop_start_idxs,
                                                                int**loop_exit_idxs, size_t*loop_exit_idxs_len, size_t*loop_exit_idxs_cap);

static int emit_jump(bytecode_generator_type_t bc_gen, size_t instruction)
{
    PUSH_BACK(bc_gen->bc->op_codes, instruction);
    PUSH_BACK(bc_gen->bc->op_codes, 0x0); /* empty instruction. */

    return bc_gen->bc->op_codes_len - 1;
}

static void patch_jump(bytecode_generator_type_t bc_gen, size_t offset)
{
    int jump = bc_gen->bc->op_codes_len - offset - 1;

    bc_gen->bc->op_codes[offset] = jump;
}

static enum BYTECODE_GENERATOR_CODES if_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct IF_STMT_AST*ast, int*loop_start_idx,
                                                                   int**loop_exit_idxs, size_t*loop_exit_idxs_len, size_t*loop_exit_idxs_cap)
{
    enum BYTECODE_GENERATOR_CODES r;
    
    int if_idx;
    int else_idx;
    
    r = logical_or_expr_ast_bytecode_generate(bc_gen, ast->condition);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    

    if_idx = emit_jump(bc_gen, BC_OP_JUMP_IF_FALSE);
    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_POP);

    r = body_ast_bytecode_generate(bc_gen, ast->if_body, loop_start_idx,
                                   loop_exit_idxs, loop_exit_idxs_len, loop_exit_idxs_cap);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    

    else_idx = emit_jump(bc_gen, BC_OP_JUMP);

    patch_jump(bc_gen, if_idx);
    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_POP);

    if (ast->else_body != NULL) {
        r = body_ast_bytecode_generate(bc_gen, ast->else_body, loop_start_idx,
                                       loop_exit_idxs, loop_exit_idxs_len, loop_exit_idxs_cap);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
    }

    patch_jump(bc_gen, else_idx);
    
    return BYTECODE_GENERATOR_OK;
}

static void emit_loop(bytecode_generator_type_t bc_gen, size_t loop_start)
{
    int offset;
    
    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_JUMP);

    offset = ((int) (bc_gen->bc->op_codes_len - loop_start + 1)) * (-1);
    
    PUSH_BACK(bc_gen->bc->op_codes, offset);
}

static enum BYTECODE_GENERATOR_CODES while_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct WHILE_STMT_AST*ast)
{
    enum BYTECODE_GENERATOR_CODES r;
    
    size_t i;
    
    int loop_start_idx;
    
    int*loop_exit_idxs = NULL;
    size_t loop_exit_idxs_len = 0;
    size_t loop_exit_idxs_cap = 0;        

    loop_start_idx = bc_gen->bc->op_codes_len;
    
    r = logical_or_expr_ast_bytecode_generate(bc_gen, ast->condition);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }

    PUSH_BACK(loop_exit_idxs, emit_jump(bc_gen, BC_OP_JUMP_IF_FALSE));
    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_POP);

    r = body_ast_bytecode_generate(bc_gen, ast->body, &loop_start_idx,
                                   &loop_exit_idxs, &loop_exit_idxs_len, &loop_exit_idxs_cap);
    if (r != BYTECODE_GENERATOR_OK) {
        return r;
    }    

    emit_loop(bc_gen, loop_start_idx);

    patch_jump(bc_gen, loop_exit_idxs[0]);
    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_POP);
    
    for (i = 1; i < loop_exit_idxs_len; i++) {
        patch_jump(bc_gen, loop_exit_idxs[i]);
    }

    free(loop_exit_idxs);
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES break_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct BREAK_STMT_AST*ast,
                                                                      int**loop_exit_idxs, size_t*loop_exit_idxs_len, size_t*loop_exit_idxs_cap)
{
    int loop_exit_idx;
    
    PREFIX_UNUSED(ast);

    if (loop_exit_idxs == NULL) {
        set_bytecode_generator_error(bc_gen, ast->line, ast->pos, BYTECODE_GENERATOR_INVALID_BREAK);
        return BYTECODE_GENERATOR_INVALID_BREAK;
    }

    loop_exit_idx = emit_jump(bc_gen, BC_OP_JUMP);

    PUSH_BACK(*loop_exit_idxs, loop_exit_idx);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES continue_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct CONTINUE_STMT_AST*ast, int*loop_start_idx)
{
    PREFIX_UNUSED(ast);

    if (loop_start_idx == NULL) {
        set_bytecode_generator_error(bc_gen, ast->line, ast->pos, BYTECODE_GENERATOR_INVALID_CONTINUE);
        return BYTECODE_GENERATOR_INVALID_CONTINUE;
    }

    emit_loop(bc_gen, (*loop_start_idx));

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES append_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct APPEND_STMT_AST*ast)
{
    PREFIX_UNUSED(bc_gen);
    PREFIX_UNUSED(ast);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES delete_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct DELETE_STMT_AST*ast)
{
    PREFIX_UNUSED(bc_gen);
    PREFIX_UNUSED(ast);    

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES return_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct RETURN_STMT_AST*ast)
{
    if (ast->result != NULL) {
        enum BYTECODE_GENERATOR_CODES r = assignment_expr_ast_bytecode_generate(bc_gen, ast->result);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
    }

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_RETURN);

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct STMT_AST*ast, int*loop_start_idx,
                                                                int**loop_exit_idxs, size_t*loop_exit_idxs_len, size_t*loop_exit_idxs_cap)
{
    enum BYTECODE_GENERATOR_CODES r;
    
    switch (ast->type) {
    case AST_STMT_TYPE_DECL:
        r = decl_stmt_ast_bytecode_generate(bc_gen, ast->decl_stmt);
        break;
    case AST_STMT_TYPE_ASSIGN:
        r = assign_stmt_ast_bytecode_generate(bc_gen, ast->assign_stmt);
        break;
    case AST_STMT_TYPE_FUNCTION_CALL:
        r = function_call_stmt_ast_bytecode_generate(bc_gen, ast->function_call_stmt);
        break;
    case AST_STMT_TYPE_IF:
        r = if_stmt_ast_bytecode_generate(bc_gen, ast->if_stmt, loop_start_idx,
                                          loop_exit_idxs, loop_exit_idxs_len, loop_exit_idxs_cap);
        break;
    case AST_STMT_TYPE_WHILE:
        r = while_stmt_ast_bytecode_generate(bc_gen, ast->while_stmt);
        break;
    case AST_STMT_TYPE_BREAK:
        r = break_stmt_ast_bytecode_generate(bc_gen, ast->break_stmt, loop_exit_idxs, loop_exit_idxs_len, loop_exit_idxs_cap);
        break;
    case AST_STMT_TYPE_CONTINUE:
        r = continue_stmt_ast_bytecode_generate(bc_gen, ast->continue_stmt, loop_start_idx);
        break;
    case AST_STMT_TYPE_APPEND:
        r = append_stmt_ast_bytecode_generate(bc_gen, ast->append_stmt);
        break;
    case AST_STMT_TYPE_DELETE:
        r = delete_stmt_ast_bytecode_generate(bc_gen, ast->delete_stmt);
        break;        
    case AST_STMT_TYPE_RETURN:
        r = return_stmt_ast_bytecode_generate(bc_gen, ast->return_stmt);
        break;
    default:
        fprintf(stderr, "Invalid STMT_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;
    }

    return r;    
}

static enum BYTECODE_GENERATOR_CODES body_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct BODY_AST*ast, int*loop_start_idx,
                                                                int**loop_exit_idxs, size_t*loop_exit_idxs_len, size_t*loop_exit_idxs_cap)
{    
    size_t i;

    bc_gen->scope_depth++;

    for (i = 0; i < ast->stmts_len; i++) {
        enum BYTECODE_GENERATOR_CODES r = stmt_ast_bytecode_generate(bc_gen, ast->stmts[i], loop_start_idx,
                                                                     loop_exit_idxs, loop_exit_idxs_len, loop_exit_idxs_cap);
        if (r != BYTECODE_GENERATOR_OK) {
            return r;
        }
    }

    bc_gen->scope_depth--;

    while ((bc_gen->locals_len > 0) &&
           (bc_gen->locals[bc_gen->locals_len - 1].depth >
            bc_gen->scope_depth)) {
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_POP);
        bc_gen->locals_len--;
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES bytecode_generator_generate(bytecode_generator_type_t bc_gen, struct BYTECODE**bc)
{   
    /* now only one function without arguments is supported. */
    struct FUNCTION_DECL_AST*f = bc_gen->ast->functions[0];
    enum BYTECODE_GENERATOR_CODES r = body_ast_bytecode_generate(bc_gen, f->body, NULL,
                                                                 NULL, NULL, NULL);
    if (r != BYTECODE_GENERATOR_OK) {
        bytecode_free(bc_gen->bc);
        return r;
    }

    (*bc) = bc_gen->bc;
    return BYTECODE_GENERATOR_OK;
}

void bytecode_generator_free(bytecode_generator_type_t bc_gen)
{
    SAFE_FREE(bc_gen->locals);
    SAFE_FREE(bc_gen);
}

bytecode_type_t create_bytecode()
{
    struct BYTECODE*bc;
    SAFE_CALLOC(bc, 1);
    return bc;
}

void dump_bytecode_to_xml_file(FILE*f, const bytecode_type_t bc)
{
    size_t i;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<bytecode>\n");

    fprintf(f, "\t<constant_pool>\n");
    for (i = 0; i < bc->constant_pool_len; i++) {
        switch (bc->constant_pool[i].type) {
        case CONSTANT_TYPE_INTEGER:
            fprintf(f, "\t\t<int_cnst idx=\"%zu\">%lld</int_cnst>\n", i, bc->constant_pool[i].int_cnst);
            break;
        case CONSTANT_TYPE_DOUBLE:
            fprintf(f, "\t\t<double_cnst idx=\"%zu\">%lf</double_cnst>\n", i, bc->constant_pool[i].double_cnst);
            break;
        case CONSTANT_TYPE_FIELDREF:
            fprintf(f, "\t\t<fieldref_cnst idx=\"%zu\">%s</fieldref_cnst>\n", i, bc->constant_pool[i].str_cnst);
            break;
        case CONSTANT_TYPE_FUNCTIONREF:
            fprintf(f, "\t\t<functionref_cnst idx=\"%zu\">%s</functionref_cnst>\n", i, bc->constant_pool[i].str_cnst);
            break;
        }
    }
    fprintf(f, "\t</constant_pool>\n");

    fprintf(f, "\t<op_codes>\n");
    
    i = 0;
    while (i < bc->op_codes_len) {
        size_t j, n;
        
        switch (bc->op_codes[i]) {
        case BC_OP_POP:
            fprintf(f, "\t\t<op>POP<op>\n");
            break;

        case BC_OP_CONSTANT:
            i++;
            fprintf(f, "\t\t<op>CONSTANT %zu</op>\n", bc->op_codes[i]);
            break;

        case BC_OP_GET_LOCAL:
            i++;
            fprintf(f, "\t\t<op>GET_LOCAL %zu</op>\n", bc->op_codes[i]);
            break;
        case BC_OP_SET_LOCAL:
            i++;
            fprintf(f, "\t\t<op>SET_LOCAL %zu</op>\n", bc->op_codes[i]);
            break;

        case BC_OP_CREATE_OBJ:
            i++;
            fprintf(f, "\t\t<op>CREATE_OBJ %zu</op>\n", bc->op_codes[i]);
            break;
        case BC_OP_INIT_OBJ_PROP:
            i++;
            fprintf(f, "\t\t<op>INIT_OBJ_PROP %zu</op>\n", bc->op_codes[i]);
            break;

        case BC_OP_CREATE_ARR:
            i++;
            fprintf(f, "\t\t<op>CREATE_ARR %zu</op>\n", bc->op_codes[i]);
            break;

        case BC_OP_GET_HEAP:
        case BC_OP_SET_HEAP:
            if (bc->op_codes[i] == BC_OP_GET_HEAP) {
                i++;
                fprintf(f, "\t\t<op>GET_HEAP %zu", bc->op_codes[i]);
            } else if (bc->op_codes[i] == BC_OP_SET_HEAP) {
                i++;
                fprintf(f, "\t\t<op>SET_HEAP %zu", bc->op_codes[i]);
            }

            i++;
            n = bc->op_codes[i];
            i++;
            for (j = 0; j < n; j++) {
                if (bc->op_codes[i] == BC_OBJECT_FIELD) {
                    i++;
                    fprintf(f, " field(%zu)", bc->op_codes[i]);
                } else if (bc->op_codes[i] == BC_ARRAY_INDEX) {
                    i++;
                    fprintf(f, " index(%zu)", bc->op_codes[i]);
                }
                i++;
            }
            i--;
            fprintf(f, "</op>\n");
            break;

        case BC_OP_APPEND:
        case BC_OP_DELETE:
            break;
            
        case BC_OP_LOGICAL_OR:
            fprintf(f, "\t\t<op>LOGICAL_OR</op>\n");
            break;
        case BC_OP_LOGICAL_AND:
            fprintf(f, "\t\t<op>LOGICAL_AND</op>\n");
            break;

        case BC_OP_EQ_EQEQ:
            fprintf(f, "\t\t<op>EQ_EQEQ</op>\n");
            break;
        case BC_OP_EQ_NEQ:
            fprintf(f, "\t\t<op>EQ_NEQ</op>\n");
            break;

        case BC_OP_REL_LT:
            fprintf(f, "\t\t<op>REL_LT</op>\n");            
            break;
        case BC_OP_REL_GT:
            fprintf(f, "\t\t<op>REL_GT</op>\n");            
            break;
        case BC_OP_REL_LE:
            fprintf(f, "\t\t<op>REL_LE</op>\n");            
            break;
        case BC_OP_REL_GE:
            fprintf(f, "\t\t<op>REL_GE</op>\n");            
            break;

        case BC_OP_ADDITIVE_PLUS:
            fprintf(f, "\t\t<op>ADDITIVE_PLUS</op>\n");
            break;
        case BC_OP_ADDITIVE_MINUS:
            fprintf(f, "\t\t<op>ADDITIVE_MINUS</op>\n");
            break;

        case BC_OP_MULTIPLICATIVE_MUL:
            fprintf(f, "\t\t<op>MULTIPLICATIVE_MUL</op>\n");
            break;
        case BC_OP_MULTIPLICATIVE_DIV:
            fprintf(f, "\t\t<op>MULTIPLICATIVE_DIV</op>\n");
            break;
        case BC_OP_MULTIPLICATIVE_MOD:
            fprintf(f, "\t\t<op>MULTIPLICATIVE_MOD</op>\n");
            break;

        case BC_OP_NEGATE:
            fprintf(f, "\t\t<op>NEGATE</op>\n");
            break;

        case BC_OP_LEN:
            fprintf(f, "\t\t<op>LEN</op>\n");
            break;
        case BC_OP_HAS_PROPERTY:
            i++;
            fprintf(f, "\t\t<op>HAS_PROPERTY %zu</op>\n", bc->op_codes[i]);
            break;            

        case BC_OP_JUMP_IF_FALSE:
            i++;
            fprintf(f, "\t\t<op>JUMP_IF_FALSE %d</op>\n", (int) bc->op_codes[i]);
            break;
        case BC_OP_JUMP:
            i++;
            fprintf(f, "\t\t<op>JUMP %d</op>\n", (int) bc->op_codes[i]);
            break;

        case BC_OP_RETURN:
            fprintf(f, "\t\t<op>RETURN</op>\n");
            break;
        }

        i++;
    }
    fprintf(f, "\t</op_codes>\n");    

    fprintf(f, "</bytecode>\n");
}

struct BYTECODE_ERROR bytecode_generator_get_error(const bytecode_generator_type_t bc_gen)
{
    return bc_gen->err;
}

void print_bytecode_error(const struct BYTECODE_ERROR*err)
{
    const char*error_str = NULL;

    switch (err->code) {
    case BYTECODE_GENERATOR_OK: {
        error_str = "ok?!";
        break;        
    }
    case BYTECODE_GENERATOR_NO_LOCAL_VARIABLE: {
        error_str = "no local variable!";
        break;
    }
    case BYTECODE_GENERATOR_ALREADY_HAVE_LOCAL_VARIABLE: {
        error_str = "already have local variable!";
        break;
    }
    case BYTECODE_GENERATOR_INVALID_BREAK: {
        error_str = "break outside of while!";
        break;
    }
    case BYTECODE_GENERATOR_INVALID_CONTINUE: {
        error_str = "continue outside of while!";
        break;
    }
    }
    
    fprintf(stderr, "%zu:%zu: error: %s\n",
            err->pos.line, err->pos.pos, error_str);
}

void bytecode_free(bytecode_type_t bc)
{
    SAFE_FREE(bc->op_codes);
    SAFE_FREE(bc->constant_pool);
    SAFE_FREE(bc);
}


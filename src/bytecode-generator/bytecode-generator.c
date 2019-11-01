#include "bytecode-generator.h"

#include "utils.h"

#include <string.h>

struct BYTECODE_GENERATOR
{
    struct UNIT_AST*ast;
    struct BYTECODE*bc;

    struct LOCAL_VARIABLE*locals;
    unsigned locals_len;
    unsigned locals_cap;

    unsigned scope_depth;
};

/* constant-pool functions. */

struct CONSTANT create_constant_from_int(long long int_cnst)
{
    struct CONSTANT cnst;
    cnst.int_cnst = int_cnst;
    cnst.type = CONSTANT_TYPE_INTEGER;
    return cnst;
}

struct CONSTANT create_constant_from_double(double double_cnst)
{
    struct CONSTANT cnst;
    cnst.double_cnst = double_cnst;
    cnst.type = CONSTANT_TYPE_DOUBLE;
    return cnst;
}

struct CONSTANT create_constant_from_fieldred(const char*str_cnst)
{
    struct CONSTANT cnst;
    strncpy(cnst.str_cnst, str_cnst, sizeof(cnst.str_cnst));
    cnst.type = CONSTANT_TYPE_FIELDREF;
    return cnst;
}

struct CONSTANT create_constant_from_functionref(const char*str_cnst)
{
    struct CONSTANT cnst;
    strncpy(cnst.str_cnst, str_cnst, sizeof(cnst.str_cnst));
    cnst.type = CONSTANT_TYPE_FUNCTIONREF;
    return cnst;
}

static int constant_pool_push_back(bytecode_type_t bc, struct CONSTANT cnst)
{
    unsigned i;

    for (i = 0; i < bc->constant_pool_len; i++) {
        if (memcmp((char*) &cnst, (char*) bc->constant_pool + i, sizeof(struct CONSTANT)) == 0) {
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
    unsigned depth;
};

static struct LOCAL_VARIABLE create_local_variable(const char*name, unsigned depth)
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

static enum BYTECODE_GENERATOR_CODES assignment_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct ASSIGNMENT_EXPR_AST*ast);

static enum BYTECODE_GENERATOR_CODES property_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct PROPERTY_AST*ast) {
    struct CONSTANT cnst;
    
    assignment_expr_ast_bytecode_generate(bc_gen, ast->value);

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_INIT_OBJ_PROP);
    cnst = create_constant_from_fieldred(ast->key->ident);
    PUSH_BACK(bc_gen->bc->op_codes, constant_pool_push_back(bc_gen->bc, cnst));

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES object_literal_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct OBJECT_LITERAL_AST*ast)
{
    unsigned i;

    for (i = 0; i < ast->properties_len; i++) {
        property_ast_bytecode_generate(bc_gen, ast->properties[i]);
    }

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_CREATE_OBJ);
    PUSH_BACK(bc_gen->bc->op_codes, ast->properties_len);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES number_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct NUMBER_AST*ast)
{
    struct CONSTANT cnst = create_constant_from_int(ast->number);
    unsigned index = constant_pool_push_back(bc_gen->bc, cnst);

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_CONSTANT);
    PUSH_BACK(bc_gen->bc->op_codes, index);

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES variable_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct VARIABLE_AST*ast)
{
    int idx = local_variable_index(bc_gen, ast->idents[0]->ident);

    if (idx == -1) {

    }    
    
    if (ast->idents_len == 1) {
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_GET_LOCAL);
        PUSH_BACK(bc_gen->bc->op_codes, idx);
    } else {
        unsigned i;
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_GET_HEAP);
        PUSH_BACK(bc_gen->bc->op_codes, idx);
        PUSH_BACK(bc_gen->bc->op_codes, ast->idents_len - 1);
        for (i = 1; i < ast->idents_len; i++) {
            struct CONSTANT cnst = create_constant_from_fieldred(ast->idents[i]->ident);
            PUSH_BACK(bc_gen->bc->op_codes, constant_pool_push_back(bc_gen->bc, cnst));
        }
    }
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES function_call_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct FUNCTION_CALL_AST*ast)
{
    /* TODO */
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES logical_or_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LOGICAL_OR_EXPR_AST*ast);

static enum BYTECODE_GENERATOR_CODES primary_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct PRIMARY_EXPR_AST*ast)
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

static enum BYTECODE_GENERATOR_CODES left_unary_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LEFT_UNARY_EXPR_AST*ast)
{

    primary_expr_ast_bytecode_generate(bc_gen, ast->expr);
    if (ast->op != AST_LEFT_UNARY_OP_PLUS) {
        if (ast->op == AST_LEFT_UNARY_OP_MINUS) {
            PUSH_BACK(bc_gen->bc->op_codes, BC_OP_NEGATE);
        }
    }

    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES multiplicative_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct MULTIPLICATIVE_EXPR_AST*ast)
{
    unsigned i;

    left_unary_expr_ast_bytecode_generate(bc_gen, ast->lues[0]);

    for (i = 1; i < ast->lues_len; i++) {
        left_unary_expr_ast_bytecode_generate(bc_gen, ast->lues[i]);
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

static enum BYTECODE_GENERATOR_CODES relational_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct RELATIONAL_EXPR_AST*ast)
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

static enum BYTECODE_GENERATOR_CODES eq_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct EQ_EXPR_AST*ast)
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

static enum BYTECODE_GENERATOR_CODES logical_and_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LOGICAL_AND_EXPR_AST*ast)
{
    unsigned i;

    eq_expr_ast_bytecode_generate(bc_gen, ast->eq_exprs[0]);
    for (i = 1; i < ast->eq_exprs_len; i++) {
        eq_expr_ast_bytecode_generate(bc_gen, ast->eq_exprs[i]);
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_LOGICAL_AND);
    }

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES logical_or_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct LOGICAL_OR_EXPR_AST*ast)
{
    unsigned i;

    logical_and_expr_ast_bytecode_generate(bc_gen, ast->and_exprs[0]);
    for (i = 1; i < ast->and_exprs_len; i++) {
        logical_and_expr_ast_bytecode_generate(bc_gen, ast->and_exprs[i]);
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_LOGICAL_OR);
    }

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES assignment_expr_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct ASSIGNMENT_EXPR_AST*ast)
{
    switch (ast->type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        object_literal_ast_bytecode_generate(bc_gen, ast->object_literal);
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

static enum BYTECODE_GENERATOR_CODES decl_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct DECL_STMT_AST*ast)
{
    int idx;

    struct LOCAL_VARIABLE lv;
    
    assignment_expr_ast_bytecode_generate(bc_gen, ast->assignment);

    idx = local_variable_index(bc_gen, ast->new_var_name->ident);

    if ((idx != -1) && (bc_gen->locals[idx].depth == bc_gen->scope_depth)) {
        
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
    int idx;
    
    assignment_expr_ast_bytecode_generate(bc_gen, ast->assignment);

    idx = local_variable_index(bc_gen, ast->var_name->idents[0]->ident);

    if (idx == -1) {
        
    }

    if (ast->var_name->idents_len == 1) {
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_SET_LOCAL);
        PUSH_BACK(bc_gen->bc->op_codes, idx);
    } else {
        unsigned i;
        PUSH_BACK(bc_gen->bc->op_codes, BC_OP_SET_HEAP);
        PUSH_BACK(bc_gen->bc->op_codes, idx);
        PUSH_BACK(bc_gen->bc->op_codes, ast->var_name->idents_len - 1);
        for (i = 1; i < ast->var_name->idents_len; i++) {
            struct CONSTANT cnst = create_constant_from_fieldred(ast->var_name->idents[i]->ident);
            PUSH_BACK(bc_gen->bc->op_codes, constant_pool_push_back(bc_gen->bc, cnst));
        }
    }

    return BYTECODE_GENERATOR_OK;    
}

enum BYTECODE_GENERATOR_CODES function_call_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct FUNCTION_CALL_STMT_AST*ast)
{
    /* TODO */
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES if_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct IF_STMT_AST*ast)
{
    /* TODO */
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES while_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct WHILE_STMT_AST*ast)
{
    /* TODO */
    
    return BYTECODE_GENERATOR_OK;
}

static enum BYTECODE_GENERATOR_CODES return_stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct RETURN_STMT_AST*ast)
{
    if (ast->result != NULL) {
        assignment_expr_ast_bytecode_generate(bc_gen, ast->result);
    }

    PUSH_BACK(bc_gen->bc->op_codes, BC_OP_RETURN);

    return BYTECODE_GENERATOR_OK;    
}

static enum BYTECODE_GENERATOR_CODES stmt_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct STMT_AST*ast)
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

static enum BYTECODE_GENERATOR_CODES body_ast_bytecode_generate(bytecode_generator_type_t bc_gen, const struct BODY_AST*ast)
{
    unsigned i;

    bc_gen->scope_depth++;

    for (i = 0; i < ast->stmts_len; i++) {
        stmt_ast_bytecode_generate(bc_gen, ast->stmts[i]);
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
    body_ast_bytecode_generate(bc_gen, f->body);

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
    unsigned i;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<bytecode>\n");

    fprintf(f, "\t<constant_pool>\n");
    for (i = 0; i < bc->constant_pool_len; i++) {
        switch (bc->constant_pool[i].type) {
        case CONSTANT_TYPE_INTEGER:
            fprintf(f, "\t\t<int_cnst idx=\"%u\">%lld</int_cnst>\n", i, bc->constant_pool[i].int_cnst);
            break;
        case CONSTANT_TYPE_DOUBLE:
            fprintf(f, "\t\t<double_cnst idx=\"%u\">%lf</double_cnst>\n", i, bc->constant_pool[i].double_cnst);
            break;
        case CONSTANT_TYPE_FIELDREF:
            fprintf(f, "\t\t<fieldref_cnst idx=\"%u\">%s</fieldref_cnst>\n", i, bc->constant_pool[i].str_cnst);
            break;
        case CONSTANT_TYPE_FUNCTIONREF:
            fprintf(f, "\t\t<functionref_cnst idx=\"%u\">%s</functionref_cnst>\n", i, bc->constant_pool[i].str_cnst);
            break;
        }
    }
    fprintf(f, "\t</constant_pool>\n");

    fprintf(f, "\t<op_codes>\n");
    
    i = 0;
    while (i < bc->op_codes_len) {
        unsigned j, n;
        
        switch (bc->op_codes[i]) {
        case BC_OP_POP:
            fprintf(f, "\t\t<op>POP<op>\n");
            break;

        case BC_OP_CONSTANT:
            i++;
            fprintf(f, "\t\t<op>CONSTANT %u</op>\n", bc->op_codes[i]);
            break;

        case BC_OP_GET_LOCAL:
            i++;
            fprintf(f, "\t\t<op>GET_LOCAL %u</op>\n", bc->op_codes[i]);
            break;
        case BC_OP_SET_LOCAL:
            i++;
            fprintf(f, "\t\t<op>SET_LOCAL %u</op>\n", bc->op_codes[i]);
            break;

        case BC_OP_CREATE_OBJ:
            i++;
            fprintf(f, "\t\t<op>CREATE_OBJ %u</op>\n", bc->op_codes[i]);
            break;
        case BC_OP_INIT_OBJ_PROP:
            i++;
            fprintf(f, "\t\t<op>INIT_OBJ_PROP %u</op>\n", bc->op_codes[i]);            
            break;

        case BC_OP_GET_HEAP:
            i++;
            fprintf(f, "\t\t<op>GET_HEAP %u", bc->op_codes[i]);
            i++;
            n = bc->op_codes[i];
            i++;
            for (j = 0; j < n; j++) {
                fprintf(f, ".%u", bc->op_codes[i]);
                i++;
            }
            i--;
            fprintf(f, "</op>\n");
            break;
        case BC_OP_SET_HEAP:
            i++;            
            fprintf(f, "\t\t<op>SET_HEAP %u", bc->op_codes[i]);
            i++;
            n = bc->op_codes[i];
            i++;
            for (j = 0; j < n; j++) {
                fprintf(f, ".%u", bc->op_codes[i]);
                i++;
            }
            i--;
            fprintf(f, "</op>\n");
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

        case BC_OP_RETURN:
            fprintf(f, "\t\t<op>RETURN</op>\n");
            break;
        }

        i++;
    }
    fprintf(f, "\t</op_codes>\n");    

    fprintf(f, "</bytecode>\n");
}

void bytecode_free(bytecode_type_t bc)
{
    SAFE_FREE(bc->op_codes);
    SAFE_FREE(bc->constant_pool);
    SAFE_FREE(bc);
}

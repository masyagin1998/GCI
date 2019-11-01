#include "ast.h"

#include "utils.h"

#include <string.h>

#define INC_SPACES_NUM()                        \
    do {                                        \
        spaces_num += 2;                        \
    } while(0)

#define DEC_SPACES_NUM()                        \
    do {                                        \
        spaces_num -= 2;                        \
    } while(0)

#define PUT_SPACES()                            \
    do {                                        \
        fprintf(f, "%*s", spaces_num, "");      \
    } while(0)

struct UNIT_AST*create_unit_ast(struct FUNCTION_DECL_AST**functions, unsigned functions_len)
{
    struct UNIT_AST*unit_ast;
    SAFE_MALLOC(unit_ast, 1);
    unit_ast->functions = functions;
    unit_ast->functions_len = functions_len;
    return unit_ast;
}

static void dump_function_decl_ast_to_file_inner(FILE*f, const struct FUNCTION_DECL_AST*ast, unsigned spaces_num);

void dump_unit_ast_to_xml_file(FILE*f, const struct UNIT_AST*ast)
{
    unsigned i;
    unsigned spaces_num = 0;

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");    
    fprintf(f, "<unit>\n");
    INC_SPACES_NUM();
    for (i = 0; i < ast->functions_len; i++) {
        dump_function_decl_ast_to_file_inner(f, ast->functions[i], spaces_num);
    }
    DEC_SPACES_NUM();
    fprintf(f, "</unit>\n");    
}

void unit_ast_free(struct UNIT_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->functions_len; i++) {
        function_decl_ast_free(ast->functions[i]);
    }
    SAFE_FREE(ast->functions);
    SAFE_FREE(ast);
}

struct FUNCTION_DECL_AST*create_function_decl_ast(struct IDENT_AST*function_name,
                                                  struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list,
                                                  struct BODY_AST*body)
{
    struct FUNCTION_DECL_AST*function_ast;
    SAFE_MALLOC(function_ast, 1);
    function_ast->function_name = function_name;
    function_ast->formal_parameters_list = formal_parameters_list;
    function_ast->body = body;
    return function_ast;
}

static void dump_ident_ast_to_file_inner(FILE*f, const struct IDENT_AST*ast, unsigned spaces_num);
static void dump_formal_parameters_list_ast_to_file_inner(FILE*f, const struct FORMAL_PARAMETERS_LIST_AST*ast, unsigned spaces_num);
static void dump_body_ast_to_file_inner(FILE*f, const struct BODY_AST*ast, unsigned spaces_num);

static void dump_function_decl_ast_to_file_inner(FILE*f, const struct FUNCTION_DECL_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<function>\n");

    INC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "<function_name>\n");
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->function_name, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_name>\n");

    dump_formal_parameters_list_ast_to_file_inner(f, ast->formal_parameters_list, spaces_num);
    
    dump_body_ast_to_file_inner(f, ast->body, spaces_num);
    DEC_SPACES_NUM();

    PUT_SPACES(); fprintf(f, "</function>\n");
}

void function_decl_ast_free(struct FUNCTION_DECL_AST*ast)
{
    ident_ast_free(ast->function_name);
    formal_parameters_list_ast_free(ast->formal_parameters_list);
    body_ast_free(ast->body);
    SAFE_FREE(ast);
}

struct FORMAL_PARAMETERS_LIST_AST*create_formal_parameters_list_ast(struct IDENT_AST**params, unsigned params_len)
{
    struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list;
    SAFE_MALLOC(formal_parameters_list, 1);
    formal_parameters_list->params = params;
    formal_parameters_list->params_len = params_len;
    return formal_parameters_list;
}

static void dump_formal_parameters_list_ast_to_file_inner(FILE*f, const struct FORMAL_PARAMETERS_LIST_AST*ast, unsigned spaces_num)
{
    unsigned i;
    
    PUT_SPACES(); fprintf(f, "<formal_parameters_list>\n");
    INC_SPACES_NUM();
    for (i = 0; i < ast->params_len; i++) {
        dump_ident_ast_to_file_inner(f, ast->params[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</formal_parameters_list>\n");
}

void formal_parameters_list_ast_free(struct FORMAL_PARAMETERS_LIST_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->params_len; i++) {
        ident_ast_free(ast->params[i]);
    }
    SAFE_FREE(ast->params);
    SAFE_FREE(ast);
}

struct BODY_AST*create_body_ast(struct STMT_AST**stmts, unsigned stmts_len)
{
    struct BODY_AST*body;
    SAFE_MALLOC(body, 1);
    body->stmts = stmts;
    body->stmts_len = stmts_len;
    return body;
}

static void dump_stmt_ast_to_file_inner(FILE*f, const struct STMT_AST*ast, unsigned spaces_num);

static void dump_body_ast_to_file_inner(FILE*f, const struct BODY_AST*ast, unsigned spaces_num)
{
    unsigned i;
    
    PUT_SPACES(); fprintf(f, "<body>\n");
    INC_SPACES_NUM();
    for (i = 0; i < ast->stmts_len; i++) {
        dump_stmt_ast_to_file_inner(f, ast->stmts[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</body>\n");
}

void body_ast_free(struct BODY_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->stmts_len; i++) {
        stmt_ast_free(ast->stmts[i]);
    }
    SAFE_FREE(ast->stmts);
    SAFE_FREE(ast);
}

struct STMT_AST*create_stmt_ast(void*stmt_ptr, enum AST_STMT_TYPE stmt_type)
{
    struct STMT_AST*stmt;
    SAFE_MALLOC(stmt, 1);
    switch (stmt_type) {
    case AST_STMT_TYPE_DECL:
        stmt->decl_stmt = stmt_ptr;
        break;
    case AST_STMT_TYPE_ASSIGN:
        stmt->assign_stmt = stmt_ptr;
        break;
    case AST_STMT_TYPE_FUNCTION_CALL:
        stmt->function_call_stmt = stmt_ptr;
        break;
    case AST_STMT_TYPE_IF:
        stmt->if_stmt = stmt_ptr;
        break;
    case AST_STMT_TYPE_WHILE:
        stmt->while_stmt = stmt_ptr;
        break;
    case AST_STMT_TYPE_RETURN:
        stmt->return_stmt = stmt_ptr;
        break;
    default:
        fprintf(stderr, "Invalid STMT_AST type: %d\n", stmt_type);
        exit(EXIT_FAILURE);
        break;
    }
    stmt->type = stmt_type;
    return stmt;
}

static void dump_decl_stmt_ast_to_file_inner(FILE*f, const struct DECL_STMT_AST*ast, unsigned spaces_num);
static void dump_assign_stmt_ast_to_file_inner(FILE*f, const struct ASSIGN_STMT_AST*ast, unsigned spaces_num);
static void dump_function_call_stmt_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_STMT_AST*ast, unsigned spaces_num);
static void dump_if_stmt_ast_to_file_inner(FILE*f, const struct IF_STMT_AST*ast, unsigned spaces_num);
static void dump_while_stmt_ast_to_file_inner(FILE*f, const struct WHILE_STMT_AST*ast, unsigned spaces_num);
static void dump_return_stmt_ast_to_file_inner(FILE*f, const struct RETURN_STMT_AST*ast, unsigned spaces_num);

static void dump_stmt_ast_to_file_inner(FILE*f, const struct STMT_AST*ast, unsigned spaces_num)
{
    switch (ast->type) {
    case AST_STMT_TYPE_DECL:
        dump_decl_stmt_ast_to_file_inner(f, ast->decl_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_ASSIGN:
        dump_assign_stmt_ast_to_file_inner(f, ast->assign_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_FUNCTION_CALL:
        dump_function_call_stmt_ast_to_file_inner(f, ast->function_call_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_IF:
        dump_if_stmt_ast_to_file_inner(f, ast->if_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_WHILE:
        dump_while_stmt_ast_to_file_inner(f, ast->while_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_RETURN:
        dump_return_stmt_ast_to_file_inner(f, ast->return_stmt, spaces_num);
        break;
    default:
        fprintf(stderr, "Invalid STMT_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;        
    }
}

void stmt_ast_free(struct STMT_AST*ast)
{
    switch (ast->type) {
    case AST_STMT_TYPE_DECL:
        decl_stmt_ast_free(ast->decl_stmt);
        break;
    case AST_STMT_TYPE_ASSIGN:
        assign_stmt_ast_free(ast->assign_stmt);
        break;
    case AST_STMT_TYPE_FUNCTION_CALL:
        function_call_stmt_ast_free(ast->function_call_stmt);
        break;
    case AST_STMT_TYPE_IF:
        if_stmt_ast_free(ast->if_stmt);
        break;
    case AST_STMT_TYPE_WHILE:
        while_stmt_ast_free(ast->while_stmt);
        break;
    case AST_STMT_TYPE_RETURN:
        return_stmt_ast_free(ast->return_stmt);
        break;
    default:
        fprintf(stderr, "Invalid STMT_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;        
    }
    SAFE_FREE(ast);
}

struct DECL_STMT_AST*create_decl_stmt_ast(struct IDENT_AST*new_var_name,
                                          struct ASSIGNMENT_EXPR_AST*assignment)
{
    struct DECL_STMT_AST*decl_stmt;
    SAFE_MALLOC(decl_stmt, 1);
    decl_stmt->new_var_name = new_var_name;
    decl_stmt->assignment = assignment;
    return decl_stmt;
}

static void dump_assignment_expr_ast_to_file_inner(FILE*f, const struct ASSIGNMENT_EXPR_AST*ast, unsigned spaces_num);

static void dump_decl_stmt_ast_to_file_inner(FILE*f, const struct DECL_STMT_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<decl_stmt>\n");
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->new_var_name, spaces_num);
    PUT_SPACES(); fprintf(f, "<op>EQ</op>\n");
    dump_assignment_expr_ast_to_file_inner(f, ast->assignment, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</decl_stmt>\n");
}

void decl_stmt_ast_free(struct DECL_STMT_AST*ast)
{
    ident_ast_free(ast->new_var_name);
    assignment_expr_ast_free(ast->assignment);
    SAFE_FREE(ast);
}

struct ASSIGN_STMT_AST*create_assign_stmt_ast(struct VARIABLE_AST*var_name,
                                              struct ASSIGNMENT_EXPR_AST*assignment)
{
    struct ASSIGN_STMT_AST*assign_stmt;
    SAFE_MALLOC(assign_stmt, 1);
    assign_stmt->var_name = var_name;
    assign_stmt->assignment = assignment;
    return assign_stmt;
}

static void dump_variable_ast_to_file_inner(FILE*f, const struct VARIABLE_AST*ast, unsigned spaces_num);

static void dump_assign_stmt_ast_to_file_inner(FILE*f, const struct ASSIGN_STMT_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<assign_stmt>\n");
    INC_SPACES_NUM();
    dump_variable_ast_to_file_inner(f, ast->var_name, spaces_num);
    PUT_SPACES(); fprintf(f, "<op>EQ</op>\n");    
    dump_assignment_expr_ast_to_file_inner(f, ast->assignment, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</assign_stmt>\n");
}

void assign_stmt_ast_free(struct ASSIGN_STMT_AST*ast)
{
    variable_ast_free(ast->var_name);
    assignment_expr_ast_free(ast->assignment);
    SAFE_FREE(ast);
}

struct FUNCTION_CALL_STMT_AST*create_function_call_stmt_ast(struct FUNCTION_CALL_AST*function_call)
{
    struct FUNCTION_CALL_STMT_AST*function_call_stmt;
    SAFE_MALLOC(function_call_stmt, 1);
    function_call_stmt->function_call = function_call;
    return function_call_stmt;
}

static void dump_function_call_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_AST*ast, unsigned spaces_num);

static void dump_function_call_stmt_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_STMT_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<function_call_stmt>\n");
    INC_SPACES_NUM();
    dump_function_call_ast_to_file_inner(f, ast->function_call, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_call_stmt>\n");
}

void function_call_stmt_ast_free(struct FUNCTION_CALL_STMT_AST*ast)
{
    function_call_ast_free(ast->function_call);
    SAFE_FREE(ast);
}

struct IF_STMT_AST*create_if_stmt_ast(struct LOGICAL_OR_EXPR_AST*condition,
                                      struct BODY_AST*if_body,
                                      struct BODY_AST*else_body)
{
    struct IF_STMT_AST*if_stmt;
    SAFE_MALLOC(if_stmt, 1);
    if_stmt->condition = condition;
    if_stmt->if_body = if_body;
    if_stmt->else_body = else_body;
    return if_stmt;
}

static void dump_if_stmt_ast_to_file_inner(FILE*f, const struct IF_STMT_AST*ast, unsigned spaces_num)
{
    (void)f;
    (void)ast;
    (void)spaces_num;
}

void if_stmt_ast_free(struct IF_STMT_AST*ast)
{
    logical_or_expr_ast_free(ast->condition);
    body_ast_free(ast->if_body);
    if (ast->else_body != NULL) {
        body_ast_free(ast->else_body);
    }
    SAFE_FREE(ast);
}

struct WHILE_STMT_AST*create_while_stmt_ast(struct LOGICAL_OR_EXPR_AST*condition,
                                            struct BODY_AST*body)
{
    struct WHILE_STMT_AST*while_stmt;
    SAFE_MALLOC(while_stmt, 1);
    while_stmt->condition = condition;
    while_stmt->body = body;
    return while_stmt;
}

static void dump_logical_or_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_OR_EXPR_AST*ast, unsigned spaces_num);

static void dump_while_stmt_ast_to_file_inner(FILE*f, const struct WHILE_STMT_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<while_stmt>\n");
    INC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "<condition>\n");
    INC_SPACES_NUM();
    dump_logical_or_expr_ast_to_file_inner(f, ast->condition, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</condition>\n");

    dump_body_ast_to_file_inner(f, ast->body, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</while_stmt>\n");
}

void while_stmt_ast_free(struct WHILE_STMT_AST*ast)
{
    logical_or_expr_ast_free(ast->condition);
    body_ast_free(ast->body);
    SAFE_FREE(ast);
}

struct RETURN_STMT_AST*create_return_stmt_ast(struct ASSIGNMENT_EXPR_AST*result)
{
    struct RETURN_STMT_AST*return_stmt;
    SAFE_MALLOC(return_stmt, 1);
    return_stmt->result = result;
    return return_stmt;
}

static void dump_return_stmt_ast_to_file_inner(FILE*f, const struct RETURN_STMT_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<return_stmt>\n");
    if (ast->result != NULL) {
        INC_SPACES_NUM();
        dump_assignment_expr_ast_to_file_inner(f, ast->result, spaces_num);
        DEC_SPACES_NUM();
    }
    PUT_SPACES(); fprintf(f, "</return_stmt>\n");
}

void return_stmt_ast_free(struct RETURN_STMT_AST*ast)
{
    if (ast->result != NULL) {
        assignment_expr_ast_free(ast->result);
    }
    SAFE_FREE(ast);
}

struct VARIABLE_AST*create_variable_ast(struct IDENT_AST**idents, unsigned idents_len)
{
    struct VARIABLE_AST*variable;
    SAFE_MALLOC(variable, 1);
    variable->idents = idents;
    variable->idents_len = idents_len;
    return variable;
}

static void dump_variable_ast_to_file_inner(FILE*f, const struct VARIABLE_AST*ast, unsigned spaces_num)
{
    unsigned i;
    
    PUT_SPACES(); fprintf(f, "<variable>\n");
    INC_SPACES_NUM();
    for (i = 0; i < ast->idents_len; i++) {
        dump_ident_ast_to_file_inner(f, ast->idents[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</variable>\n");
}

void variable_ast_free(struct VARIABLE_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->idents_len; i++) {
        ident_ast_free(ast->idents[i]);
    }
    SAFE_FREE(ast->idents);
    SAFE_FREE(ast);
}

struct ASSIGNMENT_EXPR_AST*create_assignment_expr_ast(void*assignment_ptr, enum AST_ASSIGNMENT_EXPR_TYPE type)
{
    struct ASSIGNMENT_EXPR_AST*assignment_expr;
    SAFE_MALLOC(assignment_expr, 1);
    switch (type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        assignment_expr->object_literal = assignment_ptr;
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR:
        assignment_expr->logical_or_expr = assignment_ptr;
        break;
    default:
        fprintf(stderr, "Invalid ASSIGNMENT_EXPR_AST type: %d\n", type);
        exit(EXIT_FAILURE);
        break;
    }
    assignment_expr->type = type;
    return assignment_expr;
}

static void dump_object_literal_ast_to_file_inner(FILE*f, const struct OBJECT_LITERAL_AST*ast, unsigned spaces_num);

static void dump_assignment_expr_ast_to_file_inner(FILE*f, const struct ASSIGNMENT_EXPR_AST*ast, unsigned spaces_num)
{
    switch (ast->type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        dump_object_literal_ast_to_file_inner(f, ast->object_literal, spaces_num);
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR:
        dump_logical_or_expr_ast_to_file_inner(f, ast->logical_or_expr, spaces_num);
        break;
    default:
        fprintf(stderr, "Invalid ASSIGNMENT_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);        
        break;
    }
}

void assignment_expr_ast_free(struct ASSIGNMENT_EXPR_AST*ast)
{
    switch (ast->type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        object_literal_ast_free(ast->object_literal);
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR:
        logical_or_expr_ast_free(ast->logical_or_expr);
        break;
    default:
        fprintf(stderr, "Invalid ASSIGNMENT_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);        
        break;
    }
    SAFE_FREE(ast);
}

struct OBJECT_LITERAL_AST*create_object_literal_ast(struct PROPERTY_AST**properties, unsigned properties_len)
{
    struct OBJECT_LITERAL_AST*object_literal;
    SAFE_MALLOC(object_literal, 1);
    object_literal->properties = properties;
    object_literal->properties_len = properties_len;
    return object_literal;
}

static void dump_property_ast_to_file_inner(FILE*f, const struct PROPERTY_AST*ast, unsigned spaces_num);

static void dump_object_literal_ast_to_file_inner(FILE*f, const struct OBJECT_LITERAL_AST*ast, unsigned spaces_num)
{
    unsigned i;
    
    PUT_SPACES(); fprintf(f, "<object_literal>\n");
    INC_SPACES_NUM();
    for (i = 0; i < ast->properties_len; i++) {
        dump_property_ast_to_file_inner(f, ast->properties[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</object_literal>\n");
}

void object_literal_ast_free(struct OBJECT_LITERAL_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->properties_len; i++) {
        property_ast_free(ast->properties[i]);
    }
    SAFE_FREE(ast->properties);
    SAFE_FREE(ast);
}

struct PROPERTY_AST*create_property_ast(struct IDENT_AST*key, struct ASSIGNMENT_EXPR_AST*value)
{
    struct PROPERTY_AST*property;
    SAFE_MALLOC(property, 1);
    property->key = key;
    property->value = value;
    return property;
}

static void dump_property_ast_to_file_inner(FILE*f, const struct PROPERTY_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<property>\n");
    INC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "<key>\n");
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->key, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</key>\n");

    PUT_SPACES(); fprintf(f, "<value>\n");
    INC_SPACES_NUM();
    dump_assignment_expr_ast_to_file_inner(f, ast->value, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</value>\n");
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</property>\n");
}

void property_ast_free(struct PROPERTY_AST*ast)
{
    ident_ast_free(ast->key);
    assignment_expr_ast_free(ast->value);
    SAFE_FREE(ast);
}

struct LOGICAL_OR_EXPR_AST*create_logical_or_expr_ast(struct LOGICAL_AND_EXPR_AST**and_exprs, unsigned and_exprs_len)
{
    struct LOGICAL_OR_EXPR_AST*logical_or_expr;
    SAFE_MALLOC(logical_or_expr, 1);
    logical_or_expr->and_exprs = and_exprs;
    logical_or_expr->and_exprs_len = and_exprs_len;
    return logical_or_expr;
}

static void dump_logical_and_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_AND_EXPR_AST*ast, unsigned spaces_num);

static void dump_logical_or_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_OR_EXPR_AST*ast, unsigned spaces_num)
{
    if (ast->and_exprs_len == 1) {
        dump_logical_and_expr_ast_to_file_inner(f, ast->and_exprs[0], spaces_num);
    } else {
        unsigned i;
        
        PUT_SPACES(); fprintf(f, "<logical_or_expr>\n");
        INC_SPACES_NUM();
        dump_logical_and_expr_ast_to_file_inner(f, ast->and_exprs[0], spaces_num);
        for (i = 1; i < ast->and_exprs_len; i++) {
            PUT_SPACES(); fprintf(f, "<op>OR</op>\n");
            dump_logical_and_expr_ast_to_file_inner(f, ast->and_exprs[i], spaces_num);
        }
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</logical_or_expr>\n");        
    }
}

void logical_or_expr_ast_free(struct LOGICAL_OR_EXPR_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->and_exprs_len; i++) {
        logical_and_expr_ast_free(ast->and_exprs[i]);
    }
    SAFE_FREE(ast->and_exprs);
    SAFE_FREE(ast);
}

struct LOGICAL_AND_EXPR_AST*create_logical_and_expr_ast(struct EQ_EXPR_AST**eq_exprs, unsigned eq_exprs_len)
{
    struct LOGICAL_AND_EXPR_AST*logical_and_expr;
    SAFE_MALLOC(logical_and_expr, 1);
    logical_and_expr->eq_exprs = eq_exprs;
    logical_and_expr->eq_exprs_len = eq_exprs_len;
    return logical_and_expr;
}

static void dump_eq_expr_ast_to_file_inner(FILE*f, const struct EQ_EXPR_AST*ast, unsigned spaces_num);

static void dump_logical_and_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_AND_EXPR_AST*ast, unsigned spaces_num)
{
    if (ast->eq_exprs_len == 1) {
        dump_eq_expr_ast_to_file_inner(f, ast->eq_exprs[0], spaces_num);
    } else {
        unsigned i;
        
        PUT_SPACES(); fprintf(f, "<logical_and_expr>\n");
        INC_SPACES_NUM();
        dump_eq_expr_ast_to_file_inner(f, ast->eq_exprs[0], spaces_num);
        for (i = 1; i < ast->eq_exprs_len; i++) {
            PUT_SPACES(); fprintf(f, "<op>AND</op>\n");
            dump_eq_expr_ast_to_file_inner(f, ast->eq_exprs[i], spaces_num);
        }
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</logical_and_expr>\n");        
    }
}

void logical_and_expr_ast_free(struct LOGICAL_AND_EXPR_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->eq_exprs_len; i++) {
        eq_expr_ast_free(ast->eq_exprs[i]);
    }
    SAFE_FREE(ast->eq_exprs);
    SAFE_FREE(ast);
}

struct EQ_EXPR_AST*create_eq_expr_ast(struct RELATIONAL_EXPR_AST*left,
                                      enum AST_EQ_OP eq_op,
                                      struct RELATIONAL_EXPR_AST*right)
{
    struct EQ_EXPR_AST*eq_expr;
    SAFE_MALLOC(eq_expr, 1);
    eq_expr->left = left;
    eq_expr->eq_op = eq_op;
    eq_expr->right = right;
    return eq_expr;
}

static void dump_relational_expr_ast_to_file_inner(FILE*f, const struct RELATIONAL_EXPR_AST*ast, unsigned spaces_num);

static void dump_eq_expr_ast_to_file_inner(FILE*f, const struct EQ_EXPR_AST*ast, unsigned spaces_num)
{
    if (ast->right == NULL) {
        dump_relational_expr_ast_to_file_inner(f, ast->left, spaces_num);
    } else {
        PUT_SPACES(); fprintf(f, "<eq_expr>\n");
        INC_SPACES_NUM();
        dump_relational_expr_ast_to_file_inner(f, ast->left, spaces_num);
        if (ast->eq_op == AST_EQ_OP_EQEQ) {
            PUT_SPACES(); fprintf(f, "<op>EQEQ</op>\n");
        } else if (ast->eq_op == AST_EQ_OP_NEQ) {
            PUT_SPACES(); fprintf(f, "<op>NEQ</op>\n");
        }
        dump_relational_expr_ast_to_file_inner(f, ast->right, spaces_num);
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</eq_expr>\n");
    }
}

void eq_expr_ast_free(struct EQ_EXPR_AST*ast)
{
    relational_expr_ast_free(ast->left);
    if (ast->right != NULL) {
        relational_expr_ast_free(ast->right);
    }
    SAFE_FREE(ast);
}

struct RELATIONAL_EXPR_AST*create_relational_expr_ast(struct ADDITIVE_EXPR_AST*left,
                                                      enum AST_REL_OP rel_op,
                                                      struct ADDITIVE_EXPR_AST*right)
{
    struct RELATIONAL_EXPR_AST*relational_expr;
    SAFE_MALLOC(relational_expr, 1);
    relational_expr->left = left;
    relational_expr->rel_op = rel_op;
    relational_expr->right = right;
    return relational_expr;
}

static void dump_additive_expr_ast_to_file_inner(FILE*f, const struct ADDITIVE_EXPR_AST*ast, unsigned spaces_num);

static void dump_relational_expr_ast_to_file_inner(FILE*f, const struct RELATIONAL_EXPR_AST*ast, unsigned spaces_num)
{
    if (ast->right == NULL) {
        dump_additive_expr_ast_to_file_inner(f, ast->left, spaces_num);
    } else {
        PUT_SPACES(); fprintf(f, "<relational_expr>\n");
        INC_SPACES_NUM();
        dump_additive_expr_ast_to_file_inner(f, ast->left, spaces_num);
        if (ast->rel_op == AST_REL_OP_LT) {
            PUT_SPACES(); fprintf(f, "<op>LT</op>\n");
        } else if (ast->rel_op == AST_REL_OP_GT) {
            PUT_SPACES(); fprintf(f, "<op>GT</op>\n");
        } else if (ast->rel_op == AST_REL_OP_LE) {
            PUT_SPACES(); fprintf(f, "<op>LE</op>\n");
        } else if (ast->rel_op == AST_REL_OP_GE) {
            PUT_SPACES(); fprintf(f, "<op>GE</op>\n");
        }
        dump_additive_expr_ast_to_file_inner(f, ast->right, spaces_num);
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</relational_expr>\n");
    }
}

void relational_expr_ast_free(struct RELATIONAL_EXPR_AST*ast)
{
    additive_expr_ast_free(ast->left);
    if (ast->right != NULL) {
        additive_expr_ast_free(ast->right);
    }
    SAFE_FREE(ast);
}

struct ADDITIVE_EXPR_AST*create_additive_expr_ast(struct MULTIPLICATIVE_EXPR_AST**muls,
                                                  enum AST_ADDITIVE_OP*ops,
                                                  unsigned muls_len)
{
    struct ADDITIVE_EXPR_AST*additive_expr;
    SAFE_MALLOC(additive_expr, 1);
    additive_expr->muls = muls;
    additive_expr->ops = ops;
    additive_expr->muls_len = muls_len;
    return additive_expr;
}

void additive_expr_ast_free(struct ADDITIVE_EXPR_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->muls_len; i++) {
        multiplicative_expr_ast_free(ast->muls[i]);
    }
    SAFE_FREE(ast->muls);
    SAFE_FREE(ast->ops);
    SAFE_FREE(ast);
}

static void dump_multiplicative_expr_ast_to_file_inner(FILE*f, const struct MULTIPLICATIVE_EXPR_AST*ast, unsigned spaces_num);

static void dump_additive_expr_ast_to_file_inner(FILE*f, const struct ADDITIVE_EXPR_AST*ast, unsigned spaces_num)
{
    if (ast->muls_len == 1) {
        dump_multiplicative_expr_ast_to_file_inner(f, ast->muls[0], spaces_num);
    } else {
        unsigned i;
        
        PUT_SPACES(); fprintf(f, "<additive_expr>\n");
        INC_SPACES_NUM();
        dump_multiplicative_expr_ast_to_file_inner(f, ast->muls[0], spaces_num);
        for (i = 1; i < ast->muls_len; i++) {
            if (ast->ops[i - 1] == AST_ADDITIVE_OP_PLUS) {
                PUT_SPACES(); fprintf(f, "<op>PLUS</op>\n");
            } else if (ast->ops[i - 1] == AST_ADDITIVE_OP_MINUS) {
                PUT_SPACES(); fprintf(f, "<op>MINUS</op>\n");
            }
            dump_multiplicative_expr_ast_to_file_inner(f, ast->muls[i], spaces_num);
        }
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</additive_expr>\n");        
    }
}

struct MULTIPLICATIVE_EXPR_AST*create_multiplicative_expr_ast(struct LEFT_UNARY_EXPR_AST**lues,
                                                              enum AST_MULTIPLICATIVE_OP*ops,
                                                              unsigned lues_len)
{
    struct MULTIPLICATIVE_EXPR_AST*mul_expr;
    SAFE_MALLOC(mul_expr, 1);
    mul_expr->lues = lues;
    mul_expr->ops = ops;
    mul_expr->lues_len = lues_len;
    return mul_expr;
}

static void dump_left_unary_expr_ast_to_file_inner(FILE*f, const struct LEFT_UNARY_EXPR_AST*ast, unsigned spaces_num);

static void dump_multiplicative_expr_ast_to_file_inner(FILE*f, const struct MULTIPLICATIVE_EXPR_AST*ast, unsigned spaces_num)
{
    if (ast->lues_len == 1) {
        dump_left_unary_expr_ast_to_file_inner(f, ast->lues[0], spaces_num);
    } else {
        unsigned i;
        
        PUT_SPACES(); fprintf(f, "<multiplicative_expr>\n");
        INC_SPACES_NUM();
        dump_left_unary_expr_ast_to_file_inner(f, ast->lues[0], spaces_num);
        for (i = 1; i < ast->lues_len; i++) {
            if (ast->ops[i - 1] == AST_MULTIPLICATIVE_OP_MUL) {
                PUT_SPACES(); fprintf(f, "<op>MUL</op>\n");
            } else if (ast->ops[i - 1] == AST_MULTIPLICATIVE_OP_DIV) {
                PUT_SPACES(); fprintf(f, "<op>DIV</op>\n");
            } else if (ast->ops[i - 1] == AST_MULTIPLICATIVE_OP_MOD) {
                PUT_SPACES(); fprintf(f, "<op>MOD</op>\n");
            }
            dump_left_unary_expr_ast_to_file_inner(f, ast->lues[i], spaces_num);
        }
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</multiplicative_expr>\n");
    }
}

void multiplicative_expr_ast_free(struct MULTIPLICATIVE_EXPR_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->lues_len; i++) {
        left_unary_expr_ast_free(ast->lues[i]);
    }
    SAFE_FREE(ast->lues);
    SAFE_FREE(ast->ops);
    SAFE_FREE(ast);
}

struct LEFT_UNARY_EXPR_AST*create_left_unary_expr_ast(enum AST_LEFT_UNARY_OP op, struct PRIMARY_EXPR_AST*expr)
{
    struct LEFT_UNARY_EXPR_AST*lue;
    SAFE_MALLOC(lue, 1);
    lue->op = op;
    lue->expr = expr;
    return lue;
}

static void dump_primary_expr_ast_to_file_inner(FILE*f, const struct PRIMARY_EXPR_AST*ast, unsigned spaces_num);

static void dump_left_unary_expr_ast_to_file_inner(FILE*f, const struct LEFT_UNARY_EXPR_AST*ast, unsigned spaces_num)
{
    if (ast->op != AST_LEFT_UNARY_OP_PLUS) {
        PUT_SPACES(); fprintf(f, "<left_unary_expr>\n");
        INC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "<op>UNARY_MINUS</op>\n");
        dump_primary_expr_ast_to_file_inner(f, ast->expr, spaces_num);
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</left_unary_expr>\n");
    } else {
        dump_primary_expr_ast_to_file_inner(f, ast->expr, spaces_num);
    }
}

void left_unary_expr_ast_free(struct LEFT_UNARY_EXPR_AST*ast)
{
    primary_expr_ast_free(ast->expr);
    SAFE_FREE(ast);
}

struct PRIMARY_EXPR_AST*create_primary_expr_ast(void*primary_expr_ptr, enum AST_PRIMARY_EXPR_TYPE type)
{
    struct PRIMARY_EXPR_AST*primary_expr;
    SAFE_MALLOC(primary_expr, 1);
    switch (type) {
    case AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL:
        primary_expr->function_call = primary_expr_ptr;
        break;
    case AST_PRIMARY_EXPR_TYPE_VARIABLE:
        primary_expr->var_name = primary_expr_ptr;
        break;
    case AST_PRIMARY_EXPR_TYPE_NUMBER:
        primary_expr->number = primary_expr_ptr;
        break;
    case AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR:
        primary_expr->logical_or_expr = primary_expr_ptr;
        break;
    default:
        fprintf(stderr, "Invalid PRIMARY_EXPR_AST type: %d\n", type);
        exit(EXIT_FAILURE);
        break;
    }
    primary_expr->type = type;
    return primary_expr;
}

static void dump_number_ast_to_file_inner(FILE*f, const struct NUMBER_AST*ast, unsigned spaces_num);

static void dump_primary_expr_ast_to_file_inner(FILE*f, const struct PRIMARY_EXPR_AST*ast, unsigned spaces_num)
{
    switch (ast->type) {
    case AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL:
        dump_function_call_ast_to_file_inner(f, ast->function_call, spaces_num);
        break;
    case AST_PRIMARY_EXPR_TYPE_VARIABLE:
        dump_variable_ast_to_file_inner(f, ast->var_name, spaces_num);
        break;
    case AST_PRIMARY_EXPR_TYPE_NUMBER:
        dump_number_ast_to_file_inner(f, ast->number, spaces_num);
        break;
    case AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR:
        dump_logical_or_expr_ast_to_file_inner(f, ast->logical_or_expr, spaces_num);
        break;
    default:
        fprintf(stderr, "Invalid PRIMARY_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);                
        break;
    }
}

void primary_expr_ast_free(struct PRIMARY_EXPR_AST*ast)
{
    switch (ast->type) {
    case AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL:
        function_call_ast_free(ast->function_call);
        break;
    case AST_PRIMARY_EXPR_TYPE_VARIABLE:
        variable_ast_free(ast->var_name);
        break;
    case AST_PRIMARY_EXPR_TYPE_NUMBER:
        number_ast_free(ast->number);
        break;
    case AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR:
        logical_or_expr_ast_free(ast->logical_or_expr);
        break;
    default:
        fprintf(stderr, "Invalid PRIMARY_EXPR_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);                
        break;
    }
    SAFE_FREE(ast);
}

struct FUNCTION_CALL_AST*create_function_call_ast(struct IDENT_AST*function_name, struct ARGS_LIST_AST*args_list)
{
    struct FUNCTION_CALL_AST*function_call;
    SAFE_MALLOC(function_call, 1);
    function_call->function_name = function_name;
    function_call->args_list = args_list;
    return function_call;
}

static void dump_args_list_ast_to_file_inner(FILE*f, const struct ARGS_LIST_AST*ast, unsigned spaces_num);

static void dump_function_call_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<function_call>\n");
    INC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "<function_name>\n");
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->function_name, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_name>\n");
    
    dump_args_list_ast_to_file_inner(f, ast->args_list, spaces_num);
    
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_call>\n");
}

void function_call_ast_free(struct FUNCTION_CALL_AST*ast)
{
    ident_ast_free(ast->function_name);
    args_list_ast_free(ast->args_list);
    SAFE_FREE(ast);
}

struct ARGS_LIST_AST*create_args_list_ast(struct ASSIGNMENT_EXPR_AST**assignment_exprs, unsigned assignment_exprs_len)
{
    struct ARGS_LIST_AST*args_list;
    SAFE_MALLOC(args_list, 1);
    args_list->assignment_exprs = assignment_exprs;
    args_list->assignment_exprs_len = assignment_exprs_len;
    return args_list;
}

static void dump_args_list_ast_to_file_inner(FILE*f, const struct ARGS_LIST_AST*ast, unsigned spaces_num)
{
    unsigned i;
    
    PUT_SPACES(); fprintf(f, "<args_list>\n");
    INC_SPACES_NUM();
    for (i = 0; i < ast->assignment_exprs_len; i++) {
        dump_assignment_expr_ast_to_file_inner(f, ast->assignment_exprs[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</args_list>\n");
}

void args_list_ast_free(struct ARGS_LIST_AST*ast)
{
    unsigned i;
    for (i = 0; i < ast->assignment_exprs_len; i++) {
        assignment_expr_ast_free(ast->assignment_exprs[i]);
    }
    SAFE_FREE(ast->assignment_exprs);
    SAFE_FREE(ast);
}

struct IDENT_AST*create_ident_ast(const char*ident)
{
    struct IDENT_AST*ident_ast;
    SAFE_MALLOC(ident_ast, 1);
    strncpy(ident_ast->ident, ident, sizeof(ident_ast->ident));
    return ident_ast;
}

static void dump_ident_ast_to_file_inner(FILE*f, const struct IDENT_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<ident>%s</ident>\n", ast->ident);
}

void ident_ast_free(struct IDENT_AST*ast)
{
    SAFE_FREE(ast);
}

struct NUMBER_AST*create_number_ast(long long number)
{
    struct NUMBER_AST*number_ast;
    SAFE_MALLOC(number_ast, 1);
    number_ast->number = number;
    return number_ast;
}

static void dump_number_ast_to_file_inner(FILE*f, const struct NUMBER_AST*ast, unsigned spaces_num)
{
    PUT_SPACES(); fprintf(f, "<number>%lld</number>\n", ast->number);
}

void number_ast_free(struct NUMBER_AST*ast)
{
    SAFE_FREE(ast);
}

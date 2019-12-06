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

#define PUT_SPACES()                                \
    do {                                            \
        fprintf(f, "%*s", (int) spaces_num, "");    \
    } while(0)

struct UNIT_AST*create_unit_ast(struct FUNCTION_DECL_AST**functions, size_t functions_len,
                                size_t line, size_t pos)
{
    struct UNIT_AST*unit_ast;
    SAFE_MALLOC(unit_ast, 1);
    unit_ast->functions = functions;
    unit_ast->functions_len = functions_len;
    
    unit_ast->line = line;
    unit_ast->pos = pos;
    
    return unit_ast;
}

static void dump_function_decl_ast_to_file_inner(FILE*f, const struct FUNCTION_DECL_AST*ast, size_t spaces_num);

static void dump_unit_ast_to_xml_file_inner(FILE*f, const struct UNIT_AST*ast, size_t spaces_num)
{
    size_t i;

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");    
    fprintf(f, "<unit>\n");
    INC_SPACES_NUM();
    for (i = 0; i < ast->functions_len; i++) {
        dump_function_decl_ast_to_file_inner(f, ast->functions[i], spaces_num);
    }
    DEC_SPACES_NUM();
    fprintf(f, "</unit>\n");    
}

__inline__ void dump_unit_ast_to_xml_file(FILE*f, const struct UNIT_AST*ast)
{
    return dump_unit_ast_to_xml_file_inner(f, ast, 0);
}

void unit_ast_free(struct UNIT_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->functions_len; i++) {
        function_decl_ast_free(ast->functions[i]);
    }
    SAFE_FREE(ast->functions);
    SAFE_FREE(ast);
}

struct FUNCTION_DECL_AST*create_function_decl_ast(struct IDENT_AST*function_name,
                                                  struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list,
                                                  struct BODY_AST*body,
                                                  size_t line, size_t pos)
{
    struct FUNCTION_DECL_AST*function_ast;
    SAFE_MALLOC(function_ast, 1);
    function_ast->function_name = function_name;
    function_ast->formal_parameters_list = formal_parameters_list;
    function_ast->body = body;

    function_ast->line = line;
    function_ast->pos = pos;
    
    return function_ast;
}

static void dump_ident_ast_to_file_inner(FILE*f, const struct IDENT_AST*ast, size_t spaces_num);
static void dump_formal_parameters_list_ast_to_file_inner(FILE*f, const struct FORMAL_PARAMETERS_LIST_AST*ast, size_t spaces_num);
static void dump_body_ast_to_file_inner(FILE*f, const struct BODY_AST*ast, size_t spaces_num);

static void dump_function_decl_ast_to_file_inner(FILE*f, const struct FUNCTION_DECL_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<function line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);

    INC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "<function_name line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->function_name, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_name>\n");
    if (ast->formal_parameters_list != NULL) {
        dump_formal_parameters_list_ast_to_file_inner(f, ast->formal_parameters_list, spaces_num);
    }
    dump_body_ast_to_file_inner(f, ast->body, spaces_num);
    DEC_SPACES_NUM();

    PUT_SPACES(); fprintf(f, "</function>\n");
}

__inline__ void dump_function_decl_ast_to_file(FILE*f, const struct FUNCTION_DECL_AST*ast)
{
    return dump_function_decl_ast_to_file_inner(f, ast, 0);
}

void function_decl_ast_free(struct FUNCTION_DECL_AST*ast)
{
    ident_ast_free(ast->function_name);
    if (ast->formal_parameters_list != NULL) {
        formal_parameters_list_ast_free(ast->formal_parameters_list);
    }
    body_ast_free(ast->body);
    SAFE_FREE(ast);
}

struct FORMAL_PARAMETERS_LIST_AST*create_formal_parameters_list_ast(struct IDENT_AST**params, size_t params_len,
                                                                    size_t line, size_t pos)
{
    struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list;
    SAFE_MALLOC(formal_parameters_list, 1);
    formal_parameters_list->params = params;
    formal_parameters_list->params_len = params_len;

    formal_parameters_list->line = line;
    formal_parameters_list->pos = pos;
    
    return formal_parameters_list;
}

static void dump_formal_parameters_list_ast_to_file_inner(FILE*f, const struct FORMAL_PARAMETERS_LIST_AST*ast, size_t spaces_num)
{
    size_t i;
    
    PUT_SPACES(); fprintf(f, "<formal_parameters_list line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    for (i = 0; i < ast->params_len; i++) {
        dump_ident_ast_to_file_inner(f, ast->params[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</formal_parameters_list>\n");
}

__inline__ void dump_formal_parameters_list_ast_to_file(FILE*f, const struct FORMAL_PARAMETERS_LIST_AST*ast)
{
    dump_formal_parameters_list_ast_to_file_inner(f, ast, 0);
}

void formal_parameters_list_ast_free(struct FORMAL_PARAMETERS_LIST_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->params_len; i++) {
        ident_ast_free(ast->params[i]);
    }
    SAFE_FREE(ast->params);
    SAFE_FREE(ast);
}

struct BODY_AST*create_body_ast(struct STMT_AST**stmts, size_t stmts_len,
                                size_t line, size_t pos)
{
    struct BODY_AST*body;
    SAFE_MALLOC(body, 1);
    body->stmts = stmts;
    body->stmts_len = stmts_len;

    body->line = line;
    body->pos = pos;
    
    return body;
}

static void dump_stmt_ast_to_file_inner(FILE*f, const struct STMT_AST*ast, size_t spaces_num);

static void dump_body_ast_to_file_inner(FILE*f, const struct BODY_AST*ast, size_t spaces_num)
{
    size_t i;
    
    PUT_SPACES(); fprintf(f, "<body line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    for (i = 0; i < ast->stmts_len; i++) {
        dump_stmt_ast_to_file_inner(f, ast->stmts[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</body>\n");
}

__inline__ void dump_body_ast_to_file(FILE*f, const struct BODY_AST*ast)
{
    dump_body_ast_to_file_inner(f, ast, 0);
}

void body_ast_free(struct BODY_AST*ast)
{
    size_t i;
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
    case AST_STMT_TYPE_BREAK:
        stmt->break_stmt = stmt_ptr;
        break;   
    case AST_STMT_TYPE_CONTINUE:
        stmt->continue_stmt = stmt_ptr;
        break;
    case AST_STMT_TYPE_APPEND:
        stmt->delete_stmt = stmt_ptr;
        break;   
    case AST_STMT_TYPE_DELETE:
        stmt->append_stmt = stmt_ptr;
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

static void dump_decl_stmt_ast_to_file_inner(FILE*f, const struct DECL_STMT_AST*ast, size_t spaces_num);
static void dump_assign_stmt_ast_to_file_inner(FILE*f, const struct ASSIGN_STMT_AST*ast, size_t spaces_num);
static void dump_function_call_stmt_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_STMT_AST*ast, size_t spaces_num);
static void dump_if_stmt_ast_to_file_inner(FILE*f, const struct IF_STMT_AST*ast, size_t spaces_num);
static void dump_while_stmt_ast_to_file_inner(FILE*f, const struct WHILE_STMT_AST*ast, size_t spaces_num);
static void dump_break_stmt_ast_to_file_inner(FILE*f, const struct BREAK_STMT_AST*ast, size_t spaces_num);
static void dump_continue_stmt_ast_to_file_inner(FILE*f, const struct CONTINUE_STMT_AST*ast, size_t spaces_num);
static void dump_append_stmt_ast_to_file_inner(FILE*f, const struct APPEND_STMT_AST*ast, size_t spaces_num);
static void dump_delete_stmt_ast_to_file_inner(FILE*f, const struct DELETE_STMT_AST*ast, size_t spaces_num);
static void dump_return_stmt_ast_to_file_inner(FILE*f, const struct RETURN_STMT_AST*ast, size_t spaces_num);

static void dump_stmt_ast_to_file_inner(FILE*f, const struct STMT_AST*ast, size_t spaces_num)
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
    case AST_STMT_TYPE_BREAK:
        dump_break_stmt_ast_to_file_inner(f, ast->break_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_APPEND:
        dump_append_stmt_ast_to_file_inner(f, ast->append_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_CONTINUE:
        dump_continue_stmt_ast_to_file_inner(f, ast->continue_stmt, spaces_num);
        break;
    case AST_STMT_TYPE_DELETE:
        dump_delete_stmt_ast_to_file_inner(f, ast->delete_stmt, spaces_num);
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

__inline__ void dump_stmt_ast_to_file(FILE*f, const struct STMT_AST*ast)
{
    dump_stmt_ast_to_file_inner(f, ast, 0);
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
    case AST_STMT_TYPE_BREAK:
        break_stmt_ast_free(ast->break_stmt);
        break;
    case AST_STMT_TYPE_CONTINUE:
        continue_stmt_ast_free(ast->continue_stmt);
        break;
    case AST_STMT_TYPE_APPEND:
        append_stmt_ast_free(ast->append_stmt);
        break;
    case AST_STMT_TYPE_DELETE:
        delete_stmt_ast_free(ast->delete_stmt);
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
                                          struct ASSIGNMENT_EXPR_AST*assignment,
                                          size_t line, size_t pos)
{
    struct DECL_STMT_AST*decl_stmt;
    SAFE_MALLOC(decl_stmt, 1);
    decl_stmt->new_var_name = new_var_name;
    decl_stmt->assignment = assignment;

    decl_stmt->line = line;
    decl_stmt->pos = pos;
    
    return decl_stmt;
}

static void dump_assignment_expr_ast_to_file_inner(FILE*f, const struct ASSIGNMENT_EXPR_AST*ast, size_t spaces_num);

static void dump_decl_stmt_ast_to_file_inner(FILE*f, const struct DECL_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<decl_stmt line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->new_var_name, spaces_num);
    PUT_SPACES(); fprintf(f, "<op>EQ</op>\n");
    dump_assignment_expr_ast_to_file_inner(f, ast->assignment, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</decl_stmt>\n");
}

__inline__ void dump_decl_stmt_ast_to_file(FILE*f, const struct DECL_STMT_AST*ast)
{
    dump_decl_stmt_ast_to_file_inner(f, ast, 0);
}

void decl_stmt_ast_free(struct DECL_STMT_AST*ast)
{
    ident_ast_free(ast->new_var_name);
    assignment_expr_ast_free(ast->assignment);
    SAFE_FREE(ast);
}

struct ASSIGN_STMT_AST*create_assign_stmt_ast(struct VARIABLE_AST*var_name,
                                              struct ASSIGNMENT_EXPR_AST*assignment,
                                              size_t line, size_t pos)
{
    struct ASSIGN_STMT_AST*assign_stmt;
    SAFE_MALLOC(assign_stmt, 1);
    assign_stmt->var_name = var_name;
    assign_stmt->assignment = assignment;

    assign_stmt->line = line;
    assign_stmt->pos = pos;
    
    return assign_stmt;
}

static void dump_variable_ast_to_file_inner(FILE*f, const struct VARIABLE_AST*ast, size_t spaces_num);

static void dump_assign_stmt_ast_to_file_inner(FILE*f, const struct ASSIGN_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<assign_stmt line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_variable_ast_to_file_inner(f, ast->var_name, spaces_num);
    PUT_SPACES(); fprintf(f, "<op>EQ</op>\n");    
    dump_assignment_expr_ast_to_file_inner(f, ast->assignment, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</assign_stmt>\n");
}

__inline__ void dump_assign_stmt_ast_to_file(FILE*f, const struct ASSIGN_STMT_AST*ast)
{
    dump_assign_stmt_ast_to_file_inner(f, ast, 0);
}

void assign_stmt_ast_free(struct ASSIGN_STMT_AST*ast)
{
    variable_ast_free(ast->var_name);
    assignment_expr_ast_free(ast->assignment);
    SAFE_FREE(ast);
}

struct FUNCTION_CALL_STMT_AST*create_function_call_stmt_ast(struct FUNCTION_CALL_AST*function_call,
                                                            size_t line, size_t pos)
{
    struct FUNCTION_CALL_STMT_AST*function_call_stmt;
    SAFE_MALLOC(function_call_stmt, 1);
    function_call_stmt->function_call = function_call;

    function_call_stmt->line = line;
    function_call_stmt->pos = pos;
    
    return function_call_stmt;
}

static void dump_function_call_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_AST*ast, size_t spaces_num);

static void dump_function_call_stmt_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<function_call_stmt line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_function_call_ast_to_file_inner(f, ast->function_call, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_call_stmt>\n");
}

__inline__ void dump_function_call_stmt_ast_to_file(FILE*f, const struct FUNCTION_CALL_STMT_AST*ast)
{
    dump_function_call_stmt_ast_to_file_inner(f, ast, 0);
}

void function_call_stmt_ast_free(struct FUNCTION_CALL_STMT_AST*ast)
{
    function_call_ast_free(ast->function_call);
    SAFE_FREE(ast);
}

struct IF_STMT_AST*create_if_stmt_ast(struct LOGICAL_OR_EXPR_AST*condition,
                                      struct BODY_AST*if_body,
                                      struct BODY_AST*else_body,
                                      size_t line, size_t pos)
{
    struct IF_STMT_AST*if_stmt;
    SAFE_MALLOC(if_stmt, 1);
    if_stmt->condition = condition;
    if_stmt->if_body = if_body;
    if_stmt->else_body = else_body;

    if_stmt->line = line;
    if_stmt->pos = pos;
    
    return if_stmt;
}


static void dump_logical_or_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_OR_EXPR_AST*ast, size_t spaces_num);

static void dump_if_stmt_ast_to_file_inner(FILE*f, const struct IF_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<if_stmt line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "<condition>\n");
    INC_SPACES_NUM();
    dump_logical_or_expr_ast_to_file_inner(f, ast->condition, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</condition>\n");

    PUT_SPACES(); fprintf(f, "<if_body>\n");
    INC_SPACES_NUM();
    dump_body_ast_to_file_inner(f, ast->if_body, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</if_body>\n");

    if (ast->else_body != NULL) {
        PUT_SPACES(); fprintf(f, "<else_body>\n");
        INC_SPACES_NUM();
        dump_body_ast_to_file_inner(f, ast->else_body, spaces_num);
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</else_body>\n");
    }

    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</if_stmt>\n");
}

__inline__ void dump_if_stmt_ast_to_file(FILE*f, const struct IF_STMT_AST*ast)
{
    dump_if_stmt_ast_to_file_inner(f, ast, 0);
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
                                            struct BODY_AST*body,
                                            size_t line, size_t pos)
{
    struct WHILE_STMT_AST*while_stmt;
    SAFE_MALLOC(while_stmt, 1);
    while_stmt->condition = condition;
    while_stmt->body = body;

    while_stmt->line = line;
    while_stmt->pos = pos;
    
    return while_stmt;
}

static void dump_while_stmt_ast_to_file_inner(FILE*f, const struct WHILE_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<while_stmt line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_while_stmt_ast_to_file(FILE*f, const struct WHILE_STMT_AST*ast)
{
    dump_while_stmt_ast_to_file_inner(f, ast, 0);
}

void while_stmt_ast_free(struct WHILE_STMT_AST*ast)
{
    logical_or_expr_ast_free(ast->condition);
    body_ast_free(ast->body);
    SAFE_FREE(ast);
}

struct BREAK_STMT_AST*create_break_stmt_ast(size_t line, size_t pos) {
    struct BREAK_STMT_AST*break_stmt;
    SAFE_MALLOC(break_stmt, 1);

    break_stmt->line = line;
    break_stmt->pos = pos;
    
    return break_stmt;
}

static void dump_break_stmt_ast_to_file_inner(FILE*f, const struct BREAK_STMT_AST*ast, size_t spaces_num)
{
    PREFIX_UNUSED(ast);
    PUT_SPACES(); fprintf(f, "<break></break>\n");
}

__inline__ void dump_break_stmt_ast_to_file(FILE*f, const struct BREAK_STMT_AST*ast)
{
    dump_break_stmt_ast_to_file_inner(f, ast, 0);
}

void break_stmt_ast_free(struct BREAK_STMT_AST*ast)
{
    SAFE_FREE(ast);
}

struct CONTINUE_STMT_AST*create_continue_stmt_ast(size_t line, size_t pos) {
    struct CONTINUE_STMT_AST*continue_stmt;
    SAFE_MALLOC(continue_stmt, 1);

    continue_stmt->line = line;
    continue_stmt->pos = pos;
    
    return continue_stmt;
}

static void dump_continue_stmt_ast_to_file_inner(FILE*f, const struct CONTINUE_STMT_AST*ast, size_t spaces_num)
{
    PREFIX_UNUSED(ast);
    PUT_SPACES(); fprintf(f, "<continue></continue>\n");
}

__inline__ void dump_continue_stmt_ast_to_file(FILE*f, const struct CONTINUE_STMT_AST*ast)
{
    dump_continue_stmt_ast_to_file_inner(f, ast, 0);
}

void continue_stmt_ast_free(struct CONTINUE_STMT_AST*ast)
{
    SAFE_FREE(ast);
}

struct APPEND_STMT_AST*create_append_stmt_ast(struct VARIABLE_AST*arr, struct IDENT_AST*ident,
                                              size_t line, size_t pos)
{
    struct APPEND_STMT_AST*append_stmt;
    SAFE_MALLOC(append_stmt, 1);

    append_stmt->arr = arr;
    append_stmt->ident = ident;

    append_stmt->line = line;
    append_stmt->pos = pos;

    return append_stmt;
}

void dump_append_stmt_ast_to_file_inner(FILE*f, const struct APPEND_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<append_stmt line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_variable_ast_to_file_inner(f, ast->arr, spaces_num);
    dump_ident_ast_to_file_inner(f, ast->ident, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</append_stmt>\n");
}

void dump_append_stmt_ast_to_file(FILE*f, const struct APPEND_STMT_AST*ast)
{
    dump_append_stmt_ast_to_file_inner(f, ast, 0);
}

void append_stmt_ast_free(struct APPEND_STMT_AST*ast)
{
    variable_ast_free(ast->arr);
    ident_ast_free(ast->ident);

    SAFE_FREE(ast);
}

struct DELETE_STMT_AST*create_delete_stmt_ast(struct VARIABLE_AST*var, struct IDENT_AST*ident,
                                              size_t line, size_t pos)
{
    struct DELETE_STMT_AST*delete_stmt;
    SAFE_MALLOC(delete_stmt, 1);

    delete_stmt->var = var;
    delete_stmt->ident = ident;

    delete_stmt->line = line;
    delete_stmt->pos = pos;

    return delete_stmt;
}

void dump_delete_stmt_ast_to_file_inner(FILE*f, const struct DELETE_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<delete_stmt line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_variable_ast_to_file_inner(f, ast->var, spaces_num);
    dump_ident_ast_to_file_inner(f, ast->ident, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</delete_stmt>\n");
}

void dump_delete_stmt_ast_to_file(FILE*f, const struct DELETE_STMT_AST*ast)
{
    dump_delete_stmt_ast_to_file_inner(f, ast, 0);
}

void delete_stmt_ast_free(struct DELETE_STMT_AST*ast)
{
    variable_ast_free(ast->var);
    ident_ast_free(ast->ident);

    SAFE_FREE(ast);
}

struct RETURN_STMT_AST*create_return_stmt_ast(struct ASSIGNMENT_EXPR_AST*result,
                                              size_t line, size_t pos)
{
    struct RETURN_STMT_AST*return_stmt;
    SAFE_MALLOC(return_stmt, 1);
    return_stmt->result = result;

    return_stmt->line = line;
    return_stmt->pos = pos;
    
    return return_stmt;
}

static void dump_return_stmt_ast_to_file_inner(FILE*f, const struct RETURN_STMT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<return_stmt> line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    if (ast->result != NULL) {
        INC_SPACES_NUM();
        dump_assignment_expr_ast_to_file_inner(f, ast->result, spaces_num);
        DEC_SPACES_NUM();
    }
    PUT_SPACES(); fprintf(f, "</return_stmt>\n");
}

__inline__ void dump_return_stmt_ast_to_file(FILE*f, const struct RETURN_STMT_AST*ast)
{
    dump_return_stmt_ast_to_file_inner(f, ast, 0);
}

void return_stmt_ast_free(struct RETURN_STMT_AST*ast)
{
    if (ast->result != NULL) {
        assignment_expr_ast_free(ast->result);
    }
    SAFE_FREE(ast);
}

struct VARIABLE_PART_AST*create_variable_part_ast(void*variable_part_ptr, enum AST_VARIABLE_PART_TYPE type)
{
    struct VARIABLE_PART_AST*variable_part;
    SAFE_MALLOC(variable_part, 1);
    switch (type) {
    case AST_VARIABLE_PART_TYPE_FIELD:
        variable_part->field = variable_part_ptr;
        break;
    case AST_VARIABLE_PART_TYPE_INDEX:
        variable_part->index = variable_part_ptr;
        break;
    default:
        fprintf(stderr, "Invalid VARIABLE_EXPR_AST type: %d\n", type);
        exit(EXIT_FAILURE);
        break;        
    }
    variable_part->type = type;
    return variable_part;
}

static void dump_variable_part_ast_to_file_inner(FILE*f, const struct VARIABLE_PART_AST*ast, size_t spaces_num)
{
    switch (ast->type) {
    case AST_VARIABLE_PART_TYPE_FIELD:
        PUT_SPACES(); fprintf(f, "<field>\n");
        INC_SPACES_NUM();
        dump_ident_ast_to_file_inner(f, ast->field, spaces_num);
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</field>\n");
        break;
    case AST_VARIABLE_PART_TYPE_INDEX:
        PUT_SPACES(); fprintf(f, "<index>\n");
        INC_SPACES_NUM();
        dump_logical_or_expr_ast_to_file_inner(f, ast->index, spaces_num);
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</index>\n");
        break;
    default:
        fprintf(stderr, "Invalid VARIABLE_PART_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;        
    }
}

__inline__ void dump_variable_part_ast_to_file(FILE*f, const struct VARIABLE_PART_AST*ast)
{
    dump_variable_part_ast_to_file_inner(f, ast, 0);
}

void variable_part_ast_free(struct VARIABLE_PART_AST*ast)
{
    switch (ast->type) {
    case AST_VARIABLE_PART_TYPE_FIELD:
        ident_ast_free(ast->field);
        break;
    case AST_VARIABLE_PART_TYPE_INDEX:
        logical_or_expr_ast_free(ast->index);
        break;
    default:
        fprintf(stderr, "Invalid VARIABLE_PART_AST type: %d\n", ast->type);
        exit(EXIT_FAILURE);
        break;
    }
    SAFE_FREE(ast);
}

struct VARIABLE_AST*create_variable_ast(struct IDENT_AST*ident, struct VARIABLE_PART_AST**parts, size_t parts_len,
                                        size_t line, size_t pos)
{
    struct VARIABLE_AST*variable;
    SAFE_MALLOC(variable, 1);
    variable->ident = ident;
    variable->parts = parts;
    variable->parts_len = parts_len;

    variable->line = line;
    variable->pos = pos;
    
    return variable;
}

static void dump_variable_ast_to_file_inner(FILE*f, const struct VARIABLE_AST*ast, size_t spaces_num)
{
    size_t i;
    
    PUT_SPACES(); fprintf(f, "<variable line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->ident, spaces_num);
    for (i = 0; i < ast->parts_len; i++) {
        dump_variable_part_ast_to_file_inner(f, ast->parts[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</variable>\n");
}

__inline__ void dump_variable_ast_to_file(FILE*f, const struct VARIABLE_AST*ast)
{
    dump_variable_ast_to_file_inner(f, ast, 0);
}

void variable_ast_free(struct VARIABLE_AST*ast)
{
    size_t i;
    ident_ast_free(ast->ident);
    for (i = 0; i < ast->parts_len; i++) {
        variable_part_ast_free(ast->parts[i]);
    }
    SAFE_FREE(ast->parts);
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
    case AST_ASSIGNMENT_EXPR_TYPE_ARRAY_LITERAL:
        assignment_expr->array_literal = assignment_ptr;
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

static void dump_object_literal_ast_to_file_inner(FILE*f, const struct OBJECT_LITERAL_AST*ast, size_t spaces_num);

void dump_array_literal_ast_to_file_inner(FILE*f, const struct ARRAY_LITERAL_AST*ast, size_t spaces_num);

static void dump_assignment_expr_ast_to_file_inner(FILE*f, const struct ASSIGNMENT_EXPR_AST*ast, size_t spaces_num)
{
    switch (ast->type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        dump_object_literal_ast_to_file_inner(f, ast->object_literal, spaces_num);
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_ARRAY_LITERAL:
        dump_array_literal_ast_to_file_inner(f, ast->array_literal, spaces_num);
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

__inline__ void dump_assignment_expr_ast_to_file(FILE*f, const struct ASSIGNMENT_EXPR_AST*ast)
{
    dump_assignment_expr_ast_to_file_inner(f, ast, 0);
}

void assignment_expr_ast_free(struct ASSIGNMENT_EXPR_AST*ast)
{
    switch (ast->type) {
    case AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL:
        object_literal_ast_free(ast->object_literal);
        break;
    case AST_ASSIGNMENT_EXPR_TYPE_ARRAY_LITERAL:
        array_literal_ast_free(ast->array_literal);
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

struct OBJECT_LITERAL_AST*create_object_literal_ast(struct PROPERTY_AST**properties, size_t properties_len,
                                                    size_t line, size_t pos)
{
    struct OBJECT_LITERAL_AST*object_literal;
    SAFE_MALLOC(object_literal, 1);
    object_literal->properties = properties;
    object_literal->properties_len = properties_len;

    object_literal->line = line;
    object_literal->pos = pos;

    return object_literal;
}

static void dump_property_ast_to_file_inner(FILE*f, const struct PROPERTY_AST*ast, size_t spaces_num);

static void dump_object_literal_ast_to_file_inner(FILE*f, const struct OBJECT_LITERAL_AST*ast, size_t spaces_num)
{
    size_t i;
    
    PUT_SPACES(); fprintf(f, "<object_literal line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    for (i = 0; i < ast->properties_len; i++) {
        dump_property_ast_to_file_inner(f, ast->properties[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</object_literal>\n");
}

__inline__ void dump_object_literal_ast_to_file(FILE*f, const struct OBJECT_LITERAL_AST*ast)
{
    dump_object_literal_ast_to_file_inner(f, ast, 0);
}

void object_literal_ast_free(struct OBJECT_LITERAL_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->properties_len; i++) {
        property_ast_free(ast->properties[i]);
    }
    SAFE_FREE(ast->properties);
    SAFE_FREE(ast);
}

struct PROPERTY_AST*create_property_ast(struct IDENT_AST*key, struct ASSIGNMENT_EXPR_AST*value,
                                        size_t line, size_t pos)
{
    struct PROPERTY_AST*property;
    SAFE_MALLOC(property, 1);
    property->key = key;
    property->value = value;

    property->line = line;
    property->pos = pos;
    
    return property;
}

static void dump_property_ast_to_file_inner(FILE*f, const struct PROPERTY_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<property line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_property_ast_to_file(FILE*f, const struct PROPERTY_AST*ast)
{
    dump_property_ast_to_file_inner(f, ast, 0);
}

void property_ast_free(struct PROPERTY_AST*ast)
{
    ident_ast_free(ast->key);
    assignment_expr_ast_free(ast->value);
    SAFE_FREE(ast);
}

struct ARRAY_LITERAL_AST*create_array_literal_ast(struct ARGS_LIST_AST*args_list,
                                                  size_t line, size_t pos)
{
    struct ARRAY_LITERAL_AST*array_literal;
    SAFE_MALLOC(array_literal, 1);
    array_literal->args_list = args_list;

    array_literal->line = line;
    array_literal->pos = pos;
    
    return array_literal;
}

static void dump_args_list_ast_to_file_inner(FILE*f, const struct ARGS_LIST_AST*ast, size_t spaces_num);

void dump_array_literal_ast_to_file_inner(FILE*f, const struct ARRAY_LITERAL_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<array_literal line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    if (ast->args_list != NULL) {
        dump_args_list_ast_to_file_inner(f, ast->args_list, spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</array_literal>\n");
}

__inline__ void dump_array_literal_ast_to_file(FILE*f, const struct ARRAY_LITERAL_AST*ast)
{
    dump_array_literal_ast_to_file_inner(f, ast, 0);
}

void array_literal_ast_free(struct ARRAY_LITERAL_AST*ast)
{
    if (ast->args_list != NULL) {
        args_list_ast_free(ast->args_list);
    }
    SAFE_FREE(ast);
}

struct LOGICAL_OR_EXPR_AST*create_logical_or_expr_ast(struct LOGICAL_AND_EXPR_AST**and_exprs, size_t and_exprs_len,
                                                      size_t line, size_t pos)
{
    struct LOGICAL_OR_EXPR_AST*logical_or_expr;
    SAFE_MALLOC(logical_or_expr, 1);
    logical_or_expr->and_exprs = and_exprs;
    logical_or_expr->and_exprs_len = and_exprs_len;

    logical_or_expr->line = line;
    logical_or_expr->pos = pos;
    
    return logical_or_expr;
}

static void dump_logical_and_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_AND_EXPR_AST*ast, size_t spaces_num);

static void dump_logical_or_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_OR_EXPR_AST*ast, size_t spaces_num)
{
    if (ast->and_exprs_len == 1) {
        dump_logical_and_expr_ast_to_file_inner(f, ast->and_exprs[0], spaces_num);
    } else {
        size_t i;
        
        PUT_SPACES(); fprintf(f, "<logical_or_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_logical_or_expr_ast_to_file(FILE*f, const struct LOGICAL_OR_EXPR_AST*ast)
{
    dump_logical_or_expr_ast_to_file_inner(f, ast, 0);
}

void logical_or_expr_ast_free(struct LOGICAL_OR_EXPR_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->and_exprs_len; i++) {
        logical_and_expr_ast_free(ast->and_exprs[i]);
    }
    SAFE_FREE(ast->and_exprs);
    SAFE_FREE(ast);
}

struct LOGICAL_AND_EXPR_AST*create_logical_and_expr_ast(struct EQ_EXPR_AST**eq_exprs, size_t eq_exprs_len,
                                                        size_t line, size_t pos)
{
    struct LOGICAL_AND_EXPR_AST*logical_and_expr;
    SAFE_MALLOC(logical_and_expr, 1);
    logical_and_expr->eq_exprs = eq_exprs;
    logical_and_expr->eq_exprs_len = eq_exprs_len;

    logical_and_expr->line = line;
    logical_and_expr->pos = pos;
    
    return logical_and_expr;
}

static void dump_eq_expr_ast_to_file_inner(FILE*f, const struct EQ_EXPR_AST*ast, size_t spaces_num);

static void dump_logical_and_expr_ast_to_file_inner(FILE*f, const struct LOGICAL_AND_EXPR_AST*ast, size_t spaces_num)
{
    if (ast->eq_exprs_len == 1) {
        dump_eq_expr_ast_to_file_inner(f, ast->eq_exprs[0], spaces_num);
    } else {
        size_t i;
        
        PUT_SPACES(); fprintf(f, "<logical_and_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_logical_and_expr_ast_to_file(FILE*f, const struct LOGICAL_AND_EXPR_AST*ast)
{
    dump_logical_and_expr_ast_to_file_inner(f, ast, 0);
}

void logical_and_expr_ast_free(struct LOGICAL_AND_EXPR_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->eq_exprs_len; i++) {
        eq_expr_ast_free(ast->eq_exprs[i]);
    }
    SAFE_FREE(ast->eq_exprs);
    SAFE_FREE(ast);
}

struct EQ_EXPR_AST*create_eq_expr_ast(struct RELATIONAL_EXPR_AST*left,
                                      enum AST_EQ_OP eq_op,
                                      struct RELATIONAL_EXPR_AST*right,
                                      size_t line, size_t pos)
{
    struct EQ_EXPR_AST*eq_expr;
    SAFE_MALLOC(eq_expr, 1);
    eq_expr->left = left;
    eq_expr->eq_op = eq_op;
    eq_expr->right = right;

    eq_expr->line = line;
    eq_expr->pos = pos;
    
    return eq_expr;
}

static void dump_relational_expr_ast_to_file_inner(FILE*f, const struct RELATIONAL_EXPR_AST*ast, size_t spaces_num);

static void dump_eq_expr_ast_to_file_inner(FILE*f, const struct EQ_EXPR_AST*ast, size_t spaces_num)
{
    if (ast->right == NULL) {
        dump_relational_expr_ast_to_file_inner(f, ast->left, spaces_num);
    } else {
        PUT_SPACES(); fprintf(f, "<eq_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_eq_expr_ast_to_file(FILE*f, const struct EQ_EXPR_AST*ast)
{
    dump_eq_expr_ast_to_file_inner(f, ast, 0);
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
                                                      struct ADDITIVE_EXPR_AST*right,
                                                      size_t line, size_t pos)
{
    struct RELATIONAL_EXPR_AST*relational_expr;
    SAFE_MALLOC(relational_expr, 1);
    relational_expr->left = left;
    relational_expr->rel_op = rel_op;
    relational_expr->right = right;

    relational_expr->line = line;
    relational_expr->pos = pos;
    
    return relational_expr;
}

static void dump_additive_expr_ast_to_file_inner(FILE*f, const struct ADDITIVE_EXPR_AST*ast, size_t spaces_num);

static void dump_relational_expr_ast_to_file_inner(FILE*f, const struct RELATIONAL_EXPR_AST*ast, size_t spaces_num)
{
    if (ast->right == NULL) {
        dump_additive_expr_ast_to_file_inner(f, ast->left, spaces_num);
    } else {
        PUT_SPACES(); fprintf(f, "<relational_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_relational_expr_ast_to_file(FILE*f, const struct RELATIONAL_EXPR_AST*ast)
{
    dump_relational_expr_ast_to_file_inner(f, ast, 0);
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
                                                  size_t muls_len,
                                                  size_t line, size_t pos)
{
    struct ADDITIVE_EXPR_AST*additive_expr;
    SAFE_MALLOC(additive_expr, 1);
    additive_expr->muls = muls;
    additive_expr->ops = ops;
    additive_expr->muls_len = muls_len;

    additive_expr->line = line;
    additive_expr->pos = pos;
    
    return additive_expr;
}

static void dump_multiplicative_expr_ast_to_file_inner(FILE*f, const struct MULTIPLICATIVE_EXPR_AST*ast, size_t spaces_num);

static void dump_additive_expr_ast_to_file_inner(FILE*f, const struct ADDITIVE_EXPR_AST*ast, size_t spaces_num)
{
    if (ast->muls_len == 1) {
        dump_multiplicative_expr_ast_to_file_inner(f, ast->muls[0], spaces_num);
    } else {
        size_t i;
        
        PUT_SPACES(); fprintf(f, "<additive_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_additive_expr_ast_to_file(FILE*f, const struct ADDITIVE_EXPR_AST*ast)
{
    dump_additive_expr_ast_to_file_inner(f, ast, 0);
}

void additive_expr_ast_free(struct ADDITIVE_EXPR_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->muls_len; i++) {
        multiplicative_expr_ast_free(ast->muls[i]);
    }
    SAFE_FREE(ast->muls);
    SAFE_FREE(ast->ops);
    SAFE_FREE(ast);
}

struct MULTIPLICATIVE_EXPR_AST*create_multiplicative_expr_ast(struct LEFT_UNARY_EXPR_AST**lues,
                                                              enum AST_MULTIPLICATIVE_OP*ops,
                                                              size_t lues_len,
                                                              size_t line, size_t pos)
{
    struct MULTIPLICATIVE_EXPR_AST*mul_expr;
    SAFE_MALLOC(mul_expr, 1);
    mul_expr->lues = lues;
    mul_expr->ops = ops;
    mul_expr->lues_len = lues_len;

    mul_expr->line = line;
    mul_expr->pos = pos;

    return mul_expr;
}

static void dump_left_unary_expr_ast_to_file_inner(FILE*f, const struct LEFT_UNARY_EXPR_AST*ast, size_t spaces_num);

static void dump_multiplicative_expr_ast_to_file_inner(FILE*f, const struct MULTIPLICATIVE_EXPR_AST*ast, size_t spaces_num)
{
    if (ast->lues_len == 1) {
        dump_left_unary_expr_ast_to_file_inner(f, ast->lues[0], spaces_num);
    } else {
        size_t i;
        
        PUT_SPACES(); fprintf(f, "<multiplicative_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
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

__inline__ void dump_multiplicative_expr_ast_to_file(FILE*f, const struct MULTIPLICATIVE_EXPR_AST*ast)
{
    dump_multiplicative_expr_ast_to_file_inner(f, ast, 0);
}

void multiplicative_expr_ast_free(struct MULTIPLICATIVE_EXPR_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->lues_len; i++) {
        left_unary_expr_ast_free(ast->lues[i]);
    }
    SAFE_FREE(ast->lues);
    SAFE_FREE(ast->ops);
    SAFE_FREE(ast);
}

struct LEFT_UNARY_EXPR_AST*create_left_unary_expr_ast(enum AST_LEFT_UNARY_OP op, struct PRIMARY_EXPR_AST*expr,
                                                      size_t line, size_t pos)
{
    struct LEFT_UNARY_EXPR_AST*lue;
    SAFE_MALLOC(lue, 1);
    lue->op = op;
    lue->expr = expr;

    lue->line = line;
    lue->pos = pos;
    
    return lue;
}

static void dump_primary_expr_ast_to_file_inner(FILE*f, const struct PRIMARY_EXPR_AST*ast, size_t spaces_num);

static void dump_left_unary_expr_ast_to_file_inner(FILE*f, const struct LEFT_UNARY_EXPR_AST*ast, size_t spaces_num)
{
    if (ast->op != AST_LEFT_UNARY_OP_PLUS) {
        PUT_SPACES(); fprintf(f, "<left_unary_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
        INC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "<op>UNARY_MINUS</op>\n");
        dump_primary_expr_ast_to_file_inner(f, ast->expr, spaces_num);
        DEC_SPACES_NUM();
        PUT_SPACES(); fprintf(f, "</left_unary_expr>\n");
    } else {
        dump_primary_expr_ast_to_file_inner(f, ast->expr, spaces_num);
    }
}

__inline__ void dump_left_unary_expr_ast_to_file(FILE*f, const struct LEFT_UNARY_EXPR_AST*ast)
{
    dump_left_unary_expr_ast_to_file_inner(f, ast, 0);
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
    case AST_PRIMARY_EXPR_TYPE_HAS_PROPERTY:
        primary_expr->has_property_expr = primary_expr_ptr;
        break;
    case AST_PRIMARY_EXPR_TYPE_LEN:
        primary_expr->len_expr = primary_expr_ptr;
        break;
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

static void dump_has_property_expr_ast_to_file_inner(FILE*f, const struct HAS_PROPERTY_EXPR_AST*ast, size_t spaces_num);
static void dump_len_expr_ast_to_file_inner(FILE*f, const struct LEN_EXPR_AST*ast, size_t spaces_num);
static void dump_number_ast_to_file_inner(FILE*f, const struct NUMBER_AST*ast, size_t spaces_num);

static void dump_primary_expr_ast_to_file_inner(FILE*f, const struct PRIMARY_EXPR_AST*ast, size_t spaces_num)
{
    switch (ast->type) {
    case AST_PRIMARY_EXPR_TYPE_HAS_PROPERTY:
        dump_has_property_expr_ast_to_file_inner(f, ast->has_property_expr, spaces_num);
        break;
    case AST_PRIMARY_EXPR_TYPE_LEN:
        dump_len_expr_ast_to_file_inner(f, ast->len_expr, spaces_num);
        break;
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

__inline__ void dump_primary_expr_ast_to_file(FILE*f, const struct PRIMARY_EXPR_AST*ast)
{
    dump_primary_expr_ast_to_file_inner(f, ast, 0);
}

void primary_expr_ast_free(struct PRIMARY_EXPR_AST*ast)
{
    switch (ast->type) {
    case AST_PRIMARY_EXPR_TYPE_HAS_PROPERTY:
        has_property_expr_ast_free(ast->has_property_expr);
        break;
    case AST_PRIMARY_EXPR_TYPE_LEN:
        len_expr_ast_free(ast->len_expr);
        break;
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

struct HAS_PROPERTY_EXPR_AST*create_has_property_expr_ast(struct VARIABLE_AST*obj, struct IDENT_AST*ident,
                                                          size_t line, size_t pos)
{
    struct HAS_PROPERTY_EXPR_AST*has_property;
    SAFE_MALLOC(has_property, 1);
    has_property->obj = obj;
    has_property->ident = ident;

    has_property->line = line;
    has_property->pos = pos;

    return has_property;
}

static void dump_has_property_expr_ast_to_file_inner(FILE*f, const struct HAS_PROPERTY_EXPR_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<has_property_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_variable_ast_to_file_inner(f, ast->obj, spaces_num);
    dump_ident_ast_to_file_inner(f, ast->ident, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</has_property_expr>\n");
}

__inline__ void dump_has_property_expr_ast_to_file(FILE*f, const struct HAS_PROPERTY_EXPR_AST*ast)
{
    dump_has_property_expr_ast_to_file_inner(f, ast, 0);
}

void has_property_expr_ast_free(struct HAS_PROPERTY_EXPR_AST*ast)
{
    variable_ast_free(ast->obj);
    ident_ast_free(ast->ident);
    SAFE_FREE(ast);
}

struct LEN_EXPR_AST*create_len_expr_ast(struct VARIABLE_AST*arr, size_t line, size_t pos)
{
    struct LEN_EXPR_AST*len;
    SAFE_MALLOC(len, 1);
    len->arr = arr;

    len->line = line;
    len->pos = pos;

    return len;
}

static void dump_len_expr_ast_to_file_inner(FILE*f, const struct LEN_EXPR_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<len_expr line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_variable_ast_to_file_inner(f, ast->arr, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</len_expr>\n");
}

__inline__ void dump_len_expr_ast_to_file(FILE*f, const struct LEN_EXPR_AST*ast)
{
    dump_len_expr_ast_to_file_inner(f, ast, 0);
}

void len_expr_ast_free(struct LEN_EXPR_AST*ast)
{
    variable_ast_free(ast->arr);
    SAFE_FREE(ast);
}

struct FUNCTION_CALL_AST*create_function_call_ast(struct IDENT_AST*function_name, struct ARGS_LIST_AST*args_list,
                                                  size_t line, size_t pos)
{
    struct FUNCTION_CALL_AST*function_call;
    SAFE_MALLOC(function_call, 1);
    function_call->function_name = function_name;
    function_call->args_list = args_list;

    function_call->line = line;
    function_call->pos = pos;
    
    return function_call;
}

static void dump_function_call_ast_to_file_inner(FILE*f, const struct FUNCTION_CALL_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<function_call line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "<function_name line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    dump_ident_ast_to_file_inner(f, ast->function_name, spaces_num);
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_name>\n");
    if (ast->args_list != NULL) {
        dump_args_list_ast_to_file_inner(f, ast->args_list, spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</function_call>\n");
}

__inline__ void dump_function_call_ast_to_file(FILE*f, const struct FUNCTION_CALL_AST*ast)
{
    dump_function_call_ast_to_file_inner(f, ast, 0);
}

void function_call_ast_free(struct FUNCTION_CALL_AST*ast)
{
    ident_ast_free(ast->function_name);
    if (ast->args_list != NULL) {
        args_list_ast_free(ast->args_list);
    }
    SAFE_FREE(ast);
}

struct ARGS_LIST_AST*create_args_list_ast(struct ASSIGNMENT_EXPR_AST**assignment_exprs, size_t assignment_exprs_len,
                                          size_t line, size_t pos)
{
    struct ARGS_LIST_AST*args_list;
    SAFE_MALLOC(args_list, 1);
    args_list->assignment_exprs = assignment_exprs;
    args_list->assignment_exprs_len = assignment_exprs_len;

    args_list->line = line;
    args_list->pos = pos;
    
    return args_list;
}

static void dump_args_list_ast_to_file_inner(FILE*f, const struct ARGS_LIST_AST*ast, size_t spaces_num)
{
    size_t i;
    
    PUT_SPACES(); fprintf(f, "<args_list line=\"%zu\" pos=\"%zu\">\n", ast->line, ast->pos);
    INC_SPACES_NUM();
    for (i = 0; i < ast->assignment_exprs_len; i++) {
        dump_assignment_expr_ast_to_file_inner(f, ast->assignment_exprs[i], spaces_num);
    }
    DEC_SPACES_NUM();
    PUT_SPACES(); fprintf(f, "</args_list>\n");
}

__inline__ void dump_args_list_ast_to_file(FILE*f, const struct ARGS_LIST_AST*ast)
{
    dump_args_list_ast_to_file_inner(f, ast, 0);
}

void args_list_ast_free(struct ARGS_LIST_AST*ast)
{
    size_t i;
    for (i = 0; i < ast->assignment_exprs_len; i++) {
        assignment_expr_ast_free(ast->assignment_exprs[i]);
    }
    SAFE_FREE(ast->assignment_exprs);
    SAFE_FREE(ast);
}

struct IDENT_AST*create_ident_ast(const char*ident, size_t line, size_t pos)
{
    struct IDENT_AST*ident_ast;
    SAFE_MALLOC(ident_ast, 1);
    strncpy(ident_ast->ident, ident, sizeof(ident_ast->ident));

    ident_ast->line = line;
    ident_ast->pos = pos;
    
    return ident_ast;
}

static void dump_ident_ast_to_file_inner(FILE*f, const struct IDENT_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<ident line=\"%zu\" pos=\"%zu\">%s</ident>\n", ast->line, ast->pos, ast->ident);
}

__inline__ void dump_ident_ast_to_file(FILE*f, const struct IDENT_AST*ast)
{
    dump_ident_ast_to_file_inner(f, ast, 0);
}

void ident_ast_free(struct IDENT_AST*ast)
{
    SAFE_FREE(ast);
}

struct NUMBER_AST*create_number_ast(long long number, size_t line, size_t pos)
{
    struct NUMBER_AST*number_ast;
    SAFE_MALLOC(number_ast, 1);
    number_ast->number = number;

    number_ast->line = line;
    number_ast->pos = pos;
    
    return number_ast;
}

static void dump_number_ast_to_file_inner(FILE*f, const struct NUMBER_AST*ast, size_t spaces_num)
{
    PUT_SPACES(); fprintf(f, "<number line=\"%zu\" pos=\"%zu\">%lld</number>\n", ast->line, ast->pos, ast->number);
}

__inline__ void dump_number_ast_to_file(FILE*f, const struct NUMBER_AST*ast)
{
    dump_number_ast_to_file_inner(f, ast, 0);
}

void number_ast_free(struct NUMBER_AST*ast)
{
    SAFE_FREE(ast);
}

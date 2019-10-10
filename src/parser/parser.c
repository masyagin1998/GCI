#include "parser.h"

struct PARSER
{
    lexer_type_t lexer;
};

parser_type_t create_parser()
{
    struct PARSER*ptr;
    SAFE_CALLOC(ptr, 1);
    return ptr;
}

void parser_conf(parser_type_t parser, lexer_type_t lexer)
{
    parser->lexer = lexer;
}

static enum PARSER_CODES ident_ast_read(struct IDENT_AST**ident, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES formal_parameters_list_ast_read(struct FORMAL_PARAMETERS_LIST_AST**formal_parameters_list, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES property_ast_read(struct PROPERTY_AST**property, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES assignment_expr_ast_read(struct ASSIGNMENT_EXPR_AST**assignment_expr, lexer_type_t lexer, struct TOKEN**tok);

static enum PARSER_CODES object_literal_ast_read(struct OBJECT_LITERAL_AST**object_literal, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES number_expr_ast_read(struct NUMBER_EXPR_AST**number_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES args_list_expr_ast_read(struct ARGS_LIST_EXPR_AST**args_list_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES function_call_expr_ast_read(struct FUNCTION_CALL_EXPR_AST**function_call_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES primary_expr_ast_read(struct PRIMARY_EXPR_AST**primary_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES left_unary_expr_ast_read(struct LEFT_UNARY_EXPR_AST**left_unary_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES multiplicative_expr_ast_read(struct MULTIPLICATIVE_EXPR_AST**multiplicative_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES additive_expr_ast_read(struct ADDITIVE_EXPR_AST**additive_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES relational_expr_ast_read(struct RELATIONAL_EXPR_AST**relational_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES eq_expr_ast_read(struct EQ_EXPR_AST**eq_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES logical_and_expr_ast_read(struct LOGICAL_AND_EXPR_AST**logical_and_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES logical_or_expr_ast_read(struct LOGICAL_OR_EXPR_AST**logical_or_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES assignment_expr_ast_read(struct ASSIGNMENT_EXPR_AST**assignment_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES decl_stmt_ast_read(struct DECL_STMT_AST**decl_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES variable_ast_read(struct VARIABLE_AST**variable, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES assign_stmt_ast_read(struct ASSIGN_STMT_AST**assign_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES function_call_stmt_ast_read(struct FUNCTION_CALL_STMT_AST**function_call_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES if_stmt_ast_read(struct IF_STMT_AST**if_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES while_stmt_ast_read(struct WHILE_STMT_AST**while_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES return_stmt_ast_read(struct RETURN_STMT_AST**return_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES stmt_ast_read(struct STMT_AST**stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES body_ast_read(struct BODY_AST**body, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES function_decl_ast_read(struct FUNCTION_DECL_AST**function, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct IDENT_AST*function_name;
    struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list;
    struct BODY_AST*body;

    if ((*tok)->token_type != TOKEN_TYPE_FUNCTION) {
        r = PARSER_INVALID_TOKEN;
        goto err0;
    }

    r = ident_ast_read(&function_name, lexer, tok);
    if (r != PARSER_OK) {
        
    }
    r = formal_parameters_list_ast_read(&formal_parameters_list, lexer, tok);
    if (r != PARSER_OK) {
        
    }
    r = body_ast_read(&body, lexer, tok);
    if (r != PARSER_OK) {
        
    }

    return PARSER_OK;

 err0:
    (*function) = NULL;
    return r;
}

static enum PARSER_CODES unit_ast_read(struct UNIT_AST**unit, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct FUNCTION_DECL_AST**functions;
    unsigned functions_len = 0;
    unsigned functions_cap = 1;
    SAFE_MALLOC(functions, 1);
    
    while ((*tok)->token_type != TOKEN_TYPE_EOF) {
        struct FUNCTION_DECL_AST*function;
        r = function_decl_ast_read(&function, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        PUSH_BACK(functions, functions_len, functions_cap, function);
    }

    (*unit) = create_unit_ast(functions, functions_len);

    return PARSER_OK;
    
 err0:
    {
        unsigned i;
        for (i = 0; i < functions_len; i++) {
            function_decl_ast_free(functions[i]);
        }
        SAFE_FREE(functions);
    }
    (*unit) = NULL;
    return r;
}

enum PARSER_CODES parser_parse(parser_type_t parser, struct UNIT_AST**unit)
{
    struct TOKEN*tok;
    lexer_next_token(parser->lexer, &tok);
    return unit_ast_read(unit, parser->lexer, &tok);
}

void parser_free(parser_type_t parser)
{
    SAFE_FREE(parser);
}

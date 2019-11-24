#include "parser.h"

#include "utils.h"

#include <stdarg.h>
#include <string.h>

struct PARSER
{
    lexer_type_t lexer;
    struct TOKEN*tok;

    struct PARSER_ERROR err;
};

parser_type_t create_parser()
{
    struct PARSER*parser;
    SAFE_CALLOC(parser, 1);
    return parser;
}

void parser_conf(parser_type_t parser, lexer_type_t lexer)
{
    parser->lexer = lexer;
}

static void set_parser_error(struct PARSER*parser, size_t n, ...)
{
    size_t i;
    
    va_list args;
    va_start(args, n);
    for (i = 0; i < n; i++) {
        parser->err.exp_toks[i] = va_arg(args, enum TOKEN_TYPE);
    }
    va_end(args);
    parser->err.exp_toks[n] = TOKEN_TYPE_EOF;
    parser->err.get_tok = (parser->tok)->token_type;
    parser->err.line = (parser->tok)->frag.starting.line;
    parser->err.pos = (parser->tok)->frag.starting.pos;
}

static enum PARSER_CODES ident_ast_read(struct PARSER*parser, struct IDENT_AST**ident)
{
    enum PARSER_CODES r;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_IDENT) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_IDENT);
        goto err0;
    }
    
    (*ident) = create_ident_ast((parser->tok)->str_val);

    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    return PARSER_OK;

 err0:
    (*ident) = NULL;
    return r;
}

static enum PARSER_CODES logical_or_expr_ast_read(struct PARSER*parser, struct LOGICAL_OR_EXPR_AST**logical_or_expr);

static enum PARSER_CODES variable_part_ast_read(struct PARSER*parser, struct VARIABLE_PART_AST**variable_part)
{
    enum PARSER_CODES r;
    
    switch ((parser->tok)->token_type) {
    case TOKEN_TYPE_DOT: {
        struct IDENT_AST*field;

        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = ident_ast_read(parser, &field);
        if (r != PARSER_OK) {
            goto err0;
        }

        (*variable_part) = create_variable_part_ast(field, AST_VARIABLE_PART_TYPE_FIELD);
        break;
    }
    case TOKEN_TYPE_LBRACKET: {
        struct LOGICAL_OR_EXPR_AST*index;
        
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = logical_or_expr_ast_read(parser, &index);
        if (r != PARSER_OK) {
            goto err0;
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_RBRACKET) {
            logical_or_expr_ast_free(index);
            r = PARSER_INVALID_TOKEN;
            set_parser_error(parser, 1, TOKEN_TYPE_RBRACKET);
            goto err0;
        }

        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        (*variable_part) = create_variable_part_ast(index, AST_VARIABLE_PART_TYPE_INDEX);
        break;
    }
    default: {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 2, TOKEN_TYPE_DOT, TOKEN_TYPE_LBRACKET);
        goto err0;
        break;
    }
    }

    return PARSER_OK;

 err0:
    (*variable_part) = NULL;
    return r;
}

static enum PARSER_CODES variable_ast_read(struct PARSER*parser, struct VARIABLE_AST**variable, struct IDENT_AST*ident)
{
    enum PARSER_CODES r;
    
    struct VARIABLE_PART_AST**parts = NULL;
    size_t parts_len = 0;
    size_t parts_cap = 0;

    while (((parser->tok)->token_type == TOKEN_TYPE_DOT) ||
           ((parser->tok)->token_type == TOKEN_TYPE_LBRACKET)) {
        struct VARIABLE_PART_AST*part;
        r = variable_part_ast_read(parser, &part);
        if (r != PARSER_OK) {
            goto err0;
        }

        PUSH_BACK(parts, part);
    }

    (*variable) = create_variable_ast(ident, parts, parts_len);

    return PARSER_OK;

 err0:
    {
        size_t i;
        for (i = 0; i < parts_len; i++) {
            variable_part_ast_free(parts[i]);
        }
        SAFE_FREE(parts);
    }
    (*variable) = NULL;
    return r;
}

static enum PARSER_CODES formal_parameters_list_ast_read(struct PARSER*parser, struct FORMAL_PARAMETERS_LIST_AST**formal_parameters_list)
{
    enum PARSER_CODES r;

    struct IDENT_AST**idents = NULL;
    size_t idents_len = 0;
    size_t idents_cap = 0;

    struct IDENT_AST*ident;

    r = ident_ast_read(parser, &ident);
    if (r != PARSER_OK) {
        goto err0;
    }
    
    PUSH_BACK(idents, ident);    
    
    while ((parser->tok)->token_type == TOKEN_TYPE_COMMA) {
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = ident_ast_read(parser, &ident);
        if (r != PARSER_OK) {
            goto err1;
        }
        
        PUSH_BACK(idents, ident);            
    }

    (*formal_parameters_list) = create_formal_parameters_list_ast(idents, idents_len);

    return PARSER_OK;

 err1:
    {
        size_t i;
        for (i = 0; i < idents_len; i++) {
            ident_ast_free(idents[i]);
        }
        SAFE_FREE(idents);
    }
 err0:
    (*formal_parameters_list) = NULL;
    return r;
}

static enum PARSER_CODES assignment_expr_ast_read(struct PARSER*parser, struct ASSIGNMENT_EXPR_AST**assignment_expr);

static enum PARSER_CODES number_ast_read(struct PARSER*parser, struct NUMBER_AST**number)
{
    enum PARSER_CODES r;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_NUMBER) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_NUMBER);
        goto err0;
    }    
    (*number) = create_number_ast((parser->tok)->int_val);

    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    return PARSER_OK;

 err0:
    (*number) = NULL;
    return r;
}

static enum PARSER_CODES args_list_ast_read(struct PARSER*parser, struct ARGS_LIST_AST**args_list)
{
    enum PARSER_CODES r;
    struct ASSIGNMENT_EXPR_AST**assignment_exprs = NULL;
    size_t assignment_exprs_len = 0;
    size_t assignment_exprs_cap = 0;

    struct ASSIGNMENT_EXPR_AST*assignment_expr;

    r = assignment_expr_ast_read(parser, &assignment_expr);
    if (r != PARSER_OK) {
        goto err0;
    }
    
    PUSH_BACK(assignment_exprs, assignment_expr);    
    
    while ((parser->tok)->token_type == TOKEN_TYPE_COMMA) {
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = assignment_expr_ast_read(parser, &assignment_expr);
        if (r != PARSER_OK) {
            goto err1;
        }
        
        PUSH_BACK(assignment_exprs, assignment_expr);    
    }

    (*args_list) = create_args_list_ast(assignment_exprs, assignment_exprs_len);

    return PARSER_OK;

 err1:
    {
        size_t i;
        for (i = 0; i < assignment_exprs_len; i++) {
            assignment_expr_ast_free(assignment_exprs[i]);
        }
        SAFE_FREE(assignment_exprs);
    }
 err0:
    (*args_list) = NULL;
    return r;
}

static enum PARSER_CODES function_call_ast_read(struct PARSER*parser, struct FUNCTION_CALL_AST**function_call, struct IDENT_AST*function_name)
{
    enum PARSER_CODES r;

    struct ARGS_LIST_AST*args_list = NULL;

    if ((parser->tok)->token_type != TOKEN_TYPE_LPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LPAREN);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        r = args_list_ast_read(parser, &args_list);
        if (r != PARSER_OK) {
            goto err0;
        }
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_RPAREN);
        goto err1;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));    
    
    (*function_call) = create_function_call_ast(function_name, args_list);

    return PARSER_OK;

 err1:
    if (args_list != NULL) {
        args_list_ast_free(args_list);
    }
 err0:
    (*function_call) = NULL;
    return r;
}

static enum PARSER_CODES primary_expr_ast_read(struct PARSER*parser, struct PRIMARY_EXPR_AST**primary_expr)
{
    enum PARSER_CODES r;

    switch ((parser->tok)->token_type) {
    case TOKEN_TYPE_IDENT: {
        struct IDENT_AST*ident = create_ident_ast((parser->tok)->str_val);
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        /* check, if name is part of variable or name is function name. */
        switch ((parser->tok)->token_type) {
        case TOKEN_TYPE_LPAREN: {
            struct FUNCTION_CALL_AST*function_call;

            r = function_call_ast_read(parser, &function_call, ident);
            if (r != PARSER_OK) {
                ident_ast_free(ident);
                goto err0;
            }
            
            (*primary_expr) = create_primary_expr_ast(function_call, AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL);
            
            break;
        }
        default: {
            struct VARIABLE_AST*var_name;
            r = variable_ast_read(parser, &var_name, ident);
            if (r != PARSER_OK) {
                ident_ast_free(ident);
                goto err0;
            }

            (*primary_expr) = create_primary_expr_ast(var_name, AST_PRIMARY_EXPR_TYPE_VARIABLE);
            
            break;
        }
        }        
        
        break;
    }
    case TOKEN_TYPE_NUMBER: {
        struct NUMBER_AST*number;
        r = number_ast_read(parser, &number);
        if (r != PARSER_OK) {
            goto err0;
        }

        (*primary_expr) = create_primary_expr_ast(number, AST_PRIMARY_EXPR_TYPE_NUMBER);
        
        break;
    }
    case TOKEN_TYPE_LPAREN: {
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;

        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = logical_or_expr_ast_read(parser, &logical_or_expr);
        if (r != PARSER_OK) {
            goto err0;
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
            logical_or_expr_ast_free(logical_or_expr);
            r = PARSER_INVALID_TOKEN;
            set_parser_error(parser, 1, TOKEN_TYPE_SEMI);
            goto err0;
        }        
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        (*primary_expr) = create_primary_expr_ast(logical_or_expr, AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR);
        
        break;
    }
    default: {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 3, TOKEN_TYPE_IDENT, TOKEN_TYPE_NUMBER, TOKEN_TYPE_LPAREN);
        goto err0;
        break;
    }
    }

    return PARSER_OK;

 err0:
    (*primary_expr) = NULL;
    return r;
}

static enum PARSER_CODES left_unary_expr_ast_read(struct PARSER*parser, struct LEFT_UNARY_EXPR_AST**left_unary_expr)
{
    enum PARSER_CODES r;

    enum AST_LEFT_UNARY_OP op = AST_LEFT_UNARY_OP_PLUS;

    struct PRIMARY_EXPR_AST*primary_expr;
    
    if ((parser->tok)->token_type == TOKEN_TYPE_PLUS) {
        op = AST_LEFT_UNARY_OP_PLUS;
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
    } else if ((parser->tok)->token_type == TOKEN_TYPE_MINUS) {
        op = AST_LEFT_UNARY_OP_MINUS;
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
    }

    r = primary_expr_ast_read(parser, &primary_expr);
    if (r != PARSER_OK) {
        goto err0;
    }

    (*left_unary_expr) = create_left_unary_expr_ast(op, primary_expr);

    return PARSER_OK;

 err0:
    (*left_unary_expr) = NULL;
    return r;
}

static enum PARSER_CODES multiplicative_expr_ast_read(struct PARSER*parser, struct MULTIPLICATIVE_EXPR_AST**multiplicative_expr)
{
    enum PARSER_CODES r;

    struct LEFT_UNARY_EXPR_AST**left_unary_exprs = NULL;
    size_t left_unary_exprs_len = 0;
    size_t left_unary_exprs_cap = 0;

    enum AST_MULTIPLICATIVE_OP*ops = NULL;
    size_t ops_len = 0;
    size_t ops_cap = 0;
    
    struct LEFT_UNARY_EXPR_AST*left_unary_expr;

    r = left_unary_expr_ast_read(parser, &left_unary_expr);
    if (r != PARSER_OK) {
        goto err0;
    }

    PUSH_BACK(left_unary_exprs, left_unary_expr);

    while (((parser->tok)->token_type == TOKEN_TYPE_MUL) || ((parser->tok)->token_type == TOKEN_TYPE_DIV) || ((parser->tok)->token_type == TOKEN_TYPE_MOD)) {
        if ((parser->tok)->token_type == TOKEN_TYPE_MUL) {
            PUSH_BACK(ops, AST_MULTIPLICATIVE_OP_MUL);
        } else if ((parser->tok)->token_type == TOKEN_TYPE_DIV) {
            PUSH_BACK(ops, AST_MULTIPLICATIVE_OP_DIV);
        } else if ((parser->tok)->token_type == TOKEN_TYPE_MOD) {
            PUSH_BACK(ops, AST_MULTIPLICATIVE_OP_MOD);
        }
        
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = left_unary_expr_ast_read(parser, &left_unary_expr);
        if (r != PARSER_OK) {
            goto err1;
        }

        PUSH_BACK(left_unary_exprs, left_unary_expr);
    }

    (*multiplicative_expr) = create_multiplicative_expr_ast(left_unary_exprs, ops, left_unary_exprs_len);

    return PARSER_OK;

 err1:
    {
        size_t i;
        for (i = 0; i < ops_len; i++) {
            left_unary_expr_ast_free(left_unary_exprs[i]);
        }
        SAFE_FREE(left_unary_exprs);
    }
    if (ops != NULL) {
        SAFE_FREE(ops);
    }    
 err0:
    (*multiplicative_expr) = NULL;
    return r;
}

static enum PARSER_CODES additive_expr_ast_read(struct PARSER*parser, struct ADDITIVE_EXPR_AST**additive_expr)
{
    enum PARSER_CODES r;

    struct MULTIPLICATIVE_EXPR_AST**multiplicative_exprs = NULL;
    size_t multiplicative_exprs_len = 0;
    size_t multiplicative_exprs_cap = 0;

    enum AST_ADDITIVE_OP*ops = NULL;
    size_t ops_len = 0;
    size_t ops_cap = 0;
    
    struct MULTIPLICATIVE_EXPR_AST*multiplicative_expr;

    r = multiplicative_expr_ast_read(parser, &multiplicative_expr);
    if (r != PARSER_OK) {
        goto err0;
    }

    PUSH_BACK(multiplicative_exprs, multiplicative_expr);

    while (((parser->tok)->token_type == TOKEN_TYPE_PLUS) || ((parser->tok)->token_type == TOKEN_TYPE_MINUS)) {
        if ((parser->tok)->token_type == TOKEN_TYPE_PLUS) {
            PUSH_BACK(ops, AST_ADDITIVE_OP_PLUS);
        } else if ((parser->tok)->token_type == TOKEN_TYPE_MINUS) {
            PUSH_BACK(ops, AST_ADDITIVE_OP_MINUS);
        }
        
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = multiplicative_expr_ast_read(parser, &multiplicative_expr);
        if (r != PARSER_OK) {
            goto err1;
        }

        PUSH_BACK(multiplicative_exprs, multiplicative_expr);
    }

    (*additive_expr) = create_additive_expr_ast(multiplicative_exprs, ops, multiplicative_exprs_len);

    return PARSER_OK;

 err1:
    {
        size_t i;
        for (i = 0; i < multiplicative_exprs_len; i++) {
            multiplicative_expr_ast_free(multiplicative_exprs[i]);
        }
        SAFE_FREE(multiplicative_exprs);
    }
 err0:
    (*additive_expr) = NULL;
    return r;
}

static enum PARSER_CODES relational_expr_ast_read(struct PARSER*parser, struct RELATIONAL_EXPR_AST**relational_expr)
{
    enum PARSER_CODES r;

    struct ADDITIVE_EXPR_AST*left = NULL;
    enum AST_REL_OP rel_op = AST_REL_OP_LT;
    struct ADDITIVE_EXPR_AST*right = NULL;

    r = additive_expr_ast_read(parser, &left);
    if (r != PARSER_OK) {
        goto  err0;
    }

    if (((parser->tok)->token_type == TOKEN_TYPE_LT) || ((parser->tok)->token_type == TOKEN_TYPE_GT) ||
        ((parser->tok)->token_type == TOKEN_TYPE_LE) || ((parser->tok)->token_type == TOKEN_TYPE_GE)) {
        if ((parser->tok)->token_type == TOKEN_TYPE_LT) {
            rel_op = AST_REL_OP_LT;
        } else if ((parser->tok)->token_type == TOKEN_TYPE_GT) {
            rel_op = AST_REL_OP_GT;
        } else if ((parser->tok)->token_type == TOKEN_TYPE_LE) {
            rel_op = AST_REL_OP_LE;
        } else if ((parser->tok)->token_type == TOKEN_TYPE_GE) {
            rel_op = AST_REL_OP_GE;
        }

        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = additive_expr_ast_read(parser, &right);
        if (r != PARSER_OK) {
            goto err1;
        }
    }

    (*relational_expr) = create_relational_expr_ast(left, rel_op, right);

    return PARSER_OK;

 err1:
    additive_expr_ast_free(left);
 err0:
    (*relational_expr) = NULL;
    return r;
}

static enum PARSER_CODES eq_expr_ast_read(struct PARSER*parser, struct EQ_EXPR_AST**eq_expr)
{
    enum PARSER_CODES r;

    struct RELATIONAL_EXPR_AST*left = NULL;
    enum AST_EQ_OP eq_op = AST_EQ_OP_EQEQ;
    struct RELATIONAL_EXPR_AST*right = NULL;

    r = relational_expr_ast_read(parser, &left);
    if (r != PARSER_OK) {
        goto err0;
    }

    if (((parser->tok)->token_type == TOKEN_TYPE_EQEQ) || ((parser->tok)->token_type == TOKEN_TYPE_NEQ)) {
        if ((parser->tok)->token_type == TOKEN_TYPE_EQEQ) {
            eq_op = AST_EQ_OP_EQEQ;
        } else if ((parser->tok)->token_type == TOKEN_TYPE_NEQ) {
            eq_op = AST_EQ_OP_NEQ;
        }

        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = relational_expr_ast_read(parser, &right);
        if (r != PARSER_OK) {
            goto err1;
        }
    }

    (*eq_expr) = create_eq_expr_ast(left, eq_op, right);

    return PARSER_OK;

 err1:
    relational_expr_ast_free(left);
 err0:
    (*eq_expr) = NULL;
    return r;
}

static enum PARSER_CODES logical_and_expr_ast_read(struct PARSER*parser, struct LOGICAL_AND_EXPR_AST**logical_and_expr)
{
    enum PARSER_CODES r;

    struct EQ_EXPR_AST**eq_exprs = NULL;
    size_t eq_exprs_len = 0;
    size_t eq_exprs_cap = 0;
    
    struct EQ_EXPR_AST*eq_expr;


    r = eq_expr_ast_read(parser, &eq_expr);
    if (r != PARSER_OK) {
        goto err0;
    }

    PUSH_BACK(eq_exprs, eq_expr);

    while ((parser->tok)->token_type == TOKEN_TYPE_AND) {
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = eq_expr_ast_read(parser, &eq_expr);
        if (r != PARSER_OK) {
            goto err1;
        }

        PUSH_BACK(eq_exprs, eq_expr);
    }

    (*logical_and_expr) = create_logical_and_expr_ast(eq_exprs, eq_exprs_len);

    return PARSER_OK;

 err1:
    {
        size_t i;
        for (i = 0; i < eq_exprs_len; i++) {
            eq_expr_ast_free(eq_exprs[i]);
        }
        SAFE_FREE(eq_exprs);
    }
 err0:
    (*logical_and_expr) = NULL;
    return r;
}

static enum PARSER_CODES logical_or_expr_ast_read(struct PARSER*parser, struct LOGICAL_OR_EXPR_AST**logical_or_expr)
{
    enum PARSER_CODES r;

    struct LOGICAL_AND_EXPR_AST**logical_and_exprs = NULL;
    size_t logical_and_exprs_len = 0;
    size_t logical_and_exprs_cap = 0;
    
    struct LOGICAL_AND_EXPR_AST*logical_and_expr;


    r = logical_and_expr_ast_read(parser, &logical_and_expr);
    if (r != PARSER_OK) {
        goto err0;
    }

    PUSH_BACK(logical_and_exprs, logical_and_expr);

    while ((parser->tok)->token_type == TOKEN_TYPE_OR) {
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = logical_and_expr_ast_read(parser, &logical_and_expr);
        if (r != PARSER_OK) {
            goto err1;
        }

        PUSH_BACK(logical_and_exprs, logical_and_expr);
    }

    (*logical_or_expr) = create_logical_or_expr_ast(logical_and_exprs, logical_and_exprs_len);

    return PARSER_OK;

 err1:
    {
        size_t i;
        for (i = 0; i < logical_and_exprs_len; i++) {
            logical_and_expr_ast_free(logical_and_exprs[i]);
        }
        SAFE_FREE(logical_and_exprs);
    }
 err0:
    (*logical_or_expr) = NULL;
    return r;
}

static enum PARSER_CODES property_ast_read(struct PARSER*parser, struct PROPERTY_AST**property)
{
    enum PARSER_CODES r;
    struct IDENT_AST*ident;
    struct ASSIGNMENT_EXPR_AST*assignment_expr;

    r = ident_ast_read(parser, &ident);
    if (r != PARSER_OK) {
        goto err0;
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_COLON) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_COLON);
        goto err1;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = assignment_expr_ast_read(parser, &assignment_expr);
    if (r != PARSER_OK) {
        goto err1;
    }

    (*property) = create_property_ast(ident, assignment_expr);

    return PARSER_OK;

 err1:
    ident_ast_free(ident);
 err0:
    (*property) = NULL;
    return r;
}

static enum PARSER_CODES object_literal_ast_read(struct PARSER*parser, struct OBJECT_LITERAL_AST**object_literal)
{
    enum PARSER_CODES r;
    struct PROPERTY_AST**properties = NULL;
    size_t properties_len = 0;
    size_t properties_cap = 0;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_LBRACE) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LBRACE);
        goto err0;
    }    
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    while ((parser->tok)->token_type != TOKEN_TYPE_RBRACE) {
        struct PROPERTY_AST*property;
        r = property_ast_read(parser, &property);
        if (r != PARSER_OK) {
            goto err1;
        }
        PUSH_BACK(properties, property);

        if ((parser->tok)->token_type == TOKEN_TYPE_RBRACE) {
            break;
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_COMMA) {
            r = PARSER_INVALID_TOKEN;
            set_parser_error(parser, 1, TOKEN_TYPE_COMMA);
            goto err1;
        }
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
    }
    
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*object_literal) = create_object_literal_ast(properties, properties_len);

    return PARSER_OK;

 err1:
    if (properties != NULL) {
        size_t i;
        for (i = 0; i < properties_len; i++) {
            property_ast_free(properties[i]);
        }
        SAFE_FREE(properties);
    }
 err0:
    (*object_literal) = NULL;
    return r;
}

static enum PARSER_CODES array_literal_ast_read(struct PARSER*parser, struct ARRAY_LITERAL_AST**array_literal)
{
    enum PARSER_CODES r;

    struct ARGS_LIST_AST*args_list = NULL;

    if ((parser->tok)->token_type != TOKEN_TYPE_LBRACKET) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LBRACKET);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_RBRACKET) {
        r = args_list_ast_read(parser, &args_list);
        if (r != PARSER_OK) {
            goto err0;
        }
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_RBRACKET) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_RBRACKET);
        goto err1;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));    
    
    (*array_literal) = create_array_literal_ast(args_list);

    return PARSER_OK;

 err1:
    if (args_list != NULL) {
        args_list_ast_free(args_list);
    }
 err0:
    (*array_literal) = NULL;
    return r;
}

static enum PARSER_CODES assignment_expr_ast_read(struct PARSER*parser, struct ASSIGNMENT_EXPR_AST**assignment_expr)
{
    enum PARSER_CODES r;

    switch ((parser->tok)->token_type) {
    case TOKEN_TYPE_LBRACE: {
        struct OBJECT_LITERAL_AST*object_literal;
        r = object_literal_ast_read(parser, &object_literal);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*assignment_expr) = create_assignment_expr_ast(object_literal, AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL);
        break;
    }
    case TOKEN_TYPE_LBRACKET: {
        struct ARRAY_LITERAL_AST*array_literal;
        r = array_literal_ast_read(parser, &array_literal);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*assignment_expr) = create_assignment_expr_ast(array_literal, AST_ASSIGNMENT_EXPR_TYPE_ARRAY_LITERAL);
        break;
    }
    default: {
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;
        r = logical_or_expr_ast_read(parser, &logical_or_expr);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*assignment_expr) = create_assignment_expr_ast(logical_or_expr, AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR);
        break;
    }
    }

    return PARSER_OK;

 err0:
    (*assignment_expr) = NULL;
    return r;
}

static enum PARSER_CODES decl_stmt_ast_read(struct PARSER*parser, struct DECL_STMT_AST**decl_stmt)
{
    enum PARSER_CODES r;
    struct IDENT_AST*var_name;
    struct ASSIGNMENT_EXPR_AST*assignment_expr;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_LET) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LET);
        goto err0;
    }    
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = ident_ast_read(parser, &var_name);
    if (r != PARSER_OK) {
        goto err0;
    }
    if ((parser->tok)->token_type != TOKEN_TYPE_EQ) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_EQ);
        goto err1;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = assignment_expr_ast_read(parser, &assignment_expr);
    if (r != PARSER_OK) {
        goto err1;
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_SEMI) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_SEMI);
        goto err2;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*decl_stmt) = create_decl_stmt_ast(var_name, assignment_expr);

    return PARSER_OK;

 err2:
    assignment_expr_ast_free(assignment_expr);
 err1:
    ident_ast_free(var_name);
 err0:
    (*decl_stmt) = NULL;
    return r;
}

/*
static enum PARSER_CODES function_call_stmt_ast_read(struct FUNCTION_CALL_STMT_AST**function_call_stmt, lexer_type_t lexer, struct TOKEN*parser->tok)
{
    
}
*/

static enum PARSER_CODES body_ast_read(struct PARSER*parser, struct BODY_AST**body);

static enum PARSER_CODES if_stmt_ast_read(struct PARSER*parser, struct IF_STMT_AST**if_stmt)
{
    enum PARSER_CODES r;
    struct LOGICAL_OR_EXPR_AST*condition;
    struct BODY_AST*if_body;
    struct BODY_AST*else_body;
    struct IF_STMT_AST*if_stmt_inner;

    if ((parser->tok)->token_type != TOKEN_TYPE_IF) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_IF);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_LPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LPAREN);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = logical_or_expr_ast_read(parser, &condition);
    if (r != PARSER_OK) {
        goto err0;
    }
    if ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_RPAREN);
        goto err1;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = body_ast_read(parser, &if_body);
    if (r != PARSER_OK) {
        goto err1;   
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_ELSE) {
        (*if_stmt) = create_if_stmt_ast(condition, if_body, NULL);
        return PARSER_OK;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type == TOKEN_TYPE_LBRACE) {
        r = body_ast_read(parser, &else_body);
        if (r != PARSER_OK) {
            goto err2;
        }
        (*if_stmt) = create_if_stmt_ast(condition, if_body, else_body);
        return PARSER_OK;
    } else if ((parser->tok)->token_type == TOKEN_TYPE_IF) {
        struct STMT_AST**stmts = NULL;
        size_t stmts_len = 0;
        size_t stmts_cap = 0;

        struct STMT_AST*stmt;
        
        r = if_stmt_ast_read(parser, &if_stmt_inner);
        if (r != PARSER_OK) {
            goto err2;
        }

        stmt = create_stmt_ast(if_stmt_inner, AST_STMT_TYPE_IF);
        PUSH_BACK(stmts, stmt);
        else_body = create_body_ast(stmts, stmts_len);

        (*if_stmt) = create_if_stmt_ast(condition, if_body, else_body);

        return PARSER_OK;
    }

 err2:
    body_ast_free(if_body);
 err1:
    logical_or_expr_ast_free(condition);
 err0:
    (*if_stmt) = NULL;
    return r;
}

static enum PARSER_CODES while_stmt_ast_read(struct PARSER*parser, struct WHILE_STMT_AST**while_stmt)
{
    enum PARSER_CODES r;
    struct LOGICAL_OR_EXPR_AST*condition;
    struct BODY_AST*body;

    if ((parser->tok)->token_type != TOKEN_TYPE_WHILE) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_WHILE);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_LPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LPAREN);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = logical_or_expr_ast_read(parser, &condition);
    if (r != PARSER_OK) {
        goto err0;
    }
    if ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_RPAREN);
        goto err1;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = body_ast_read(parser, &body);
    if (r != PARSER_OK) {
        goto err1;
    }

    (*while_stmt) = create_while_stmt_ast(condition, body);

    return PARSER_OK;

 err1:
    logical_or_expr_ast_free(condition);
 err0:
    (*while_stmt) = NULL;
    return r;
}

static enum PARSER_CODES break_stmt_ast_read(struct PARSER*parser, struct BREAK_STMT_AST**break_stmt)
{
    enum PARSER_CODES r;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_BREAK) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_BREAK);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_SEMI) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_SEMI);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*break_stmt) = create_break_stmt_ast();

    return PARSER_OK;

 err0:
    (*break_stmt) = NULL;
    return r;
}

static enum PARSER_CODES continue_stmt_ast_read(struct PARSER*parser, struct CONTINUE_STMT_AST**continue_stmt)
{
    enum PARSER_CODES r;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_CONTINUE) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_CONTINUE);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_SEMI) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_SEMI);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*continue_stmt) = create_continue_stmt_ast();

    return PARSER_OK;

 err0:
    (*continue_stmt) = NULL;
    return r;
}

static enum PARSER_CODES return_stmt_ast_read(struct PARSER*parser, struct RETURN_STMT_AST**return_stmt)
{
    enum PARSER_CODES r;    
    struct ASSIGNMENT_EXPR_AST*assignment_expr;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_RETURN) {
        
    }    
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type == TOKEN_TYPE_SEMI) {
        assignment_expr = NULL;
    } else {
        r = assignment_expr_ast_read(parser, &assignment_expr);
        if (r != PARSER_OK) {
            goto err0;
        }
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_SEMI) {
        r = PARSER_INVALID_TOKEN;
        goto err1;
    }

    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*return_stmt) = create_return_stmt_ast(assignment_expr);

    return PARSER_OK;

 err1:
    if (assignment_expr != NULL) {
        assignment_expr_ast_free(assignment_expr);
    }
 err0:
    (*return_stmt) = NULL;
    return r;
}

static enum PARSER_CODES stmt_ast_read(struct PARSER*parser, struct STMT_AST**stmt)
{
    enum PARSER_CODES r;
    
    switch ((parser->tok)->token_type) {
    case TOKEN_TYPE_LET: {
        struct DECL_STMT_AST*decl_stmt;
        r = decl_stmt_ast_read(parser, &decl_stmt);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(decl_stmt, AST_STMT_TYPE_DECL);
        break;
    }
    case TOKEN_TYPE_IDENT: {
        struct IDENT_AST*ident = create_ident_ast((parser->tok)->str_val);
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
        /* check, if name is part of variable or name is function name. */
        switch ((parser->tok)->token_type) {
        case TOKEN_TYPE_LPAREN: {
            struct FUNCTION_CALL_AST*function_call;    
            struct FUNCTION_CALL_STMT_AST*function_call_stmt;

            r = function_call_ast_read(parser, &function_call, ident);
            if (r != PARSER_OK) {
                ident_ast_free(ident);
                goto err0;
            }

            function_call_stmt = create_function_call_stmt_ast(function_call);
            (*stmt) = create_stmt_ast(function_call_stmt, AST_STMT_TYPE_FUNCTION_CALL);
            break;
        }
        default: {
            struct ASSIGN_STMT_AST*assign_stmt;
            struct VARIABLE_AST*var_name;
            struct ASSIGNMENT_EXPR_AST*assignment_expr;

            r = variable_ast_read(parser, &var_name, ident);
            if (r != PARSER_OK) {
                ident_ast_free(ident);
                goto err0;
            }

            if ((parser->tok)->token_type != TOKEN_TYPE_EQ) {
                ident_ast_free(ident);
                variable_ast_free(var_name);
                r = PARSER_INVALID_TOKEN;
                set_parser_error(parser, 1, TOKEN_TYPE_EQ);
                goto err0;
            }            
            token_free((parser->tok));
            lexer_next_token(parser->lexer, &(parser->tok));

            /* reading assignment expr. */
            r = assignment_expr_ast_read(parser, &assignment_expr);
            if (r != PARSER_OK) {
                ident_ast_free(ident);
                variable_ast_free(var_name);
                goto err0;
            }

            assign_stmt = create_assign_stmt_ast(var_name, assignment_expr);
            (*stmt) = create_stmt_ast(assign_stmt, AST_STMT_TYPE_ASSIGN);
        }
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_SEMI) {
            r = PARSER_INVALID_TOKEN;
            set_parser_error(parser, 1, TOKEN_TYPE_SEMI);
            goto err0;
        }
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
        
        break;
    }
    case TOKEN_TYPE_IF: {
        struct IF_STMT_AST*if_stmt;
        r = if_stmt_ast_read(parser, &if_stmt);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(if_stmt, AST_STMT_TYPE_IF);
        break;
    }
    case TOKEN_TYPE_WHILE: {
        struct WHILE_STMT_AST*while_stmt;
        r = while_stmt_ast_read(parser, &while_stmt);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(while_stmt, AST_STMT_TYPE_WHILE);
        break;
    }
    case TOKEN_TYPE_BREAK: {
        struct BREAK_STMT_AST*break_stmt;
        r = break_stmt_ast_read(parser, &break_stmt);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(break_stmt, AST_STMT_TYPE_BREAK);
        break;
    }
    case TOKEN_TYPE_CONTINUE: {
        struct CONTINUE_STMT_AST*continue_stmt;
        r = continue_stmt_ast_read(parser, &continue_stmt);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(continue_stmt, AST_STMT_TYPE_CONTINUE);        
        break;
    }
    case TOKEN_TYPE_RETURN: {
        struct RETURN_STMT_AST*return_stmt;
        r = return_stmt_ast_read(parser, &return_stmt);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(return_stmt, AST_STMT_TYPE_RETURN);
        break;
    }
    default: {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 7, TOKEN_TYPE_LET, TOKEN_TYPE_IDENT, TOKEN_TYPE_IF,
                         TOKEN_TYPE_WHILE, TOKEN_TYPE_BREAK, TOKEN_TYPE_CONTINUE,
                         TOKEN_TYPE_RETURN);
        goto err0;
        break;
    }
    }

    return PARSER_OK;

 err0:
    (*stmt) = NULL;
    return r;
}

static enum PARSER_CODES body_ast_read(struct PARSER*parser, struct BODY_AST**body)
{
    enum PARSER_CODES r;
    struct STMT_AST**stmts = NULL;
    size_t stmts_len = 0;
    size_t stmts_cap = 0;

    if ((parser->tok)->token_type != TOKEN_TYPE_LBRACE) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LBRACE);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    while ((parser->tok)->token_type != TOKEN_TYPE_RBRACE) {
        struct STMT_AST*stmt;
        r = stmt_ast_read(parser, &stmt);
        if (r != PARSER_OK) {
            goto err1;
        }
        PUSH_BACK(stmts, stmt);
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_RBRACE) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_RBRACE);
        goto err1;
    }    
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*body) = create_body_ast(stmts, stmts_len);

    return PARSER_OK;

 err1:
    if (stmts != NULL) {
        size_t i;
        for (i = 0; i < stmts_len; i++) {
            stmt_ast_free(stmts[i]);
        }
        SAFE_FREE(stmts);
    }
 err0:
    (*body) = NULL;
    return r;
}

static enum PARSER_CODES function_decl_ast_read(struct PARSER*parser, struct FUNCTION_DECL_AST**function)
{
    enum PARSER_CODES r;
    struct IDENT_AST*function_name;
    struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list = NULL;
    struct BODY_AST*body;

    if ((parser->tok)->token_type != TOKEN_TYPE_FUNCTION) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_FUNCTION);
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = ident_ast_read(parser, &function_name);
    if (r != PARSER_OK) {
        goto err0;
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_LPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_LPAREN);
        goto err1;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        r = formal_parameters_list_ast_read(parser, &formal_parameters_list);
        if (r != PARSER_OK) {
            goto err1;
        }
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        r = PARSER_INVALID_TOKEN;
        set_parser_error(parser, 1, TOKEN_TYPE_RPAREN);
        goto err2;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));
    
    r = body_ast_read(parser, &body);
    if (r != PARSER_OK) {
        goto err2;
    }

    (*function) = create_function_decl_ast(function_name, formal_parameters_list, body);

    return PARSER_OK;

 err2:
    if (formal_parameters_list != NULL) {
        formal_parameters_list_ast_free(formal_parameters_list);
    }
 err1:
    ident_ast_free(function_name);
 err0:
    (*function) = NULL;
    return r;
}

static enum PARSER_CODES unit_ast_read(struct PARSER*parser, struct UNIT_AST**unit)
{
    enum PARSER_CODES r;

    struct FUNCTION_DECL_AST**functions = NULL;
    size_t functions_len = 0;
    size_t functions_cap = 0;
    
    while ((parser->tok)->token_type != TOKEN_TYPE_EOF) {
        struct FUNCTION_DECL_AST*function;
        r = function_decl_ast_read(parser, &function);
        if (r != PARSER_OK) {
            goto err0;
        }
        PUSH_BACK(functions, function);
    }

    (*unit) = create_unit_ast(functions, functions_len);

    return PARSER_OK;
    
 err0:
    if (functions != NULL) {
        size_t i;
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
    enum PARSER_CODES r;
    lexer_next_token(parser->lexer, &(parser->tok));
    r = unit_ast_read(parser, unit);
    token_free(parser->tok);
    return r;
}

void parser_free(parser_type_t parser)
{
    SAFE_FREE(parser);
}

struct PARSER_ERROR parser_get_error(const parser_type_t parser)
{
    return parser->err;
}

void token_type_to_str(enum TOKEN_TYPE type, char*buf, size_t buflen)
{
    switch (type) {
    case TOKEN_TYPE_FUNCTION:
        strncpy(buf, "FUNCTION", buflen);
        break;
    case TOKEN_TYPE_LET:
        strncpy(buf, "LET", buflen);
        break;
    case TOKEN_TYPE_IF:
        strncpy(buf, "IF", buflen);
        break;
    case TOKEN_TYPE_ELSE:
        strncpy(buf, "ELSE", buflen);
        break;
    case TOKEN_TYPE_WHILE:
        strncpy(buf, "WHILE", buflen);
        break;
    case TOKEN_TYPE_BREAK:
        strncpy(buf, "BREAK", buflen);
        break;
    case TOKEN_TYPE_CONTINUE:
        strncpy(buf, "CONTINUE", buflen);
        break;
    case TOKEN_TYPE_APPEND:
        strncpy(buf, "APPEND", buflen);
        break;
    case TOKEN_TYPE_DELETE:
        strncpy(buf, "DELETE", buflen);
        break;        
    case TOKEN_TYPE_RETURN:
        strncpy(buf, "RETURN", buflen);
        break;
    case TOKEN_TYPE_IDENT:
        strncpy(buf, "IDENT", buflen);
        break;
    case TOKEN_TYPE_OR:
        strncpy(buf, "OR", buflen);
        break;
    case TOKEN_TYPE_AND:
        strncpy(buf, "AND", buflen);
        break;
    case TOKEN_TYPE_EQEQ:
        strncpy(buf, "EQEQ", buflen);
        break;
    case TOKEN_TYPE_NEQ:
        strncpy(buf, "NEQ", buflen);
        break;
    case TOKEN_TYPE_LT:
        strncpy(buf, "LT", buflen);
        break;
    case TOKEN_TYPE_GT:
        strncpy(buf, "GT", buflen);
        break;
    case TOKEN_TYPE_LE:
        strncpy(buf, "LE", buflen);
        break;
    case TOKEN_TYPE_GE:
        strncpy(buf, "GE", buflen);
        break;
    case TOKEN_TYPE_EQ:
        strncpy(buf, "EQ", buflen);
        break;
    case TOKEN_TYPE_PLUS:
        strncpy(buf, "PLUS", buflen);
        break;
    case TOKEN_TYPE_MINUS:
        strncpy(buf, "MINUS", buflen);
        break;
    case TOKEN_TYPE_MUL:
        strncpy(buf, "MUL", buflen);
        break;
    case TOKEN_TYPE_DIV:
        strncpy(buf, "DIV", buflen);
        break;
    case TOKEN_TYPE_MOD:
        strncpy(buf, "MOD", buflen);
        break;
    case TOKEN_TYPE_LPAREN:
        strncpy(buf, "LPAREN", buflen);
        break;
    case TOKEN_TYPE_RPAREN:
        strncpy(buf, "RPAREN", buflen);
        break;
    case TOKEN_TYPE_NUMBER:
        strncpy(buf, "NUMBER", buflen);
        break;
    case TOKEN_TYPE_LBRACKET:
        strncpy(buf, "LBRACKET", buflen);
        break;
    case TOKEN_TYPE_RBRACKET:
        strncpy(buf, "RBRACKET", buflen);
        break;        
    case TOKEN_TYPE_LBRACE:
        strncpy(buf, "LBRACE", buflen);
        break;
    case TOKEN_TYPE_RBRACE:
        strncpy(buf, "RBRACE", buflen);
        break;
    case TOKEN_TYPE_COMMA:
        strncpy(buf, "COMMA", buflen);
        break;
    case TOKEN_TYPE_SEMI:
        strncpy(buf, "SEMI", buflen);
        break;
    case TOKEN_TYPE_DOT:
        strncpy(buf, "DOT", buflen);
        break;
    case TOKEN_TYPE_COLON:
        strncpy(buf, "COLON", buflen);
        break;
    case TOKEN_TYPE_EOF:
        strncpy(buf, "EOF", buflen);
        break;
    case TOKEN_TYPE_UNKNOWN:
        strncpy(buf, "UNKNOWN", buflen);
        break;        
    }
}

void print_parser_error(const struct PARSER_ERROR*err)
{
    size_t i;
    
    char tmp[128];

    token_type_to_str(err->get_tok, tmp, sizeof(tmp));
    fprintf(stderr, "%zu:%zu: error: invalid token: \"%s\"; expected tokens: ",
            err->line, err->pos, tmp);

    i = 0;
    while (err->exp_toks[i] != TOKEN_TYPE_EOF) {
        token_type_to_str(err->exp_toks[i], tmp, sizeof(tmp));        
        fprintf(stderr, "\"%s\" ", tmp);
        i++;
    }

    fprintf(stderr, "\n");
}

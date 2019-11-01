#include "parser.h"

#include "utils.h"

struct PARSER
{
    lexer_type_t lexer;
    struct TOKEN*tok;
    
    unsigned err_line;
    unsigned err_pos;
    unsigned err_index;

    enum TOKEN_TYPE exp_tok;
    enum TOKEN_TYPE get_tok;
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

static enum PARSER_CODES ident_ast_read(struct PARSER*parser, struct IDENT_AST**ident)
{
    enum PARSER_CODES r;
    
    if ((parser->tok)->token_type != TOKEN_TYPE_IDENT) {
        r = PARSER_INVALID_TOKEN;
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

static enum PARSER_CODES variable_ast_read(struct PARSER*parser, struct VARIABLE_AST**variable, struct IDENT_AST*ident)
{
    enum PARSER_CODES r;
    
    struct IDENT_AST**idents = NULL;
    unsigned idents_len = 0;
    unsigned idents_cap = 0;
    int is_dot = 1;

    /* reading variable. */
    PUSH_BACK(idents, ident);
    while (1) {
        if (is_dot && ((parser->tok)->token_type == TOKEN_TYPE_DOT)) {
            token_free((parser->tok));
            lexer_next_token(parser->lexer, &(parser->tok));
            is_dot = 0;
        } else if (!is_dot && ((parser->tok)->token_type == TOKEN_TYPE_IDENT)) {
            PUSH_BACK(idents, create_ident_ast((parser->tok)->str_val));
            token_free((parser->tok));
            lexer_next_token(parser->lexer, &(parser->tok));
            is_dot = 1;
        } else {
            break;
        }
    }
    (*variable) = create_variable_ast(idents, idents_len);

    return PARSER_OK;
}

static enum PARSER_CODES formal_parameters_list_ast_read(struct PARSER*parser, struct FORMAL_PARAMETERS_LIST_AST**formal_parameters_list)
{
    enum PARSER_CODES r;
    struct IDENT_AST**idents = NULL;
    unsigned idents_len = 0;
    unsigned idents_cap = 0;

    /* no check, because it could be only TOKEN_TYPE_LPAREN. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));
    
    while ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        struct IDENT_AST*ident;
        r = ident_ast_read(parser, &ident);
        if (r != PARSER_OK) {
            
        }
        PUSH_BACK(idents, ident);

        if ((parser->tok)->token_type == TOKEN_TYPE_RPAREN) {
            break;
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_COMMA) {
            r = PARSER_INVALID_TOKEN;
        }
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
    }

    /* no check, because it could be only TOKEN_TYPE_RPAREN. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*formal_parameters_list) = create_formal_parameters_list_ast(idents, idents_len);

    return PARSER_OK;
}

static enum PARSER_CODES assignment_expr_ast_read(struct PARSER*parser, struct ASSIGNMENT_EXPR_AST**assignment_expr);

static enum PARSER_CODES number_ast_read(struct PARSER*parser, struct NUMBER_AST**number)
{
    /* no check, because it could be only TOKEN_TYPE_NUMBER. */
    (*number) = create_number_ast((parser->tok)->int_val);

    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    return PARSER_OK;
}

static enum PARSER_CODES args_list_ast_read(struct PARSER*parser, struct ARGS_LIST_AST**args_list)
{
    enum PARSER_CODES r;
    struct ASSIGNMENT_EXPR_AST**assignment_exprs = NULL;
    unsigned assignment_exprs_len = 0;
    unsigned assignment_exprs_cap = 0;

    /* no check, because it could be only TOKEN_TYPE_LPAREN. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));
    
    while ((parser->tok)->token_type != TOKEN_TYPE_RPAREN) {
        struct ASSIGNMENT_EXPR_AST*assignment_expr;
        r = assignment_expr_ast_read(parser, &assignment_expr);
        if (r != PARSER_OK) {
            
        }

        PUSH_BACK(assignment_exprs, assignment_expr);

        if ((parser->tok)->token_type == TOKEN_TYPE_RPAREN) {
            break;
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_COMMA) {
            
        }
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
    }

    /* no check, because it could be only TOKEN_TYPE_RPAREN. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*args_list) = create_args_list_ast(assignment_exprs, assignment_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES function_call_ast_read(struct PARSER*parser, struct FUNCTION_CALL_AST**function_call, struct IDENT_AST*function_name)
{
    enum PARSER_CODES r;

    struct ARGS_LIST_AST*args_list;
    
    r = args_list_ast_read(parser, &args_list);
    if (r != PARSER_OK) {
        
    }
    
    (*function_call) = create_function_call_ast(function_name, args_list);

    return PARSER_OK;
}

static enum PARSER_CODES logical_or_expr_ast_read(struct PARSER*parser, struct LOGICAL_OR_EXPR_AST**logical_or_expr);

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
                
            }
            
            (*primary_expr) = create_primary_expr_ast(function_call, AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL);
            
            break;
        }
        default: {
            struct VARIABLE_AST*var_name;
            r = variable_ast_read(parser, &var_name, ident);
            if (r != PARSER_OK) {
                
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
            
        }

        /* no check, because it could be only TOKEN_TYPE_RPAREN. */
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        (*primary_expr) = create_primary_expr_ast(logical_or_expr, AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR);
        
        break;
    }
    default: {
        
        break;
    }
    }

    return PARSER_OK;
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
        
    }

    (*left_unary_expr) = create_left_unary_expr_ast(op, primary_expr);

    return PARSER_OK;
}

static enum PARSER_CODES multiplicative_expr_ast_read(struct PARSER*parser, struct MULTIPLICATIVE_EXPR_AST**multiplicative_expr)
{
    enum PARSER_CODES r;

    struct LEFT_UNARY_EXPR_AST**left_unary_exprs = NULL;
    unsigned left_unary_exprs_len = 0;
    unsigned left_unary_exprs_cap = 0;

    enum AST_MULTIPLICATIVE_OP*ops = NULL;
    unsigned ops_len = 0;
    unsigned ops_cap = 0;
    
    struct LEFT_UNARY_EXPR_AST*left_unary_expr;

    r = left_unary_expr_ast_read(parser, &left_unary_expr);
    if (r != PARSER_OK) {

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
            
        }

        PUSH_BACK(left_unary_exprs, left_unary_expr);
    }

    (*multiplicative_expr) = create_multiplicative_expr_ast(left_unary_exprs, ops, left_unary_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES additive_expr_ast_read(struct PARSER*parser, struct ADDITIVE_EXPR_AST**additive_expr)
{
    enum PARSER_CODES r;

    struct MULTIPLICATIVE_EXPR_AST**multiplicative_exprs = NULL;
    unsigned multiplicative_exprs_len = 0;
    unsigned multiplicative_exprs_cap = 0;

    enum AST_ADDITIVE_OP*ops = NULL;
    unsigned ops_len = 0;
    unsigned ops_cap = 0;
    
    struct MULTIPLICATIVE_EXPR_AST*multiplicative_expr;

    r = multiplicative_expr_ast_read(parser, &multiplicative_expr);
    if (r != PARSER_OK) {

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
            
        }

        PUSH_BACK(multiplicative_exprs, multiplicative_expr);
    }

    (*additive_expr) = create_additive_expr_ast(multiplicative_exprs, ops, multiplicative_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES relational_expr_ast_read(struct PARSER*parser, struct RELATIONAL_EXPR_AST**relational_expr)
{
    enum PARSER_CODES r;

    struct ADDITIVE_EXPR_AST*left = NULL;
    enum AST_REL_OP rel_op = AST_REL_OP_LT;
    struct ADDITIVE_EXPR_AST*right = NULL;

    r = additive_expr_ast_read(parser, &left);
    if (r != PARSER_OK) {
        
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
            
        }
    }

    (*relational_expr) = create_relational_expr_ast(left, rel_op, right);

    return PARSER_OK;
}

static enum PARSER_CODES eq_expr_ast_read(struct PARSER*parser, struct EQ_EXPR_AST**eq_expr)
{
    enum PARSER_CODES r;

    struct RELATIONAL_EXPR_AST*left = NULL;
    enum AST_EQ_OP eq_op = AST_EQ_OP_EQEQ;
    struct RELATIONAL_EXPR_AST*right = NULL;

    r = relational_expr_ast_read(parser, &left);
    if (r != PARSER_OK) {
        
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
            
        }
    }

    (*eq_expr) = create_eq_expr_ast(left, eq_op, right);

    return PARSER_OK;
}

static enum PARSER_CODES logical_and_expr_ast_read(struct PARSER*parser, struct LOGICAL_AND_EXPR_AST**logical_and_expr)
{
    enum PARSER_CODES r;

    struct EQ_EXPR_AST**eq_exprs = NULL;
    unsigned eq_exprs_len = 0;
    unsigned eq_exprs_cap = 0;
    
    struct EQ_EXPR_AST*eq_expr;


    r = eq_expr_ast_read(parser, &eq_expr);
    if (r != PARSER_OK) {

    }

    PUSH_BACK(eq_exprs, eq_expr);

    while ((parser->tok)->token_type == TOKEN_TYPE_AND) {
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = eq_expr_ast_read(parser, &eq_expr);
        if (r != PARSER_OK) {
            
        }

        PUSH_BACK(eq_exprs, eq_expr);
    }

    (*logical_and_expr) = create_logical_and_expr_ast(eq_exprs, eq_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES logical_or_expr_ast_read(struct PARSER*parser, struct LOGICAL_OR_EXPR_AST**logical_or_expr)
{
    enum PARSER_CODES r;

    struct LOGICAL_AND_EXPR_AST**logical_and_exprs = NULL;
    unsigned logical_and_exprs_len = 0;
    unsigned logical_and_exprs_cap = 0;
    
    struct LOGICAL_AND_EXPR_AST*logical_and_expr;


    r = logical_and_expr_ast_read(parser, &logical_and_expr);
    if (r != PARSER_OK) {

    }

    PUSH_BACK(logical_and_exprs, logical_and_expr);

    while ((parser->tok)->token_type == TOKEN_TYPE_OR) {
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));

        r = logical_and_expr_ast_read(parser, &logical_and_expr);
        if (r != PARSER_OK) {
            
        }

        PUSH_BACK(logical_and_exprs, logical_and_expr);
    }

    (*logical_or_expr) = create_logical_or_expr_ast(logical_and_exprs, logical_and_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES property_ast_read(struct PARSER*parser, struct PROPERTY_AST**property)
{
    enum PARSER_CODES r;
    struct IDENT_AST*ident;
    struct ASSIGNMENT_EXPR_AST*assignment_expr;

    r = ident_ast_read(parser, &ident);
    if (r != PARSER_OK) {
        
    }

    if ((parser->tok)->token_type != TOKEN_TYPE_COLON) {
        
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = assignment_expr_ast_read(parser, &assignment_expr);
    if (r != PARSER_OK) {
        
    }

    (*property) = create_property_ast(ident, assignment_expr);

    return PARSER_OK;
}

static enum PARSER_CODES object_literal_ast_read(struct PARSER*parser, struct OBJECT_LITERAL_AST**object_literal)
{
    enum PARSER_CODES r;
    struct PROPERTY_AST**properties = NULL;
    unsigned properties_len = 0;
    unsigned properties_cap = 0;
    
    /* no check, because it could be only TOKEN_TYPE_LBRACE. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    while ((parser->tok)->token_type != TOKEN_TYPE_RBRACE) {
        struct PROPERTY_AST*property;
        r = property_ast_read(parser, &property);
        if (r != PARSER_OK) {
            goto err0;
        }
        PUSH_BACK(properties, property);

        if ((parser->tok)->token_type == TOKEN_TYPE_RBRACE) {
            break;
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_COMMA) {
            
        }
        token_free((parser->tok));
        lexer_next_token(parser->lexer, &(parser->tok));
    }

    /* no check, because it could be only TOKEN_TYPE_RBRACE. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*object_literal) = create_object_literal_ast(properties, properties_len);

    return PARSER_OK;

 err0:
    if (properties != NULL) {
        unsigned i;
        for (i = 0; i < properties_len; i++) {
            property_ast_free(properties[i]);
        }
        SAFE_FREE(properties);
    }

    (*object_literal) = NULL;
    return r;
}

static enum PARSER_CODES assignment_expr_ast_read(struct PARSER*parser, struct ASSIGNMENT_EXPR_AST**assignment_expr)
{
    enum PARSER_CODES r;
    
    if ((parser->tok)->token_type == TOKEN_TYPE_LBRACE) {
        struct OBJECT_LITERAL_AST*object_literal;
        r = object_literal_ast_read(parser, &object_literal);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*assignment_expr) = create_assignment_expr_ast(object_literal, AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL);
    } else {
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;
        r = logical_or_expr_ast_read(parser, &logical_or_expr);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*assignment_expr) = create_assignment_expr_ast(logical_or_expr, AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR);        
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
    
    /* no check, because it could be only TOKEN_TYPE_LET. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = ident_ast_read(parser, &var_name);
    if (r != PARSER_OK) {
        goto err0;
    }
    if ((parser->tok)->token_type != TOKEN_TYPE_EQ) {
        r = PARSER_INVALID_TOKEN;
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
static enum PARSER_CODES assign_stmt_ast_read(struct ASSIGN_STMT_AST**assign_stmt, lexer_type_t lexer, struct TOKEN*parser->tok)
{
    
}

static enum PARSER_CODES function_call_stmt_ast_read(struct FUNCTION_CALL_STMT_AST**function_call_stmt, lexer_type_t lexer, struct TOKEN*parser->tok)
{
    
}
*/

static enum PARSER_CODES body_ast_read(struct PARSER*parser, struct BODY_AST**body);

static enum PARSER_CODES if_stmt_ast_read(struct PARSER*parser, struct IF_STMT_AST**if_stmt)
{
    
}

static enum PARSER_CODES while_stmt_ast_read(struct PARSER*parser, struct WHILE_STMT_AST**while_stmt)
{
    enum PARSER_CODES r;
    struct LOGICAL_OR_EXPR_AST*condition;
    struct BODY_AST*body;

    /* no check, because it could be only TOKEN_TYPE_WHILE. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    if ((parser->tok)->token_type != TOKEN_TYPE_LPAREN) {
        r = PARSER_INVALID_TOKEN;
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

static enum PARSER_CODES return_stmt_ast_read(struct PARSER*parser, struct RETURN_STMT_AST**return_stmt)
{
    enum PARSER_CODES r;    
    struct ASSIGNMENT_EXPR_AST*assignment_expr;
    
    /* no check, because it could be only TOKEN_TYPE_RETURN. */
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
                
            }


            if ((parser->tok)->token_type != TOKEN_TYPE_EQ) {
                r = PARSER_INVALID_TOKEN;
                goto err0;
            }            
            token_free((parser->tok));
            lexer_next_token(parser->lexer, &(parser->tok));

            /* reading assignment expr. */
            r = assignment_expr_ast_read(parser, &assignment_expr);
            if (r != PARSER_OK) {
                variable_ast_free(var_name);
                goto err0;
            }

            assign_stmt = create_assign_stmt_ast(var_name, assignment_expr);
            (*stmt) = create_stmt_ast(assign_stmt, AST_STMT_TYPE_ASSIGN);
                        
        }
        }

        if ((parser->tok)->token_type != TOKEN_TYPE_SEMI) {
            r = PARSER_INVALID_TOKEN;
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
    unsigned stmts_len = 0;
    unsigned stmts_cap = 0;

    if ((parser->tok)->token_type != TOKEN_TYPE_LBRACE) {
        r = PARSER_INVALID_TOKEN;
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

    /* no check, because it could be only TOKEN_TYPE_RBRACE. */
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    (*body) = create_body_ast(stmts, stmts_len);

    return PARSER_OK;

 err1:
    if (stmts != NULL) {
        unsigned i;
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
    struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list;
    struct BODY_AST*body;

    if ((parser->tok)->token_type != TOKEN_TYPE_FUNCTION) {
        r = PARSER_INVALID_TOKEN;
        goto err0;
    }
    token_free((parser->tok));
    lexer_next_token(parser->lexer, &(parser->tok));

    r = ident_ast_read(parser, &function_name);
    if (r != PARSER_OK) {
        goto err0;
    }
    
    r = formal_parameters_list_ast_read(parser, &formal_parameters_list);
    if (r != PARSER_OK) {
        goto err1;
    }
    r = body_ast_read(parser, &body);
    if (r != PARSER_OK) {
        goto err2;
    }

    (*function) = create_function_decl_ast(function_name, formal_parameters_list, body);

    return PARSER_OK;

 err2:
    formal_parameters_list_ast_free(formal_parameters_list);
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
    unsigned functions_len = 0;
    unsigned functions_cap = 0;
    
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
    lexer_next_token(parser->lexer, &(parser->tok));
    return unit_ast_read(parser, unit);
}

void parser_free(parser_type_t parser)
{
    SAFE_FREE(parser);
}

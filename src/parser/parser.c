#include "parser.h"

struct PARSER
{
    lexer_type_t lexer;
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

static enum PARSER_CODES ident_ast_read(struct IDENT_AST**ident, lexer_type_t lexer, struct TOKEN**tok)
{
    /* no check, because it could be only TOKEN_TYPE_IDENT. */
    (*ident) = create_ident_ast((*tok)->str_val);

    token_free((*tok));
    lexer_next_token(lexer, tok);

    return PARSER_OK;
}

static enum PARSER_CODES variable_ast_read(struct VARIABLE_AST**variable, lexer_type_t lexer, struct TOKEN**tok, struct IDENT_AST*ident)
{
    struct IDENT_AST**idents = NULL;
    unsigned idents_len = 0;
    unsigned idents_cap = 0;
    int is_dot = 1;

    /* reading variable. */
    PUSH_BACK(idents, ident);
    while ((*tok)->token_type != TOKEN_TYPE_EQ) {
        if (is_dot && ((*tok)->token_type == TOKEN_TYPE_DOT)) {
            token_free((*tok));
            lexer_next_token(lexer, tok);
            is_dot = 0;
        } else if (!is_dot && ((*tok)->token_type == TOKEN_TYPE_IDENT)) {
            PUSH_BACK(idents, create_ident_ast((*tok)->str_val));
            token_free((*tok));
            lexer_next_token(lexer, tok);
            is_dot = 1;
        } else {
            unsigned i;
            for (i = 0; i < idents_len; i++) {
                ident_ast_free(idents[i]);
            }
            SAFE_FREE(idents);
            
        }
    }
    (*variable) = create_variable_ast(idents, idents_len);

    return PARSER_OK;
}

static enum PARSER_CODES formal_parameters_list_ast_read(struct FORMAL_PARAMETERS_LIST_AST**formal_parameters_list, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct IDENT_AST**idents = NULL;
    unsigned idents_len = 0;
    unsigned idents_cap = 0;

    /* no check, because it could be only TOKEN_TYPE_LPAREN. */
    token_free((*tok));
    lexer_next_token(lexer, tok);
    
    while ((*tok)->token_type != TOKEN_TYPE_RPAREN) {
        struct IDENT_AST*ident;
        r = ident_ast_read(&ident, lexer, tok);
        if (r != PARSER_OK) {
            
        }
        PUSH_BACK(idents, ident);

        if ((*tok)->token_type == TOKEN_TYPE_RPAREN) {
            break;
        }

        if ((*tok)->token_type != TOKEN_TYPE_COMMA) {
            r = PARSER_INVALID_TOKEN;
        }
        token_free((*tok));
        lexer_next_token(lexer, tok);
    }

    /* no check, because it could be only TOKEN_TYPE_RPAREN. */
    token_free((*tok));
    lexer_next_token(lexer, tok);

    (*formal_parameters_list) = create_formal_parameters_list_ast(idents, idents_len);

    return PARSER_OK;
}

static enum PARSER_CODES assignment_expr_ast_read(struct ASSIGNMENT_EXPR_AST**assignment_expr, lexer_type_t lexer, struct TOKEN**tok);

static enum PARSER_CODES number_ast_read(struct NUMBER_AST**number, lexer_type_t lexer, struct TOKEN**tok)
{
    /* no check, because it could be only TOKEN_TYPE_NUMBER. */
    (*number) = create_number_ast((*tok)->int_val);

    token_free((*tok));
    lexer_next_token(lexer, tok);

    return PARSER_OK;
}

static enum PARSER_CODES args_list_ast_read(struct ARGS_LIST_AST**args_list, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct ASSIGNMENT_EXPR_AST**assignment_exprs = NULL;
    unsigned assignment_exprs_len = 0;
    unsigned assignment_exprs_cap = 0;
    
    while ((*tok)->token_type != TOKEN_TYPE_RPAREN) {
        struct ASSIGNMENT_EXPR_AST*assignment_expr;
        r = assignment_expr_ast_read(&assignment_expr, lexer, tok);
        if (r != PARSER_OK) {
            
        }
        PUSH_BACK(assignment_exprs, assignment_expr);

        if ((*tok)->token_type != TOKEN_TYPE_COMMA) {
            r = PARSER_INVALID_TOKEN;
            
        }
        token_free((*tok));
        lexer_next_token(lexer, tok);
    }

    (*args_list) = create_args_list_ast(assignment_exprs, assignment_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES function_call_ast_read(struct FUNCTION_CALL_AST**function_call, lexer_type_t lexer, struct TOKEN**tok, struct IDENT_AST*function_name)
{
    enum PARSER_CODES r;

    struct ARGS_LIST_AST*args_list;
    
    r = args_list_ast_read(&args_list, lexer, tok);
    if (r != PARSER_OK) {
        
    }
    
    /* no check, because it could be only TOKEN_TYPE_RPAREN. */
    token_free((*tok));
    lexer_next_token(lexer, tok);
    
    (*function_call) = create_function_call_ast(function_name, args_list);

    return PARSER_OK;
}

static enum PARSER_CODES logical_or_expr_ast_read(struct LOGICAL_OR_EXPR_AST**logical_or_expr, lexer_type_t lexer, struct TOKEN**tok);

static enum PARSER_CODES primary_expr_ast_read(struct PRIMARY_EXPR_AST**primary_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    
    switch ((*tok)->token_type) {
    case TOKEN_TYPE_IDENT: {
        struct IDENT_AST*ident = create_ident_ast((*tok)->str_val);
        token_free((*tok));
        lexer_next_token(lexer, tok);

        /* check, if name is part of variable or name is function name. */
        switch ((*tok)->token_type) {
        case TOKEN_TYPE_DOT: {
            struct VARIABLE_AST*var_name;

            r = variable_ast_read(&var_name, lexer, tok, ident);
            if (r != PARSER_OK) {
                
            }

            (*primary_expr) = create_primary_expr_ast(var_name, PRIMARY_EXPR_TYPE_VARIABLE);
            
            break;
        }
        case TOKEN_TYPE_LPAREN: {
            struct FUNCTION_CALL_AST*function_call;

            r = function_call_ast_read(&function_call, lexer, tok, ident);
            if (r != PARSER_OK) {
                
            }
            
            (*primary_expr) = create_primary_expr_ast(function_call, PRIMARY_EXPR_TYPE_FUNCTION_CALL);
            
            break;
        }
        default: {
            r = PARSER_INVALID_TOKEN;
            
            break;
        }
        }        
        
        break;
    }
    case TOKEN_TYPE_NUMBER: {
        struct NUMBER_AST*number;
        r = number_ast_read(&number, lexer, tok);
        if (r != PARSER_OK) {
            
        }

        (*primary_expr) = create_primary_expr_ast(number, PRIMARY_EXPR_TYPE_NUMBER);
        
        break;
    }
    case TOKEN_TYPE_LPAREN: {
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;

        token_free((*tok));
        lexer_next_token(lexer, tok);

        r = logical_or_expr_ast_read(&logical_or_expr, lexer, tok);
        if (r != PARSER_OK) {
            
        }

        /* no check, because it could be only TOKEN_TYPE_RPAREN. */
        token_free((*tok));
        lexer_next_token(lexer, tok);

        (*primary_expr) = create_primary_expr_ast(logical_or_expr, PRIMARY_EXPR_TYPE_LOGICAL_EXPR);
        
        break;
    }
    default: {
        
        break;
    }
    }

    return PARSER_OK;
}

static enum PARSER_CODES left_unary_expr_ast_read(struct LEFT_UNARY_EXPR_AST**left_unary_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    enum LEFT_UNARY_EXPR_OP op = UNARY_PLUS;

    struct PRIMARY_EXPR_AST*primary_expr;
    
    if ((*tok)->token_type == TOKEN_TYPE_PLUS) {
        op = UNARY_PLUS;
        token_free((*tok));
        lexer_next_token(lexer, tok);
    } else if ((*tok)->token_type == TOKEN_TYPE_MINUS) {
        op = UNARY_MINUS;
        token_free((*tok));
        lexer_next_token(lexer, tok);        
    }

    r = primary_expr_ast_read(&primary_expr, lexer, tok);
    if (r != PARSER_OK) {
        
    }

    (*left_unary_expr) = create_left_unary_expr_ast(op, primary_expr);

    return PARSER_OK;
}

static enum PARSER_CODES multiplicative_expr_ast_read(struct MULTIPLICATIVE_EXPR_AST**multiplicative_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct LEFT_UNARY_EXPR_AST**left_unary_exprs = NULL;
    unsigned left_unary_exprs_len = 0;
    unsigned left_unary_exprs_cap = 0;

    enum MULTIPLICATIVE_OP*ops = NULL;
    unsigned ops_len = 0;
    unsigned ops_cap = 0;
    
    struct LEFT_UNARY_EXPR_AST*left_unary_expr;

    r = left_unary_expr_ast_read(&left_unary_expr, lexer, tok);
    if (r != PARSER_OK) {

    }

    PUSH_BACK(left_unary_exprs, left_unary_expr);

    while (((*tok)->token_type == TOKEN_TYPE_MUL) || ((*tok)->token_type == TOKEN_TYPE_DIV) || ((*tok)->token_type == TOKEN_TYPE_MOD)) {
        if ((*tok)->token_type == TOKEN_TYPE_MUL) {
            PUSH_BACK(ops, MUL);
        } else if ((*tok)->token_type == TOKEN_TYPE_DIV) {
            PUSH_BACK(ops, DIV);
        } else if ((*tok)->token_type == TOKEN_TYPE_MOD) {
            PUSH_BACK(ops, MOD);
        }
        
        token_free((*tok));
        lexer_next_token(lexer, tok);

        r = left_unary_expr_ast_read(&left_unary_expr, lexer, tok);
        if (r != PARSER_OK) {
            
        }

        PUSH_BACK(left_unary_exprs, left_unary_expr);
    }

    (*multiplicative_expr) = create_multiplicative_expr_ast(left_unary_exprs, ops, left_unary_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES additive_expr_ast_read(struct ADDITIVE_EXPR_AST**additive_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct MULTIPLICATIVE_EXPR_AST**multiplicative_exprs = NULL;
    unsigned multiplicative_exprs_len = 0;
    unsigned multiplicative_exprs_cap = 0;

    enum ADDITIVE_OP*ops = NULL;
    unsigned ops_len = 0;
    unsigned ops_cap = 0;
    
    struct MULTIPLICATIVE_EXPR_AST*multiplicative_expr;

    r = multiplicative_expr_ast_read(&multiplicative_expr, lexer, tok);
    if (r != PARSER_OK) {

    }

    PUSH_BACK(multiplicative_exprs, multiplicative_expr);

    while (((*tok)->token_type == TOKEN_TYPE_PLUS) || ((*tok)->token_type == TOKEN_TYPE_MINUS)) {
        if ((*tok)->token_type == TOKEN_TYPE_PLUS) {
            PUSH_BACK(ops, PLUS);
        } else if ((*tok)->token_type == TOKEN_TYPE_MINUS) {
            PUSH_BACK(ops, MINUS);
        }
        
        token_free((*tok));
        lexer_next_token(lexer, tok);

        r = multiplicative_expr_ast_read(&multiplicative_expr, lexer, tok);
        if (r != PARSER_OK) {
            
        }

        PUSH_BACK(multiplicative_exprs, multiplicative_expr);
    }

    (*additive_expr) = create_additive_expr_ast(multiplicative_exprs, ops, multiplicative_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES relational_expr_ast_read(struct RELATIONAL_EXPR_AST**relational_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct ADDITIVE_EXPR_AST*left = NULL;
    enum REL_OP rel_op = LT;
    struct ADDITIVE_EXPR_AST*right = NULL;

    r = additive_expr_ast_read(&left, lexer, tok);
    if (r != PARSER_OK) {
        
    }

    if (((*tok)->token_type == TOKEN_TYPE_LT) || ((*tok)->token_type == TOKEN_TYPE_GT) ||
        ((*tok)->token_type == TOKEN_TYPE_LE) || ((*tok)->token_type == TOKEN_TYPE_GE)) {
        if ((*tok)->token_type == TOKEN_TYPE_LT) {
            rel_op = LT;
        } else if ((*tok)->token_type == TOKEN_TYPE_GT) {
            rel_op = GT;
        } else if ((*tok)->token_type == TOKEN_TYPE_LE) {
            rel_op = LE;
        } else if ((*tok)->token_type == TOKEN_TYPE_GE) {
            rel_op = GE;
        }

        token_free((*tok));
        lexer_next_token(lexer, tok);        

        r = additive_expr_ast_read(&right, lexer, tok);
        if (r != PARSER_OK) {
            
        }
    }

    (*relational_expr) = create_relational_expr_ast(left, rel_op, right);

    return PARSER_OK;
}

static enum PARSER_CODES eq_expr_ast_read(struct EQ_EXPR_AST**eq_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct RELATIONAL_EXPR_AST*left = NULL;
    enum EQ_OP eq_op = EQEQ;
    struct RELATIONAL_EXPR_AST*right = NULL;

    r = relational_expr_ast_read(&left, lexer, tok);
    if (r != PARSER_OK) {
        
    }

    if (((*tok)->token_type == TOKEN_TYPE_EQEQ) || ((*tok)->token_type == TOKEN_TYPE_NEQ)) {
        if ((*tok)->token_type == TOKEN_TYPE_EQEQ) {
            eq_op = EQEQ;
        } else if ((*tok)->token_type == TOKEN_TYPE_NEQ) {
            eq_op = NEQ;
        }

        token_free((*tok));
        lexer_next_token(lexer, tok);

        r = relational_expr_ast_read(&right, lexer, tok);
        if (r != PARSER_OK) {
            
        }
    }

    (*eq_expr) = create_eq_expr_ast(left, eq_op, right);

    return PARSER_OK;
}

static enum PARSER_CODES logical_and_expr_ast_read(struct LOGICAL_AND_EXPR_AST**logical_and_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct EQ_EXPR_AST**eq_exprs = NULL;
    unsigned eq_exprs_len = 0;
    unsigned eq_exprs_cap = 0;
    
    struct EQ_EXPR_AST*eq_expr;


    r = eq_expr_ast_read(&eq_expr, lexer, tok);
    if (r != PARSER_OK) {

    }

    PUSH_BACK(eq_exprs, eq_expr);

    while ((*tok)->token_type == TOKEN_TYPE_AND) {
        token_free((*tok));
        lexer_next_token(lexer, tok);

        r = eq_expr_ast_read(&eq_expr, lexer, tok);
        if (r != PARSER_OK) {
            
        }

        PUSH_BACK(eq_exprs, eq_expr);
    }

    (*logical_and_expr) = create_logical_and_expr_ast(eq_exprs, eq_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES logical_or_expr_ast_read(struct LOGICAL_OR_EXPR_AST**logical_or_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct LOGICAL_AND_EXPR_AST**logical_and_exprs = NULL;
    unsigned logical_and_exprs_len = 0;
    unsigned logical_and_exprs_cap = 0;
    
    struct LOGICAL_AND_EXPR_AST*logical_and_expr;


    r = logical_and_expr_ast_read(&logical_and_expr, lexer, tok);
    if (r != PARSER_OK) {

    }

    PUSH_BACK(logical_and_exprs, logical_and_expr);

    while ((*tok)->token_type == TOKEN_TYPE_OR) {
        token_free((*tok));
        lexer_next_token(lexer, tok);

        r = logical_and_expr_ast_read(&logical_and_expr, lexer, tok);
        if (r != PARSER_OK) {
            
        }

        PUSH_BACK(logical_and_exprs, logical_and_expr);
    }

    (*logical_or_expr) = create_logical_or_expr_ast(logical_and_exprs, logical_and_exprs_len);

    return PARSER_OK;
}

static enum PARSER_CODES property_ast_read(struct PROPERTY_AST**property, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct IDENT_AST*ident;
    struct ASSIGNMENT_EXPR_AST*assignment_expr;

    r = ident_ast_read(&ident, lexer, tok);
    if (r != PARSER_OK) {
        
    }

    if ((*tok)->token_type != TOKEN_TYPE_COLON) {
        
    }
    token_free((*tok));
    lexer_next_token(lexer, tok);

    r = assignment_expr_ast_read(&assignment_expr, lexer, tok);
    if (r != PARSER_OK) {
        
    }

    (*property) = create_property_ast(ident, assignment_expr);

    return PARSER_OK;
}

static enum PARSER_CODES object_literal_ast_read(struct OBJECT_LITERAL_AST**object_literal, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct PROPERTY_AST**properties = NULL;
    unsigned properties_len = 0;
    unsigned properties_cap = 0;
    
    /* no check, because it could be only TOKEN_TYPE_LBRACE. */
    token_free((*tok));
    lexer_next_token(lexer, tok);

    while ((*tok)->token_type != TOKEN_TYPE_RBRACE) {
        struct PROPERTY_AST*property;
        r = property_ast_read(&property, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        PUSH_BACK(properties, property);

        if ((*tok)->token_type != TOKEN_TYPE_COMMA) {
            break;
        }
        token_free((*tok));
        lexer_next_token(lexer, tok);
    }

    /* no check, because it could be only TOKEN_TYPE_RBRACE. */
    token_free((*tok));
    lexer_next_token(lexer, tok);

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

static enum PARSER_CODES assignment_expr_ast_read(struct ASSIGNMENT_EXPR_AST**assignment_expr, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    
    if ((*tok)->token_type == TOKEN_TYPE_LBRACE) {
        struct OBJECT_LITERAL_AST*object_literal;
        r = object_literal_ast_read(&object_literal, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*assignment_expr) = create_assignment_expr_ast(object_literal, ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL);
    } else {
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;
        r = logical_or_expr_ast_read(&logical_or_expr, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*assignment_expr) = create_assignment_expr_ast(logical_or_expr, ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR);        
    }

    return PARSER_OK;

 err0:
    (*assignment_expr) = NULL;
    return r;
}

static enum PARSER_CODES decl_stmt_ast_read(struct DECL_STMT_AST**decl_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct IDENT_AST*var_name;
    struct ASSIGNMENT_EXPR_AST*assignment_expr;
    
    /* no check, because it could be only TOKEN_TYPE_LET. */
    token_free((*tok));
    lexer_next_token(lexer, tok);

    r = ident_ast_read(&var_name, lexer, tok);
    if (r != PARSER_OK) {
        goto err0;
    }
    if ((*tok)->token_type != TOKEN_TYPE_EQ) {
        r = PARSER_INVALID_TOKEN;
        goto err1;
    }
    token_free((*tok));
    lexer_next_token(lexer, tok);
    r = assignment_expr_ast_read(&assignment_expr, lexer, tok);
    if (r != PARSER_OK) {
        goto err1;
    }
    if ((*tok)->token_type != TOKEN_TYPE_SEMI) {
        r = PARSER_INVALID_TOKEN;
        goto err2;
    }
    token_free((*tok));
    lexer_next_token(lexer, tok);

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
static enum PARSER_CODES assign_stmt_ast_read(struct ASSIGN_STMT_AST**assign_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES function_call_stmt_ast_read(struct FUNCTION_CALL_STMT_AST**function_call_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}
*/

static enum PARSER_CODES body_ast_read(struct BODY_AST**body, lexer_type_t lexer, struct TOKEN**tok);

static enum PARSER_CODES if_stmt_ast_read(struct IF_STMT_AST**if_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    
}

static enum PARSER_CODES while_stmt_ast_read(struct WHILE_STMT_AST**while_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct LOGICAL_OR_EXPR_AST*condition;
    struct BODY_AST*body;

    /* no check, because it could be only TOKEN_TYPE_WHILE. */
    token_free((*tok));
    lexer_next_token(lexer, tok);

    if ((*tok)->token_type != TOKEN_TYPE_LPAREN) {
        r = PARSER_INVALID_TOKEN;
        goto err0;
    }
    token_free((*tok));
    lexer_next_token(lexer, tok);

    r = logical_or_expr_ast_read(&condition, lexer, tok);
    if (r != PARSER_OK) {
        goto err0;
    }
    if ((*tok)->token_type != TOKEN_TYPE_RPAREN) {
        r = PARSER_INVALID_TOKEN;
        goto err1;
    }
    token_free((*tok));
    lexer_next_token(lexer, tok);

    r = body_ast_read(&body, lexer, tok);
    if (r != PARSER_OK) {
        goto err1;
    }

    return PARSER_OK;

 err1:
    logical_or_expr_ast_free(condition);
 err0:
    (*while_stmt) = NULL;
    return r;
}

static enum PARSER_CODES return_stmt_ast_read(struct RETURN_STMT_AST**return_stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;    
    struct ASSIGNMENT_EXPR_AST*assignment_expr;
    
    /* no check, because it could be only TOKEN_TYPE_WHILE. */
    token_free((*tok));
    lexer_next_token(lexer, tok);

    if ((*tok)->token_type == TOKEN_TYPE_SEMI) {
        assignment_expr = NULL;
    } else {
        r = assignment_expr_ast_read(&assignment_expr, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
    }

    if ((*tok)->token_type != TOKEN_TYPE_SEMI) {
        r = PARSER_INVALID_TOKEN;
        goto err1;
    }

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

static enum PARSER_CODES stmt_ast_read(struct STMT_AST**stmt, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    
    switch ((*tok)->token_type) {
    case TOKEN_TYPE_LET: {
        struct DECL_STMT_AST*decl_stmt;
        r = decl_stmt_ast_read(&decl_stmt, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(decl_stmt, STMT_TYPE_DECL);
        break;
    }
    case TOKEN_TYPE_IDENT: {
        struct IDENT_AST*ident = create_ident_ast((*tok)->str_val);
        token_free((*tok));
        lexer_next_token(lexer, tok);
        /* check, if name is part of variable or name is function name. */
        switch ((*tok)->token_type) {
        case TOKEN_TYPE_EQ: {
            struct ASSIGN_STMT_AST*assign_stmt;

            struct IDENT_AST**idents = NULL;
            unsigned idents_len = 0;
            unsigned idents_cap = 0;
            struct VARIABLE_AST*var_name;

            struct ASSIGNMENT_EXPR_AST*assignment_expr;
            
            /* reading variable. */
            PUSH_BACK(idents, ident);
            var_name = create_variable_ast(idents, idents_len);
            /* no check, because it could be only TOKEN_TYPE_EQ. */            
            token_free((*tok));
            lexer_next_token(lexer, tok);
            
            /* reading assignment expr. */
            r = assignment_expr_ast_read(&assignment_expr, lexer, tok);
            if (r != PARSER_OK) {
                variable_ast_free(var_name);
                goto err0;
            }          

            assign_stmt = create_assign_stmt_ast(var_name, assignment_expr);
            (*stmt) = create_stmt_ast(assign_stmt, STMT_TYPE_ASSIGN);
            break;
        }
        case TOKEN_TYPE_DOT: {
            struct ASSIGN_STMT_AST*assign_stmt;

            struct VARIABLE_AST*var_name;
            struct ASSIGNMENT_EXPR_AST*assignment_expr;

            r = variable_ast_read(&var_name, lexer, tok, ident);
            if (r != PARSER_OK) {
                
            }
            
            /* no check, because it could be only TOKEN_TYPE_EQ. */            
            token_free((*tok));
            lexer_next_token(lexer, tok);
            /* reading assignment expr. */
            r = assignment_expr_ast_read(&assignment_expr, lexer, tok);
            if (r != PARSER_OK) {
                variable_ast_free(var_name);
                goto err0;
            }

            assign_stmt = create_assign_stmt_ast(var_name, assignment_expr);
            (*stmt) = create_stmt_ast(assign_stmt, STMT_TYPE_ASSIGN);
            break;
        }
        case TOKEN_TYPE_LPAREN: {
            struct FUNCTION_CALL_AST*function_call;
            
            struct FUNCTION_CALL_STMT_AST*function_call_stmt;

            r = function_call_ast_read(&function_call, lexer, tok, ident);
            if (r != PARSER_OK) {
                goto err0;
            }

            function_call_stmt = create_function_call_stmt_ast(function_call);
            (*stmt) = create_stmt_ast(function_call_stmt, STMT_TYPE_FUNCTION_CALL);
            break;
        }
        default: {
            r = PARSER_INVALID_TOKEN;
            goto err0;
            break;
        }
        }

        if ((*tok)->token_type != TOKEN_TYPE_SEMI) {
            r = PARSER_INVALID_TOKEN;
            goto err0;
        }

        token_free((*tok));
        lexer_next_token(lexer, tok);
        
        break;
    }
    case TOKEN_TYPE_IF: {
        struct IF_STMT_AST*if_stmt;
        r = if_stmt_ast_read(&if_stmt, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(if_stmt, STMT_TYPE_IF);
        break;
    }
    case TOKEN_TYPE_WHILE: {
        struct WHILE_STMT_AST*while_stmt;
        r = while_stmt_ast_read(&while_stmt, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(while_stmt, STMT_TYPE_WHILE);
        break;
    }
    case TOKEN_TYPE_RETURN: {
        struct RETURN_STMT_AST*return_stmt;
        r = return_stmt_ast_read(&return_stmt, lexer, tok);
        if (r != PARSER_OK) {
            goto err0;
        }
        (*stmt) = create_stmt_ast(return_stmt, STMT_TYPE_RETURN);
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

static enum PARSER_CODES body_ast_read(struct BODY_AST**body, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;
    struct STMT_AST**stmts = NULL;
    unsigned stmts_len = 0;
    unsigned stmts_cap = 0;

    if ((*tok)->token_type != TOKEN_TYPE_LBRACE) {
        r = PARSER_INVALID_TOKEN;
        goto err0;
    }
    token_free((*tok));
    lexer_next_token(lexer, tok);

    while ((*tok)->token_type != TOKEN_TYPE_RBRACE) {
        struct STMT_AST*stmt;
        r = stmt_ast_read(&stmt, lexer, tok);
        if (r != PARSER_OK) {
            goto err1;
        }
        PUSH_BACK(stmts, stmt);
    }

    /* no check, because it could be only TOKEN_TYPE_RBRACE. */
    token_free((*tok));
    lexer_next_token(lexer, tok);

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
    token_free((*tok));
    lexer_next_token(lexer, tok);

    r = ident_ast_read(&function_name, lexer, tok);
    if (r != PARSER_OK) {
        goto err0;
    }
    
    r = formal_parameters_list_ast_read(&formal_parameters_list, lexer, tok);
    if (r != PARSER_OK) {
        goto err1;
    }
    r = body_ast_read(&body, lexer, tok);
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

static enum PARSER_CODES unit_ast_read(struct UNIT_AST**unit, lexer_type_t lexer, struct TOKEN**tok)
{
    enum PARSER_CODES r;

    struct FUNCTION_DECL_AST**functions = NULL;
    unsigned functions_len = 0;
    unsigned functions_cap = 0;
    
    while ((*tok)->token_type != TOKEN_TYPE_EOF) {
        struct FUNCTION_DECL_AST*function;
        r = function_decl_ast_read(&function, lexer, tok);
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
    struct TOKEN*tok;
    lexer_next_token(parser->lexer, &tok);
    return unit_ast_read(unit, parser->lexer, &tok);
}

void parser_free(parser_type_t parser)
{
    SAFE_FREE(parser);
}

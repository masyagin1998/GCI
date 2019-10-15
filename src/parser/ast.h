#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include <stdio.h>

/*
  unit = function_decl*
*/

struct UNIT_AST
{
    struct FUNCTION_DECL_AST**functions;
    unsigned functions_len;
};

struct UNIT_AST*create_unit_ast(struct FUNCTION_DECL_AST**functions, unsigned functions_len);
void dump_unit_ast_to_file(FILE*f, const struct UNIT_AST*unit);
void unit_ast_free(struct UNIT_AST*ast);

/*
  function_decl = FUNCTION IDENT formal_parameters_list body
*/

struct FUNCTION_DECL_AST
{
    struct IDENT_AST*function_name;
    struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list;
    struct BODY_AST*body;
};

struct FUNCTION_DECL_AST*create_function_decl_ast(struct IDENT_AST*function_name,
                                                  struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list,
                                                  struct BODY_AST*body);
void dump_function_decl_ast_to_file(FILE*f, const struct UNIT_AST*unit);
void function_decl_ast_free(struct FUNCTION_DECL_AST*ast);

/*
  formal_parameters_list = LPAREN IDENT (COMMA IDENT)* RPAREN |
                           LPAREN RPAREN
*/

struct FORMAL_PARAMETERS_LIST_AST
{
    struct IDENT_AST**params;
    unsigned          params_len;
};

struct FORMAL_PARAMETERS_LIST_AST*create_formal_parameters_list_ast(struct IDENT_AST**params, unsigned params_len);
void dump_formal_parameters_list_ast_to_file(FILE*f, const struct UNIT_AST*unit);
void formal_parameters_list_ast_free(struct FORMAL_PARAMETERS_LIST_AST*ast);

/*
  body = LBRACE statement* RBRACE
 */

struct BODY_AST
{
    struct STMT_AST**stmts;
    unsigned         stmts_len;
};

struct BODY_AST*create_body_ast(struct STMT_AST**stmts, unsigned stmts_len);
void dump_body_ast_to_file(FILE*f, const struct UNIT_AST*unit);
void body_ast_free(struct BODY_AST*ast);

/*
statement = decl_statement |
            assign_statement |
            function_call_statement |
            if_statement |
            while_statement |
            return_statement
*/

enum STMT_TYPE
{
    STMT_TYPE_DECL,
    STMT_TYPE_ASSIGN,
    STMT_TYPE_FUNCTION_CALL,
    STMT_TYPE_IF,
    STMT_TYPE_WHILE,
    STMT_TYPE_RETURN,
};

struct STMT_AST
{
    union
    {
        struct DECL_STMT_AST*decl_stmt;
        struct ASSIGN_STMT_AST*assign_stmt;
        struct FUNCTION_CALL_STMT_AST*function_call_stmt;
        struct IF_STMT_AST*if_stmt;
        struct WHILE_STMT_AST*while_stmt;
        struct RETURN_STMT_AST*return_stmt;
    };

    enum STMT_TYPE type;
};

struct STMT_AST*create_stmt_ast(void*stmt_ptr, enum STMT_TYPE stmt_type);

void stmt_ast_free(struct STMT_AST*ast);

/*
  decl_statement = LET IDENT EQ assignment_expr SEMI
*/

struct DECL_STMT_AST
{
    struct IDENT_AST*new_var_name;
    struct ASSIGNMENT_EXPR_AST*assignment;
};

struct DECL_STMT_AST*create_decl_stmt_ast(struct IDENT_AST*new_var_name,
                                          struct ASSIGNMENT_EXPR_AST*assignment);

void decl_stmt_ast_free(struct DECL_STMT_AST*ast);

/*
  assign_statement = variable EQ assignment_expr SEMI
*/

struct ASSIGN_STMT_AST
{
    struct VARIABLE_AST*var_name;
    struct ASSIGNMENT_EXPR_AST*assignment;
};

struct ASSIGN_STMT_AST*create_assign_stmt_ast(struct VARIABLE_AST*var_name,
                                              struct ASSIGNMENT_EXPR_AST*assignment);

void assign_stmt_ast_free(struct ASSIGN_STMT_AST*ast);

/*
  function_call_statement = function_call SEMI
*/

struct FUNCTION_CALL_STMT_AST
{
    struct FUNCTION_CALL_AST*function_call;
};
 
struct FUNCTION_CALL_STMT_AST*create_function_call_stmt_ast(struct FUNCTION_CALL_AST*function_call);

void function_call_stmt_ast_free(struct FUNCTION_CALL_STMT_AST*ast);

/*
  if_statement = IF LPAREN logical_or_expr RPAREN body ELSE body |
                 IF LPAREN logical_or_expr RPAREN body
*/

struct IF_STMT_AST
{
    struct LOGICAL_OR_EXPR_AST*condition;
    struct BODY_AST*if_body;
    struct BODY_AST*else_body;
};

struct IF_STMT_AST*create_if_stmt_ast(struct LOGICAL_OR_EXPR_AST*condition,
                                      struct BODY_AST*if_body,
                                      struct BODY_AST*else_body);

void if_stmt_ast_free(struct IF_STMT_AST*ast);

/*
  while_statement = WHILE LPAREN logical_or_expr RPAREN body
*/

struct WHILE_STMT_AST
{
    struct LOGICAL_OR_EXPR_AST*condition;
    struct BODY_AST*body;
};

struct WHILE_STMT_AST*create_while_stmt_ast(struct LOGICAL_OR_EXPR_AST*condition,
                                            struct BODY_AST*body);

void while_stmt_ast_free(struct WHILE_STMT_AST*ast);

/*
return_statement = RETURN assignment_expr SEMI |
                   RETURN SEMI
*/

struct RETURN_STMT_AST
{
    struct ASSIGNMENT_EXPR_AST*result;
};

struct RETURN_STMT_AST*create_return_stmt_ast(struct ASSIGNMENT_EXPR_AST*result);

void return_stmt_ast_free(struct RETURN_STMT_AST*ast);

/*
  variable = IDENT (DOT IDENT)*
*/

struct VARIABLE_AST
{
    struct IDENT_AST**idents;
    unsigned          idents_len;
};

struct VARIABLE_AST*create_variable_ast(struct IDENT_AST**idents, unsigned idents_len);

void variable_ast_free(struct VARIABLE_AST*ast);

/*
  assignment_expr = object_literal | logical_or_expr
*/

enum ASSIGNMENT_EXPR_TYPE
{
    ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL,
    ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR,
};

struct ASSIGNMENT_EXPR_AST
{
    union
    {
        struct OBJECT_LITERAL_AST*object_literal;
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;
    };

    enum ASSIGNMENT_EXPR_TYPE type;
};

struct ASSIGNMENT_EXPR_AST*create_assignment_expr_ast(void*assignment_ptr, enum ASSIGNMENT_EXPR_TYPE type);

void assignment_expr_ast_free(struct ASSIGNMENT_EXPR_AST*ast);

/*
  object_literal = LBRACE properties_list RBRACE |
                   LBRACE RBRACE
*/

struct OBJECT_LITERAL_AST
{
    struct PROPERTY_AST**properties;
    unsigned         properties_len;
};

struct OBJECT_LITERAL_AST*create_object_literal_ast(struct PROPERTY_AST**properties, unsigned properties_len);

void object_literal_ast_free(struct OBJECT_LITERAL_AST*ast);

/*
  properties_list     = property_assignment (COMMA property_assignment)
  property_assignment = IDENT COLON assignment_expr
*/

struct PROPERTY_AST
{
    struct IDENT_AST*key;
    struct ASSIGNMENT_EXPR_AST*value;
};

struct PROPERTY_AST*create_property_ast(struct IDENT_AST*key, struct ASSIGNMENT_EXPR_AST*value);

void property_ast_free(struct PROPERTY_AST*ast);

/*
  logical_or_expr = logical_and_expr (OR logical_and_expr)*
*/

struct LOGICAL_OR_EXPR_AST
{
    struct LOGICAL_AND_EXPR_AST**and_exprs;
    unsigned                     and_exprs_len;
};

struct LOGICAL_OR_EXPR_AST*create_logical_or_expr_ast(struct LOGICAL_AND_EXPR_AST**and_exprs, unsigned and_exprs_len);

void logical_or_expr_ast_free(struct LOGICAL_OR_EXPR_AST*ast);

/*
  logical_and_expr = eq_expr (AND eq_expr)*
*/

struct LOGICAL_AND_EXPR_AST
{
    struct EQ_EXPR_AST**eq_exprs;
    unsigned            eq_exprs_len;
};

struct LOGICAL_AND_EXPR_AST*create_logical_and_expr_ast(struct EQ_EXPR_AST**eq_exprs, unsigned eq_exprs_len);

void logical_and_expr_ast_free(struct LOGICAL_AND_EXPR_AST*ast);

/*
  eq_expr = relational_expr (eq_op relationa_expr)?
  eq_op   = EQEQ | NEQ
*/

enum EQ_OP
{
    EQEQ,
    NEQ,
};

struct EQ_EXPR_AST
{
    struct RELATIONAL_EXPR_AST*left;
    enum EQ_OP eq_op;
    struct RELATIONAL_EXPR_AST*right;
};

struct EQ_EXPR_AST*create_eq_expr_ast(struct RELATIONAL_EXPR_AST*left,
                                      enum EQ_OP eq_op,
                                      struct RELATIONAL_EXPR_AST*right);

void eq_expr_ast_free(struct EQ_EXPR_AST*ast);

/*
  relational_expr = additive_expr (relational_op additive_expr)?
  relational_op   = LT | GT | LE | GE
*/

enum REL_OP
{
    LT,
    GT,
    LE,
    GE, 
};

struct RELATIONAL_EXPR_AST
{
    struct ADDITIVE_EXPR_AST*left;
    enum REL_OP rel_op;
    struct ADDITIVE_EXPR_AST*right;
};

struct RELATIONAL_EXPR_AST*create_relational_expr_ast(struct ADDITIVE_EXPR_AST*left,
                                                      enum REL_OP rel_op,
                                                      struct ADDITIVE_EXPR_AST*right);

void relational_expr_ast_free(struct RELATIONAL_EXPR_AST*ast);

/*
  additive_expr = multiplicative_expr (additive_op multiplicative_expr)*
  additive_op   = PLUS | MINUS
*/

enum ADDITIVE_OP
{
    PLUS,
    MINUS,
};

struct ADDITIVE_EXPR_AST
{
    struct MULTIPLICATIVE_EXPR_AST**muls;
    enum ADDITIVE_OP*ops;
    unsigned muls_len;
};

struct ADDITIVE_EXPR_AST*create_additive_expr_ast(struct MULTIPLICATIVE_EXPR_AST**muls,
                                                  enum ADDITIVE_OP*ops,
                                                  unsigned muls_len);

void additive_expr_ast_free(struct ADDITIVE_EXPR_AST*ast);

/*
  multiplicative_expr = left_unary_expr | (multiplicative_op left_unary_expr)*
  multiplicative_op   = MUL | DIV | MOD
*/

enum MULTIPLICATIVE_OP
{
    MUL,
    DIV,
    MOD,
};

struct MULTIPLICATIVE_EXPR_AST
{
    struct LEFT_UNARY_EXPR_AST**lues;
    enum MULTIPLICATIVE_OP*ops;
    unsigned lues_len;
};

struct MULTIPLICATIVE_EXPR_AST*create_multiplicative_expr_ast(struct LEFT_UNARY_EXPR_AST**lues,
                                                              enum MULTIPLICATIVE_OP*ops,
                                                              unsigned lues_len);

void multiplicative_expr_ast_free(struct MULTIPLICATIVE_EXPR_AST*ast);

/*
  left_unary_expr = left_unary_op primary_expr | primary_expr
  left_unary_op   = PLUS | MINUS
*/

enum LEFT_UNARY_EXPR_OP
{
    UNARY_PLUS,
    UNARY_MINUS,
};

struct LEFT_UNARY_EXPR_AST
{
    enum LEFT_UNARY_EXPR_OP op;
    struct PRIMARY_EXPR_AST*expr;
};

struct LEFT_UNARY_EXPR_AST*create_left_unary_expr_ast(enum LEFT_UNARY_EXPR_OP op, struct PRIMARY_EXPR_AST*expr);

void left_unary_expr_ast_free(struct LEFT_UNARY_EXPR_AST*ast);

/*
  primary_expr = function_call | variable | NUMBER | LPAREN logical_or_expr RPAREN
*/

enum PRIMARY_EXPR_TYPE
{
    PRIMARY_EXPR_TYPE_FUNCTION_CALL,
    PRIMARY_EXPR_TYPE_VARIABLE,
    PRIMARY_EXPR_TYPE_NUMBER,
    PRIMARY_EXPR_TYPE_LOGICAL_EXPR,
};

struct PRIMARY_EXPR_AST
{
    union {
        struct FUNCTION_CALL_AST*function_call;
        struct VARIABLE_AST*var_name;
        struct NUMBER_AST*number;
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;
    };

    enum PRIMARY_EXPR_TYPE type;
};

struct PRIMARY_EXPR_AST*create_primary_expr_ast(void*primary_expr_ptr, enum PRIMARY_EXPR_TYPE type);

void primary_expr_ast_free(struct PRIMARY_EXPR_AST*ast);

/*
  function_call = IDENT LPAREN args_list RPAREN |
                  IDENT LPAREN RPAREN
*/

struct FUNCTION_CALL_AST
{
    struct IDENT_AST*function_name;
    struct ARGS_LIST_AST*args_list;
};

struct FUNCTION_CALL_AST*create_function_call_ast(struct IDENT_AST*function_name, struct ARGS_LIST_AST*args_list);

void function_call_ast_free(struct FUNCTION_CALL_AST*ast);

/*
  args_list = primary_expr (COMMA primary_expr)*
*/

struct ARGS_LIST_AST
{
    struct ASSIGNMENT_EXPR_AST**assignment_exprs;
    unsigned                    assignment_exprs_len;
};

struct ARGS_LIST_AST*create_args_list_ast(struct ASSIGNMENT_EXPR_AST**assignment_exprs, unsigned assignment_exprs_len);

void args_list_ast_free(struct ARGS_LIST_AST*ast);

/*
  TERMINALS.
*/

/*
  IDENT.
*/

struct IDENT_AST
{
    char ident[32];
};

struct IDENT_AST*create_ident_ast(const char*ident);

void ident_ast_free(struct IDENT_AST*ast);

/*
  NUMBER.
*/

struct NUMBER_AST
{
    long long number;
};

struct NUMBER_AST*create_number_ast(long long number);

void number_ast_free(struct NUMBER_AST*ast);

#endif  /* AST_H_INCLUDED */

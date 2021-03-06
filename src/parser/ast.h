#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include <stdio.h>

/*
  unit = function_decl*
*/

struct UNIT_AST
{
    struct FUNCTION_DECL_AST**functions;
    size_t functions_len;

    size_t line;
    size_t pos;
};

struct UNIT_AST*create_unit_ast(struct FUNCTION_DECL_AST**functions, size_t functions_len, size_t line, size_t pos);
void dump_unit_ast_to_xml_file(FILE*f, const struct UNIT_AST*ast);
void unit_ast_free(struct UNIT_AST*ast);

/*
function_decl = FUNCTION IDENT LPAREN formal_parameters_list RPAREN body |
                FUNCTION IDENT LPAREN RPAREN body
*/

struct FUNCTION_DECL_AST
{
    struct IDENT_AST*function_name;
    struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list;
    struct BODY_AST*body;

    size_t line;
    size_t pos;
};

struct FUNCTION_DECL_AST*create_function_decl_ast(struct IDENT_AST*function_name,
                                                  struct FORMAL_PARAMETERS_LIST_AST*formal_parameters_list,
                                                  struct BODY_AST*body,
                                                  size_t line, size_t pos);
void dump_function_decl_ast_to_file(FILE*f, const struct FUNCTION_DECL_AST*ast);
void function_decl_ast_free(struct FUNCTION_DECL_AST*ast);

/*
  formal_parameters_list  = IDENT (COMMA IDENT)* RPAREN
*/

struct FORMAL_PARAMETERS_LIST_AST
{
    struct IDENT_AST**params;
    size_t          params_len;

    size_t line;
    size_t pos;
};

struct FORMAL_PARAMETERS_LIST_AST*create_formal_parameters_list_ast(struct IDENT_AST**params, size_t params_len, size_t line, size_t pos);
void dump_formal_parameters_list_ast_to_file(FILE*f, const struct FORMAL_PARAMETERS_LIST_AST*ast);
void formal_parameters_list_ast_free(struct FORMAL_PARAMETERS_LIST_AST*ast);

/*
  body = LBRACE statement* RBRACE
*/

struct BODY_AST
{
    struct STMT_AST**stmts;
    size_t         stmts_len;

    size_t line;
    size_t pos;
};

struct BODY_AST*create_body_ast(struct STMT_AST**stmts, size_t stmts_len, size_t line, size_t pos);
void dump_body_ast_to_file(FILE*f, const struct BODY_AST*ast);
void body_ast_free(struct BODY_AST*ast);

/*
statement = decl_statement |
            assign_statement |
            function_call_statement |
            if_statement |
            while_statement |
            break_statement |
            continue_statement |
            return_statement
*/

enum AST_STMT_TYPE
{
    AST_STMT_TYPE_DECL,
    AST_STMT_TYPE_ASSIGN,
    AST_STMT_TYPE_FUNCTION_CALL,
    AST_STMT_TYPE_IF,
    AST_STMT_TYPE_WHILE,
    AST_STMT_TYPE_BREAK,
    AST_STMT_TYPE_CONTINUE,
    AST_STMT_TYPE_APPEND,
    AST_STMT_TYPE_DELETE,
    AST_STMT_TYPE_RETURN,
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
        struct BREAK_STMT_AST*break_stmt;
        struct CONTINUE_STMT_AST*continue_stmt;
        struct APPEND_STMT_AST*append_stmt;
        struct DELETE_STMT_AST*delete_stmt;
        struct RETURN_STMT_AST*return_stmt;
    };

    enum AST_STMT_TYPE type;
};

struct STMT_AST*create_stmt_ast(void*stmt_ptr, enum AST_STMT_TYPE stmt_type);
void dump_stmt_ast_to_file(FILE*f, const struct STMT_AST*ast);
void stmt_ast_free(struct STMT_AST*ast);

/*
  decl_statement = LET IDENT EQ assignment_expr SEMI
*/

struct DECL_STMT_AST
{
    struct IDENT_AST*new_var_name;
    struct ASSIGNMENT_EXPR_AST*assignment;

    size_t line;
    size_t pos;
};

struct DECL_STMT_AST*create_decl_stmt_ast(struct IDENT_AST*new_var_name,
                                          struct ASSIGNMENT_EXPR_AST*assignment,
                                          size_t line, size_t pos);
void dump_decl_stmt_ast_to_file(FILE*f, const struct DECL_STMT_AST*ast);
void decl_stmt_ast_free(struct DECL_STMT_AST*ast);

/*
  assign_statement = variable EQ assignment_expr SEMI
*/

struct ASSIGN_STMT_AST
{
    struct VARIABLE_AST*var_name;
    struct ASSIGNMENT_EXPR_AST*assignment;

    size_t line;
    size_t pos;
};

struct ASSIGN_STMT_AST*create_assign_stmt_ast(struct VARIABLE_AST*var_name,
                                              struct ASSIGNMENT_EXPR_AST*assignment,
                                              size_t line, size_t pos);
void dump_assign_stmt_ast_to_file(FILE*f, const struct ASSIGN_STMT_AST*ast);
void assign_stmt_ast_free(struct ASSIGN_STMT_AST*ast);

/*
  function_call_statement = function_call SEMI
*/

struct FUNCTION_CALL_STMT_AST
{
    struct FUNCTION_CALL_AST*function_call;

    size_t line;
    size_t pos;
};
 
struct FUNCTION_CALL_STMT_AST*create_function_call_stmt_ast(struct FUNCTION_CALL_AST*function_call,
                                                            size_t line, size_t pos);
void dump_function_call_stmt_ast_to_file(FILE*f, const struct FUNCTION_CALL_STMT_AST*ast);
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

    size_t line;
    size_t pos;    
};

struct IF_STMT_AST*create_if_stmt_ast(struct LOGICAL_OR_EXPR_AST*condition,
                                      struct BODY_AST*if_body,
                                      struct BODY_AST*else_body,
                                      size_t line, size_t pos);
void dump_if_stmt_ast_to_file(FILE*f, const struct IF_STMT_AST*ast);
void if_stmt_ast_free(struct IF_STMT_AST*ast);

/*
  while_statement = WHILE LPAREN logical_or_expr RPAREN body
*/

struct WHILE_STMT_AST
{
    struct LOGICAL_OR_EXPR_AST*condition;
    struct BODY_AST*body;

    size_t line;
    size_t pos;
};

struct WHILE_STMT_AST*create_while_stmt_ast(struct LOGICAL_OR_EXPR_AST*condition,
                                            struct BODY_AST*body,
                                            size_t line, size_t pos);
void dump_while_stmt_ast_to_file(FILE*f, const struct WHILE_STMT_AST*ast);
void while_stmt_ast_free(struct WHILE_STMT_AST*ast);

/*
  break_statement = BREAK SEMI
*/

struct BREAK_STMT_AST {
    size_t line;
    size_t pos;
};

struct BREAK_STMT_AST*create_break_stmt_ast(size_t line, size_t pos);
void dump_break_stmt_ast_to_file(FILE*f, const struct BREAK_STMT_AST*ast);
void break_stmt_ast_free(struct BREAK_STMT_AST*ast);

/*
  continue_statement = CONTINUE SEMI
*/

struct CONTINUE_STMT_AST {
    size_t line;
    size_t pos;
};

struct CONTINUE_STMT_AST*create_continue_stmt_ast(size_t line, size_t pos);
void dump_continue_stmt_ast_to_file(FILE*f, const struct CONTINUE_STMT_AST*ast);
void continue_stmt_ast_free(struct CONTINUE_STMT_AST*ast);

struct APPEND_STMT_AST {
    struct VARIABLE_AST*arr;
    struct IDENT_AST*ident;
    
    size_t line;
    size_t pos;
};

struct APPEND_STMT_AST*create_append_stmt_ast(struct VARIABLE_AST*arr, struct IDENT_AST*ident,
                                              size_t line, size_t pos);
void dump_append_stmt_ast_to_file(FILE*f, const struct APPEND_STMT_AST*ast);
void append_stmt_ast_free(struct APPEND_STMT_AST*ast);

struct DELETE_STMT_AST {
    struct VARIABLE_AST*var;
    struct IDENT_AST*ident;
    
    size_t line;
    size_t pos;
};

struct DELETE_STMT_AST*create_delete_stmt_ast(struct VARIABLE_AST*var, struct IDENT_AST*ident, size_t line, size_t pos);
void dump_delete_stmt_ast_to_file(FILE*f, const struct DELETE_STMT_AST*ast);
void delete_stmt_ast_free(struct DELETE_STMT_AST*ast);

/*
  return_statement = RETURN assignment_expr SEMI |
                     RETURN SEMI
*/

struct RETURN_STMT_AST
{
    struct ASSIGNMENT_EXPR_AST*result;

    size_t line;
    size_t pos;
};

struct RETURN_STMT_AST*create_return_stmt_ast(struct ASSIGNMENT_EXPR_AST*result,
                                              size_t line, size_t pos);
void dump_return_stmt_ast_to_file(FILE*f, const struct RETURN_STMT_AST*ast);
void return_stmt_ast_free(struct RETURN_STMT_AST*ast);

/*
  variable = variable_part (DOT variable_part)*
*/

struct VARIABLE_AST
{
    struct IDENT_AST*ident;
    struct VARIABLE_PART_AST**parts;
    size_t                    parts_len;

    size_t line;
    size_t pos;
};

struct VARIABLE_AST*create_variable_ast(struct IDENT_AST*ident, struct VARIABLE_PART_AST**parts, size_t parts_len,
                                        size_t line, size_t pos);
void dump_variable_ast_to_file(FILE*f, const struct VARIABLE_AST*ast);
void variable_ast_free(struct VARIABLE_AST*ast);

/*
  variable_part = IDENT (LBRACKET logical_or_expr RBRACKET)?
*/

enum AST_VARIABLE_PART_TYPE
{
    AST_VARIABLE_PART_TYPE_FIELD,
    AST_VARIABLE_PART_TYPE_INDEX,
};

struct VARIABLE_PART_AST
{
    union
    {
        struct IDENT_AST*field;
        struct LOGICAL_OR_EXPR_AST*index;
    };

    enum AST_VARIABLE_PART_TYPE type;
};

struct VARIABLE_PART_AST*create_variable_part_ast(void*variable_part_ptr, enum AST_VARIABLE_PART_TYPE type);
void dump_variable_part_ast_to_file(FILE*f, const struct VARIABLE_PART_AST*ast);
void variable_part_ast_free(struct VARIABLE_PART_AST*ast);

/*
  assignment_expr = object_literal | array_literal | logical_or_expr
*/

enum AST_ASSIGNMENT_EXPR_TYPE
{
    AST_ASSIGNMENT_EXPR_TYPE_OBJECT_LITERAL,
    AST_ASSIGNMENT_EXPR_TYPE_ARRAY_LITERAL,
    AST_ASSIGNMENT_EXPR_TYPE_LOGICAL_OR_EXPR,
};

struct ASSIGNMENT_EXPR_AST
{
    union
    {
        struct OBJECT_LITERAL_AST*object_literal;
        struct ARRAY_LITERAL_AST*array_literal;
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;
    };

    enum AST_ASSIGNMENT_EXPR_TYPE type;
};

struct ASSIGNMENT_EXPR_AST*create_assignment_expr_ast(void*assignment_ptr, enum AST_ASSIGNMENT_EXPR_TYPE type);
void dump_assignment_expr_ast_to_file(FILE*f, const struct ASSIGNMENT_EXPR_AST*ast);
void assignment_expr_ast_free(struct ASSIGNMENT_EXPR_AST*ast);

/*
  object_literal = LBRACE properties_list RBRACE |
                   LBRACE RBRACE
*/

struct OBJECT_LITERAL_AST
{
    struct PROPERTY_AST**properties;
    size_t         properties_len;

    size_t line;
    size_t pos;
};

struct OBJECT_LITERAL_AST*create_object_literal_ast(struct PROPERTY_AST**properties, size_t properties_len,
                                                    size_t line, size_t pos);
void dump_object_literal_ast_to_file(FILE*f, const struct OBJECT_LITERAL_AST*ast);
void object_literal_ast_free(struct OBJECT_LITERAL_AST*ast);

/*
  properties_list     = property_assignment (COMMA property_assignment)
  property_assignment = IDENT COLON assignment_expr
*/

struct PROPERTY_AST
{
    struct IDENT_AST*key;
    struct ASSIGNMENT_EXPR_AST*value;

    size_t line;
    size_t pos;
};

struct PROPERTY_AST*create_property_ast(struct IDENT_AST*key, struct ASSIGNMENT_EXPR_AST*value, size_t line, size_t pos);
void dump_property_ast_to_file(FILE*f, const struct PROPERTY_AST*ast);
void property_ast_free(struct PROPERTY_AST*ast);

/*
  array_literal = LBRACKET args_list RBRACKET |
                  LBRACKET RBRACKET
*/

struct ARRAY_LITERAL_AST
{
    struct ARGS_LIST_AST*args_list;

    size_t line;
    size_t pos;
};

struct ARRAY_LITERAL_AST*create_array_literal_ast(struct ARGS_LIST_AST*args_list, size_t line, size_t pos);
void dump_array_literal_ast_to_file(FILE*f, const struct ARRAY_LITERAL_AST*ast);
void array_literal_ast_free(struct ARRAY_LITERAL_AST*ast);

/*
  logical_or_expr = logical_and_expr (OR logical_and_expr)*
*/

struct LOGICAL_OR_EXPR_AST
{
    struct LOGICAL_AND_EXPR_AST**and_exprs;
    size_t                     and_exprs_len;

    size_t line;
    size_t pos;
};

struct LOGICAL_OR_EXPR_AST*create_logical_or_expr_ast(struct LOGICAL_AND_EXPR_AST**and_exprs, size_t and_exprs_len,
                                                      size_t line, size_t pos);
void dump_logical_or_expr_ast_to_file(FILE*f, const struct LOGICAL_OR_EXPR_AST*ast);
void logical_or_expr_ast_free(struct LOGICAL_OR_EXPR_AST*ast);

/*
  logical_and_expr = eq_expr (AND eq_expr)*
*/

struct LOGICAL_AND_EXPR_AST
{
    struct EQ_EXPR_AST**eq_exprs;
    size_t            eq_exprs_len;

    size_t line;
    size_t pos;
};

struct LOGICAL_AND_EXPR_AST*create_logical_and_expr_ast(struct EQ_EXPR_AST**eq_exprs, size_t eq_exprs_len,
                                                        size_t line, size_t pos);
void dump_logical_and_expr_ast_to_file(FILE*f, const struct LOGICAL_AND_EXPR_AST*ast);
void logical_and_expr_ast_free(struct LOGICAL_AND_EXPR_AST*ast);

/*
  eq_expr = relational_expr (eq_op relationa_expr)?
  eq_op   = EQEQ | NEQ
*/

enum AST_EQ_OP
{
    AST_EQ_OP_EQEQ,
    AST_EQ_OP_NEQ,
};

struct EQ_EXPR_AST
{
    struct RELATIONAL_EXPR_AST*left;
    enum AST_EQ_OP eq_op;
    struct RELATIONAL_EXPR_AST*right;

    size_t line;
    size_t pos;
};

struct EQ_EXPR_AST*create_eq_expr_ast(struct RELATIONAL_EXPR_AST*left,
                                      enum AST_EQ_OP eq_op,
                                      struct RELATIONAL_EXPR_AST*right,
                                      size_t line, size_t pos);
void dump_eq_expr_ast_to_file(FILE*f, const struct EQ_EXPR_AST*ast);
void eq_expr_ast_free(struct EQ_EXPR_AST*ast);

/*
  relational_expr = additive_expr (relational_op additive_expr)?
  relational_op   = LT | GT | LE | GE
*/

enum AST_REL_OP
{
    AST_REL_OP_LT,
    AST_REL_OP_GT,
    AST_REL_OP_LE,
    AST_REL_OP_GE, 
};

struct RELATIONAL_EXPR_AST
{
    struct ADDITIVE_EXPR_AST*left;
    enum AST_REL_OP rel_op;
    struct ADDITIVE_EXPR_AST*right;

    size_t line;
    size_t pos;
};

struct RELATIONAL_EXPR_AST*create_relational_expr_ast(struct ADDITIVE_EXPR_AST*left,
                                                      enum AST_REL_OP rel_op,
                                                      struct ADDITIVE_EXPR_AST*right,
                                                      size_t line, size_t pos);
void dump_relational_expr_ast_to_file(FILE*f, const struct RELATIONAL_EXPR_AST*ast);
void relational_expr_ast_free(struct RELATIONAL_EXPR_AST*ast);

/*
  additive_expr = multiplicative_expr (additive_op multiplicative_expr)*
  additive_op   = PLUS | MINUS
*/

enum AST_ADDITIVE_OP
{
    AST_ADDITIVE_OP_PLUS,
    AST_ADDITIVE_OP_MINUS,
};

struct ADDITIVE_EXPR_AST
{
    struct MULTIPLICATIVE_EXPR_AST**muls;
    enum AST_ADDITIVE_OP*ops;
    size_t muls_len;

    size_t line;
    size_t pos;
};

struct ADDITIVE_EXPR_AST*create_additive_expr_ast(struct MULTIPLICATIVE_EXPR_AST**muls,
                                                  enum AST_ADDITIVE_OP*ops,
                                                  size_t muls_len,
                                                  size_t line, size_t pos);
void dump_additive_expr_ast_to_file(FILE*f, const struct ADDITIVE_EXPR_AST*ast);
void additive_expr_ast_free(struct ADDITIVE_EXPR_AST*ast);

/*
  multiplicative_expr = left_unary_expr | (multiplicative_op left_unary_expr)*
  multiplicative_op   = MUL | DIV | MOD
*/

enum AST_MULTIPLICATIVE_OP
{
    AST_MULTIPLICATIVE_OP_MUL,
    AST_MULTIPLICATIVE_OP_DIV,
    AST_MULTIPLICATIVE_OP_MOD,
};

struct MULTIPLICATIVE_EXPR_AST
{
    struct LEFT_UNARY_EXPR_AST**lues;
    enum AST_MULTIPLICATIVE_OP*ops;
    size_t lues_len;

    size_t line;
    size_t pos;
};

struct MULTIPLICATIVE_EXPR_AST*create_multiplicative_expr_ast(struct LEFT_UNARY_EXPR_AST**lues,
                                                              enum AST_MULTIPLICATIVE_OP*ops,
                                                              size_t lues_len,
                                                              size_t line, size_t pos);
void dump_multiplicative_expr_ast_to_file(FILE*f, const struct MULTIPLICATIVE_EXPR_AST*ast);
void multiplicative_expr_ast_free(struct MULTIPLICATIVE_EXPR_AST*ast);

/*
  left_unary_expr = left_unary_op primary_expr | primary_expr
  left_unary_op   = PLUS | MINUS
*/

enum AST_LEFT_UNARY_OP
{
    AST_LEFT_UNARY_OP_PLUS,
    AST_LEFT_UNARY_OP_MINUS,
};

struct LEFT_UNARY_EXPR_AST
{
    enum AST_LEFT_UNARY_OP op;
    struct PRIMARY_EXPR_AST*expr;

    size_t line;
    size_t pos;
};

struct LEFT_UNARY_EXPR_AST*create_left_unary_expr_ast(enum AST_LEFT_UNARY_OP op, struct PRIMARY_EXPR_AST*expr,
                                                      size_t line, size_t pos);
void dump_left_unary_expr_ast_to_file(FILE*f, const struct LEFT_UNARY_EXPR_AST*ast);
void left_unary_expr_ast_free(struct LEFT_UNARY_EXPR_AST*ast);

/*
  primary_expr = function_call | variable | NUMBER | LPAREN logical_or_expr RPAREN
*/

enum AST_PRIMARY_EXPR_TYPE
{
    AST_PRIMARY_EXPR_TYPE_HAS_PROPERTY, 
    AST_PRIMARY_EXPR_TYPE_LEN,
    AST_PRIMARY_EXPR_TYPE_FUNCTION_CALL,
    AST_PRIMARY_EXPR_TYPE_VARIABLE,
    AST_PRIMARY_EXPR_TYPE_NUMBER,
    AST_PRIMARY_EXPR_TYPE_LOGICAL_OR_EXPR,
};

struct PRIMARY_EXPR_AST
{
    union {
        struct HAS_PROPERTY_EXPR_AST*has_property_expr;
        struct LEN_EXPR_AST*len_expr;
        struct FUNCTION_CALL_AST*function_call;
        struct VARIABLE_AST*var_name;
        struct NUMBER_AST*number;
        struct LOGICAL_OR_EXPR_AST*logical_or_expr;
    };

    enum AST_PRIMARY_EXPR_TYPE type;
};

struct PRIMARY_EXPR_AST*create_primary_expr_ast(void*primary_expr_ptr, enum AST_PRIMARY_EXPR_TYPE type);
void dump_primary_expr_ast_to_file(FILE*f, const struct PRIMARY_EXPR_AST*ast);
void primary_expr_ast_free(struct PRIMARY_EXPR_AST*ast);

/*
  has_property = HAS_PROPERTY LPAREN variable COMMA IDENT RPAREN
*/

struct HAS_PROPERTY_EXPR_AST
{
    struct VARIABLE_AST*obj;
    struct IDENT_AST*ident;

    size_t line;
    size_t pos;
};

struct HAS_PROPERTY_EXPR_AST*create_has_property_expr_ast(struct VARIABLE_AST*obj, struct IDENT_AST*ident, size_t line, size_t pos);
void dump_has_property_expr_ast_to_file(FILE*f, const struct HAS_PROPERTY_EXPR_AST*ast);
void has_property_expr_ast_free(struct HAS_PROPERTY_EXPR_AST*ast);

/*
  len_expr = LEN LPAREN variable RPAREN
*/

struct LEN_EXPR_AST
{
    struct VARIABLE_AST*arr;

    size_t line;
    size_t pos;    
};

struct LEN_EXPR_AST*create_len_expr_ast(struct VARIABLE_AST*arr, size_t line, size_t pos);
void dump_len_expr_ast_to_file(FILE*f, const struct LEN_EXPR_AST*ast);
void len_expr_ast_free(struct LEN_EXPR_AST*ast);

/*
  function_call = IDENT LPAREN args_list RPAREN |
                  IDENT LPAREN RPAREN
*/

struct FUNCTION_CALL_AST
{
    struct IDENT_AST*function_name;
    struct ARGS_LIST_AST*args_list;

    size_t line;
    size_t pos;
};

struct FUNCTION_CALL_AST*create_function_call_ast(struct IDENT_AST*function_name, struct ARGS_LIST_AST*args_list,
                                                  size_t line, size_t pos);
void dump_function_call_ast_to_file(FILE*f, const struct FUNCTION_CALL_AST*ast);
void function_call_ast_free(struct FUNCTION_CALL_AST*ast);

/*
  args_list = primary_expr (COMMA primary_expr)*
*/

struct ARGS_LIST_AST
{
    struct ASSIGNMENT_EXPR_AST**assignment_exprs;
    size_t                    assignment_exprs_len;

    size_t line;
    size_t pos;
};

struct ARGS_LIST_AST*create_args_list_ast(struct ASSIGNMENT_EXPR_AST**assignment_exprs, size_t assignment_exprs_len,
                                          size_t line, size_t pos);
void dump_args_list_ast_to_file(FILE*f, const struct ARGS_LIST_AST*ast);
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

    size_t line;
    size_t pos;
};

struct IDENT_AST*create_ident_ast(const char*ident, size_t line, size_t pos);
void dump_ident_ast_to_file(FILE*f, const struct IDENT_AST*ast);
void ident_ast_free(struct IDENT_AST*ast);

/*
  NUMBER.
*/

struct NUMBER_AST
{
    long long number;

    size_t line;
    size_t pos;
};

struct NUMBER_AST*create_number_ast(long long number, size_t line, size_t pos);
void dump_number_ast_to_file(FILE*f, const struct NUMBER_AST*ast);
void number_ast_free(struct NUMBER_AST*ast);

#endif  /* AST_H_INCLUDED */

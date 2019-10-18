#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "utils.h"

enum LEXER_CODES
{
    LEXER_OK                 =  0,
    LEXER_INVALID_TOKEN      = -1,
};

struct TOKEN;

struct LEXER;
typedef struct LEXER* lexer_type_t;

lexer_type_t create_lexer();

void lexer_conf(lexer_type_t lexer, const char*fname);

/*
  TOKEN is allocated inside function.
 */
void lexer_next_token(lexer_type_t lexer, struct TOKEN**token);

void lexer_free(lexer_type_t lexer);

struct POS
{
    char*program;
    unsigned program_len;

    unsigned line;
    unsigned pos;
    unsigned index;
};

struct FRAG
{
    struct POS starting;
    struct POS following;
};

enum TOKEN_TYPE
{
    TOKEN_TYPE_FUNCTION, /* function            */
    TOKEN_TYPE_LET,      /* let                 */
    TOKEN_TYPE_IF,       /* if                  */
    TOKEN_TYPE_ELSE,     /* else                */
    TOKEN_TYPE_WHILE,    /* while               */
    TOKEN_TYPE_RETURN,   /* return              */

    TOKEN_TYPE_IDENT,    /* [a-z]([a-z]|[0-9])* */

    TOKEN_TYPE_OR,       /* ||                  */
    TOKEN_TYPE_AND,      /* &&                  */

    TOKEN_TYPE_EQEQ,     /* ==                  */
    TOKEN_TYPE_NEQ,      /* !=                  */

    TOKEN_TYPE_LT,       /* <                   */
    TOKEN_TYPE_GT,       /* >                   */    
    TOKEN_TYPE_LE,       /* <=                  */
    TOKEN_TYPE_GE,       /* >=                  */

    TOKEN_TYPE_EQ,       /* =                   */
    TOKEN_TYPE_PLUS,     /* +                   */
    TOKEN_TYPE_MINUS,    /* -                   */
    TOKEN_TYPE_MUL,      /* *                   */
    TOKEN_TYPE_DIV,      /* /                   */
    TOKEN_TYPE_MOD,      /* %                   */    
    TOKEN_TYPE_LPAREN,   /* (                   */
    TOKEN_TYPE_RPAREN,   /* )                   */

    TOKEN_TYPE_NUMBER,   /* [0-9]*(.[0-9]*)?    */       
    
    TOKEN_TYPE_LBRACE,   /* {                   */
    TOKEN_TYPE_RBRACE,   /* }                   */

    TOKEN_TYPE_COMMA,    /* ,                   */
    TOKEN_TYPE_SEMI,     /* ;                   */
    TOKEN_TYPE_DOT,      /* .                   */
    TOKEN_TYPE_COLON,    /* :                   */

    TOKEN_TYPE_EOF,      /* EOF                 */
    TOKEN_TYPE_UNKNOWN,  /* UNKNOWN TOKEN       */
};

enum GROUP_TYPE
{
    GROUP_TYPE_KEYWORDS, /* function, var, if, else, while, return */
    GROUP_TYPE_IDENTS,   /* all idents                             */
    GROUP_TYPE_NUMBERS,  /* all numbers                            */
    GROUP_TYPE_OPS,      /* all operators                          */
    GROUP_TYPE_AUX,      /* auxiliary tokens: EOF and UNKNOWN      */
};

struct TOKEN
{
    enum TOKEN_TYPE token_type;
    enum GROUP_TYPE group_type;

    struct FRAG frag;

    union
    {
        long long int_val;
        double double_val;
        char   str_val[32];
    };
};

struct TOKEN*create_token();
char*token_to_string(const struct TOKEN*tok);
void token_free(struct TOKEN*tok);

#endif  /* LEXER_H_INCLUDED */

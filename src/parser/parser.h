#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "lexer.h"
#include "ast.h"

enum PARSER_CODES
{
    PARSER_OK             =  0,
    PARSER_INVALID_TOKEN  = -1,
};

struct PARSER_ERROR
{
    enum TOKEN_TYPE exp_toks[16];
    enum TOKEN_TYPE get_tok;    
    
    size_t line;
    size_t pos;
};

struct PARSER;
typedef struct PARSER* parser_type_t;

parser_type_t create_parser();

void parser_conf(parser_type_t parser, lexer_type_t lexer);

enum PARSER_CODES parser_parse(parser_type_t parser, struct UNIT_AST**unit);

struct PARSER_ERROR parser_get_error(const parser_type_t parser);

void parser_free(parser_type_t parser);

#endif  /* PARSER_H_INCLUDED */

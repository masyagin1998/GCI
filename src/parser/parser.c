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

enum PARSER_CODES parser_parse(parser_type_t parser, struct UNIT_AST**unit)
{
    struct TOKEN*tok;
    lexer_next_token(parser->lexer, &tok);
    unit_ast_read(&tok, lexer);
}

void parser_free(parser_type_t parser)
{
    SAFE_FREE(parser);
}

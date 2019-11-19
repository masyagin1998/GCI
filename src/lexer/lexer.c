#include "lexer.h"
#include "lexer_priv.h"

#include "utils.h"
#include <stdio.h>
#include <string.h>

const char function_keyword[] = "function";
const char let_keyword[]      = "let";
const char if_keyword[]       = "if";
const char else_keyword[]     = "else";
const char while_keyword[]    = "while";
const char break_keyword[]    = "break";
const char continue_keyword[] = "continue";
const char return_keyword[]   = "return";

const size_t function_keyword_len = sizeof(function_keyword) - 1;
const size_t let_keyword_len      = sizeof(let_keyword) - 1;
const size_t if_keyword_len       = sizeof(if_keyword) - 1;
const size_t else_keyword_len     = sizeof(else_keyword) - 1;
const size_t while_keyword_len    = sizeof(while_keyword) - 1;
const size_t break_keyword_len    = sizeof(break_keyword) - 1;
const size_t continue_keyword_len = sizeof(continue_keyword) - 1;
const size_t return_keyword_len   = sizeof(return_keyword) - 1;

struct LEXER
{
    char     *program;
    size_t program_len;
    char     program_name[256];

    struct POS cur;
};

lexer_type_t create_lexer()
{
    struct LEXER*ptr = NULL;
    SAFE_CALLOC(ptr, 1);
    return ptr;
}

void lexer_conf(lexer_type_t lexer, const char*fname)
{
    FILE*f;

    size_t read;
    
    strncpy(lexer->program_name, fname, sizeof(lexer->program_name));

    if (strcmp(fname, "stdin") == 0) {
        f = stdin;
    } else {
        f = file_open(fname, "r");
    }

    fseek(f, 0, SEEK_END);
    lexer->program_len = ftell(f);
    fseek(f, 0, SEEK_SET);

    SAFE_MALLOC(lexer->program, lexer->program_len + 1);

    read = fread(lexer->program, sizeof(char), lexer->program_len, f);
    if (read != lexer->program_len) {
        fprintf(stderr, "Unable to read input file \"%s\"", fname);
        exit(EXIT_FAILURE);
    }
    lexer->program[lexer->program_len] = '\0';

    lexer->cur.program = lexer->program;
    lexer->cur.program_len = lexer->program_len;

    fclose(f);
}

void lexer_next_token(lexer_type_t lexer, struct TOKEN**tok)
{    
    struct POS*cur = &(lexer->cur);
    struct POS cur_next;
    (*tok) = NULL;

    while (!pos_is_eof(cur)) {
        int found_eof = 0;
        while (pos_is_whitespace(cur) || pos_is_newline(cur)) {
            (*cur) = pos_next(cur);
            if (pos_is_eof(cur)) {
                found_eof = 1;
                break;
            }
        }

        if (found_eof) {
            break;
        }

        cur_next = pos_next(cur);

        /* keywords */
        switch (pos_get_code(cur)) {
        case 'f': {
            if (pos_check_keyword(cur, function_keyword, function_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_FUNCTION, function_keyword, function_keyword_len);
            } else {
                goto eat_ident;
            }
            break;
        }
        case 'l': {
            if (pos_check_keyword(cur, let_keyword, let_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_LET, let_keyword, let_keyword_len);   
            } else {
                goto eat_ident;
            }
            break;
        }
        case 'i': {
            if (pos_check_keyword(cur, if_keyword, if_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_IF, if_keyword, if_keyword_len);
            } else {
                goto eat_ident;
            }
            break;
        }
        case 'e': {
            if (pos_check_keyword(cur, else_keyword, else_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_ELSE, else_keyword, else_keyword_len);   
            } else {
                goto eat_ident;
            }
            break;
        }
        case 'w': {
            if (pos_check_keyword(cur, while_keyword, while_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_WHILE, while_keyword, while_keyword_len);
            } else {
                goto eat_ident;
            }
            break;
        }
        case 'b': {
            if (pos_check_keyword(cur, break_keyword, break_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_BREAK, break_keyword, break_keyword_len);
            } else {
                goto eat_ident;
            }            
            break;
        }
        case 'c': {
            if (pos_check_keyword(cur, continue_keyword, continue_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_CONTINUE, continue_keyword, continue_keyword_len);
            } else {
                goto eat_ident;
            }
            break;
        }
        case 'r': {
            if (pos_check_keyword(cur, return_keyword, return_keyword_len)) {
                token_read_keyword(tok, cur, TOKEN_TYPE_RETURN, return_keyword, return_keyword_len);
            } else {
                goto eat_ident;
            }            
            break;
        }

            /* operators */
        case '|': {
            if (pos_get_code(&cur_next) == '|') {
                cur_next = pos_next(&cur_next);
                token_read_op(tok, TOKEN_TYPE_OR, cur, &cur_next);
            }
            break;
        }
        case '&': {
            if (pos_get_code(&cur_next) == '&') {
                cur_next = pos_next(&cur_next);
                token_read_op(tok, TOKEN_TYPE_AND, cur, &cur_next);
            }            
            break;
        }
        case '=': {
            if (pos_get_code(&cur_next) == '=') {
                cur_next = pos_next(&cur_next);
                token_read_op(tok, TOKEN_TYPE_EQEQ, cur, &cur_next);
            } else {
                token_read_op(tok, TOKEN_TYPE_EQ, cur, &cur_next);
            }
            break;            
        }
        case '!': {
            if (pos_get_code(&cur_next) == '=') {
                cur_next = pos_next(&cur_next);
                token_read_op(tok, TOKEN_TYPE_NEQ, cur, &cur_next);
            }
            break;            
        }
        case '<': {
            if (pos_get_code(&cur_next) == '=') {
                cur_next = pos_next(&cur_next);
                token_read_op(tok, TOKEN_TYPE_LE, cur, &cur_next);
            } else {
                token_read_op(tok, TOKEN_TYPE_LT, cur, &cur_next);
            }
            break;            
        }
        case '>': {
            if (pos_get_code(&cur_next) == '=') {
                cur_next = pos_next(&cur_next);
                token_read_op(tok, TOKEN_TYPE_GE, cur, &cur_next);
            } else {
                token_read_op(tok, TOKEN_TYPE_GT, cur, &cur_next);
            }            
            break;            
        }
        case '+': {
            token_read_op(tok, TOKEN_TYPE_PLUS, cur, &cur_next);
            break;            
        }
        case '-': {
            token_read_op(tok, TOKEN_TYPE_MINUS, cur, &cur_next);
            break;            
        }
        case '*': {
            token_read_op(tok, TOKEN_TYPE_MUL, cur, &cur_next);
            break;            
        }
        case '/': {
            token_read_op(tok, TOKEN_TYPE_DIV, cur, &cur_next);
            break;            
        }
        case '%': {
            token_read_op(tok, TOKEN_TYPE_MOD, cur, &cur_next);
            break;            
        }
        case '(': {
            token_read_op(tok, TOKEN_TYPE_LPAREN, cur, &cur_next);
            break;            
        }
        case ')': {
            token_read_op(tok, TOKEN_TYPE_RPAREN, cur, &cur_next);
            break;            
        }
        case '{': {
            token_read_op(tok, TOKEN_TYPE_LBRACE, cur, &cur_next);
            break;            
        }
        case '}': {
            token_read_op(tok, TOKEN_TYPE_RBRACE, cur, &cur_next);
            break;            
        }
        case ',': {
            token_read_op(tok, TOKEN_TYPE_COMMA, cur, &cur_next);
            break;            
        }
        case ';': {
            token_read_op(tok, TOKEN_TYPE_SEMI, cur, &cur_next);
            break;            
        }
        case '.': {
            token_read_op(tok, TOKEN_TYPE_DOT, cur, &cur_next);
            break;            
        }
        case ':': {
            token_read_op(tok, TOKEN_TYPE_COLON, cur, &cur_next);
            break;            
        }
        default: {
            if (pos_is_digit(cur)) {
                enum LEXER_CODES r = token_read_number(tok, cur);
                if (r == LEXER_INVALID_TOKEN) {
                    (*tok)->token_type = TOKEN_TYPE_UNKNOWN;
                    (*tok)->group_type = GROUP_TYPE_AUX;
                }
            } else if (pos_is_letter(cur)) {
            eat_ident:
                token_read_ident(tok, cur);
            } else {
                token_read_unknown(tok, cur);
            }
            break;
        }
        }

        (*cur) = (*tok)->frag.following;
        if ((*tok)->token_type == TOKEN_TYPE_UNKNOWN) {
            fprintf(stderr, "%s:%zu:%zu: warning: unknown token ‘‘\n",
                    lexer->program_name,
                    (*tok)->frag.starting.line,
                    (*tok)->frag.starting.pos);
            token_free((*tok));
        } else {
            return;
        }
    }

    /* EOF */
    (*tok) = create_token();

    (*tok)->token_type = TOKEN_TYPE_EOF;
    (*tok)->group_type = GROUP_TYPE_AUX;
    (*tok)->frag.starting = (*cur);
    (*tok)->frag.following = (*cur);
}

void dump_lexer_to_xml_file(FILE*f, lexer_type_t lexer)
{
    struct TOKEN*tok;
    char tmp_str[4096];

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<tokens>\n");

    lexer_next_token(lexer, &tok);
    while (tok->token_type != TOKEN_TYPE_EOF) {
        token_to_xml_string(tok, tmp_str, sizeof(tmp_str));
        fprintf(f, "\t%s\n", tmp_str);
        token_free(tok);
        lexer_next_token(lexer, &tok);
    }
    token_free(tok);

    fprintf(f, "</tokens>\n");        
}

void lexer_free(lexer_type_t lexer)
{
    SAFE_FREE(lexer->program);
    SAFE_FREE(lexer);
}

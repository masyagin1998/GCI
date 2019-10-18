#include "lexer.h"
#include "lexer_priv.h"
#include "macro.h"

#include <stdlib.h>

struct TOKEN*create_token()
{
    struct TOKEN*ptr = NULL;
    SAFE_CALLOC(ptr, 1);
    return ptr;
}

void token_read_keyword(struct TOKEN**tok, const struct POS*pos, enum TOKEN_TYPE tok_type, const char*keyword, unsigned keyword_len)
{
    struct TOKEN*new_tok;
    struct POS p = (*pos);
    unsigned i;

    PREFIX_UNUSED(keyword);
    
    new_tok = create_token();

    new_tok->token_type = tok_type;
    new_tok->group_type = GROUP_TYPE_KEYWORDS;

    i = 0;
    while (i != keyword_len) {
        p = pos_next(&p);
        i++;
    }

    new_tok->frag.starting = (*pos);
    new_tok->frag.following = p;

    (*tok) = new_tok;
}

int token_read_number(struct TOKEN**tok, const struct POS*pos)
{
    enum LEXER_CODES r = LEXER_OK;
    
    struct TOKEN*new_tok;
    struct POS p = (*pos);
    
    new_tok = create_token();

    new_tok->token_type = TOKEN_TYPE_NUMBER;
    new_tok->group_type = GROUP_TYPE_NUMBERS;

    while (pos_is_digit(&p)) {
        p = pos_next(&p);
    }
    if (pos_get_code(&p) == '.') {
        r = LEXER_INVALID_TOKEN;
        p = pos_next(&p);
        while (pos_is_digit(&p)) {
            r = LEXER_OK;
            p = pos_next(&p);
        }
    }

    new_tok->frag.starting = (*pos);
    new_tok->frag.following = p;

    char tmp_str[64];
    snprintf(tmp_str, sizeof(tmp_str),
             "%.*s",
             new_tok->frag.following.index - new_tok->frag.starting.index,
             new_tok->frag.starting.program + new_tok->frag.starting.index);

    new_tok->int_val = atoll(tmp_str);

    (*tok) = new_tok;

    return r;
}

void token_read_ident(struct TOKEN**tok,  const struct POS*pos)
{
    struct TOKEN*new_tok;
    struct POS p = (*pos);
    
    new_tok = create_token();

    new_tok->token_type = TOKEN_TYPE_IDENT;
    new_tok->group_type = GROUP_TYPE_IDENTS;

    while (pos_is_digit(&p) || pos_is_letter(&p)) {
        p = pos_next(&p);
    }    

    new_tok->frag.starting = (*pos);
    new_tok->frag.following = p;

    snprintf(new_tok->str_val, sizeof(new_tok->str_val),
             "%.*s",
             new_tok->frag.following.index - new_tok->frag.starting.index,
             new_tok->frag.starting.program + new_tok->frag.starting.index);

    (*tok) = new_tok;
}

void token_read_op(struct TOKEN**tok, enum TOKEN_TYPE tok_type, const struct POS*starting, const struct POS*following)
{
    struct TOKEN*new_tok = create_token();

    new_tok->token_type = tok_type;
    new_tok->group_type = GROUP_TYPE_OPS;
    new_tok->frag.starting = (*starting);
    new_tok->frag.following = (*following);

    (*tok) = new_tok;
}

void token_read_unknown(struct TOKEN**tok, const struct POS*pos)
{
    struct TOKEN*new_tok;
    struct POS p = (*pos);

    new_tok = create_token();

    while (pos_is_unknown(&p)) {
        p = pos_next(&p);
    }

    new_tok->token_type = TOKEN_TYPE_UNKNOWN;
    new_tok->group_type = GROUP_TYPE_AUX;
    new_tok->frag.starting = (*pos);
    new_tok->frag.following = p;

    (*tok) = new_tok;
}

#define TOKEN_GEN_STR(group_type)                                       \
    do {                                                                \
        snprintf(str, STR_BUF_SIZE, "[%u:%u] %s: %.*s",                 \
                 tok->frag.starting.line, tok->frag.starting.pos,       \
                 group_type,                                            \
                 tok->frag.following.index - tok->frag.starting.index,  \
                 tok->frag.starting.program + tok->frag.starting.index); \
    } while(0)                                                          \
        
char*token_to_string(const struct TOKEN*tok)
{
    char*str = NULL;
    SAFE_MALLOC(str, STR_BUF_SIZE);

    switch (tok->group_type) {
    case GROUP_TYPE_KEYWORDS:
        TOKEN_GEN_STR("KEYWORD");
        break;
    case GROUP_TYPE_IDENTS:
        TOKEN_GEN_STR("IDENT");
        break;
    case GROUP_TYPE_NUMBERS:
        TOKEN_GEN_STR("NUMBER");
        break;
    case GROUP_TYPE_OPS:
        TOKEN_GEN_STR("OPERATOR");
        break;
    case GROUP_TYPE_AUX:
        TOKEN_GEN_STR("AUX");
        break;
    }

    return str;
}

void token_free(struct TOKEN*tok)
{
    SAFE_FREE(tok);
}

#ifndef LEXER_PRIV_H_INCLUDED
#define LEXER_PRIV_H_INCLUDED

#include "lexer.h"

int pos_is_eof(const struct POS*pos);
int pos_is_whitespace(const struct POS*pos);
int pos_is_newline(const struct POS*pos);
int pos_is_digit(const struct POS*pos);
int pos_is_letter(const struct POS*pos);
int pos_is_unknown(const struct POS*pos);
int pos_get_code(const struct POS*pos);

struct POS pos_next(const struct POS*pos);

int pos_check_keyword(const struct POS*pos, const char*keyword, size_t keyword_len);

void token_read_keyword(struct TOKEN**tok, const struct POS*pos, enum TOKEN_TYPE tok_type, const char*keyword, size_t keyword_len);
int token_read_number(struct TOKEN**tok, const struct POS*pos);
void token_read_ident(struct TOKEN**tok,  const struct POS*pos);
void token_read_op(struct TOKEN**tok, enum TOKEN_TYPE tok_type, const struct POS*starting, const struct POS*following);
void token_read_unknown(struct TOKEN**tok, const struct POS*pos);

#endif  /* LEXER_PRIV_H_INCLUDED */

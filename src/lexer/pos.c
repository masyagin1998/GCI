#include "lexer.h"
#include "lexer_priv.h"

__inline__ int pos_is_eof(const struct POS*pos)
{
    return ((pos->program_len == 0) || (pos->index == (pos->program_len - 1)));
}

__inline__ int pos_is_whitespace(const struct POS*pos)
{
    return ((pos_get_code(pos) == ' ') ||
            (pos_get_code(pos) == '\t'));
}

__inline__ int pos_is_newline(const struct POS*pos)
{
    if ((pos->program[pos->index] == '\r') &&
        (pos->index + 1 < pos->program_len)) {
        return (pos->program[pos->index + 1] == '\n');
    }

    return (pos->program[pos->index] == '\n');
}

__inline__ int pos_is_digit(const struct POS*pos)
{
    return ((pos_get_code(pos) >= '0') && (pos_get_code(pos) <= '9'));
}

__inline__ int pos_is_letter(const struct POS*pos)
{
    return (((pos_get_code(pos) >= 'a') && (pos_get_code(pos) <= 'z')) ||
            ((pos_get_code(pos) >= 'A') && (pos_get_code(pos) <= 'Z')));
}

int pos_is_unknown(const struct POS*pos)
{
    size_t i;
    const char symbols[] = { '|', '&', '=', '!', '<', '>', '+', '-', '*', '/', '%', '(', ')', '{', '}', ',', ';', '.', ':' };
    
    if (pos_is_whitespace(pos) || pos_is_newline(pos) || pos_is_digit(pos) || pos_is_letter(pos)) {
        return 0;
    }

    for (i = 0; i < sizeof(symbols) / sizeof(char); i++) {
        if (symbols[i] == pos_get_code(pos)) {
            return 0;
        }
    }

    return 1;
}

__inline__ int pos_get_code(const struct POS*pos)
{
    return pos->program[pos->index];
}

struct POS pos_next(const struct POS*pos)
{
    size_t new_line = pos->line;
    size_t new_pos = pos->pos;
    size_t new_index = pos->index;

    if (!pos_is_eof(pos)) {
        if (pos_is_newline(pos)) {
            new_line++;
            new_pos = 1;
        } else {
            new_pos++;
        }
        new_index++;
    }

    return (struct POS) {
        .program     = pos->program,
        .program_len = pos->program_len,
        .line        = new_line,
        .pos         = new_pos,    
        .index       = new_index,
    };
}

int pos_check_keyword(const struct POS*pos, const char*keyword, size_t keyword_len)
{
    size_t i = 0;

    struct POS p = (*pos);

    while (i != keyword_len) {
        if (keyword[i] != pos_get_code(&p)) {
            return 0;
        }
        p = pos_next(&p);
        i++;
    }

    if (!pos_is_letter(&p) && !pos_is_digit(&p)) {
        return 1;
    }

    return 0;
}

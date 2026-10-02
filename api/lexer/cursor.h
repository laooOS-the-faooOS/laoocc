#ifndef LAOOCC_LEXER_CURSOR_H
#define LAOOCC_LEXER_CURSOR_H

#include <stddef.h>

#include "lexer/token.h"

typedef struct lo_lexer_cursor {
    const char *source;
    size_t length;
    size_t position;

    size_t line;
    size_t column;
} lo_lexer_cursor;

void lo_lexer_cursor_init(
    lo_lexer_cursor *cursor,
    const char *source,
    size_t length
);

int lo_lexer_cursor_at_end(
    const lo_lexer_cursor *cursor
);

char lo_lexer_cursor_current(
    const lo_lexer_cursor *cursor
);

char lo_lexer_cursor_peek(
    const lo_lexer_cursor *cursor
);

void lo_lexer_cursor_advance(
    lo_lexer_cursor *cursor
);

#endif /* LAOOCC_LEXER_CURSOR_H */

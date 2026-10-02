#ifndef LAOOCC_LEXER_CURSOR_H
#define LAOOCC_LEXER_CURSOR_H

#include <stddef.h>

typedef struct lo_lexer_cursor {
    const char *data;
    size_t length;
    size_t offset;
    size_t line;
    size_t column;
} lo_lexer_cursor;

#endif

#ifndef LAOOCC_LEXER_H
#define LAOOCC_LEXER_H

#include <stddef.h>

#include "token.h"
#include "error.h"

typedef struct lo_lexer {
    const char *source;
    size_t length;
    size_t offset;
    size_t line;
    size_t column;
} lo_lexer;

void lo_lexer_init(
    lo_lexer *lexer,
    const char *source,
    size_t length
);

int lo_lexer_next(
    lo_lexer *lexer,
    lo_token *token,
    lo_lexer_error *error
);

#endif

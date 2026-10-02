#ifndef LAOOCC_LEXER_LEXER_H
#define LAOOCC_LEXER_LEXER_H

#include <stddef.h>

#include "lexer/cursor.h"
#include "lexer/error.h"
#include "lexer/token.h"

typedef struct lo_lexer {
    lo_lexer_cursor cursor;

    lo_lexer_token *tokens;
    size_t token_count;
    size_t token_capacity;

    lo_lexer_error error;
} lo_lexer;

void lo_lexer_init(
    lo_lexer *lexer,
    const char *source,
    size_t length
);

void lo_lexer_free(
    lo_lexer *lexer
);

int lo_lexer_run(
    lo_lexer *lexer
);

int lo_lexer_failed(
    const lo_lexer *lexer
);

const lo_lexer_error *lo_lexer_get_error(
    const lo_lexer *lexer
);

const lo_lexer_token *lo_lexer_tokens(
    const lo_lexer *lexer
);

size_t lo_lexer_token_count(
    const lo_lexer *lexer
);

#endif /* LAOOCC_LEXER_LEXER_H */

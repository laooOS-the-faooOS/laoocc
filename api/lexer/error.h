#ifndef LAOOCC_LEXER_ERROR_H
#define LAOOCC_LEXER_ERROR_H

#include <stddef.h>

typedef enum lo_lexer_error_kind {
    LO_LEXER_ERROR_NONE = 0,
    LO_LEXER_ERROR_INVALID_CHARACTER,
    LO_LEXER_ERROR_INVALID_NUMBER,
    LO_LEXER_ERROR_UNTERMINATED_STRING,
    LO_LEXER_ERROR_UNTERMINATED_CHARACTER,
    LO_LEXER_ERROR_UNTERMINATED_COMMENT
} lo_lexer_error_kind;

typedef struct lo_lexer_error {
    lo_lexer_error_kind kind;

    const char *message;

    size_t line;
    size_t column;
} lo_lexer_error;

void lo_lexer_error_init(
    lo_lexer_error *error
);

#endif /* LAOOCC_LEXER_ERROR_H */

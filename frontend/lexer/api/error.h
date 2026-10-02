#ifndef LAOOCC_LEXER_ERROR_H
#define LAOOCC_LEXER_ERROR_H

typedef enum lo_lexer_error_kind {
    LO_LEXER_ERROR_NONE = 0,
    LO_LEXER_ERROR_INVALID_CHARACTER,
    LO_LEXER_ERROR_UNTERMINATED_STRING,
    LO_LEXER_ERROR_UNTERMINATED_CHARACTER,
    LO_LEXER_ERROR_UNTERMINATED_COMMENT
} lo_lexer_error_kind;

typedef struct lo_lexer_error {
    lo_lexer_error_kind kind;
    const char *message;
    unsigned long line;
    unsigned long column;
} lo_lexer_error;

const char *lo_lexer_error_name(lo_lexer_error_kind kind);

#endif

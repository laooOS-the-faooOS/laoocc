#ifndef LAOOCC_PARSER_ERROR_H
#define LAOOCC_PARSER_ERROR_H

#include <stddef.h>

typedef enum lo_parser_error_kind {
    LO_PARSER_ERROR_NONE = 0,
    LO_PARSER_ERROR_UNEXPECTED_TOKEN,
    LO_PARSER_ERROR_UNEXPECTED_EOF,
    LO_PARSER_ERROR_INVALID_EXPRESSION,
    LO_PARSER_ERROR_INVALID_DECLARATION,
    LO_PARSER_ERROR_INVALID_STATEMENT,
    LO_PARSER_ERROR_INVALID_TYPE
} lo_parser_error_kind;

typedef struct lo_parser_error {
    lo_parser_error_kind kind;
    const char *message;
    size_t line;
    size_t column;
} lo_parser_error;

void lo_parser_error_init(lo_parser_error *error);

const char *lo_parser_error_name(lo_parser_error_kind kind);

#endif /* LAOOCC_PARSER_ERROR_H */

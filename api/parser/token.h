#ifndef LAOOCC_PARSER_TOKEN_H
#define LAOOCC_PARSER_TOKEN_H

#include <stddef.h>

typedef enum lo_parser_token_kind {
    LO_TOKEN_UNKNOWN = 0,

    LO_TOKEN_EOF,

    LO_TOKEN_IDENTIFIER,
    LO_TOKEN_NUMBER,
    LO_TOKEN_STRING,
    LO_TOKEN_CHARACTER,

    LO_TOKEN_KEYWORD,

    LO_TOKEN_PLUS,
    LO_TOKEN_MINUS,
    LO_TOKEN_STAR,
    LO_TOKEN_SLASH,
    LO_TOKEN_PERCENT,

    LO_TOKEN_EQUAL,
    LO_TOKEN_EQUAL_EQUAL,
    LO_TOKEN_NOT_EQUAL,

    LO_TOKEN_LESS,
    LO_TOKEN_LESS_EQUAL,
    LO_TOKEN_GREATER,
    LO_TOKEN_GREATER_EQUAL,

    LO_TOKEN_AND,
    LO_TOKEN_OR,
    LO_TOKEN_XOR,

    LO_TOKEN_LOGICAL_AND,
    LO_TOKEN_LOGICAL_OR,
    LO_TOKEN_NOT,

    LO_TOKEN_LEFT_PAREN,
    LO_TOKEN_RIGHT_PAREN,
    LO_TOKEN_LEFT_BRACKET,
    LO_TOKEN_RIGHT_BRACKET,
    LO_TOKEN_LEFT_BRACE,
    LO_TOKEN_RIGHT_BRACE,

    LO_TOKEN_COMMA,
    LO_TOKEN_DOT,
    LO_TOKEN_SEMICOLON,
    LO_TOKEN_COLON,

    LO_TOKEN_ARROW
} lo_parser_token_kind;

typedef struct lo_parser_token {
    lo_parser_token_kind kind;

    const char *data;
    size_t length;

    unsigned line;
    unsigned column;
} lo_parser_token;

void lo_parser_token_init(lo_parser_token *token,
                          lo_parser_token_kind kind);

#endif /* LAOOCC_PARSER_TOKEN_H */

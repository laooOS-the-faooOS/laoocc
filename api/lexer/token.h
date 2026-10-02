#ifndef LAOOCC_LEXER_TOKEN_H
#define LAOOCC_LEXER_TOKEN_H

#include <stddef.h>

typedef enum lo_lexer_token_kind {
    LO_LEXER_TOKEN_UNKNOWN = 0,
    LO_LEXER_TOKEN_EOF,

    LO_LEXER_TOKEN_IDENTIFIER,
    LO_LEXER_TOKEN_NUMBER,
    LO_LEXER_TOKEN_STRING,
    LO_LEXER_TOKEN_CHARACTER,

    LO_LEXER_TOKEN_KEYWORD,

    LO_LEXER_TOKEN_PLUS,
    LO_LEXER_TOKEN_MINUS,
    LO_LEXER_TOKEN_STAR,
    LO_LEXER_TOKEN_SLASH,
    LO_LEXER_TOKEN_PERCENT,

    LO_LEXER_TOKEN_EQUAL,
    LO_LEXER_TOKEN_EQUAL_EQUAL,
    LO_LEXER_TOKEN_NOT_EQUAL,

    LO_LEXER_TOKEN_LESS,
    LO_LEXER_TOKEN_LESS_EQUAL,
    LO_LEXER_TOKEN_GREATER,
    LO_LEXER_TOKEN_GREATER_EQUAL,

    LO_LEXER_TOKEN_AND,
    LO_LEXER_TOKEN_OR,
    LO_LEXER_TOKEN_XOR,

    LO_LEXER_TOKEN_LOGICAL_AND,
    LO_LEXER_TOKEN_LOGICAL_OR,
    LO_LEXER_TOKEN_NOT,

    LO_LEXER_TOKEN_LEFT_PAREN,
    LO_LEXER_TOKEN_RIGHT_PAREN,

    LO_LEXER_TOKEN_LEFT_BRACKET,
    LO_LEXER_TOKEN_RIGHT_BRACKET,

    LO_LEXER_TOKEN_LEFT_BRACE,
    LO_LEXER_TOKEN_RIGHT_BRACE,

    LO_LEXER_TOKEN_COMMA,
    LO_LEXER_TOKEN_DOT,
    LO_LEXER_TOKEN_SEMICOLON,
    LO_LEXER_TOKEN_COLON,
    LO_LEXER_TOKEN_ARROW
} lo_lexer_token_kind;

typedef struct lo_lexer_token {
    lo_lexer_token_kind kind;

    const char *data;
    size_t length;

    size_t line;
    size_t column;
} lo_lexer_token;

const char *lo_lexer_token_kind_name(
    lo_lexer_token_kind kind
);

void lo_lexer_token_init(
    lo_lexer_token *token
);

#endif /* LAOOCC_LEXER_TOKEN_H */

#include "lexer/lexer_api.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int lo_lexer_push(
    lo_lexer *lexer,
    lo_lexer_token token)
{
    lo_lexer_token *tokens;
    size_t capacity;

    if (!lexer)
        return -1;

    if (lexer->token_count == lexer->token_capacity) {
        capacity = lexer->token_capacity ?
                   lexer->token_capacity * 2 : 64;

        if (capacity < lexer->token_capacity)
            return -1;

        tokens = realloc(
            lexer->tokens,
            capacity * sizeof(*tokens)
        );

        if (!tokens)
            return -1;

        lexer->tokens = tokens;
        lexer->token_capacity = capacity;
    }

    lexer->tokens[lexer->token_count++] = token;
    return 0;
}

static int lo_lexer_is_identifier_start(char c)
{
    return isalpha((unsigned char)c) || c == '_';
}

static int lo_lexer_is_identifier_part(char c)
{
    return isalnum((unsigned char)c) || c == '_';
}

static int lo_lexer_is_keyword(const char *data, size_t length)
{
    static const char *keywords[] = {
        "auto",
        "break",
        "case",
        "char",
        "const",
        "continue",
        "default",
        "do",
        "double",
        "else",
        "enum",
        "extern",
        "float",
        "for",
        "goto",
        "if",
        "inline",
        "int",
        "long",
        "register",
        "restrict",
        "return",
        "short",
        "signed",
        "sizeof",
        "static",
        "struct",
        "switch",
        "typedef",
        "union",
        "unsigned",
        "void",
        "volatile",
        "while"
    };

    size_t count = sizeof(keywords) / sizeof(keywords[0]);

    for (size_t i = 0; i < count; ++i) {
        if (strlen(keywords[i]) == length &&
            memcmp(keywords[i], data, length) == 0)
            return 1;
    }

    return 0;
}

static void lo_lexer_set_error(
    lo_lexer *lexer,
    lo_lexer_error_kind kind,
    const char *message)
{
    lexer->error.kind = kind;
    lexer->error.message = message;
    lexer->error.line = lexer->cursor.line;
    lexer->error.column = lexer->cursor.column;
}

static int lo_lexer_emit(
    lo_lexer *lexer,
    lo_lexer_token_kind kind,
    size_t start,
    size_t length,
    size_t line,
    size_t column)
{
    lo_lexer_token token;

    token.kind = kind;
    token.data = lexer->cursor.source + start;
    token.length = length;
    token.line = line;
    token.column = column;

    return lo_lexer_push(lexer, token);
}

static void lo_lexer_skip_whitespace(
    lo_lexer *lexer)
{
    while (!lo_lexer_cursor_at_end(&lexer->cursor)) {
        char c = lo_lexer_cursor_current(&lexer->cursor);

        if (!isspace((unsigned char)c))
            break;

        lo_lexer_cursor_advance(&lexer->cursor);
    }
}

static int lo_lexer_skip_comment(
    lo_lexer *lexer)
{
    lo_lexer_cursor *cursor = &lexer->cursor;

    if (lo_lexer_cursor_current(cursor) != '/')
        return 0;

    if (lo_lexer_cursor_peek(cursor) == '/') {
        while (!lo_lexer_cursor_at_end(cursor) &&
               lo_lexer_cursor_current(cursor) != '\n')
            lo_lexer_cursor_advance(cursor);

        return 1;
    }

    if (lo_lexer_cursor_peek(cursor) == '*') {
        lo_lexer_cursor_advance(cursor);
        lo_lexer_cursor_advance(cursor);

        while (!lo_lexer_cursor_at_end(cursor)) {
            if (lo_lexer_cursor_current(cursor) == '*' &&
                lo_lexer_cursor_peek(cursor) == '/') {
                lo_lexer_cursor_advance(cursor);
                lo_lexer_cursor_advance(cursor);
                return 1;
            }

            lo_lexer_cursor_advance(cursor);
        }

        lo_lexer_set_error(
            lexer,
            LO_LEXER_ERROR_UNTERMINATED_COMMENT,
            "unterminated comment"
        );

        return -1;
    }

    return 0;
}

static int lo_lexer_lex_identifier(
    lo_lexer *lexer)
{
    size_t start = lexer->cursor.position;
    size_t line = lexer->cursor.line;
    size_t column = lexer->cursor.column;

    while (!lo_lexer_cursor_at_end(&lexer->cursor) &&
           lo_lexer_is_identifier_part(
               lo_lexer_cursor_current(&lexer->cursor)))
        lo_lexer_cursor_advance(&lexer->cursor);

    {
        size_t length = lexer->cursor.position - start;
        lo_lexer_token_kind kind =
            lo_lexer_is_keyword(
                lexer->cursor.source + start,
                length)
            ? LO_LEXER_TOKEN_KEYWORD
            : LO_LEXER_TOKEN_IDENTIFIER;

        return lo_lexer_emit(
            lexer,
            kind,
            start,
            length,
            line,
            column
        );
    }
}

static int lo_lexer_lex_number(
    lo_lexer *lexer)
{
    size_t start = lexer->cursor.position;
    size_t line = lexer->cursor.line;
    size_t column = lexer->cursor.column;

    while (!lo_lexer_cursor_at_end(&lexer->cursor) &&
           isdigit((unsigned char)
                   lo_lexer_cursor_current(&lexer->cursor)))
        lo_lexer_cursor_advance(&lexer->cursor);

    return lo_lexer_emit(
        lexer,
        LO_LEXER_TOKEN_NUMBER,
        start,
        lexer->cursor.position - start,
        line,
        column
    );
}

static int lo_lexer_lex_string(
    lo_lexer *lexer)
{
    size_t start = lexer->cursor.position;
    size_t line = lexer->cursor.line;
    size_t column = lexer->cursor.column;

    lo_lexer_cursor_advance(&lexer->cursor);

    while (!lo_lexer_cursor_at_end(&lexer->cursor)) {
        char c = lo_lexer_cursor_current(&lexer->cursor);

        if (c == '\\') {
            lo_lexer_cursor_advance(&lexer->cursor);

            if (!lo_lexer_cursor_at_end(&lexer->cursor))
                lo_lexer_cursor_advance(&lexer->cursor);

            continue;
        }

        if (c == '"') {
            lo_lexer_cursor_advance(&lexer->cursor);

            return lo_lexer_emit(
                lexer,
                LO_LEXER_TOKEN_STRING,
                start,
                lexer->cursor.position - start,
                line,
                column
            );
        }

        if (c == '\n') {
            lo_lexer_set_error(
                lexer,
                LO_LEXER_ERROR_UNTERMINATED_STRING,
                "unterminated string"
            );

            return -1;
        }

        lo_lexer_cursor_advance(&lexer->cursor);
    }

    lo_lexer_set_error(
        lexer,
        LO_LEXER_ERROR_UNTERMINATED_STRING,
        "unterminated string"
    );

    return -1;
}

static int lo_lexer_lex_character(
    lo_lexer *lexer)
{
    size_t start = lexer->cursor.position;
    size_t line = lexer->cursor.line;
    size_t column = lexer->cursor.column;

    lo_lexer_cursor_advance(&lexer->cursor);

    while (!lo_lexer_cursor_at_end(&lexer->cursor)) {
        char c = lo_lexer_cursor_current(&lexer->cursor);

        if (c == '\\') {
            lo_lexer_cursor_advance(&lexer->cursor);

            if (!lo_lexer_cursor_at_end(&lexer->cursor))
                lo_lexer_cursor_advance(&lexer->cursor);

            continue;
        }

        if (c == '\'') {
            lo_lexer_cursor_advance(&lexer->cursor);

            return lo_lexer_emit(
                lexer,
                LO_LEXER_TOKEN_CHARACTER,
                start,
                lexer->cursor.position - start,
                line,
                column
            );
        }

        if (c == '\n') {
            lo_lexer_set_error(
                lexer,
                LO_LEXER_ERROR_UNTERMINATED_CHARACTER,
                "unterminated character"
            );

            return -1;
        }

        lo_lexer_cursor_advance(&lexer->cursor);
    }

    lo_lexer_set_error(
        lexer,
        LO_LEXER_ERROR_UNTERMINATED_CHARACTER,
        "unterminated character"
    );

    return -1;
}

static lo_lexer_token_kind lo_lexer_operator(
    lo_lexer *lexer)
{
    lo_lexer_cursor *cursor = &lexer->cursor;
    char c = lo_lexer_cursor_current(cursor);
    char n = lo_lexer_cursor_peek(cursor);

    if (c == '+' )
        return LO_LEXER_TOKEN_PLUS;

    if (c == '-')
        return n == '>' ?
               LO_LEXER_TOKEN_ARROW :
               LO_LEXER_TOKEN_MINUS;

    if (c == '*')
        return LO_LEXER_TOKEN_STAR;

    if (c == '/')
        return LO_LEXER_TOKEN_SLASH;

    if (c == '%')
        return LO_LEXER_TOKEN_PERCENT;

    if (c == '=')
        return n == '=' ?
               LO_LEXER_TOKEN_EQUAL_EQUAL :
               LO_LEXER_TOKEN_EQUAL;

    if (c == '!')
        return n == '=' ?
               LO_LEXER_TOKEN_NOT_EQUAL :
               LO_LEXER_TOKEN_NOT;

    if (c == '<')
        return n == '=' ?
               LO_LEXER_TOKEN_LESS_EQUAL :
               LO_LEXER_TOKEN_LESS;

    if (c == '>')
        return n == '=' ?
               LO_LEXER_TOKEN_GREATER_EQUAL :
               LO_LEXER_TOKEN_GREATER;

    if (c == '&')
        return n == '&' ?
               LO_LEXER_TOKEN_LOGICAL_AND :
               LO_LEXER_TOKEN_AND;

    if (c == '|')
        return n == '|' ?
               LO_LEXER_TOKEN_LOGICAL_OR :
               LO_LEXER_TOKEN_OR;

    if (c == '^')
        return LO_LEXER_TOKEN_XOR;

    if (c == '(')
        return LO_LEXER_TOKEN_LEFT_PAREN;

    if (c == ')')
        return LO_LEXER_TOKEN_RIGHT_PAREN;

    if (c == '[')
        return LO_LEXER_TOKEN_LEFT_BRACKET;

    if (c == ']')
        return LO_LEXER_TOKEN_RIGHT_BRACKET;

    if (c == '{')
        return LO_LEXER_TOKEN_LEFT_BRACE;

    if (c == '}')
        return LO_LEXER_TOKEN_RIGHT_BRACE;

    if (c == ',')
        return LO_LEXER_TOKEN_COMMA;

    if (c == '.')
        return LO_LEXER_TOKEN_DOT;

    if (c == ';')
        return LO_LEXER_TOKEN_SEMICOLON;

    if (c == ':')
        return LO_LEXER_TOKEN_COLON;

    return LO_LEXER_TOKEN_UNKNOWN;
}

static size_t lo_lexer_operator_length(
    lo_lexer_token_kind kind)
{
    switch (kind) {
    case LO_LEXER_TOKEN_ARROW:
    case LO_LEXER_TOKEN_EQUAL_EQUAL:
    case LO_LEXER_TOKEN_NOT_EQUAL:
    case LO_LEXER_TOKEN_LESS_EQUAL:
    case LO_LEXER_TOKEN_GREATER_EQUAL:
    case LO_LEXER_TOKEN_LOGICAL_AND:
    case LO_LEXER_TOKEN_LOGICAL_OR:
        return 2;

    default:
        return 1;
    }
}

const char *lo_lexer_token_kind_name(
    lo_lexer_token_kind kind)
{
    switch (kind) {
    case LO_LEXER_TOKEN_EOF: return "eof";
    case LO_LEXER_TOKEN_IDENTIFIER: return "identifier";
    case LO_LEXER_TOKEN_NUMBER: return "number";
    case LO_LEXER_TOKEN_STRING: return "string";
    case LO_LEXER_TOKEN_CHARACTER: return "character";
    case LO_LEXER_TOKEN_KEYWORD: return "keyword";
    case LO_LEXER_TOKEN_PLUS: return "plus";
    case LO_LEXER_TOKEN_MINUS: return "minus";
    case LO_LEXER_TOKEN_STAR: return "star";
    case LO_LEXER_TOKEN_SLASH: return "slash";
    case LO_LEXER_TOKEN_PERCENT: return "percent";
    case LO_LEXER_TOKEN_EQUAL: return "equal";
    case LO_LEXER_TOKEN_EQUAL_EQUAL: return "equal_equal";
    case LO_LEXER_TOKEN_NOT_EQUAL: return "not_equal";
    case LO_LEXER_TOKEN_LESS: return "less";
    case LO_LEXER_TOKEN_LESS_EQUAL: return "less_equal";
    case LO_LEXER_TOKEN_GREATER: return "greater";
    case LO_LEXER_TOKEN_GREATER_EQUAL: return "greater_equal";
    case LO_LEXER_TOKEN_AND: return "and";
    case LO_LEXER_TOKEN_OR: return "or";
    case LO_LEXER_TOKEN_XOR: return "xor";
    case LO_LEXER_TOKEN_LOGICAL_AND: return "logical_and";
    case LO_LEXER_TOKEN_LOGICAL_OR: return "logical_or";
    case LO_LEXER_TOKEN_NOT: return "not";
    case LO_LEXER_TOKEN_LEFT_PAREN: return "left_paren";
    case LO_LEXER_TOKEN_RIGHT_PAREN: return "right_paren";
    case LO_LEXER_TOKEN_LEFT_BRACKET: return "left_bracket";
    case LO_LEXER_TOKEN_RIGHT_BRACKET: return "right_bracket";
    case LO_LEXER_TOKEN_LEFT_BRACE: return "left_brace";
    case LO_LEXER_TOKEN_RIGHT_BRACE: return "right_brace";
    case LO_LEXER_TOKEN_COMMA: return "comma";
    case LO_LEXER_TOKEN_DOT: return "dot";
    case LO_LEXER_TOKEN_SEMICOLON: return "semicolon";
    case LO_LEXER_TOKEN_COLON: return "colon";
    case LO_LEXER_TOKEN_ARROW: return "arrow";
    default: return "unknown";
    }
}

void lo_lexer_token_init(
    lo_lexer_token *token)
{
    if (!token)
        return;

    token->kind = LO_LEXER_TOKEN_UNKNOWN;
    token->data = NULL;
    token->length = 0;
    token->line = 0;
    token->column = 0;
}

void lo_lexer_cursor_init(
    lo_lexer_cursor *cursor,
    const char *source,
    size_t length)
{
    if (!cursor)
        return;

    cursor->source = source;
    cursor->length = length;
    cursor->position = 0;
    cursor->line = 1;
    cursor->column = 1;
}

int lo_lexer_cursor_at_end(
    const lo_lexer_cursor *cursor)
{
    return !cursor ||
           cursor->position >= cursor->length;
}

char lo_lexer_cursor_current(
    const lo_lexer_cursor *cursor)
{
    if (lo_lexer_cursor_at_end(cursor))
        return '\0';

    return cursor->source[cursor->position];
}

char lo_lexer_cursor_peek(
    const lo_lexer_cursor *cursor)
{
    if (!cursor ||
        cursor->position + 1 >= cursor->length)
        return '\0';

    return cursor->source[cursor->position + 1];
}

void lo_lexer_cursor_advance(
    lo_lexer_cursor *cursor)
{
    char c;

    if (lo_lexer_cursor_at_end(cursor))
        return;

    c = cursor->source[cursor->position++];

    if (c == '\n') {
        cursor->line++;
        cursor->column = 1;
    } else {
        cursor->column++;
    }
}

void lo_lexer_error_init(
    lo_lexer_error *error)
{
    if (!error)
        return;

    error->kind = LO_LEXER_ERROR_NONE;
    error->message = NULL;
    error->line = 0;
    error->column = 0;
}

void lo_lexer_init(
    lo_lexer *lexer,
    const char *source,
    size_t length)
{
    if (!lexer)
        return;

    lo_lexer_cursor_init(
        &lexer->cursor,
        source,
        length
    );

    lexer->tokens = NULL;
    lexer->token_count = 0;
    lexer->token_capacity = 0;

    lo_lexer_error_init(&lexer->error);
}

void lo_lexer_free(
    lo_lexer *lexer)
{
    if (!lexer)
        return;

    free(lexer->tokens);

    lexer->tokens = NULL;
    lexer->token_count = 0;
    lexer->token_capacity = 0;
}

int lo_lexer_run(
    lo_lexer *lexer)
{
    if (!lexer || !lexer->cursor.source)
        return -1;

    while (!lo_lexer_cursor_at_end(&lexer->cursor)) {
        char c;
        size_t start;
        size_t line;
        size_t column;
        lo_lexer_token_kind kind;

        lo_lexer_skip_whitespace(lexer);

        if (lo_lexer_cursor_at_end(&lexer->cursor))
            break;

        if (lo_lexer_skip_comment(lexer) != 0) {
            if (lexer->error.kind != LO_LEXER_ERROR_NONE)
                return -1;

            continue;
        }

        c = lo_lexer_cursor_current(&lexer->cursor);

        if (lo_lexer_is_identifier_start(c)) {
            if (lo_lexer_lex_identifier(lexer) != 0)
                return -1;

            continue;
        }

        if (isdigit((unsigned char)c)) {
            if (lo_lexer_lex_number(lexer) != 0)
                return -1;

            continue;
        }

        if (c == '"') {
            if (lo_lexer_lex_string(lexer) != 0)
                return -1;

            continue;
        }

        if (c == '\'') {
            if (lo_lexer_lex_character(lexer) != 0)
                return -1;

            continue;
        }

        start = lexer->cursor.position;
        line = lexer->cursor.line;
        column = lexer->cursor.column;

        kind = lo_lexer_operator(lexer);

        if (kind == LO_LEXER_TOKEN_UNKNOWN) {
            lo_lexer_set_error(
                lexer,
                LO_LEXER_ERROR_INVALID_CHARACTER,
                "invalid character"
            );

            return -1;
        }

        {
            size_t length =
                lo_lexer_operator_length(kind);

            for (size_t i = 0; i < length; ++i)
                lo_lexer_cursor_advance(&lexer->cursor);
        }

        if (lo_lexer_emit(
                lexer,
                kind,
                start,
                lo_lexer_operator_length(kind),
                line,
                column) != 0)
            return -1;
    }

    return lo_lexer_emit(
        lexer,
        LO_LEXER_TOKEN_EOF,
        lexer->cursor.position,
        0,
        lexer->cursor.line,
        lexer->cursor.column
    );
}

int lo_lexer_failed(
    const lo_lexer *lexer)
{
    return !lexer ||
           lexer->error.kind != LO_LEXER_ERROR_NONE;
}

const lo_lexer_error *lo_lexer_get_error(
    const lo_lexer *lexer)
{
    if (!lexer)
        return NULL;

    return &lexer->error;
}

const lo_lexer_token *lo_lexer_tokens(
    const lo_lexer *lexer)
{
    if (!lexer)
        return NULL;

    return lexer->tokens;
}

size_t lo_lexer_token_count(
    const lo_lexer *lexer)
{
    if (!lexer)
        return 0;

    return lexer->token_count;
}

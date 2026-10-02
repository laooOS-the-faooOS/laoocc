#include "parser/parser_api.h"

#include <stdint.h>

/* Token */

void lo_parser_token_init(lo_parser_token *token,
                          lo_parser_token_kind kind)
{
    if (!token)
        return;

    token->kind = kind;
    token->data = NULL;
    token->length = 0;
    token->line = 0;
    token->column = 0;
}

/* Cursor */

void lo_parser_cursor_init(lo_parser_cursor *cursor,
                           const lo_parser_token *tokens,
                           size_t count)
{
    if (!cursor)
        return;

    cursor->tokens = tokens;
    cursor->count = count;
    cursor->position = 0;
}

const lo_parser_token *lo_parser_cursor_current(
    const lo_parser_cursor *cursor)
{
    if (!cursor || !cursor->tokens)
        return NULL;

    if (cursor->position >= cursor->count)
        return NULL;

    return &cursor->tokens[cursor->position];
}

const lo_parser_token *lo_parser_cursor_peek(
    const lo_parser_cursor *cursor,
    size_t offset)
{
    size_t position;

    if (!cursor || !cursor->tokens)
        return NULL;

    if (offset > SIZE_MAX - cursor->position)
        return NULL;

    position = cursor->position + offset;

    if (position >= cursor->count)
        return NULL;

    return &cursor->tokens[position];
}

int lo_parser_cursor_at_end(
    const lo_parser_cursor *cursor)
{
    const lo_parser_token *token;

    if (!cursor)
        return 1;

    token = lo_parser_cursor_current(cursor);

    if (!token)
        return 1;

    return token->kind == LO_TOKEN_EOF;
}

int lo_parser_cursor_advance(
    lo_parser_cursor *cursor)
{
    if (!cursor)
        return -1;

    if (cursor->position >= cursor->count)
        return -1;

    cursor->position++;

    return 0;
}

int lo_parser_cursor_match(
    lo_parser_cursor *cursor,
    lo_parser_token_kind kind)
{
    const lo_parser_token *token;

    if (!cursor)
        return 0;

    token = lo_parser_cursor_current(cursor);

    if (!token || token->kind != kind)
        return 0;

    cursor->position++;

    return 1;
}

/* Parser Error */

void lo_parser_error_init(lo_parser_error *error)
{
    if (!error)
        return;

    error->kind = LO_PARSER_ERROR_NONE;
    error->message = NULL;
    error->line = 0;
    error->column = 0;
}

/* Parser */

void lo_parser_init(lo_parser *parser,
                    const lo_parser_token *tokens,
                    size_t count)
{
    if (!parser)
        return;

    lo_parser_cursor_init(&parser->cursor, tokens, count);
    lo_parser_error_init(&parser->error);
}

int lo_parser_parse(lo_parser *parser,
                    lo_ast_node *root)
{
    const lo_parser_token *token;

    if (!parser || !root)
        return -1;

    if (!parser->cursor.tokens || parser->cursor.count == 0) {
        parser->error.kind = LO_PARSER_ERROR_UNEXPECTED_EOF;
        parser->error.message = "no tokens available";
        return -1;
    }

    lo_ast_node_init(root, LO_AST_TRANSLATION_UNIT);

    while (!lo_parser_cursor_at_end(&parser->cursor)) {
        token = lo_parser_cursor_current(&parser->cursor);

        if (!token) {
            parser->error.kind = LO_PARSER_ERROR_UNEXPECTED_EOF;
            parser->error.message = "unexpected end of token stream";
            return -1;
        }

        /*
         * Full C grammar parsing will be added as the parser
         * expression, declaration, and statement APIs are built.
         */
        if (lo_parser_cursor_advance(&parser->cursor) != 0) {
            parser->error.kind = LO_PARSER_ERROR_UNEXPECTED_EOF;
            parser->error.message = "failed to advance parser";
            parser->error.line = token->line;
            parser->error.column = token->column;
            return -1;
        }
    }

    return 0;
}

int lo_parser_failed(const lo_parser *parser)
{
    if (!parser)
        return 1;

    return parser->error.kind != LO_PARSER_ERROR_NONE;
}

const lo_parser_error *lo_parser_get_error(
    const lo_parser *parser)
{
    if (!parser)
        return NULL;

    return &parser->error;
}

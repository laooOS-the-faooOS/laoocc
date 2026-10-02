#ifndef LAOOCC_PARSER_CURSOR_H
#define LAOOCC_PARSER_CURSOR_H

#include <stddef.h>

#include "parser/token.h"

typedef struct lo_parser_cursor {
    const lo_parser_token *tokens;

    size_t count;
    size_t position;
} lo_parser_cursor;

void lo_parser_cursor_init(lo_parser_cursor *cursor,
                           const lo_parser_token *tokens,
                           size_t count);

const lo_parser_token *lo_parser_cursor_current(
    const lo_parser_cursor *cursor);

const lo_parser_token *lo_parser_cursor_peek(
    const lo_parser_cursor *cursor,
    size_t offset);

int lo_parser_cursor_at_end(
    const lo_parser_cursor *cursor);

int lo_parser_cursor_advance(
    lo_parser_cursor *cursor);

int lo_parser_cursor_match(
    lo_parser_cursor *cursor,
    lo_parser_token_kind kind);

#endif /* LAOOCC_PARSER_CURSOR_H */

#ifndef LAOOCC_PARSER_PARSER_H
#define LAOOCC_PARSER_PARSER_H

#include <stddef.h>
#include "ast/ast.h"
#include "cursor.h"
#include "error.h"
#include "token.h"

typedef struct lo_typedef_entry {
    char *name;
    struct lo_typedef_entry *next;
} lo_typedef_entry;

typedef struct lo_scope {
    lo_typedef_entry *typedefs;
    struct lo_scope *parent;
} lo_scope;

typedef struct lo_parser {
    lo_parser_cursor cursor;
    lo_parser_error error;
    lo_scope *scope;
} lo_parser;

void lo_parser_init(lo_parser *parser,
                    const lo_token *tokens,
                    size_t count);

int lo_parser_parse(lo_parser *parser,
                    lo_ast_node *root);

int lo_parser_failed(const lo_parser *parser);

const lo_parser_error *lo_parser_get_error(
    const lo_parser *parser);

void lo_parser_free(lo_parser *parser);

#endif /* LAOOCC_PARSER_PARSER_H */

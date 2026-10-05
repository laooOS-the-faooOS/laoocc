#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------- Errors */

void lo_parser_error_init(lo_parser_error *error)
{
    if (!error)
        return;
    error->kind = LO_PARSER_ERROR_NONE;
    error->message = NULL;
    error->line = 0;
    error->column = 0;
}

const char *lo_parser_error_name(lo_parser_error_kind kind)
{
    switch (kind) {
    case LO_PARSER_ERROR_NONE: return "none";
    case LO_PARSER_ERROR_UNEXPECTED_TOKEN: return "unexpected token";
    case LO_PARSER_ERROR_UNEXPECTED_EOF: return "unexpected end of file";
    case LO_PARSER_ERROR_INVALID_EXPRESSION: return "invalid expression";
    case LO_PARSER_ERROR_INVALID_DECLARATION: return "invalid declaration";
    case LO_PARSER_ERROR_INVALID_STATEMENT: return "invalid statement";
    case LO_PARSER_ERROR_INVALID_TYPE: return "invalid type";
    }
    return "unknown";
}

static int set_parser_error(lo_parser *p, lo_parser_error_kind kind, const char *msg, size_t line, size_t col)
{
    p->error.kind = kind;
    p->error.message = msg;
    p->error.line = line;
    p->error.column = col;
    return -1;
}

/* ----------------------------------------------------------------- Cursor */

void lo_parser_cursor_init(lo_parser_cursor *cursor, const lo_token *tokens, size_t count)
{
    if (!cursor)
        return;
    cursor->tokens = tokens;
    cursor->count = count;
    cursor->position = 0;
}

const lo_token *lo_parser_cursor_current(const lo_parser_cursor *cursor)
{
    if (!cursor || !cursor->tokens || cursor->position >= cursor->count)
        return NULL;
    return &cursor->tokens[cursor->position];
}

const lo_token *lo_parser_cursor_peek(const lo_parser_cursor *cursor, size_t offset)
{
    if (!cursor || !cursor->tokens)
        return NULL;
    if (cursor->position + offset >= cursor->count)
        return NULL;
    return &cursor->tokens[cursor->position + offset];
}

int lo_parser_cursor_at_end(const lo_parser_cursor *cursor)
{
    const lo_token *tok = lo_parser_cursor_current(cursor);
    if (!tok)
        return 1;
    return tok->kind == LO_LEXER_TOKEN_EOF;
}

int lo_parser_cursor_advance(lo_parser_cursor *cursor)
{
    if (!cursor || cursor->position >= cursor->count)
        return -1;
    cursor->position++;
    return 0;
}

int lo_parser_cursor_match(lo_parser_cursor *cursor, lo_token_kind kind)
{
    const lo_token *tok = lo_parser_cursor_current(cursor);
    if (!tok || tok->kind != kind)
        return 0;
    cursor->position++;
    return 1;
}

/* ----------------------------------------------------------------- Scope */

static lo_scope *lo_scope_push(lo_scope *parent)
{
    lo_scope *s = malloc(sizeof(lo_scope));
    if (!s)
        return NULL;
    s->typedefs = NULL;
    s->parent = parent;
    return s;
}

static void lo_scope_pop(lo_scope **scope)
{
    lo_scope *s;
    lo_typedef_entry *entry;
    if (!scope || !*scope)
        return;
    s = *scope;
    entry = s->typedefs;
    while (entry) {
        lo_typedef_entry *next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }
    *scope = s->parent;
    free(s);
}

static void lo_scope_insert_typedef(lo_scope *scope, const char *name, size_t len)
{
    lo_typedef_entry *e;
    if (!scope || !name)
        return;
    e = malloc(sizeof(lo_typedef_entry));
    if (!e)
        return;
    e->name = malloc(len + 1);
    if (!e->name) {
        free(e);
        return;
    }
    memcpy(e->name, name, len);
    e->name[len] = '\0';
    e->next = scope->typedefs;
    scope->typedefs = e;
}

static int lo_scope_is_typedef(const lo_scope *scope, const char *name, size_t len)
{
    const lo_scope *s = scope;
    while (s) {
        lo_typedef_entry *e = s->typedefs;
        while (e) {
            if (strlen(e->name) == len && memcmp(e->name, name, len) == 0)
                return 1;
            e = e->next;
        }
        s = s->parent;
    }
    return 0;
}

/* ------------------------------------------------------------- Parser Init */

void lo_parser_init(lo_parser *parser, const lo_token *tokens, size_t count)
{
    if (!parser)
        return;
    lo_parser_cursor_init(&parser->cursor, tokens, count);
    lo_parser_error_init(&parser->error);
    parser->scope = lo_scope_push(NULL);
}

void lo_parser_free(lo_parser *parser)
{
    if (!parser)
        return;
    while (parser->scope) {
        lo_scope_pop(&parser->scope);
    }
}

int lo_parser_failed(const lo_parser *parser)
{
    if (!parser)
        return 1;
    return parser->error.kind != LO_PARSER_ERROR_NONE;
}

const lo_parser_error *lo_parser_get_error(const lo_parser *parser)
{
    if (!parser)
        return NULL;
    return &parser->error;
}

/* ------------------------------------------------------- Forward Declarations */

static lo_ast_node *parse_expression(lo_parser *p);
static lo_ast_node *parse_assignment_expression(lo_parser *p);
static lo_ast_node *parse_statement(lo_parser *p);
static lo_ast_node *parse_compound_statement(lo_parser *p);
static int parse_declaration_specifiers(lo_parser *p, lo_ast_type *type, int *storage_class, int *is_inline);
static int is_declaration_start(lo_parser *p, const lo_token *tok);

/* ------------------------------------------------------- Token Classification */

static int is_type_specifier(lo_parser *p, const lo_token *tok)
{
    if (!tok)
        return 0;
    switch (tok->kind) {
    case LO_LEXER_TOKEN_KW_VOID:
    case LO_LEXER_TOKEN_KW_CHAR:
    case LO_LEXER_TOKEN_KW_SHORT:
    case LO_LEXER_TOKEN_KW_INT:
    case LO_LEXER_TOKEN_KW_LONG:
    case LO_LEXER_TOKEN_KW_FLOAT:
    case LO_LEXER_TOKEN_KW_DOUBLE:
    case LO_LEXER_TOKEN_KW_SIGNED:
    case LO_LEXER_TOKEN_KW_UNSIGNED:
    case LO_LEXER_TOKEN_KW_BOOL:
    case LO_LEXER_TOKEN_KW_STRUCT:
    case LO_LEXER_TOKEN_KW_UNION:
    case LO_LEXER_TOKEN_KW_ENUM:
        return 1;
    case LO_LEXER_TOKEN_IDENTIFIER:
        return lo_scope_is_typedef(p->scope, tok->start, tok->length);
    default:
        return 0;
    }
}

static int is_storage_class_specifier(const lo_token *tok)
{
    if (!tok)
        return 0;
    switch (tok->kind) {
    case LO_LEXER_TOKEN_KW_TYPEDEF:
    case LO_LEXER_TOKEN_KW_EXTERN:
    case LO_LEXER_TOKEN_KW_STATIC:
    case LO_LEXER_TOKEN_KW_AUTO:
    case LO_LEXER_TOKEN_KW_REGISTER:
        return 1;
    default:
        return 0;
    }
}

static int is_type_qualifier(const lo_token *tok)
{
    if (!tok)
        return 0;
    switch (tok->kind) {
    case LO_LEXER_TOKEN_KW_CONST:
    case LO_LEXER_TOKEN_KW_VOLATILE:
    case LO_LEXER_TOKEN_KW_RESTRICT:
        return 1;
    default:
        return 0;
    }
}

static int is_function_specifier(const lo_token *tok)
{
    if (!tok)
        return 0;
    return tok->kind == LO_LEXER_TOKEN_KW_INLINE;
}

static int is_declaration_start(lo_parser *p, const lo_token *tok)
{
    if (!tok)
        return 0;
    return is_type_specifier(p, tok) ||
           is_storage_class_specifier(tok) ||
           is_type_qualifier(tok) ||
           is_function_specifier(tok);
}

/* ------------------------------------------------------- Expression Parsing */

static lo_ast_node *parse_primary_expression(lo_parser *p)
{
    const lo_token *tok = lo_parser_cursor_current(&p->cursor);
    if (!tok) {
        set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_EOF, "unexpected end of file in expression", 0, 0);
        return NULL;
    }

    if (tok->kind == LO_LEXER_TOKEN_IDENTIFIER) {
        lo_ast_expr *id = lo_ast_identifier_new(tok->start, tok->length);
        id->node.line = tok->line;
        id->node.column = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        return (lo_ast_node *)id;
    }

    if (tok->kind == LO_LEXER_TOKEN_NUMBER) {
        lo_ast_expr *lit = lo_ast_literal_new(LO_AST_LITERAL_NUMBER, tok->start, tok->length);
        lit->node.line = tok->line;
        lit->node.column = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        return (lo_ast_node *)lit;
    }

    if (tok->kind == LO_LEXER_TOKEN_CHARACTER) {
        lo_ast_expr *lit = lo_ast_literal_new(LO_AST_LITERAL_CHARACTER, tok->start, tok->length);
        lit->node.line = tok->line;
        lit->node.column = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        return (lo_ast_node *)lit;
    }

    if (tok->kind == LO_LEXER_TOKEN_STRING) {
        lo_ast_expr *lit = lo_ast_literal_new(LO_AST_LITERAL_STRING, tok->start, tok->length);
        lit->node.line = tok->line;
        lit->node.column = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        return (lo_ast_node *)lit;
    }

    if (tok->kind == LO_LEXER_TOKEN_LPAREN) {
        size_t start_line = tok->line;
        size_t start_col = tok->column;
        lo_parser_cursor_advance(&p->cursor);

        /* Check if this is a Cast expression: (type_name)expr */
        const lo_token *inner = lo_parser_cursor_current(&p->cursor);
        if (inner && is_declaration_start(p, inner)) {
            size_t save_pos = p->cursor.position;
            lo_ast_type cast_type;
            int dummy_sc = 0, dummy_in = 0;
            if (parse_declaration_specifiers(p, &cast_type, &dummy_sc, &dummy_in) == 0) {
                /* check for abstract pointer */
                while (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_STAR)) {
                    lo_ast_type *ptr = lo_ast_type_pointer_to(&cast_type);
                    lo_ast_type_free_members(&cast_type);
                    cast_type = *ptr;
                    free(ptr);
                }
                if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
                    /* Successfully parsed (type_name), now parse cast operand */
                    lo_ast_node *operand = parse_assignment_expression(p);
                    if (operand) {
                        lo_ast_expr *cast_node = lo_ast_cast_new(&cast_type, operand);
                        cast_node->node.line = start_line;
                        cast_node->node.column = start_col;
                        lo_ast_type_free_members(&cast_type);
                        return (lo_ast_node *)cast_node;
                    }
                }
            }
            /* Not a cast; rewind to after '(' and parse as parenthesized expr */
            lo_ast_type_free_members(&cast_type);
            p->cursor.position = save_pos;
            lo_parser_error_init(&p->error);
        }

        lo_ast_node *expr = parse_expression(p);
        if (!expr)
            return NULL;
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ')' after expression", tok->line, tok->column);
            lo_ast_node_free(expr);
            free(expr);
            return NULL;
        }
        return expr;
    }

    set_parser_error(p, LO_PARSER_ERROR_INVALID_EXPRESSION, "unexpected token in primary expression", tok->line, tok->column);
    return NULL;
}

static lo_ast_node *parse_postfix_expression(lo_parser *p)
{
    lo_ast_node *expr = parse_primary_expression(p);
    if (!expr)
        return NULL;

    for (;;) {
        const lo_token *tok = lo_parser_cursor_current(&p->cursor);
        if (!tok)
            break;

        if (tok->kind == LO_LEXER_TOKEN_LBRACKET) {
            lo_parser_cursor_advance(&p->cursor);
            lo_ast_node *idx = parse_expression(p);
            if (!idx) {
                lo_ast_node_free(expr);
                free(expr);
                return NULL;
            }
            if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACKET)) {
                set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ']' after subscript", tok->line, tok->column);
                lo_ast_node_free(expr);
                free(expr);
                lo_ast_node_free(idx);
                free(idx);
                return NULL;
            }
            expr = (lo_ast_node *)lo_ast_subscript_new(expr, idx);
        } else if (tok->kind == LO_LEXER_TOKEN_LPAREN) {
            lo_parser_cursor_advance(&p->cursor);
            lo_ast_expr *call = lo_ast_call_new(expr);
            call->node.line = tok->line;
            call->node.column = tok->column;

            if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
                for (;;) {
                    lo_ast_node *arg = parse_assignment_expression(p);
                    if (!arg) {
                        lo_ast_node_free(&call->node);
                        free(call);
                        return NULL;
                    }
                    lo_ast_node_add_child(&call->node, arg);

                    if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COMMA))
                        continue;
                    if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN))
                        break;

                    set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ',' or ')' in argument list", tok->line, tok->column);
                    lo_ast_node_free(&call->node);
                    free(call);
                    return NULL;
                }
            }
            expr = (lo_ast_node *)call;
        } else if (tok->kind == LO_LEXER_TOKEN_DOT || tok->kind == LO_LEXER_TOKEN_ARROW) {
            int is_arrow = (tok->kind == LO_LEXER_TOKEN_ARROW);
            lo_parser_cursor_advance(&p->cursor);
            const lo_token *id = lo_parser_cursor_current(&p->cursor);
            if (!id || id->kind != LO_LEXER_TOKEN_IDENTIFIER) {
                set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected member identifier", tok->line, tok->column);
                lo_ast_node_free(expr);
                free(expr);
                return NULL;
            }
            expr = (lo_ast_node *)lo_ast_member_new(expr, id->start, id->length, is_arrow);
            lo_parser_cursor_advance(&p->cursor);
        } else if (tok->kind == LO_LEXER_TOKEN_INCREMENT) {
            lo_parser_cursor_advance(&p->cursor);
            expr = (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_POST_INC, expr);
        } else if (tok->kind == LO_LEXER_TOKEN_DECREMENT) {
            lo_parser_cursor_advance(&p->cursor);
            expr = (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_POST_DEC, expr);
        } else {
            break;
        }
    }

    return expr;
}

static lo_ast_node *parse_unary_expression(lo_parser *p)
{
    const lo_token *tok = lo_parser_cursor_current(&p->cursor);
    if (!tok)
        return NULL;

    if (tok->kind == LO_LEXER_TOKEN_INCREMENT) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_PRE_INC, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_DECREMENT) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_PRE_DEC, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_PLUS) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_PLUS, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_MINUS) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_MINUS, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_BANG) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_NOT, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_TILDE) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_BIT_NOT, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_STAR) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_DEREFERENCE, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_AMPERSAND) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_ADDRESS, operand);
    }
    if (tok->kind == LO_LEXER_TOKEN_KW_SIZEOF) {
        lo_parser_cursor_advance(&p->cursor);
        const lo_token *next = lo_parser_cursor_current(&p->cursor);
        if (next && next->kind == LO_LEXER_TOKEN_LPAREN) {
            const lo_token *after_paren = lo_parser_cursor_peek(&p->cursor, 1);
            if (after_paren && is_declaration_start(p, after_paren)) {
                lo_parser_cursor_advance(&p->cursor); /* consume '(' */
                lo_ast_type stype;
                int dummy_sc = 0, dummy_in = 0;
                parse_declaration_specifiers(p, &stype, &dummy_sc, &dummy_in);
                while (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_STAR)) {
                    lo_ast_type *ptr = lo_ast_type_pointer_to(&stype);
                    lo_ast_type_free_members(&stype);
                    stype = *ptr;
                    free(ptr);
                }
                lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN);
                lo_ast_expr *sz = lo_ast_unary_new(LO_AST_UNARY_SIZEOF, NULL);
                sz->cast_type = stype;
                return (lo_ast_node *)sz;
            }
        }
        lo_ast_node *operand = parse_unary_expression(p);
        if (!operand) return NULL;
        return (lo_ast_node *)lo_ast_unary_new(LO_AST_UNARY_SIZEOF, operand);
    }

    return parse_postfix_expression(p);
}

/* Binary operator precedence climbing */

static int get_binary_op_precedence(lo_token_kind kind, lo_ast_binary_op *bin_op)
{
    switch (kind) {
    case LO_LEXER_TOKEN_STAR:    *bin_op = LO_AST_BIN_MUL; return 12;
    case LO_LEXER_TOKEN_SLASH:   *bin_op = LO_AST_BIN_DIV; return 12;
    case LO_LEXER_TOKEN_PERCENT: *bin_op = LO_AST_BIN_MOD; return 12;

    case LO_LEXER_TOKEN_PLUS:    *bin_op = LO_AST_BIN_ADD; return 11;
    case LO_LEXER_TOKEN_MINUS:   *bin_op = LO_AST_BIN_SUB; return 11;

    case LO_LEXER_TOKEN_SHL:     *bin_op = LO_AST_BIN_SHL; return 10;
    case LO_LEXER_TOKEN_SHR:     *bin_op = LO_AST_BIN_SHR; return 10;

    case LO_LEXER_TOKEN_LESS:          *bin_op = LO_AST_BIN_LT; return 9;
    case LO_LEXER_TOKEN_LESS_EQUAL:    *bin_op = LO_AST_BIN_LE; return 9;
    case LO_LEXER_TOKEN_GREATER:       *bin_op = LO_AST_BIN_GT; return 9;
    case LO_LEXER_TOKEN_GREATER_EQUAL: *bin_op = LO_AST_BIN_GE; return 9;

    case LO_LEXER_TOKEN_EQUAL:     *bin_op = LO_AST_BIN_EQ; return 8;
    case LO_LEXER_TOKEN_NOT_EQUAL: *bin_op = LO_AST_BIN_NE; return 8;

    case LO_LEXER_TOKEN_AMPERSAND: *bin_op = LO_AST_BIN_BIT_AND; return 7;
    case LO_LEXER_TOKEN_CARET:     *bin_op = LO_AST_BIN_BIT_XOR; return 6;
    case LO_LEXER_TOKEN_PIPE:      *bin_op = LO_AST_BIN_BIT_OR;  return 5;

    case LO_LEXER_TOKEN_AND:       *bin_op = LO_AST_BIN_LOGICAL_AND; return 4;
    case LO_LEXER_TOKEN_OR:        *bin_op = LO_AST_BIN_LOGICAL_OR;  return 3;

    default:
        *bin_op = LO_AST_BIN_UNKNOWN;
        return -1;
    }
}

static lo_ast_node *parse_binary_expression_precedence(lo_parser *p, int min_prec)
{
    lo_ast_node *left = parse_unary_expression(p);
    if (!left)
        return NULL;

    for (;;) {
        const lo_token *tok = lo_parser_cursor_current(&p->cursor);
        if (!tok)
            break;

        lo_ast_binary_op op;
        int prec = get_binary_op_precedence(tok->kind, &op);
        if (prec < min_prec)
            break;

        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *right = parse_binary_expression_precedence(p, prec + 1);
        if (!right) {
            lo_ast_node_free(left);
            free(left);
            return NULL;
        }

        left = (lo_ast_node *)lo_ast_binary_new(op, left, right);
    }

    return left;
}

static lo_ast_node *parse_conditional_expression(lo_parser *p)
{
    lo_ast_node *cond = parse_binary_expression_precedence(p, 0);
    if (!cond)
        return NULL;

    if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_QUESTION)) {
        lo_ast_node *true_expr = parse_expression(p);
        if (!true_expr) {
            lo_ast_node_free(cond);
            free(cond);
            return NULL;
        }
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COLON)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ':' in conditional expression", 0, 0);
            lo_ast_node_free(cond);
            free(cond);
            lo_ast_node_free(true_expr);
            free(true_expr);
            return NULL;
        }
        lo_ast_node *false_expr = parse_conditional_expression(p);
        if (!false_expr) {
            lo_ast_node_free(cond);
            free(cond);
            lo_ast_node_free(true_expr);
            free(true_expr);
            return NULL;
        }
        return (lo_ast_node *)lo_ast_conditional_new(cond, true_expr, false_expr);
    }

    return cond;
}

static int get_assignment_op(lo_token_kind kind, lo_ast_assign_op *op)
{
    switch (kind) {
    case LO_LEXER_TOKEN_ASSIGN:     *op = LO_AST_ASSIGN_EQUAL; return 1;
    case LO_LEXER_TOKEN_ADD_ASSIGN: *op = LO_AST_ASSIGN_ADD;   return 1;
    case LO_LEXER_TOKEN_SUB_ASSIGN: *op = LO_AST_ASSIGN_SUB;   return 1;
    case LO_LEXER_TOKEN_MUL_ASSIGN: *op = LO_AST_ASSIGN_MUL;   return 1;
    case LO_LEXER_TOKEN_DIV_ASSIGN: *op = LO_AST_ASSIGN_DIV;   return 1;
    case LO_LEXER_TOKEN_MOD_ASSIGN: *op = LO_AST_ASSIGN_MOD;   return 1;
    case LO_LEXER_TOKEN_SHL_ASSIGN: *op = LO_AST_ASSIGN_SHL;   return 1;
    case LO_LEXER_TOKEN_SHR_ASSIGN: *op = LO_AST_ASSIGN_SHR;   return 1;
    case LO_LEXER_TOKEN_AND_ASSIGN: *op = LO_AST_ASSIGN_AND;   return 1;
    case LO_LEXER_TOKEN_XOR_ASSIGN: *op = LO_AST_ASSIGN_XOR;   return 1;
    case LO_LEXER_TOKEN_OR_ASSIGN:  *op = LO_AST_ASSIGN_OR;    return 1;
    default: return 0;
    }
}

static lo_ast_node *parse_assignment_expression(lo_parser *p)
{
    lo_ast_node *left = parse_conditional_expression(p);
    if (!left)
        return NULL;

    const lo_token *tok = lo_parser_cursor_current(&p->cursor);
    if (tok) {
        lo_ast_assign_op op;
        if (get_assignment_op(tok->kind, &op)) {
            lo_parser_cursor_advance(&p->cursor);
            lo_ast_node *right = parse_assignment_expression(p);
            if (!right) {
                lo_ast_node_free(left);
                free(left);
                return NULL;
            }
            return (lo_ast_node *)lo_ast_assign_new(op, left, right);
        }
    }

    return left;
}

static lo_ast_node *parse_expression(lo_parser *p)
{
    lo_ast_node *left = parse_assignment_expression(p);
    if (!left)
        return NULL;

    while (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COMMA)) {
        lo_ast_node *right = parse_assignment_expression(p);
        if (!right) {
            lo_ast_node_free(left);
            free(left);
            return NULL;
        }
        left = (lo_ast_node *)lo_ast_binary_new(LO_AST_BIN_COMMA, left, right);
    }

    return left;
}

/* ------------------------------------------------------------- Initializers */

static lo_ast_node *parse_initializer(lo_parser *p)
{
    const lo_token *tok = lo_parser_cursor_current(&p->cursor);
    if (!tok)
        return NULL;

    if (tok->kind == LO_LEXER_TOKEN_LBRACE) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_expr *init_list = lo_ast_expr_new(LO_AST_INIT_LIST_EXPR);
        if (!init_list) return NULL;

        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACE)) {
            for (;;) {
                lo_ast_node *elem = parse_initializer(p);
                if (!elem) {
                    lo_ast_node_free(&init_list->node);
                    free(init_list);
                    return NULL;
                }
                lo_ast_node_add_child(&init_list->node, elem);

                if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COMMA)) {
                    if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACE))
                        break;
                    continue;
                }
                if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACE))
                    break;

                set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ',' or '}' in initializer list", tok->line, tok->column);
                lo_ast_node_free(&init_list->node);
                free(init_list);
                return NULL;
            }
        }
        return (lo_ast_node *)init_list;
    }

    return parse_assignment_expression(p);
}

/* ---------------------------------------------------- Declaration Specifiers */

static int parse_declaration_specifiers(lo_parser *p, lo_ast_type *type, int *storage_class, int *is_inline)
{
    lo_ast_type_init(type, LO_AST_TYPE_UNKNOWN);
    *storage_class = 0;
    *is_inline = 0;

    int type_specified = 0;
    int is_long = 0;
    int is_short = 0;
    int is_signed = 0;
    int is_unsigned = 0;

    for (;;) {
        const lo_token *tok = lo_parser_cursor_current(&p->cursor);
        if (!tok)
            break;

        /* Storage class */
        if (tok->kind == LO_LEXER_TOKEN_KW_TYPEDEF) {
            *storage_class = LO_LEXER_TOKEN_KW_TYPEDEF;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_EXTERN) {
            *storage_class = LO_LEXER_TOKEN_KW_EXTERN;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_STATIC) {
            *storage_class = LO_LEXER_TOKEN_KW_STATIC;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_AUTO) {
            *storage_class = LO_LEXER_TOKEN_KW_AUTO;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_REGISTER) {
            *storage_class = LO_LEXER_TOKEN_KW_REGISTER;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }

        /* Qualifiers */
        if (tok->kind == LO_LEXER_TOKEN_KW_CONST) {
            type->is_const = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_VOLATILE) {
            type->is_volatile = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_RESTRICT) {
            type->is_restrict = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_INLINE) {
            *is_inline = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }

        /* Type specifiers */
        if (tok->kind == LO_LEXER_TOKEN_KW_VOID) {
            type->kind = LO_AST_TYPE_VOID;
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);
            break;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_BOOL) {
            type->kind = LO_AST_TYPE_BOOL;
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);
            break;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_CHAR) {
            type->kind = LO_AST_TYPE_CHAR;
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);
            break;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_SHORT) {
            is_short = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_LONG) {
            is_long++;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_INT) {
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_SIGNED) {
            is_signed = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_UNSIGNED) {
            is_unsigned = 1;
            lo_parser_cursor_advance(&p->cursor);
            continue;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_FLOAT) {
            type->kind = LO_AST_TYPE_FLOAT;
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);
            break;
        }
        if (tok->kind == LO_LEXER_TOKEN_KW_DOUBLE) {
            type->kind = LO_AST_TYPE_DOUBLE;
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);
            break;
        }

        /* Struct or Union */
        if (tok->kind == LO_LEXER_TOKEN_KW_STRUCT || tok->kind == LO_LEXER_TOKEN_KW_UNION) {
            int is_union = (tok->kind == LO_LEXER_TOKEN_KW_UNION);
            type->kind = is_union ? LO_AST_TYPE_UNION : LO_AST_TYPE_STRUCT;
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);

            const lo_token *tag = lo_parser_cursor_current(&p->cursor);
            if (tag && tag->kind == LO_LEXER_TOKEN_IDENTIFIER) {
                type->tag_or_typedef_name = malloc(tag->length + 1);
                memcpy(type->tag_or_typedef_name, tag->start, tag->length);
                type->tag_or_typedef_name[tag->length] = '\0';
                lo_parser_cursor_advance(&p->cursor);
            }

            /* Struct body { ... } */
            if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LBRACE)) {
                while (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACE) &&
                       !lo_parser_cursor_at_end(&p->cursor)) {
                    lo_ast_type mtype;
                    int msc = 0, minl = 0;
                    if (parse_declaration_specifiers(p, &mtype, &msc, &minl) != 0) {
                        break;
                    }
                    /* member declarators */
                    for (;;) {
                        while (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_STAR)) {}
                        const lo_token *mid = lo_parser_cursor_current(&p->cursor);
                        if (mid && mid->kind == LO_LEXER_TOKEN_IDENTIFIER) {
                            lo_parser_cursor_advance(&p->cursor);
                        }
                        if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LBRACKET)) {
                            while (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACKET) &&
                                   !lo_parser_cursor_at_end(&p->cursor)) {
                                lo_parser_cursor_advance(&p->cursor);
                            }
                        }
                        if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COMMA))
                            continue;
                        break;
                    }
                    lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
                    lo_ast_type_free_members(&mtype);
                }
            }
            break;
        }

        /* Enum */
        if (tok->kind == LO_LEXER_TOKEN_KW_ENUM) {
            type->kind = LO_AST_TYPE_ENUM;
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);

            const lo_token *tag = lo_parser_cursor_current(&p->cursor);
            if (tag && tag->kind == LO_LEXER_TOKEN_IDENTIFIER) {
                type->tag_or_typedef_name = malloc(tag->length + 1);
                memcpy(type->tag_or_typedef_name, tag->start, tag->length);
                type->tag_or_typedef_name[tag->length] = '\0';
                lo_parser_cursor_advance(&p->cursor);
            }

            if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LBRACE)) {
                while (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACE) &&
                       !lo_parser_cursor_at_end(&p->cursor)) {
                    const lo_token *eid = lo_parser_cursor_current(&p->cursor);
                    if (eid && eid->kind == LO_LEXER_TOKEN_IDENTIFIER) {
                        lo_parser_cursor_advance(&p->cursor);
                        if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_ASSIGN)) {
                            lo_ast_node *val = parse_assignment_expression(p);
                            lo_ast_node_free(val);
                            free(val);
                        }
                    }
                    if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COMMA))
                        continue;
                    if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACE))
                        break;
                    break;
                }
            }
            break;
        }

        /* Typedef Name */
        if (tok->kind == LO_LEXER_TOKEN_IDENTIFIER && lo_scope_is_typedef(p->scope, tok->start, tok->length)) {
            type->kind = LO_AST_TYPE_TYPEDEF_NAME;
            type->tag_or_typedef_name = malloc(tok->length + 1);
            memcpy(type->tag_or_typedef_name, tok->start, tok->length);
            type->tag_or_typedef_name[tok->length] = '\0';
            type_specified = 1;
            lo_parser_cursor_advance(&p->cursor);
            break;
        }

        break;
    }

    if (type_specified) {
        if (type->kind == LO_AST_TYPE_UNKNOWN) {
            if (is_short) type->kind = LO_AST_TYPE_SHORT;
            else if (is_long >= 2) type->kind = LO_AST_TYPE_LONG_LONG;
            else if (is_long == 1) type->kind = LO_AST_TYPE_LONG;
            else type->kind = LO_AST_TYPE_INT;
        }
    } else if (is_short || is_long || is_signed || is_unsigned) {
        if (is_short) type->kind = LO_AST_TYPE_SHORT;
        else if (is_long >= 2) type->kind = LO_AST_TYPE_LONG_LONG;
        else if (is_long == 1) type->kind = LO_AST_TYPE_LONG;
        else type->kind = LO_AST_TYPE_INT;
        type_specified = 1;
    }

    if (type_specified) {
        type->is_signed = is_signed || !is_unsigned;
        return 0;
    }

    return -1;
}

/* -------------------------------------------------------- Declarator Parsing */

typedef struct lo_declarator_result {
    char *name;
    size_t name_len;
    lo_ast_type type;
    int is_function;
    lo_ast_node *param_list;
} lo_declarator_result;

static void lo_declarator_result_init(lo_declarator_result *r)
{
    r->name = NULL;
    r->name_len = 0;
    lo_ast_type_init(&r->type, LO_AST_TYPE_UNKNOWN);
    r->is_function = 0;
    r->param_list = NULL;
}

static int parse_declarator(lo_parser *p, const lo_ast_type *base_type, lo_declarator_result *result)
{
    lo_declarator_result_init(result);

    /* Pointer prefix: * [const] * ... */
    size_t pointer_levels = 0;
    while (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_STAR)) {
        pointer_levels++;
        while (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_KW_CONST) ||
               lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_KW_VOLATILE) ||
               lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_KW_RESTRICT)) {
            /* qualifiers on pointer */
        }
    }

    /* Direct declarator */
    int is_paren = 0;
    if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LPAREN)) {
        is_paren = 1;
        /* nested declarator */
        parse_declarator(p, base_type, result);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN);
    } else {
        const lo_token *id = lo_parser_cursor_current(&p->cursor);
        if (id && id->kind == LO_LEXER_TOKEN_IDENTIFIER) {
            result->name = malloc(id->length + 1);
            memcpy(result->name, id->start, id->length);
            result->name[id->length] = '\0';
            result->name_len = id->length;
            lo_parser_cursor_advance(&p->cursor);
        }
    }

    /* Postfix: array bounds [N] or function params (params) */
    for (;;) {
        const lo_token *tok = lo_parser_cursor_current(&p->cursor);
        if (!tok)
            break;

        if (tok->kind == LO_LEXER_TOKEN_LBRACKET) {
            lo_parser_cursor_advance(&p->cursor);
            size_t arr_size = 0;
            int has_size = 0;
            const lo_token *sz_tok = lo_parser_cursor_current(&p->cursor);
            if (sz_tok && sz_tok->kind == LO_LEXER_TOKEN_NUMBER) {
                arr_size = (size_t)strtoul(sz_tok->start, NULL, 0);
                has_size = 1;
                lo_parser_cursor_advance(&p->cursor);
            }
            lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACKET);

            /* Wrap base type in array */
            lo_ast_type *arr = lo_ast_type_array_of(base_type, arr_size, has_size);
            result->type = *arr;
            free(arr);
        } else if (tok->kind == LO_LEXER_TOKEN_LPAREN && !is_paren) {
            lo_parser_cursor_advance(&p->cursor);
            result->is_function = 1;
            result->param_list = (lo_ast_node *)lo_ast_compound_stmt_new();

            if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
                if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_KW_VOID) &&
                    lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
                    /* (void) -> 0 params */
                } else {
                    for (;;) {
                        if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_ELLIPSIS)) {
                            /* variadic */
                            break;
                        }

                        lo_ast_type ptype;
                        int psc = 0, pinl = 0;
                        if (parse_declaration_specifiers(p, &ptype, &psc, &pinl) != 0) {
                            break;
                        }

                        lo_declarator_result pres;
                        parse_declarator(p, &ptype, &pres);
                        lo_ast_decl *pdecl = lo_ast_param_decl_new(pres.name, pres.name_len, &pres.type);
                        lo_ast_node_add_child(result->param_list, (lo_ast_node *)pdecl);

                        free(pres.name);
                        lo_ast_type_free_members(&ptype);

                        if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COMMA))
                            continue;
                        if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN))
                            break;

                        set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ',' or ')' in parameter list", tok->line, tok->column);
                        break;
                    }
                }
            }
        } else {
            break;
        }
    }

    if (result->type.kind == LO_AST_TYPE_UNKNOWN) {
        result->type = *lo_ast_type_clone(base_type);
    }

    while (pointer_levels > 0) {
        lo_ast_type *ptr = lo_ast_type_pointer_to(&result->type);
        lo_ast_type_free_members(&result->type);
        result->type = *ptr;
        free(ptr);
        pointer_levels--;
    }

    return 0;
}

/* --------------------------------------------------------- Statement Parsing */

static lo_ast_node *parse_statement(lo_parser *p)
{
    const lo_token *tok = lo_parser_cursor_current(&p->cursor);
    if (!tok) {
        set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_EOF, "unexpected end of file in statement", 0, 0);
        return NULL;
    }

    /* Compound statement */
    if (tok->kind == LO_LEXER_TOKEN_LBRACE) {
        return parse_compound_statement(p);
    }

    /* Return statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_RETURN) {
        size_t line = tok->line;
        size_t col = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *val = NULL;
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON)) {
            val = parse_expression(p);
            if (!val)
                return NULL;
            if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON)) {
                set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ';' after return value", tok->line, tok->column);
                lo_ast_node_free(val);
                free(val);
                return NULL;
            }
        }
        lo_ast_stmt *ret = lo_ast_return_stmt_new(val);
        ret->node.line = line;
        ret->node.column = col;
        return (lo_ast_node *)ret;
    }

    /* If statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_IF) {
        size_t line = tok->line;
        size_t col = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LPAREN)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected '(' after 'if'", tok->line, tok->column);
            return NULL;
        }
        lo_ast_node *cond = parse_expression(p);
        if (!cond) return NULL;
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ')' after if condition", tok->line, tok->column);
            lo_ast_node_free(cond);
            free(cond);
            return NULL;
        }
        lo_ast_node *then_b = parse_statement(p);
        if (!then_b) {
            lo_ast_node_free(cond);
            free(cond);
            return NULL;
        }
        lo_ast_node *else_b = NULL;
        if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_KW_ELSE)) {
            else_b = parse_statement(p);
            if (!else_b) {
                lo_ast_node_free(cond);
                free(cond);
                lo_ast_node_free(then_b);
                free(then_b);
                return NULL;
            }
        }
        lo_ast_stmt *if_stmt = lo_ast_if_stmt_new(cond, then_b, else_b);
        if_stmt->node.line = line;
        if_stmt->node.column = col;
        return (lo_ast_node *)if_stmt;
    }

    /* While statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_WHILE) {
        size_t line = tok->line;
        size_t col = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LPAREN)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected '(' after 'while'", tok->line, tok->column);
            return NULL;
        }
        lo_ast_node *cond = parse_expression(p);
        if (!cond) return NULL;
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ')' after while condition", tok->line, tok->column);
            lo_ast_node_free(cond);
            free(cond);
            return NULL;
        }
        lo_ast_node *body = parse_statement(p);
        if (!body) {
            lo_ast_node_free(cond);
            free(cond);
            return NULL;
        }
        lo_ast_stmt *while_stmt = lo_ast_while_stmt_new(cond, body);
        while_stmt->node.line = line;
        while_stmt->node.column = col;
        return (lo_ast_node *)while_stmt;
    }

    /* Do-While statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_DO) {
        size_t line = tok->line;
        size_t col = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *body = parse_statement(p);
        if (!body) return NULL;
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_KW_WHILE)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected 'while' after do-while body", tok->line, tok->column);
            lo_ast_node_free(body);
            free(body);
            return NULL;
        }
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LPAREN)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected '(' after while", tok->line, tok->column);
            lo_ast_node_free(body);
            free(body);
            return NULL;
        }
        lo_ast_node *cond = parse_expression(p);
        if (!cond) {
            lo_ast_node_free(body);
            free(body);
            return NULL;
        }
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
        lo_ast_stmt *dw = lo_ast_do_while_stmt_new(body, cond);
        dw->node.line = line;
        dw->node.column = col;
        return (lo_ast_node *)dw;
    }

    /* For statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_FOR) {
        size_t line = tok->line;
        size_t col = tok->column;
        lo_parser_cursor_advance(&p->cursor);
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LPAREN)) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected '(' after 'for'", tok->line, tok->column);
            return NULL;
        }

        p->scope = lo_scope_push(p->scope);

        lo_ast_node *init = NULL;
        const lo_token *cur = lo_parser_cursor_current(&p->cursor);
        if (cur && is_declaration_start(p, cur)) {
            lo_ast_type itype;
            int isc = 0, iin = 0;
            parse_declaration_specifiers(p, &itype, &isc, &iin);
            lo_declarator_result ires;
            parse_declarator(p, &itype, &ires);
            lo_ast_node *i_init = NULL;
            if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_ASSIGN)) {
                i_init = parse_initializer(p);
            }
            init = (lo_ast_node *)lo_ast_var_decl_new(ires.name, ires.name_len, &ires.type, i_init);
            free(ires.name);
            lo_ast_type_free_members(&itype);
            lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
        } else if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON)) {
            init = parse_expression(p);
            lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
        }

        lo_ast_node *cond = NULL;
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON)) {
            cond = parse_expression(p);
            lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
        }

        lo_ast_node *step = NULL;
        if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN)) {
            step = parse_expression(p);
            lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN);
        }

        lo_ast_node *body = parse_statement(p);
        lo_scope_pop(&p->scope);

        if (!body) {
            if (init) { lo_ast_node_free(init); free(init); }
            if (cond) { lo_ast_node_free(cond); free(cond); }
            if (step) { lo_ast_node_free(step); free(step); }
            return NULL;
        }

        lo_ast_stmt *for_stmt = lo_ast_for_stmt_new(init, cond, step, body);
        for_stmt->node.line = line;
        for_stmt->node.column = col;
        return (lo_ast_node *)for_stmt;
    }

    /* Switch statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_SWITCH) {
        lo_parser_cursor_advance(&p->cursor);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LPAREN);
        lo_ast_node *cond = parse_expression(p);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RPAREN);
        lo_ast_node *body = parse_statement(p);
        return (lo_ast_node *)lo_ast_switch_stmt_new(cond, body);
    }

    /* Case statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_CASE) {
        lo_parser_cursor_advance(&p->cursor);
        lo_ast_node *c_expr = parse_conditional_expression(p);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COLON);
        lo_ast_node *sub = parse_statement(p);
        return (lo_ast_node *)lo_ast_case_stmt_new(c_expr, sub);
    }

    /* Default statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_DEFAULT) {
        lo_parser_cursor_advance(&p->cursor);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COLON);
        lo_ast_node *sub = parse_statement(p);
        return (lo_ast_node *)lo_ast_default_stmt_new(sub);
    }

    /* Break statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_BREAK) {
        lo_parser_cursor_advance(&p->cursor);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
        return (lo_ast_node *)lo_ast_break_stmt_new();
    }

    /* Continue statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_CONTINUE) {
        lo_parser_cursor_advance(&p->cursor);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
        return (lo_ast_node *)lo_ast_continue_stmt_new();
    }

    /* Goto statement */
    if (tok->kind == LO_LEXER_TOKEN_KW_GOTO) {
        lo_parser_cursor_advance(&p->cursor);
        const lo_token *lid = lo_parser_cursor_current(&p->cursor);
        if (!lid || lid->kind != LO_LEXER_TOKEN_IDENTIFIER) {
            set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected label name after 'goto'", tok->line, tok->column);
            return NULL;
        }
        lo_ast_stmt *gt = lo_ast_goto_stmt_new(lid->start, lid->length);
        lo_parser_cursor_advance(&p->cursor);
        lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON);
        return (lo_ast_node *)gt;
    }

    /* Labeled statement: identifier : statement */
    if (tok->kind == LO_LEXER_TOKEN_IDENTIFIER) {
        const lo_token *after = lo_parser_cursor_peek(&p->cursor, 1);
        if (after && after->kind == LO_LEXER_TOKEN_COLON) {
            lo_ast_stmt *lbl = lo_ast_label_stmt_new(tok->start, tok->length, NULL);
            lo_parser_cursor_advance(&p->cursor); /* id */
            lo_parser_cursor_advance(&p->cursor); /* : */
            lo_ast_node *sub = parse_statement(p);
            if (sub) lo_ast_node_add_child(&lbl->node, sub);
            return (lo_ast_node *)lbl;
        }
    }

    /* Expression statement */
    if (tok->kind == LO_LEXER_TOKEN_SEMICOLON) {
        lo_parser_cursor_advance(&p->cursor);
        return (lo_ast_node *)lo_ast_expr_stmt_new(NULL);
    }

    lo_ast_node *expr = parse_expression(p);
    if (!expr)
        return NULL;
    if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON)) {
        set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ';' after expression", tok->line, tok->column);
        lo_ast_node_free(expr);
        free(expr);
        return NULL;
    }
    return (lo_ast_node *)lo_ast_expr_stmt_new(expr);
}

static lo_ast_node *parse_compound_statement(lo_parser *p)
{
    if (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_LBRACE)) {
        set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected '{'", 0, 0);
        return NULL;
    }

    p->scope = lo_scope_push(p->scope);
    lo_ast_stmt *block = lo_ast_compound_stmt_new();

    while (!lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_RBRACE) &&
           !lo_parser_cursor_at_end(&p->cursor)) {
        const lo_token *tok = lo_parser_cursor_current(&p->cursor);
        if (!tok)
            break;

        /* In C99/C11, declarations and statements can be freely mixed */
        if (is_declaration_start(p, tok)) {
            lo_ast_type base_type;
            int storage_class = 0, is_inline = 0;
            if (parse_declaration_specifiers(p, &base_type, &storage_class, &is_inline) != 0) {
                set_parser_error(p, LO_PARSER_ERROR_INVALID_DECLARATION, "invalid declaration specifiers", tok->line, tok->column);
                lo_scope_pop(&p->scope);
                lo_ast_node_free(&block->node);
                free(block);
                return NULL;
            }

            if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON)) {
                lo_ast_type_free_members(&base_type);
                continue;
            }

            for (;;) {
                lo_declarator_result dres;
                parse_declarator(p, &base_type, &dres);

                lo_ast_node *init = NULL;
                if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_ASSIGN)) {
                    init = parse_initializer(p);
                }

                if (storage_class == LO_LEXER_TOKEN_KW_TYPEDEF) {
                    lo_scope_insert_typedef(p->scope, dres.name, dres.name_len);
                    lo_ast_decl *tdecl = lo_ast_decl_new(LO_AST_TYPEDEF_DECL);
                    tdecl->name = dres.name;
                    tdecl->type = dres.type;
                    tdecl->is_typedef = 1;
                    lo_ast_node_add_child(&block->node, (lo_ast_node *)tdecl);
                } else {
                    lo_ast_decl *vdecl = lo_ast_var_decl_new(dres.name, dres.name_len, &dres.type, init);
                    free(dres.name);
                    lo_ast_node_add_child(&block->node, (lo_ast_node *)vdecl);
                }

                if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_COMMA))
                    continue;
                if (lo_parser_cursor_match(&p->cursor, LO_LEXER_TOKEN_SEMICOLON))
                    break;

                set_parser_error(p, LO_PARSER_ERROR_UNEXPECTED_TOKEN, "expected ';' after declaration", tok->line, tok->column);
                lo_scope_pop(&p->scope);
                lo_ast_node_free(&block->node);
                free(block);
                return NULL;
            }

            lo_ast_type_free_members(&base_type);
        } else {
            lo_ast_node *stmt = parse_statement(p);
            if (!stmt) {
                lo_scope_pop(&p->scope);
                lo_ast_node_free(&block->node);
                free(block);
                return NULL;
            }
            lo_ast_node_add_child(&block->node, stmt);
        }
    }

    lo_scope_pop(&p->scope);
    return (lo_ast_node *)block;
}

/* ----------------------------------------------------------- Top-Level Parser */

int lo_parser_parse(lo_parser *parser, lo_ast_node *root)
{
    if (!parser || !root)
        return -1;

    lo_ast_node_init(root, LO_AST_TRANSLATION_UNIT);

    if (!parser->cursor.tokens || parser->cursor.count == 0) {
        return set_parser_error(
            parser,
            LO_PARSER_ERROR_UNEXPECTED_EOF,
            "no tokens available",
            0,
            0
        );
    }

    while (!lo_parser_cursor_at_end(&parser->cursor)) {
        const lo_token *tok =
            lo_parser_cursor_current(&parser->cursor);

        if (!tok)
            break;

        if (tok->kind == LO_LEXER_TOKEN_SEMICOLON) {
            lo_parser_cursor_advance(&parser->cursor);
            continue;
        }

        if (!is_declaration_start(parser, tok)) {
            return set_parser_error(
                parser,
                LO_PARSER_ERROR_UNEXPECTED_TOKEN,
                "expected declaration or function definition",
                tok->line,
                tok->column
            );
        }

        lo_ast_type base_type;
        int storage_class = 0;
        int is_inline = 0;

        if (parse_declaration_specifiers(
                parser,
                &base_type,
                &storage_class,
                &is_inline) != 0) {

            return set_parser_error(
                parser,
                LO_PARSER_ERROR_INVALID_DECLARATION,
                "invalid declaration specifiers",
                tok->line,
                tok->column
            );
        }

        /*
         * Standalone struct/union/enum declaration:
         *
         *     struct Foo { ... };
         *     enum Foo { ... };
         */
        if (lo_parser_cursor_match(
                &parser->cursor,
                LO_LEXER_TOKEN_SEMICOLON)) {

            if (base_type.kind == LO_AST_TYPE_STRUCT ||
                base_type.kind == LO_AST_TYPE_UNION) {

                lo_ast_decl *sdecl =
                    lo_ast_struct_decl_new(
                        base_type.tag_or_typedef_name,
                        base_type.tag_or_typedef_name
                            ? strlen(base_type.tag_or_typedef_name)
                            : 0,
                        base_type.kind == LO_AST_TYPE_UNION,
                        1
                    );

                lo_ast_node_add_child(
                    root,
                    (lo_ast_node *)sdecl
                );

            } else if (base_type.kind == LO_AST_TYPE_ENUM) {

                lo_ast_decl *edecl =
                    lo_ast_enum_decl_new(
                        base_type.tag_or_typedef_name,
                        base_type.tag_or_typedef_name
                            ? strlen(base_type.tag_or_typedef_name)
                            : 0,
                        1
                    );

                lo_ast_node_add_child(
                    root,
                    (lo_ast_node *)edecl
                );
            }

            lo_ast_type_free_members(&base_type);
            continue;
        }

        /*
         * A declaration may contain multiple declarators:
         *
         *     int a, b, c;
         */
        for (;;) {
            lo_declarator_result dres;

            if (parse_declarator(
                    parser,
                    &base_type,
                    &dres) != 0) {

                lo_ast_type_free_members(&base_type);

                return set_parser_error(
                    parser,
                    LO_PARSER_ERROR_INVALID_DECLARATION,
                    "invalid declarator",
                    tok->line,
                    tok->column
                );
            }

            const lo_token *next =
                lo_parser_cursor_current(&parser->cursor);

            /*
             * Function definition:
             *
             *     int main(void) { ... }
             */
            if (dres.is_function &&
                next &&
                next->kind == LO_LEXER_TOKEN_LBRACE) {

                lo_ast_decl *fdecl =
                    lo_ast_func_decl_new(
                        dres.name,
                        dres.name_len,
                        &dres.type,
                        1
                    );

                if (!fdecl) {
                    free(dres.name);
                    lo_ast_type_free_members(&base_type);
                    return -1;
                }

                fdecl->is_static =
                    storage_class == LO_LEXER_TOKEN_KW_STATIC;

                fdecl->is_extern =
                    storage_class == LO_LEXER_TOKEN_KW_EXTERN;

                fdecl->is_inline = is_inline;

                /*
                 * Do NOT attach dres.param_list here.
                 *
                 * The current parser uses a temporary compound node
                 * for parameters. Until a real PARAM_LIST AST node
                 * exists, attaching it creates a bogus COMPOUND_STMT.
                 */

                lo_ast_node *body =
                    parse_compound_statement(parser);

                if (!body) {
                    free(dres.name);
                    lo_ast_type_free_members(&base_type);
                    lo_ast_node_free(&fdecl->node);
                    free(fdecl);
                    return -1;
                }

                lo_ast_node_add_child(
                    &fdecl->node,
                    body
                );

                lo_ast_node_add_child(
                    root,
                    (lo_ast_node *)fdecl
                );

                free(dres.name);

                /*
                 * A function definition cannot have another
                 * comma-separated declarator here.
                 */
                break;
            }

            /*
             * Optional initializer.
             */
            lo_ast_node *init = NULL;

            if (lo_parser_cursor_match(
                    &parser->cursor,
                    LO_LEXER_TOKEN_ASSIGN)) {

                init = parse_initializer(parser);

                if (!init) {
                    free(dres.name);
                    lo_ast_type_free_members(&base_type);
                    return -1;
                }
            }

            /*
             * Typedef declaration.
             */
            if (storage_class == LO_LEXER_TOKEN_KW_TYPEDEF) {

                lo_scope_insert_typedef(
                    parser->scope,
                    dres.name,
                    dres.name_len
                );

                lo_ast_decl *tdecl =
                    lo_ast_decl_new(
                        LO_AST_TYPEDEF_DECL
                    );

                if (!tdecl) {
                    free(dres.name);
                    lo_ast_type_free_members(&base_type);
                    lo_ast_node_free(init);
                    return -1;
                }

                tdecl->name = dres.name;
                tdecl->type = dres.type;
                tdecl->is_typedef = 1;

                lo_ast_node_add_child(
                    root,
                    (lo_ast_node *)tdecl
                );

            /*
             * Function prototype.
             */
            } else if (dres.is_function) {

                lo_ast_decl *fdecl =
                    lo_ast_func_decl_new(
                        dres.name,
                        dres.name_len,
                        &dres.type,
                        0
                    );

                if (!fdecl) {
                    free(dres.name);
                    lo_ast_type_free_members(&base_type);
                    lo_ast_node_free(init);
                    return -1;
                }

                fdecl->is_static =
                    storage_class == LO_LEXER_TOKEN_KW_STATIC;

                fdecl->is_extern =
                    storage_class == LO_LEXER_TOKEN_KW_EXTERN;

                fdecl->is_inline = is_inline;

                /*
                 * Same rule as function definitions:
                 * do not attach the temporary parameter node.
                 */

                lo_ast_node_add_child(
                    root,
                    (lo_ast_node *)fdecl
                );

                free(dres.name);

            /*
             * Variable declaration.
             */
            } else {

                lo_ast_decl *vdecl =
                    lo_ast_var_decl_new(
                        dres.name,
                        dres.name_len,
                        &dres.type,
                        init
                    );

                if (!vdecl) {
                    free(dres.name);
                    lo_ast_type_free_members(&base_type);
                    return -1;
                }

                vdecl->is_static =
                    storage_class == LO_LEXER_TOKEN_KW_STATIC;

                vdecl->is_extern =
                    storage_class == LO_LEXER_TOKEN_KW_EXTERN;

                lo_ast_node_add_child(
                    root,
                    (lo_ast_node *)vdecl
                );

                free(dres.name);
            }

            /*
             * More declarators:
             *
             *     int a, b;
             */
            if (lo_parser_cursor_match(
                    &parser->cursor,
                    LO_LEXER_TOKEN_COMMA)) {

                continue;
            }

            /*
             * End of declaration.
             */
            if (lo_parser_cursor_match(
                    &parser->cursor,
                    LO_LEXER_TOKEN_SEMICOLON)) {

                break;
            }

            const lo_token *cur =
                lo_parser_cursor_current(&parser->cursor);

            size_t error_line =
                cur ? cur->line : 0;

            size_t error_column =
                cur ? cur->column : 0;

            lo_ast_type_free_members(&base_type);

            return set_parser_error(
                parser,
                LO_PARSER_ERROR_UNEXPECTED_TOKEN,
                "expected ';' after declaration",
                error_line,
                error_column
            );
        }

        lo_ast_type_free_members(&base_type);
    }

    return 0;
}


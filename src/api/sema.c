#include "sema/sema.h"

#include <stdlib.h>
#include <string.h>

/* Semantic Symbol */

void lo_sema_symbol_init(lo_sema_symbol *symbol,
                         const char *name,
                         lo_sema_symbol_kind kind)
{
    if (!symbol)
        return;

    symbol->name = name;
    symbol->kind = kind;

    lo_ast_type_init(&symbol->type, LO_AST_TYPE_UNKNOWN);

    symbol->defined = 0;
    symbol->initialized = 0;
}

/* Semantic Type */

int lo_sema_type_equal(const lo_ast_type *lhs,
                       const lo_ast_type *rhs)
{
    if (!lhs || !rhs)
        return 0;

    return lhs->kind == rhs->kind &&
           lhs->bits == rhs->bits &&
           lhs->is_signed == rhs->is_signed;
}

int lo_sema_type_compatible(const lo_ast_type *lhs,
                            const lo_ast_type *rhs)
{
    if (!lhs || !rhs)
        return 0;

    if (lo_sema_type_equal(lhs, rhs))
        return 1;

    /*
     * Integer types are currently considered compatible.
     * More complete C conversion rules will be added later.
     */
    if (lo_sema_type_is_integer(lhs) &&
        lo_sema_type_is_integer(rhs))
        return 1;

    if (lo_sema_type_is_floating(lhs) &&
        lo_sema_type_is_floating(rhs))
        return 1;

    return 0;
}

int lo_sema_type_is_integer(const lo_ast_type *type)
{
    if (!type)
        return 0;

    return type->kind == LO_AST_TYPE_BOOL ||
           type->kind == LO_AST_TYPE_CHAR ||
           type->kind == LO_AST_TYPE_INT;
}

int lo_sema_type_is_floating(const lo_ast_type *type)
{
    if (!type)
        return 0;

    return type->kind == LO_AST_TYPE_FLOAT ||
           type->kind == LO_AST_TYPE_DOUBLE;
}

int lo_sema_type_is_scalar(const lo_ast_type *type)
{
    if (!type)
        return 0;

    return lo_sema_type_is_integer(type) ||
           lo_sema_type_is_floating(type) ||
           type->kind == LO_AST_TYPE_POINTER;
}

/* Semantic Scope */

void lo_sema_scope_init(lo_sema_scope *scope,
                        lo_sema_scope *parent)
{
    if (!scope)
        return;

    scope->parent = parent;
    scope->symbols = NULL;
    scope->symbol_count = 0;
    scope->symbol_capacity = 0;
}

void lo_sema_scope_free(lo_sema_scope *scope)
{
    if (!scope)
        return;

    free(scope->symbols);

    scope->symbols = NULL;
    scope->symbol_count = 0;
    scope->symbol_capacity = 0;
    scope->parent = NULL;
}

int lo_sema_scope_add(lo_sema_scope *scope,
                      void *symbol)
{
    void **symbols;
    size_t capacity;

    if (!scope || !symbol)
        return -1;

    if (scope->symbol_count == scope->symbol_capacity) {
        capacity = scope->symbol_capacity
                 ? scope->symbol_capacity * 2
                 : 8;

        if (capacity < scope->symbol_capacity)
            return -1;

        symbols = realloc(scope->symbols,
                          capacity * sizeof(*symbols));

        if (!symbols)
            return -1;

        scope->symbols = symbols;
        scope->symbol_capacity = capacity;
    }

    scope->symbols[scope->symbol_count++] = symbol;

    return 0;
}

void *lo_sema_scope_find(lo_sema_scope *scope,
                         const char *name)
{
    size_t i;

    if (!name)
        return NULL;

    while (scope) {
        for (i = 0; i < scope->symbol_count; i++) {
            lo_sema_symbol *symbol =
                (lo_sema_symbol *)scope->symbols[i];

            if (symbol && symbol->name &&
                strcmp(symbol->name, name) == 0)
                return symbol;
        }

        scope = scope->parent;
    }

    return NULL;
}

/* Diagnostics */

void lo_sema_diagnostic_init(
    lo_sema_diagnostic *diagnostic)
{
    if (!diagnostic)
        return;

    diagnostic->kind = LO_SEMA_DIAGNOSTIC_NONE;
    diagnostic->message = NULL;
    diagnostic->line = 0;
    diagnostic->column = 0;
}

/* Semantic Analyzer */

void lo_sema_init(lo_sema *sema)
{
    if (!sema)
        return;

    sema->scope = NULL;
    sema->diagnostics = NULL;
    sema->diagnostic_count = 0;
    sema->diagnostic_capacity = 0;
}

void lo_sema_free(lo_sema *sema)
{
    if (!sema)
        return;

    if (sema->scope) {
        lo_sema_scope_free(sema->scope);
        free(sema->scope);
    }

    free(sema->diagnostics);

    sema->scope = NULL;
    sema->diagnostics = NULL;
    sema->diagnostic_count = 0;
    sema->diagnostic_capacity = 0;
}

int lo_sema_analyze(lo_sema *sema,
                    const lo_ast_node *root)
{
    lo_sema_scope *scope;

    if (!sema || !root)
        return -1;

    if (sema->scope) {
        lo_sema_scope_free(sema->scope);
        free(sema->scope);
        sema->scope = NULL;
    }

    scope = malloc(sizeof(*scope));

    if (!scope)
        return -1;

    lo_sema_scope_init(scope, NULL);
    sema->scope = scope;

    /*
     * AST traversal and full C semantic analysis will be
     * implemented as declaration, expression, and statement
     * semantics are added.
     */

    return 0;
}

int lo_sema_failed(const lo_sema *sema)
{
    unsigned i;

    if (!sema)
        return 1;

    for (i = 0; i < sema->diagnostic_count; i++) {
        if (sema->diagnostics[i].kind ==
            LO_SEMA_DIAGNOSTIC_ERROR)
            return 1;
    }

    return 0;
}

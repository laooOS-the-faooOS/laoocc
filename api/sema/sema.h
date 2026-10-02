#ifndef LAOOCC_SEMA_SEMA_H
#define LAOOCC_SEMA_SEMA_H

#include "ast/ast.h"
#include "sema/scope.h"
#include "sema/symbol.h"
#include "sema/type.h"
#include "sema/diagnostic.h"

typedef struct lo_sema {
    lo_sema_scope *scope;

    lo_sema_diagnostic *diagnostics;
    unsigned diagnostic_count;
    unsigned diagnostic_capacity;
} lo_sema;

void lo_sema_init(lo_sema *sema);

void lo_sema_free(lo_sema *sema);

int lo_sema_analyze(lo_sema *sema,
                    const lo_ast_node *root);

int lo_sema_failed(const lo_sema *sema);

#endif /* LAOOCC_SEMA_SEMA_H */

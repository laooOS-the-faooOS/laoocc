#ifndef LAOOCC_SEMA_SCOPE_H
#define LAOOCC_SEMA_SCOPE_H

#include <stddef.h>

typedef struct lo_sema_scope {
    struct lo_sema_scope *parent;

    void **symbols;
    size_t symbol_count;
    size_t symbol_capacity;
} lo_sema_scope;

void lo_sema_scope_init(lo_sema_scope *scope,
                        lo_sema_scope *parent);

void lo_sema_scope_free(lo_sema_scope *scope);

int lo_sema_scope_add(lo_sema_scope *scope,
                      void *symbol);

void *lo_sema_scope_find(lo_sema_scope *scope,
                         const char *name);

#endif /* LAOOCC_SEMA_SCOPE_H */

#ifndef LAOOCC_SEMA_SYMBOL_H
#define LAOOCC_SEMA_SYMBOL_H

#include "ast/type.h"

typedef enum lo_sema_symbol_kind {
    LO_SEMA_SYMBOL_UNKNOWN = 0,
    LO_SEMA_SYMBOL_VARIABLE,
    LO_SEMA_SYMBOL_FUNCTION,
    LO_SEMA_SYMBOL_PARAMETER,
    LO_SEMA_SYMBOL_TYPE
} lo_sema_symbol_kind;

typedef struct lo_sema_symbol {
    const char *name;

    lo_sema_symbol_kind kind;

    lo_ast_type type;

    int defined;
    int initialized;
} lo_sema_symbol;

void lo_sema_symbol_init(lo_sema_symbol *symbol,
                         const char *name,
                         lo_sema_symbol_kind kind);

#endif /* LAOOCC_SEMA_SYMBOL_H */

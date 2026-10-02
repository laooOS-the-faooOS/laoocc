#ifndef LAOOCC_SYMBOL_SYMBOL_H
#define LAOOCC_SYMBOL_SYMBOL_H

#include <stddef.h>
#include <stdint.h>

#include "symbol/kind.h"
#include "symbol/visibility.h"

typedef struct lo_symbol {
    const char          *name;
    uintptr_t            value;
    size_t               size;

    lo_symbol_kind       kind;
    lo_symbol_visibility visibility;

    int                  defined;
    int                  global;
    int                  weak;
} lo_symbol;

void lo_symbol_init(lo_symbol *symbol);

void lo_symbol_reset(lo_symbol *symbol);

int lo_symbol_set_name(lo_symbol *symbol,
                       const char *name);

int lo_symbol_defined(const lo_symbol *symbol);

int lo_symbol_global(const lo_symbol *symbol);

int lo_symbol_weak(const lo_symbol *symbol);

#endif /* LAOOCC_SYMBOL_SYMBOL_H */

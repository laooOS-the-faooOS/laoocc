#ifndef LAOOCC_SYMBOL_TABLE_H
#define LAOOCC_SYMBOL_TABLE_H

#include <stddef.h>

#include "symbol/symbol.h"

typedef struct lo_symbol_table {
    lo_symbol *symbols;
    size_t     length;
    size_t     capacity;
} lo_symbol_table;

void lo_symbol_table_init(lo_symbol_table *table);

void lo_symbol_table_free(lo_symbol_table *table);

int lo_symbol_table_reserve(lo_symbol_table *table,
                            size_t capacity);

int lo_symbol_table_add(lo_symbol_table *table,
                        const lo_symbol *symbol);

lo_symbol *lo_symbol_table_find(lo_symbol_table *table,
                                const char *name);

const lo_symbol *lo_symbol_table_find_const(
    const lo_symbol_table *table,
    const char *name);

void lo_symbol_table_clear(lo_symbol_table *table);

#endif /* LAOOCC_SYMBOL_TABLE_H */

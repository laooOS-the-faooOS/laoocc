#include "symbol/symbol.h"
#include "symbol/symbol_table.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Symbol */

void lo_symbol_init(lo_symbol *symbol)
{
    if (!symbol)
        return;

    symbol->name = NULL;
    symbol->value = 0;
    symbol->size = 0;
    symbol->kind = LO_SYMBOL_UNKNOWN;
    symbol->visibility = LO_SYMBOL_DEFAULT;
    symbol->defined = 0;
    symbol->global = 0;
    symbol->weak = 0;
}

void lo_symbol_reset(lo_symbol *symbol)
{
    lo_symbol_init(symbol);
}

int lo_symbol_set_name(lo_symbol *symbol,
                       const char *name)
{
    if (!symbol)
        return -1;

    symbol->name = name;

    return 0;
}

int lo_symbol_defined(const lo_symbol *symbol)
{
    if (!symbol)
        return 0;

    return symbol->defined != 0;
}

int lo_symbol_global(const lo_symbol *symbol)
{
    if (!symbol)
        return 0;

    return symbol->global != 0;
}

int lo_symbol_weak(const lo_symbol *symbol)
{
    if (!symbol)
        return 0;

    return symbol->weak != 0;
}

/* Symbol table */

void lo_symbol_table_init(lo_symbol_table *table)
{
    if (!table)
        return;

    table->symbols = NULL;
    table->length = 0;
    table->capacity = 0;
}

void lo_symbol_table_free(lo_symbol_table *table)
{
    if (!table)
        return;

    free(table->symbols);

    table->symbols = NULL;
    table->length = 0;
    table->capacity = 0;
}

int lo_symbol_table_reserve(lo_symbol_table *table,
                            size_t capacity)
{
    lo_symbol *symbols;

    if (!table)
        return -1;

    if (capacity <= table->capacity)
        return 0;

    if (capacity > SIZE_MAX / sizeof(lo_symbol))
        return -1;

    symbols = realloc(table->symbols,
                      capacity * sizeof(lo_symbol));

    if (!symbols)
        return -1;

    table->symbols = symbols;
    table->capacity = capacity;

    return 0;
}

int lo_symbol_table_add(lo_symbol_table *table,
                        const lo_symbol *symbol)
{
    size_t capacity;

    if (!table || !symbol)
        return -1;

    if (table->length == table->capacity) {
        capacity = table->capacity ?
                   table->capacity * 2 : 16;

        if (capacity < table->capacity)
            return -1;

        if (lo_symbol_table_reserve(table, capacity) != 0)
            return -1;
    }

    table->symbols[table->length] = *symbol;
    table->length++;

    return 0;
}

lo_symbol *lo_symbol_table_find(lo_symbol_table *table,
                                const char *name)
{
    size_t i;

    if (!table || !name)
        return NULL;

    for (i = 0; i < table->length; i++) {
        if (table->symbols[i].name &&
            strcmp(table->symbols[i].name, name) == 0)
            return &table->symbols[i];
    }

    return NULL;
}

const lo_symbol *lo_symbol_table_find_const(
    const lo_symbol_table *table,
    const char *name)
{
    size_t i;

    if (!table || !name)
        return NULL;

    for (i = 0; i < table->length; i++) {
        if (table->symbols[i].name &&
            strcmp(table->symbols[i].name, name) == 0)
            return &table->symbols[i];
    }

    return NULL;
}

void lo_symbol_table_clear(lo_symbol_table *table)
{
    if (!table)
        return;

    table->length = 0;
}

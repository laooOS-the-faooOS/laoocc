#ifndef LAOOCC_SYMBOL_KIND_H
#define LAOOCC_SYMBOL_KIND_H

typedef enum lo_symbol_kind {
    LO_SYMBOL_UNKNOWN = 0,
    LO_SYMBOL_OBJECT,
    LO_SYMBOL_FUNCTION,
    LO_SYMBOL_SECTION,
    LO_SYMBOL_FILE,
    LO_SYMBOL_TLS,
    LO_SYMBOL_COMMON
} lo_symbol_kind;

#endif /* LAOOCC_SYMBOL_KIND_H */

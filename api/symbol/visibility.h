#ifndef LAOOCC_SYMBOL_VISIBILITY_H
#define LAOOCC_SYMBOL_VISIBILITY_H

typedef enum lo_symbol_visibility {
    LO_SYMBOL_DEFAULT = 0,
    LO_SYMBOL_HIDDEN,
    LO_SYMBOL_PROTECTED,
    LO_SYMBOL_INTERNAL
} lo_symbol_visibility;

#endif /* LAOOCC_SYMBOL_VISIBILITY_H */

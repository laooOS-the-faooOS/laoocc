#ifndef LAOOCC_PREPROCESSOR_MACRO_H
#define LAOOCC_PREPROCESSOR_MACRO_H

#include <stddef.h>

typedef struct lo_pp_macro {
    const char *name;
    size_t name_length;

    const char *replacement;
    size_t replacement_length;
} lo_pp_macro;

#endif

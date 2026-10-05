#ifndef LAOOCC_PP_MACRO_H
#define LAOOCC_PP_MACRO_H

typedef struct lo_pp_macro {
    char *name;
    char *value;
    struct lo_pp_macro *next;
} lo_pp_macro;

#endif

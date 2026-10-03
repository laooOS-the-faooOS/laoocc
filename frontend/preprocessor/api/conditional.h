#ifndef LAOOCC_PREPROCESSOR_CONDITIONAL_H
#define LAOOCC_PREPROCESSOR_CONDITIONAL_H

#include <stddef.h>

typedef struct lo_pp_conditional {
    int parent_active;
    int active;
    int else_seen;
} lo_pp_conditional;

#endif

#ifndef LAOOCC_PP_CONDITIONAL_H
#define LAOOCC_PP_CONDITIONAL_H

typedef struct lo_pp_conditional {
    int parent_active;
    int active;
    int else_seen;
} lo_pp_conditional;

#endif

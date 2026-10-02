#ifndef LAOOCC_MATH_INTEGER_H
#define LAOOCC_MATH_INTEGER_H

#include <stdint.h>

#define LO_INT_MIN(a, b) \
    ((a) < (b) ? (a) : (b))

#define LO_INT_MAX(a, b) \
    ((a) > (b) ? (a) : (b))

#define LO_INT_ABS(x) \
    ((x) < 0 ? -(x) : (x))

#define LO_INT_CLAMP(x, min, max) \
    ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define LO_INT_SIGN(x) \
    ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

int64_t lo_int_min(int64_t a, int64_t b);
int64_t lo_int_max(int64_t a, int64_t b);
int64_t lo_int_abs(int64_t value);
int64_t lo_int_clamp(int64_t value, int64_t min, int64_t max);
int     lo_int_sign(int64_t value);

#endif /* LAOOCC_MATH_INTEGER_H */

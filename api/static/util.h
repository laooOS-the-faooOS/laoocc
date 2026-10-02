#ifndef LAOOCC_STATIC_UTIL_H
#define LAOOCC_STATIC_UTIL_H

#define LO_UNUSED(value) \
    ((void)(value))

#define LO_SWAP(type, a, b) \
    do { \
        type lo_swap_tmp = (a); \
        (a) = (b); \
        (b) = lo_swap_tmp; \
    } while (0)

#define LO_MIN(a, b) \
    ((a) < (b) ? (a) : (b))

#define LO_MAX(a, b) \
    ((a) > (b) ? (a) : (b))

#define LO_CLAMP(value, min, max) \
    ((value) < (min) ? (min) : \
     ((value) > (max) ? (max) : (value)))

#endif /* LAOOCC_STATIC_UTIL_H */

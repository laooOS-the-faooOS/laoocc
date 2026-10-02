#ifndef LAOOCC_CORE_ARRAY_H
#define LAOOCC_CORE_ARRAY_H

#include <stddef.h>

#define LO_ARRAY_COUNT(array) \
    (sizeof(array) / sizeof((array)[0]))

#define LO_ARRAY_SIZE(array) \
    (sizeof(array))

#define LO_ARRAY_BEGIN(array) \
    (&(array)[0])

#define LO_ARRAY_END(array) \
    (&(array)[LO_ARRAY_COUNT(array)])

#endif /* LAOOCC_CORE_ARRAY_H */

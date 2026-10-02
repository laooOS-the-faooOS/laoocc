#ifndef LAOOCC_CORE_BITS_H
#define LAOOCC_CORE_BITS_H

#include "core/types.h"

#define LO_BIT(n) \
    ((lo_u64)1ULL << (n))

#define LO_BIT_SET(value, bit) \
    ((value) |= (bit))

#define LO_BIT_CLEAR(value, bit) \
    ((value) &= ~(bit))

#define LO_BIT_TOGGLE(value, bit) \
    ((value) ^= (bit))

#define LO_BIT_TEST(value, bit) \
    (((value) & (bit)) != 0)

#define LO_BIT_TEST_ALL(value, bits) \
    (((value) & (bits)) == (bits))

#define LO_BIT_TEST_ANY(value, bits) \
    (((value) & (bits)) != 0)

#endif /* LAOOCC_CORE_BITS_H */

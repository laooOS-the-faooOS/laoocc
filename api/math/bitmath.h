#ifndef LAOOCC_MATH_BITMATH_H
#define LAOOCC_MATH_BITMATH_H

#include <stdint.h>

#define LO_IS_POWER_OF_TWO(x) \
    ((x) != 0 && (((x) & ((x) - 1)) == 0))

#define LO_ALIGN_UP(value, alignment) \
    (((value) + ((alignment) - 1)) & ~((alignment) - 1))

#define LO_ALIGN_DOWN(value, alignment) \
    ((value) & ~((alignment) - 1))

uint32_t lo_bitmath_popcount32(uint32_t value);
uint64_t lo_bitmath_popcount64(uint64_t value);

uint32_t lo_bitmath_clz32(uint32_t value);
uint64_t lo_bitmath_clz64(uint64_t value);

uint32_t lo_bitmath_ctz32(uint32_t value);
uint64_t lo_bitmath_ctz64(uint64_t value);

uint32_t lo_bitmath_next_power_of_two32(uint32_t value);
uint64_t lo_bitmath_next_power_of_two64(uint64_t value);

#endif /* LAOOCC_MATH_BITMATH_H */

#include "math/math.h"

#include <stdint.h>

int64_t lo_int_min(int64_t a, int64_t b)
{
    return a < b ? a : b;
}

int64_t lo_int_max(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

int64_t lo_int_abs(int64_t value)
{
    return value < 0 ? -value : value;
}

int64_t lo_int_clamp(int64_t value, int64_t min, int64_t max)
{
    if (value < min)
        return min;

    if (value > max)
        return max;

    return value;
}

int lo_int_sign(int64_t value)
{
    if (value > 0)
        return 1;

    if (value < 0)
        return -1;

    return 0;
}

double lo_float_min(double a, double b)
{
    return a < b ? a : b;
}

double lo_float_max(double a, double b)
{
    return a > b ? a : b;
}

double lo_float_abs(double value)
{
    return value < 0.0 ? -value : value;
}

double lo_float_clamp(double value, double min, double max)
{
    if (value < min)
        return min;

    if (value > max)
        return max;

    return value;
}

int lo_float_sign(double value)
{
    if (value > 0.0)
        return 1;

    if (value < 0.0)
        return -1;

    return 0;
}

uint32_t lo_bitmath_popcount32(uint32_t value)
{
    uint32_t count = 0;

    while (value) {
        value &= value - 1;
        count++;
    }

    return count;
}

uint64_t lo_bitmath_popcount64(uint64_t value)
{
    uint64_t count = 0;

    while (value) {
        value &= value - 1;
        count++;
    }

    return count;
}

uint32_t lo_bitmath_clz32(uint32_t value)
{
    uint32_t count = 0;

    if (value == 0)
        return 32;

    while ((value & 0x80000000U) == 0) {
        value <<= 1;
        count++;
    }

    return count;
}

uint64_t lo_bitmath_clz64(uint64_t value)
{
    uint64_t count = 0;

    if (value == 0)
        return 64;

    while ((value & 0x8000000000000000ULL) == 0) {
        value <<= 1;
        count++;
    }

    return count;
}

uint32_t lo_bitmath_ctz32(uint32_t value)
{
    uint32_t count = 0;

    if (value == 0)
        return 32;

    while ((value & 1U) == 0) {
        value >>= 1;
        count++;
    }

    return count;
}

uint64_t lo_bitmath_ctz64(uint64_t value)
{
    uint64_t count = 0;

    if (value == 0)
        return 64;

    while ((value & 1ULL) == 0) {
        value >>= 1;
        count++;
    }

    return count;
}

uint32_t lo_bitmath_next_power_of_two32(uint32_t value)
{
    uint32_t result = 1;

    if (value == 0)
        return 1;

    while (result < value && result <= UINT32_MAX / 2)
        result <<= 1;

    return result;
}

uint64_t lo_bitmath_next_power_of_two64(uint64_t value)
{
    uint64_t result = 1;

    if (value == 0)
        return 1;

    while (result < value && result <= UINT64_MAX / 2)
        result <<= 1;

    return result;
}

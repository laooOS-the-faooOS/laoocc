#include "static/static.h"

#include <stdint.h>

uint64_t lo_hash_bytes(const void *data, size_t length)
{
    const uint8_t *bytes = data;
    uint64_t hash = 14695981039346656037ULL;

    for (size_t i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }

    return hash;
}

uint64_t lo_hash_string(const char *string)
{
    uint64_t hash = 14695981039346656037ULL;

    while (*string) {
        hash ^= (uint8_t)*string++;
        hash *= 1099511628211ULL;
    }

    return hash;
}

#ifndef LAOOCC_STATIC_HASH_H
#define LAOOCC_STATIC_HASH_H

#include <stddef.h>
#include <stdint.h>

uint64_t lo_hash_bytes(const void *data, size_t length);

uint64_t lo_hash_string(const char *string);

#endif /* LAOOCC_STATIC_HASH_H */

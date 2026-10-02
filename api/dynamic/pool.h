#ifndef LAOOCC_DYNAMIC_POOL_H
#define LAOOCC_DYNAMIC_POOL_H

#include <stddef.h>

typedef struct lo_pool_block {
    struct lo_pool_block *next;
    size_t capacity;
    size_t used;
    unsigned char data[];
} lo_pool_block;

typedef struct lo_pool {
    lo_pool_block *blocks;
    size_t block_size;
} lo_pool;

int lo_pool_init(lo_pool *pool, size_t block_size);

void lo_pool_free(lo_pool *pool);

void *lo_pool_alloc(lo_pool *pool, size_t size);

void lo_pool_clear(lo_pool *pool);

#endif /* LAOOCC_DYNAMIC_POOL_H */

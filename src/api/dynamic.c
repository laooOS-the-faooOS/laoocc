#include "dynamic/dynamic.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Map */

void lo_map_init(lo_map *map)
{
    if (!map)
        return;

    map->entries = NULL;
    map->length = 0;
    map->capacity = 0;
}

void lo_map_free(lo_map *map)
{
    if (!map)
        return;

    free(map->entries);

    map->entries = NULL;
    map->length = 0;
    map->capacity = 0;
}

int lo_map_reserve(lo_map *map, size_t capacity)
{
    lo_map_entry *entries;

    if (!map)
        return -1;

    if (capacity <= map->capacity)
        return 0;

    if (capacity > SIZE_MAX / sizeof(lo_map_entry))
        return -1;

    entries = realloc(map->entries,
                      capacity * sizeof(lo_map_entry));

    if (!entries)
        return -1;

    map->entries = entries;
    map->capacity = capacity;

    return 0;
}

int lo_map_insert(lo_map *map,
                  const void *key,
                  void *value)
{
    size_t i;
    size_t capacity;

    if (!map || !key)
        return -1;

    for (i = 0; i < map->length; i++) {
        if (map->entries[i].key == key) {
            map->entries[i].value = value;
            return 0;
        }
    }

    if (map->length == map->capacity) {
        capacity = map->capacity ? map->capacity * 2 : 8;

        if (capacity < map->capacity)
            return -1;

        if (lo_map_reserve(map, capacity) != 0)
            return -1;
    }

    map->entries[map->length].key = key;
    map->entries[map->length].value = value;
    map->length++;

    return 0;
}

void *lo_map_get(const lo_map *map, const void *key)
{
    size_t i;

    if (!map || !key)
        return NULL;

    for (i = 0; i < map->length; i++) {
        if (map->entries[i].key == key)
            return map->entries[i].value;
    }

    return NULL;
}

int lo_map_remove(lo_map *map, const void *key)
{
    size_t i;

    if (!map || !key)
        return -1;

    for (i = 0; i < map->length; i++) {
        if (map->entries[i].key == key) {
            if (i + 1 < map->length) {
                memmove(&map->entries[i],
                        &map->entries[i + 1],
                        (map->length - i - 1) *
                        sizeof(lo_map_entry));
            }

            map->length--;
            return 0;
        }
    }

    return -1;
}

void lo_map_clear(lo_map *map)
{
    if (map)
        map->length = 0;
}

/* Pool */

static lo_pool_block *lo_pool_new_block(size_t capacity)
{
    lo_pool_block *block;

    if (capacity > SIZE_MAX - sizeof(*block))
        return NULL;

    block = malloc(sizeof(*block) + capacity);

    if (!block)
        return NULL;

    block->next = NULL;
    block->capacity = capacity;
    block->used = 0;

    return block;
}

int lo_pool_init(lo_pool *pool, size_t block_size)
{
    if (!pool || block_size == 0)
        return -1;

    pool->blocks = NULL;
    pool->block_size = block_size;

    return 0;
}

void lo_pool_free(lo_pool *pool)
{
    lo_pool_block *block;
    lo_pool_block *next;

    if (!pool)
        return;

    block = pool->blocks;

    while (block) {
        next = block->next;
        free(block);
        block = next;
    }

    pool->blocks = NULL;
    pool->block_size = 0;
}

void *lo_pool_alloc(lo_pool *pool, size_t size)
{
    lo_pool_block *block;
    size_t capacity;
    void *ptr;

    if (!pool || size == 0)
        return NULL;

    block = pool->blocks;

    if (!block || size > block->capacity - block->used) {
        capacity = pool->block_size;

        if (capacity < size)
            capacity = size;

        block = lo_pool_new_block(capacity);

        if (!block)
            return NULL;

        block->next = pool->blocks;
        pool->blocks = block;
    }

    ptr = block->data + block->used;
    block->used += size;

    return ptr;
}

void lo_pool_clear(lo_pool *pool)
{
    lo_pool_block *block;

    if (!pool)
        return;

    for (block = pool->blocks; block; block = block->next)
        block->used = 0;
}

#ifndef LAOOCC_DYNAMIC_MAP_H
#define LAOOCC_DYNAMIC_MAP_H

#include <stddef.h>

typedef struct lo_map_entry {
    const void *key;
    void       *value;
} lo_map_entry;

typedef struct lo_map {
    lo_map_entry *entries;
    size_t        length;
    size_t        capacity;
} lo_map;

void lo_map_init(lo_map *map);

void lo_map_free(lo_map *map);

int lo_map_reserve(lo_map *map, size_t capacity);

int lo_map_insert(lo_map *map,
                  const void *key,
                  void *value);

void *lo_map_get(const lo_map *map,
                 const void *key);

int lo_map_remove(lo_map *map,
                  const void *key);

void lo_map_clear(lo_map *map);

#endif /* LAOOCC_DYNAMIC_MAP_H */

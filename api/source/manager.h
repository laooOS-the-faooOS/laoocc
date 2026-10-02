#ifndef LAOOCC_SOURCE_MANAGER_H
#define LAOOCC_SOURCE_MANAGER_H

#include <stddef.h>

#include "source/source.h"

typedef struct lo_source_manager {
    lo_source *sources;

    size_t source_count;
    size_t source_capacity;
} lo_source_manager;

void lo_source_manager_init(
    lo_source_manager *manager
);

void lo_source_manager_free(
    lo_source_manager *manager
);

int lo_source_manager_add(
    lo_source_manager *manager,
    lo_source *source
);

lo_source *lo_source_manager_get(
    lo_source_manager *manager,
    size_t index
);

const lo_source *lo_source_manager_cget(
    const lo_source_manager *manager,
    size_t index
);

size_t lo_source_manager_count(
    const lo_source_manager *manager
);

#endif /* LAOOCC_SOURCE_MANAGER_H */

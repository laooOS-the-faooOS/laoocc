#ifndef LAOOCC_SOURCE_SOURCE_H
#define LAOOCC_SOURCE_SOURCE_H

#include "source/buffer.h"
#include "source/location.h"

typedef struct lo_source {
    const char *name;
    const char *path;

    lo_source_buffer buffer;
} lo_source;

void lo_source_init(
    lo_source *source
);

void lo_source_free(
    lo_source *source
);

int lo_source_load(
    lo_source *source,
    const char *name,
    const char *path
);

int lo_source_set_data(
    lo_source *source,
    const char *data,
    size_t length
);

const char *lo_source_data(
    const lo_source *source
);

size_t lo_source_size(
    const lo_source *source
);

int lo_source_get_location(
    const lo_source *source,
    size_t offset,
    lo_source_location *location
);

#endif /* LAOOCC_SOURCE_SOURCE_H */

#ifndef LAOOCC_SOURCE_H
#define LAOOCC_SOURCE_H

#include <stddef.h>

typedef struct lo_source {
    char *data;
    size_t length;

    const char *path;

    size_t line_count;
} lo_source;

int lo_source_load(
    lo_source *source,
    const char *path
);

int lo_source_init(
    lo_source *source,
    const char *data,
    size_t length,
    const char *path
);

void lo_source_free(
    lo_source *source
);

const char *lo_source_line(
    const lo_source *source,
    size_t line
);

size_t lo_source_line_count(
    const lo_source *source
);

#endif

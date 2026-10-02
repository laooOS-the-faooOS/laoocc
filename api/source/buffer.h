#ifndef LAOOCC_SOURCE_BUFFER_H
#define LAOOCC_SOURCE_BUFFER_H

#include <stddef.h>

typedef struct lo_source_buffer {
    char   *data;
    size_t  length;
    size_t  capacity;
} lo_source_buffer;

void lo_source_buffer_init(
    lo_source_buffer *buffer
);

void lo_source_buffer_free(
    lo_source_buffer *buffer
);

int lo_source_buffer_load(
    lo_source_buffer *buffer,
    const char *data,
    size_t length
);

int lo_source_buffer_append(
    lo_source_buffer *buffer,
    const char *data,
    size_t length
);

void lo_source_buffer_clear(
    lo_source_buffer *buffer
);

const char *lo_source_buffer_data(
    const lo_source_buffer *buffer
);

size_t lo_source_buffer_size(
    const lo_source_buffer *buffer
);

#endif /* LAOOCC_SOURCE_BUFFER_H */

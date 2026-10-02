#ifndef LAOOCC_TEXT_BUFFER_H
#define LAOOCC_TEXT_BUFFER_H

#include <stddef.h>

typedef struct lo_text_buffer {
    char   *data;
    size_t  length;
    size_t  capacity;
} lo_text_buffer;

void lo_text_buffer_init(lo_text_buffer *buffer);
void lo_text_buffer_free(lo_text_buffer *buffer);

int lo_text_buffer_reserve(lo_text_buffer *buffer, size_t capacity);

int lo_text_buffer_write(lo_text_buffer *buffer,
                         const void *data,
                         size_t length);

int lo_text_buffer_putc(lo_text_buffer *buffer, char character);

void lo_text_buffer_clear(lo_text_buffer *buffer);

#endif /* LAOOCC_TEXT_BUFFER_H */

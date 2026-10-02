#ifndef LAOOCC_VECTOR_VECTOR_H
#define LAOOCC_VECTOR_VECTOR_H

#include <stddef.h>

typedef struct lo_vector {
    void   *data;
    size_t  length;
    size_t  capacity;
    size_t  element_size;
} lo_vector;

int lo_vector_init(lo_vector *vector, size_t element_size);

void lo_vector_free(lo_vector *vector);

int lo_vector_reserve(lo_vector *vector, size_t capacity);

int lo_vector_resize(lo_vector *vector, size_t length);

int lo_vector_push(lo_vector *vector, const void *element);

int lo_vector_pop(lo_vector *vector, void *element);

void *lo_vector_at(lo_vector *vector, size_t index);

const void *lo_vector_cat(const lo_vector *vector, size_t index);

void lo_vector_clear(lo_vector *vector);

int lo_vector_empty(const lo_vector *vector);

size_t lo_vector_size(const lo_vector *vector);

size_t lo_vector_capacity(const lo_vector *vector);

#endif /* LAOOCC_VECTOR_VECTOR_H */

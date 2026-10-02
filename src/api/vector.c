#include "vector/vector.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int lo_vector_init(lo_vector *vector, size_t element_size)
{
    if (!vector || element_size == 0)
        return -1;

    vector->data = NULL;
    vector->length = 0;
    vector->capacity = 0;
    vector->element_size = element_size;

    return 0;
}

void lo_vector_free(lo_vector *vector)
{
    if (!vector)
        return;

    free(vector->data);

    vector->data = NULL;
    vector->length = 0;
    vector->capacity = 0;
    vector->element_size = 0;
}

int lo_vector_reserve(lo_vector *vector, size_t capacity)
{
    void *data;

    if (!vector || vector->element_size == 0)
        return -1;

    if (capacity <= vector->capacity)
        return 0;

    if (capacity > SIZE_MAX / vector->element_size)
        return -1;

    data = realloc(vector->data,
                   capacity * vector->element_size);

    if (!data)
        return -1;

    vector->data = data;
    vector->capacity = capacity;

    return 0;
}

int lo_vector_resize(lo_vector *vector, size_t length)
{
    size_t old_length;
    size_t capacity;

    if (!vector || vector->element_size == 0)
        return -1;

    if (length > SIZE_MAX / vector->element_size)
        return -1;

    if (length > vector->capacity) {
        capacity = vector->capacity ? vector->capacity : 8;

        while (capacity < length) {
            if (capacity > SIZE_MAX / 2)
                return -1;

            capacity *= 2;
        }

        if (lo_vector_reserve(vector, capacity) != 0)
            return -1;
    }

    old_length = vector->length;

    if (length > old_length) {
        memset((unsigned char *)vector->data +
               old_length * vector->element_size,
               0,
               (length - old_length) * vector->element_size);
    }

    vector->length = length;

    return 0;
}

int lo_vector_push(lo_vector *vector, const void *element)
{
    void *destination;

    if (!vector || !element)
        return -1;

    if (vector->length == SIZE_MAX)
        return -1;

    if (lo_vector_resize(vector, vector->length + 1) != 0)
        return -1;

    destination = (unsigned char *)vector->data +
                  (vector->length - 1) * vector->element_size;

    memcpy(destination, element, vector->element_size);

    return 0;
}

int lo_vector_pop(lo_vector *vector, void *element)
{
    void *source;

    if (!vector || vector->length == 0)
        return -1;

    vector->length--;

    source = (unsigned char *)vector->data +
             vector->length * vector->element_size;

    if (element)
        memcpy(element, source, vector->element_size);

    return 0;
}

void *lo_vector_at(lo_vector *vector, size_t index)
{
    if (!vector || index >= vector->length)
        return NULL;

    return (unsigned char *)vector->data +
           index * vector->element_size;
}

const void *lo_vector_cat(const lo_vector *vector, size_t index)
{
    if (!vector || index >= vector->length)
        return NULL;

    return (const unsigned char *)vector->data +
           index * vector->element_size;
}

void lo_vector_clear(lo_vector *vector)
{
    if (!vector)
        return;

    vector->length = 0;
}

int lo_vector_empty(const lo_vector *vector)
{
    if (!vector)
        return 1;

    return vector->length == 0;
}

size_t lo_vector_size(const lo_vector *vector)
{
    if (!vector)
        return 0;

    return vector->length;
}

size_t lo_vector_capacity(const lo_vector *vector)
{
    if (!vector)
        return 0;

    return vector->capacity;
}

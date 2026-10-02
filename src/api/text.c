#include "text/text.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static int lo_text_grow(void **data,
                        size_t *capacity,
                        size_t required,
                        size_t element_size)
{
    size_t new_capacity;
    void *new_data;

    if (required <= *capacity)
        return 0;

    new_capacity = *capacity ? *capacity : 16;

    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2)
            return -1;

        new_capacity *= 2;
    }

    if (new_capacity > SIZE_MAX / element_size)
        return -1;

    new_data = realloc(*data, new_capacity * element_size);

    if (!new_data)
        return -1;

    *data = new_data;
    *capacity = new_capacity;

    return 0;
}

/* String */

void lo_text_string_init(lo_text_string *string)
{
    string->data = NULL;
    string->length = 0;
    string->capacity = 0;
}

void lo_text_string_free(lo_text_string *string)
{
    free(string->data);

    string->data = NULL;
    string->length = 0;
    string->capacity = 0;
}

int lo_text_string_reserve(lo_text_string *string, size_t capacity)
{
    return lo_text_grow(
        (void **)&string->data,
        &string->capacity,
        capacity,
        sizeof(char)
    );
}

int lo_text_string_resize(lo_text_string *string, size_t length)
{
    if (lo_text_string_reserve(string, length + 1) != 0)
        return -1;

    if (length > string->length)
        memset(string->data + string->length, 0,
               length - string->length);

    string->length = length;

    if (string->data)
        string->data[string->length] = '\0';

    return 0;
}

int lo_text_string_append(lo_text_string *string,
                          const char *data,
                          size_t length)
{
    size_t new_length;

    if (length > SIZE_MAX - string->length - 1)
        return -1;

    new_length = string->length + length;

    if (lo_text_string_reserve(string, new_length + 1) != 0)
        return -1;

    memcpy(string->data + string->length, data, length);

    string->length = new_length;
    string->data[string->length] = '\0';

    return 0;
}

int lo_text_string_push(lo_text_string *string, char character)
{
    return lo_text_string_append(string, &character, 1);
}

int lo_text_string_insert(lo_text_string *string,
                          size_t position,
                          const char *data,
                          size_t length)
{
    size_t new_length;

    if (position > string->length)
        return -1;

    if (length > SIZE_MAX - string->length - 1)
        return -1;

    new_length = string->length + length;

    if (lo_text_string_reserve(string, new_length + 1) != 0)
        return -1;

    memmove(string->data + position + length,
            string->data + position,
            string->length - position);

    memcpy(string->data + position, data, length);

    string->length = new_length;
    string->data[string->length] = '\0';

    return 0;
}

int lo_text_string_remove(lo_text_string *string,
                          size_t position,
                          size_t length)
{
    if (position > string->length)
        return -1;

    if (length > string->length - position)
        return -1;

    memmove(string->data + position,
            string->data + position + length,
            string->length - position - length);

    string->length -= length;

    if (string->data)
        string->data[string->length] = '\0';

    return 0;
}

void lo_text_string_clear(lo_text_string *string)
{
    string->length = 0;

    if (string->data)
        string->data[0] = '\0';
}

/* View */

lo_text_view lo_text_view_make(const char *data, size_t length)
{
    lo_text_view view;

    view.data = data;
    view.length = length;

    return view;
}

int lo_text_view_empty(lo_text_view view)
{
    return view.length == 0;
}

int lo_text_view_equal(lo_text_view lhs, lo_text_view rhs)
{
    if (lhs.length != rhs.length)
        return 0;

    if (lhs.length == 0)
        return 1;

    return memcmp(lhs.data, rhs.data, lhs.length) == 0;
}

int lo_text_view_compare(lo_text_view lhs, lo_text_view rhs)
{
    size_t length;
    int result;

    length = lhs.length < rhs.length ? lhs.length : rhs.length;

    if (length != 0) {
        result = memcmp(lhs.data, rhs.data, length);

        if (result != 0)
            return result;
    }

    if (lhs.length < rhs.length)
        return -1;

    if (lhs.length > rhs.length)
        return 1;

    return 0;
}

lo_text_view lo_text_view_subview(lo_text_view view,
                                  size_t position,
                                  size_t length)
{
    lo_text_view result;

    if (position > view.length)
        return LO_TEXT_VIEW_EMPTY;

    if (length > view.length - position)
        length = view.length - position;

    result.data = view.data + position;
    result.length = length;

    return result;
}

/* Buffer */

void lo_text_buffer_init(lo_text_buffer *buffer)
{
    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

void lo_text_buffer_free(lo_text_buffer *buffer)
{
    free(buffer->data);

    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

int lo_text_buffer_reserve(lo_text_buffer *buffer, size_t capacity)
{
    return lo_text_grow(
        (void **)&buffer->data,
        &buffer->capacity,
        capacity,
        sizeof(char)
    );
}

int lo_text_buffer_write(lo_text_buffer *buffer,
                         const void *data,
                         size_t length)
{
    if (length > SIZE_MAX - buffer->length)
        return -1;

    if (lo_text_buffer_reserve(buffer, buffer->length + length) != 0)
        return -1;

    memcpy(buffer->data + buffer->length, data, length);

    buffer->length += length;

    return 0;
}

int lo_text_buffer_putc(lo_text_buffer *buffer, char character)
{
    return lo_text_buffer_write(buffer, &character, 1);
}

void lo_text_buffer_clear(lo_text_buffer *buffer)
{
    buffer->length = 0;
}

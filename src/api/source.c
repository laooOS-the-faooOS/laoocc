#include "source/source_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int lo_source_buffer_reserve(
    lo_source_buffer *buffer,
    size_t capacity)
{
    char *data;

    if (!buffer)
        return -1;

    if (capacity <= buffer->capacity)
        return 0;

    data = realloc(buffer->data, capacity);

    if (!data)
        return -1;

    buffer->data = data;
    buffer->capacity = capacity;

    return 0;
}

void lo_source_location_init(
    lo_source_location *location)
{
    if (!location)
        return;

    location->offset = 0;
    location->line = 1;
    location->column = 1;
}

void lo_source_buffer_init(
    lo_source_buffer *buffer)
{
    if (!buffer)
        return;

    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

void lo_source_buffer_free(
    lo_source_buffer *buffer)
{
    if (!buffer)
        return;

    free(buffer->data);

    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}

int lo_source_buffer_load(
    lo_source_buffer *buffer,
    const char *data,
    size_t length)
{
    if (!buffer || (!data && length != 0))
        return -1;

    if (lo_source_buffer_reserve(buffer, length + 1) != 0)
        return -1;

    if (length != 0)
        memcpy(buffer->data, data, length);

    buffer->length = length;
    buffer->data[length] = '\0';

    return 0;
}

int lo_source_buffer_append(
    lo_source_buffer *buffer,
    const char *data,
    size_t length)
{
    size_t required;

    if (!buffer || (!data && length != 0))
        return -1;

    if (length > SIZE_MAX - buffer->length - 1)
        return -1;

    required = buffer->length + length + 1;

    if (required > buffer->capacity) {
        size_t capacity =
            buffer->capacity ? buffer->capacity : 256;

        while (capacity < required) {
            if (capacity > SIZE_MAX / 2)
                return -1;

            capacity *= 2;
        }

        if (lo_source_buffer_reserve(buffer, capacity) != 0)
            return -1;
    }

    if (length != 0)
        memcpy(buffer->data + buffer->length, data, length);

    buffer->length += length;
    buffer->data[buffer->length] = '\0';

    return 0;
}

void lo_source_buffer_clear(
    lo_source_buffer *buffer)
{
    if (!buffer)
        return;

    buffer->length = 0;

    if (buffer->data)
        buffer->data[0] = '\0';
}

const char *lo_source_buffer_data(
    const lo_source_buffer *buffer)
{
    if (!buffer || !buffer->data)
        return "";

    return buffer->data;
}

size_t lo_source_buffer_size(
    const lo_source_buffer *buffer)
{
    if (!buffer)
        return 0;

    return buffer->length;
}

void lo_source_init(
    lo_source *source)
{
    if (!source)
        return;

    source->name = NULL;
    source->path = NULL;

    lo_source_buffer_init(&source->buffer);
}

void lo_source_free(
    lo_source *source)
{
    if (!source)
        return;

    lo_source_buffer_free(&source->buffer);

    source->name = NULL;
    source->path = NULL;
}

int lo_source_load(
    lo_source *source,
    const char *name,
    const char *path)
{
    FILE *file;
    long size;
    char *data;
    size_t read_size;

    if (!source || !path)
        return -1;

    file = fopen(path, "rb");

    if (!file)
        return -1;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }

    size = ftell(file);

    if (size < 0) {
        fclose(file);
        return -1;
    }

    rewind(file);

    data = malloc((size_t)size + 1);

    if (!data) {
        fclose(file);
        return -1;
    }

    read_size = fread(data, 1, (size_t)size, file);

    fclose(file);

    if (read_size != (size_t)size) {
        free(data);
        return -1;
    }

    data[read_size] = '\0';

    if (lo_source_buffer_load(
            &source->buffer,
            data,
            read_size) != 0) {
        free(data);
        return -1;
    }

    free(data);

    source->name = name;
    source->path = path;

    return 0;
}

int lo_source_set_data(
    lo_source *source,
    const char *data,
    size_t length)
{
    if (!source)
        return -1;

    return lo_source_buffer_load(
        &source->buffer,
        data,
        length
    );
}

const char *lo_source_data(
    const lo_source *source)
{
    if (!source)
        return "";

    return lo_source_buffer_data(&source->buffer);
}

size_t lo_source_size(
    const lo_source *source)
{
    if (!source)
        return 0;

    return lo_source_buffer_size(&source->buffer);
}

int lo_source_get_location(
    const lo_source *source,
    size_t offset,
    lo_source_location *location)
{
    size_t i;
    size_t line = 1;
    size_t column = 1;

    if (!source || !location)
        return -1;

    if (offset > source->buffer.length)
        return -1;

    for (i = 0; i < offset; ++i) {
        if (source->buffer.data[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }

    location->offset = offset;
    location->line = line;
    location->column = column;

    return 0;
}

void lo_source_manager_init(
    lo_source_manager *manager)
{
    if (!manager)
        return;

    manager->sources = NULL;
    manager->source_count = 0;
    manager->source_capacity = 0;
}

void lo_source_manager_free(
    lo_source_manager *manager)
{
    size_t i;

    if (!manager)
        return;

    for (i = 0; i < manager->source_count; ++i)
        lo_source_free(&manager->sources[i]);

    free(manager->sources);

    manager->sources = NULL;
    manager->source_count = 0;
    manager->source_capacity = 0;
}

int lo_source_manager_add(
    lo_source_manager *manager,
    lo_source *source)
{
    lo_source *sources;
    size_t capacity;

    if (!manager || !source)
        return -1;

    if (manager->source_count == manager->source_capacity) {
        capacity = manager->source_capacity ?
                   manager->source_capacity * 2 : 8;

        if (capacity < manager->source_capacity)
            return -1;

        sources = realloc(
            manager->sources,
            capacity * sizeof(*sources)
        );

        if (!sources)
            return -1;

        manager->sources = sources;
        manager->source_capacity = capacity;
    }

    manager->sources[manager->source_count++] = *source;

    source->buffer.data = NULL;
    source->buffer.length = 0;
    source->buffer.capacity = 0;

    return 0;
}

lo_source *lo_source_manager_get(
    lo_source_manager *manager,
    size_t index)
{
    if (!manager || index >= manager->source_count)
        return NULL;

    return &manager->sources[index];
}

const lo_source *lo_source_manager_cget(
    const lo_source_manager *manager,
    size_t index)
{
    if (!manager || index >= manager->source_count)
        return NULL;

    return &manager->sources[index];
}

size_t lo_source_manager_count(
    const lo_source_manager *manager)
{
    if (!manager)
        return 0;

    return manager->source_count;
}

#include "source/source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t count_lines(const char *data, size_t length)
{
    size_t count = 1;

    for (size_t i = 0; i < length; ++i) {
        if (data[i] == '\n')
            ++count;
    }

    return count;
}

int lo_source_init(
    lo_source *source,
    const char *data,
    size_t length,
    const char *path
)
{
    if (!source || (!data && length != 0))
        return -1;

    source->data = malloc(length + 1);

    if (!source->data)
        return -1;

    if (length != 0)
        memcpy(source->data, data, length);

    source->data[length] = '\0';

    source->length = length;
    source->path = path;
    source->line_count = count_lines(source->data, length);

    return 0;
}

int lo_source_load(
    lo_source *source,
    const char *path
)
{
    FILE *file;
    long size;
    char *data;

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

    if (size != 0 &&
        fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return -1;
    }

    fclose(file);

    data[size] = '\0';

    source->data = data;
    source->length = (size_t)size;
    source->path = path;
    source->line_count = count_lines(data, (size_t)size);

    return 0;
}

void lo_source_free(
    lo_source *source
)
{
    if (!source)
        return;

    free(source->data);

    source->data = NULL;
    source->length = 0;
    source->path = NULL;
    source->line_count = 0;
}

const char *lo_source_line(
    const lo_source *source,
    size_t line
)
{
    size_t current = 1;
    const char *start;

    if (!source || !source->data || line == 0 ||
        line > source->line_count)
        return NULL;

    start = source->data;

    for (size_t i = 0; i < source->length; ++i) {
        if (current == line)
            return start;

        if (source->data[i] == '\n') {
            ++current;
            start = &source->data[i + 1];
        }
    }

    return current == line ? start : NULL;
}

size_t lo_source_line_count(
    const lo_source *source
)
{
    if (!source)
        return 0;

    return source->line_count;
}

#include "codegen/codegen.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int lo_codegen_emitter_reserve(lo_codegen_emitter *emitter,
                                      size_t capacity)
{
    char *data;

    if (!emitter)
        return -1;

    if (capacity <= emitter->capacity)
        return 0;

    data = realloc(emitter->data, capacity);

    if (!data)
        return -1;

    emitter->data = data;
    emitter->capacity = capacity;

    return 0;
}

void lo_codegen_context_init(lo_codegen_context *context)
{
    if (!context)
        return;

    context->module = NULL;
    context->target = NULL;
    context->output = NULL;
}

void lo_codegen_options_init(lo_codegen_options *options)
{
    if (!options)
        return;

    options->output_kind = LO_CODEGEN_OUTPUT_ASSEMBLY;
    options->optimize = 0;
    options->debug = 0;
    options->verbose = 0;
    options->output = NULL;
}

void lo_codegen_emitter_init(lo_codegen_emitter *emitter)
{
    if (!emitter)
        return;

    emitter->data = NULL;
    emitter->length = 0;
    emitter->capacity = 0;
}

void lo_codegen_emitter_free(lo_codegen_emitter *emitter)
{
    if (!emitter)
        return;

    free(emitter->data);

    emitter->data = NULL;
    emitter->length = 0;
    emitter->capacity = 0;
}

int lo_codegen_emitter_write(lo_codegen_emitter *emitter,
                             const char *data,
                             size_t length)
{
    size_t required;

    if (!emitter || (!data && length != 0))
        return -1;

    if (length > SIZE_MAX - emitter->length - 1)
        return -1;

    required = emitter->length + length + 1;

    if (required > emitter->capacity) {
        size_t capacity = emitter->capacity ? emitter->capacity : 256;

        while (capacity < required) {
            if (capacity > SIZE_MAX / 2)
                return -1;

            capacity *= 2;
        }

        if (lo_codegen_emitter_reserve(emitter, capacity) != 0)
            return -1;
    }

    if (length != 0)
        memcpy(emitter->data + emitter->length, data, length);

    emitter->length += length;
    emitter->data[emitter->length] = '\0';

    return 0;
}

int lo_codegen_emitter_puts(lo_codegen_emitter *emitter,
                            const char *string)
{
    if (!string)
        return -1;

    return lo_codegen_emitter_write(
        emitter,
        string,
        strlen(string)
    );
}

int lo_codegen_emitter_putc(lo_codegen_emitter *emitter,
                            char character)
{
    return lo_codegen_emitter_write(
        emitter,
        &character,
        1
    );
}

void lo_codegen_emitter_clear(lo_codegen_emitter *emitter)
{
    if (!emitter)
        return;

    emitter->length = 0;

    if (emitter->data)
        emitter->data[0] = '\0';
}

void lo_codegen_init(lo_codegen *codegen)
{
    if (!codegen)
        return;

    lo_codegen_context_init(&codegen->context);
    lo_codegen_options_init(&codegen->options);
    lo_codegen_emitter_init(&codegen->emitter);
}

int lo_codegen_generate(lo_codegen *codegen)
{
    if (!codegen)
        return -1;

    if (!codegen->context.module ||
        !codegen->context.target)
        return -1;

    lo_codegen_emitter_clear(&codegen->emitter);

    /*
     * Code generation backend will be implemented here.
     *
     * Current stage only establishes the API and verifies
     * that IR and target information are available.
     */

    if (lo_codegen_emitter_puts(
            &codegen->emitter,
            "/* laoocc codegen */\n") != 0)
        return -1;

    return 0;
}

const char *lo_codegen_data(const lo_codegen *codegen)
{
    if (!codegen || !codegen->emitter.data)
        return "";

    return codegen->emitter.data;
}

size_t lo_codegen_size(const lo_codegen *codegen)
{
    if (!codegen)
        return 0;

    return codegen->emitter.length;
}

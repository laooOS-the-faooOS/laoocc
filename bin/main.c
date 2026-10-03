#include <stdio.h>
#include <stdlib.h>

#include "preprocessor_api.h"

static char *read_file(const char *path, size_t *length)
{
    FILE *file;
    char *buffer;
    long size;
    size_t read_size;

    file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "laoocc: cannot open '%s'\n", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "laoocc: cannot seek '%s'\n", path);
        fclose(file);
        return NULL;
    }

    size = ftell(file);
    if (size < 0) {
        fprintf(stderr, "laoocc: cannot determine size of '%s'\n", path);
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "laoocc: cannot rewind '%s'\n", path);
        fclose(file);
        return NULL;
    }

    buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fprintf(stderr, "laoocc: out of memory\n");
        fclose(file);
        return NULL;
    }

    read_size = fread(buffer, 1, (size_t)size, file);

    if (read_size != (size_t)size) {
        fprintf(stderr, "laoocc: failed to read '%s'\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[read_size] = '\0';

    fclose(file);

    *length = read_size;
    return buffer;
}

static int preprocess(const char *path)
{
    char *source;
    char *output = NULL;
    size_t source_length;
    size_t output_length = 0;
    lo_preprocessor preprocessor;
    lo_pp_error error;
    int result;

    source = read_file(path, &source_length);
    if (!source)
        return 1;

    lo_preprocessor_init(
        &preprocessor,
        source,
        source_length
    );

    error.kind = LO_PP_ERROR_NONE;
    error.message = NULL;
    error.line = 0;
    error.column = 0;

    result = lo_preprocessor_run(
        &preprocessor,
        &output,
        &output_length,
        &error
    );

    if (result != 0) {
        fprintf(
            stderr,
            "laoocc: %s:%zu:%zu: error: %s: %s\n",
            path,
            error.line,
            error.column,
            lo_pp_error_name(error.kind),
            error.message ? error.message : "unknown error"
        );

        lo_preprocessor_free(&preprocessor);
        free(source);

        return 1;
    }

    if (output && output_length != 0) {
        if (fwrite(output, 1, output_length, stdout) != output_length) {
            fprintf(stderr, "laoocc: failed to write output\n");

            free(output);
            lo_preprocessor_free(&preprocessor);
            free(source);

            return 1;
        }
    }

    free(output);
    lo_preprocessor_free(&preprocessor);
    free(source);

    return 0;
}

static void usage(const char *program)
{
    fprintf(
        stderr,
        "usage: %s <input.c>\n",
        program
    );
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    return preprocess(argv[1]);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocessor_api.h"
#include "lexer_api.h"
#include "parser_api.h"

#define LAOOCC_NAME "laoocc"
#define LAOOCC_VERSION "0.1.0"

typedef struct compiler_options {
    char **include_dirs;
    size_t include_count;
    size_t include_capacity;
} compiler_options;

static void print_version(void)
{
    printf("%s %s\n", LAOOCC_NAME, LAOOCC_VERSION);
}

static void print_help(const char *program)
{
    printf(
        "usage: %s [options] <input.c>\n"
        "\n"
        "Options:\n"
        "  -I<dir>          Add include search directory\n"
        "  -I <dir>         Add include search directory\n"
        "  -v               Show compiler version\n"
        "  --version        Show compiler version\n"
        "  -h               Show this help\n"
        "  --help           Show this help\n"
        "\n"
        "More compiler options will be added later.\n",
        program
    );
}

static void options_free(compiler_options *options)
{
    if (!options)
        return;

    for (size_t i = 0; i < options->include_count; ++i)
        free(options->include_dirs[i]);

    free(options->include_dirs);

    options->include_dirs = NULL;
    options->include_count = 0;
    options->include_capacity = 0;
}

static int add_include_dir(
    compiler_options *options,
    const char *path)
{
    if (!options || !path || !*path)
        return 0;

    if (options->include_count == options->include_capacity) {
        size_t new_capacity =
            options->include_capacity
                ? options->include_capacity * 2
                : 8;

        char **new_dirs = realloc(
            options->include_dirs,
            new_capacity * sizeof(*new_dirs)
        );

        if (!new_dirs)
            return 0;

        options->include_dirs = new_dirs;
        options->include_capacity = new_capacity;
    }

    options->include_dirs[options->include_count] =
        malloc(strlen(path) + 1);

    if (!options->include_dirs[options->include_count])
        return 0;

    strcpy(
        options->include_dirs[options->include_count],
        path
    );

    options->include_count++;

    return 1;
}

static char *read_file(
    const char *path,
    size_t *length_out)
{
    FILE *file;
    long size;
    size_t length;
    char *buffer;

    if (!path || !length_out)
        return NULL;

    file = fopen(path, "rb");

    if (!file)
        return NULL;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    size = ftell(file);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    length = (size_t)size;

    buffer = malloc(length + 1);

    if (!buffer) {
        fclose(file);
        return NULL;
    }

    if (length != 0 &&
        fread(buffer, 1, length, file) != length) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0';

    fclose(file);

    *length_out = length;

    return buffer;
}

static int collect_tokens(
    const char *source,
    size_t source_length,
    lo_token **tokens_out,
    size_t *count_out)
{
    size_t capacity = 256;
    size_t count = 0;

    lo_token *tokens = malloc(
        capacity * sizeof(*tokens)
    );

    if (!tokens) {
        fprintf(
            stderr,
            "%s: out of memory allocating token buffer\n",
            LAOOCC_NAME
        );
        return 0;
    }

    lo_lexer lexer;

    lo_lexer_init(
        &lexer,
        source,
        source_length
    );

    for (;;) {
        lo_token token;
        lo_lexer_error error;

        int result = lo_lexer_next(
            &lexer,
            &token,
            &error
        );

        if (result < 0) {
            fprintf(
                stderr,
                "%s: lexer error at %zu:%zu: %s\n",
                LAOOCC_NAME,
                error.line,
                error.column,
                error.message
                    ? error.message
                    : lo_lexer_error_name(error.kind)
            );

            free(tokens);
            return 0;
        }

        if (count == capacity) {
            size_t new_capacity = capacity * 2;

            lo_token *new_tokens = realloc(
                tokens,
                new_capacity * sizeof(*new_tokens)
            );

            if (!new_tokens) {
                fprintf(
                    stderr,
                    "%s: out of memory growing token buffer\n",
                    LAOOCC_NAME
                );

                free(tokens);
                return 0;
            }

            tokens = new_tokens;
            capacity = new_capacity;
        }

        tokens[count++] = token;

        /*
         * Lexer contract:
         *
         *   0  = token produced
         *   1  = EOF
         *  -1  = error
         */
        if (result == 1)
            break;
    }

    *tokens_out = tokens;
    *count_out = count;

    return 1;
}

static void dump_ast(
    const lo_ast_node *node,
    unsigned depth)
{
    if (!node)
        return;

    for (unsigned i = 0; i < depth; ++i)
        printf("  ");

    printf(
        "%s",
        lo_ast_kind_name(node->kind)
    );

    if (node->line != 0 || node->column != 0) {
        printf(
            " [%u:%u]",
            node->line,
            node->column
        );
    }

    putchar('\n');

    for (size_t i = 0; i < node->child_count; ++i)
        dump_ast(node->children[i], depth + 1);
}

int main(int argc, char **argv)
{
    compiler_options options = {0};
    const char *input_path = NULL;

    char *source = NULL;
    size_t source_length = 0;

    char *preprocessed = NULL;
    size_t preprocessed_length = 0;

    lo_token *tokens = NULL;
    size_t token_count = 0;

    lo_parser parser;
    lo_ast_node root;

    int parser_initialized = 0;
    int result = 1;

    /*
     * Command-line options.
     */
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "-v") == 0 ||
            strcmp(arg, "--version") == 0) {
            print_version();
            options_free(&options);
            return 0;
        }

        if (strcmp(arg, "-h") == 0 ||
            strcmp(arg, "--help") == 0) {
            print_help(argv[0]);
            options_free(&options);
            return 0;
        }

        if (strncmp(arg, "-I", 2) == 0) {
            const char *path = arg + 2;

            if (*path == '\0') {
                if (i + 1 >= argc) {
                    fprintf(
                        stderr,
                        "%s: option '-I' requires an argument\n",
                        LAOOCC_NAME
                    );
                    options_free(&options);
                    return 1;
                }

                path = argv[++i];
            }

            if (!add_include_dir(&options, path)) {
                fprintf(
                    stderr,
                    "%s: failed to add include directory: %s\n",
                    LAOOCC_NAME,
                    path
                );
                options_free(&options);
                return 1;
            }

            continue;
        }

        if (arg[0] == '-') {
            fprintf(
                stderr,
                "%s: unknown option: %s\n",
                LAOOCC_NAME,
                arg
            );
            options_free(&options);
            return 1;
        }

        if (input_path) {
            fprintf(
                stderr,
                "%s: multiple input files are not supported\n",
                LAOOCC_NAME
            );
            options_free(&options);
            return 1;
        }

        input_path = arg;
    }

    if (!input_path) {
        fprintf(
            stderr,
            "%s: no input file\n",
            LAOOCC_NAME
        );
        print_help(argv[0]);
        options_free(&options);
        return 1;
    }

    /*
     * Read source.
     */
    source = read_file(
        input_path,
        &source_length
    );

    if (!source) {
        fprintf(
            stderr,
            "%s: failed to read '%s'\n",
            LAOOCC_NAME,
            input_path
        );
        options_free(&options);
        return 1;
    }

    /*
     * Preprocessor.
     */
    lo_preprocessor preprocessor;

    lo_preprocessor_init(
        &preprocessor,
        source,
        source_length
    );

    if (!lo_pp_set_source_file(
            &preprocessor,
            input_path)) {
        fprintf(
            stderr,
            "%s: failed to set source file\n",
            LAOOCC_NAME
        );

        lo_preprocessor_free(&preprocessor);
        free(source);
        options_free(&options);
        return 1;
    }

    for (size_t i = 0; i < options.include_count; ++i) {
        if (!lo_pp_add_include_dir(
                &preprocessor,
                options.include_dirs[i])) {
            fprintf(
                stderr,
                "%s: failed to add include directory: %s\n",
                LAOOCC_NAME,
                options.include_dirs[i]
            );

            lo_preprocessor_free(&preprocessor);
            free(source);
            options_free(&options);
            return 1;
        }
    }

    lo_pp_error pp_error = {0};

    if (lo_preprocessor_run(
            &preprocessor,
            &preprocessed,
            &preprocessed_length,
            &pp_error) != 0) {

        fprintf(
            stderr,
            "%s: %s:%zu:%zu: preprocessor error: %s\n",
            LAOOCC_NAME,
            input_path,
            pp_error.line,
            pp_error.column,
            pp_error.message
                ? pp_error.message
                : lo_pp_error_name(pp_error.kind)
        );

        lo_preprocessor_free(&preprocessor);
        free(source);
        options_free(&options);
        return 1;
    }

    lo_preprocessor_free(&preprocessor);
    free(source);

    /*
     * Lexer.
     */
    if (!collect_tokens(
            preprocessed,
            preprocessed_length,
            &tokens,
            &token_count)) {
        free(preprocessed);
        options_free(&options);
        return 1;
    }

    /*
     * Parser.
     */
    lo_parser_init(
        &parser,
        tokens,
        token_count
    );

    parser_initialized = 1;

    lo_ast_node_init(
        &root,
        LO_AST_TRANSLATION_UNIT
    );

    if (lo_parser_parse(
            &parser,
            &root) != 0) {

        const lo_parser_error *error =
            lo_parser_get_error(&parser);

        if (error) {
            fprintf(
                stderr,
                "%s: parser error at %zu:%zu: %s\n",
                LAOOCC_NAME,
                error->line,
                error->column,
                error->message
                    ? error->message
                    : "unknown parser error"
            );
        } else {
            fprintf(
                stderr,
                "%s: parser error\n",
                LAOOCC_NAME
            );
        }

        goto cleanup;
    }

    /*
     * Temporary AST output.
     */
    dump_ast(&root, 0);

    result = 0;

cleanup:
    lo_ast_node_free(&root);

    if (parser_initialized)
        lo_parser_free(&parser);

    free(tokens);
    free(preprocessed);
    options_free(&options);

    return result;
}

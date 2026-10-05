#include "preprocessor.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PP_MAX_EXPAND_DEPTH 64
#define PP_MAX_INCLUDE_DEPTH 32

typedef struct pp_buffer {
    char *data;
    size_t length;
    size_t capacity;
} pp_buffer;

typedef struct pp_conditional {
    int parent_active;
    int active;
    int branch_taken;
    struct pp_conditional *next;
} pp_conditional;

typedef struct pp_context {
    lo_preprocessor *pp;
    pp_buffer *output;
    lo_pp_error *error;
    unsigned include_depth;
    pp_conditional *conditionals;
} pp_context;

/* ------------------------------------------------------------------ */
/* buffer                                                              */
/* ------------------------------------------------------------------ */

static int buffer_reserve(pp_buffer *buffer, size_t extra)
{
    size_t required;
    size_t capacity;
    char *data;

    if (extra > SIZE_MAX - buffer->length - 1)
        return -1;

    required = buffer->length + extra + 1;

    if (required <= buffer->capacity)
        return 0;

    capacity = buffer->capacity ? buffer->capacity : 256;

    while (capacity < required) {
        if (capacity > SIZE_MAX / 2)
            return -1;

        capacity *= 2;
    }

    data = realloc(buffer->data, capacity);
    if (!data)
        return -1;

    buffer->data = data;
    buffer->capacity = capacity;

    return 0;
}

static int buffer_append(
    pp_buffer *buffer,
    const char *data,
    size_t length)
{
    if (length == 0)
        return 0;

    if (buffer_reserve(buffer, length) != 0)
        return -1;

    memcpy(buffer->data + buffer->length, data, length);

    buffer->length += length;
    buffer->data[buffer->length] = '\0';

    return 0;
}

static int buffer_putc(
    pp_buffer *buffer,
    char c)
{
    return buffer_append(buffer, &c, 1);
}

/* ------------------------------------------------------------------ */
/* strings                                                             */
/* ------------------------------------------------------------------ */

static char *pp_strdup(const char *string)
{
    size_t length;
    char *copy;

    if (!string)
        return NULL;

    length = strlen(string);

    copy = malloc(length + 1);
    if (!copy)
        return NULL;

    memcpy(copy, string, length + 1);

    return copy;
}

static char *pp_strndup(
    const char *string,
    size_t length)
{
    char *copy;

    copy = malloc(length + 1);
    if (!copy)
        return NULL;

    memcpy(copy, string, length);
    copy[length] = '\0';

    return copy;
}

/* ------------------------------------------------------------------ */
/* characters                                                          */
/* ------------------------------------------------------------------ */

static int is_space_char(char c)
{
    return c == ' ' ||
           c == '\t' ||
           c == '\r' ||
           c == '\v' ||
           c == '\f';
}

static int is_identifier_start(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static int is_identifier_char(char c)
{
    return is_identifier_start(c) ||
           (c >= '0' && c <= '9');
}

/* ------------------------------------------------------------------ */
/* errors                                                              */
/* ------------------------------------------------------------------ */

static int pp_error(
    pp_context *context,
    lo_pp_error_kind kind,
    const char *message,
    size_t line,
    size_t column)
{
    if (context->error) {
        context->error->kind = kind;
        context->error->message = message;
        context->error->line = line;
        context->error->column = column;
    }

    return -1;
}

static int pp_oom(pp_context *context, size_t line)
{
    return pp_error(
        context,
        LO_PP_ERROR_MEMORY,
        "out of memory",
        line,
        1
    );
}

const char *lo_pp_error_name(lo_pp_error_kind kind)
{
    switch (kind) {
    case LO_PP_ERROR_NONE:
        return "none";

    case LO_PP_ERROR_INVALID_DIRECTIVE:
        return "invalid directive";

    case LO_PP_ERROR_INVALID_DEFINE:
        return "invalid define";

    case LO_PP_ERROR_INVALID_UNDEF:
        return "invalid undef";

    case LO_PP_ERROR_INVALID_CONDITIONAL:
        return "invalid conditional";

    case LO_PP_ERROR_INCLUDE_NOT_FOUND:
        return "include not found";

    case LO_PP_ERROR_INCLUDE_ERROR:
        return "include error";

    case LO_PP_ERROR_MEMORY:
        return "memory error";
    }

    return "unknown";
}

/* ------------------------------------------------------------------ */
/* macros                                                              */
/* ------------------------------------------------------------------ */

static lo_pp_macro *find_macro(
    lo_preprocessor *preprocessor,
    const char *name)
{
    lo_pp_macro *macro;

    for (macro = preprocessor->macros;
         macro;
         macro = macro->next) {

        if (strcmp(macro->name, name) == 0)
            return macro;
    }

    return NULL;
}

static int define_macro(
    lo_preprocessor *preprocessor,
    const char *name,
    const char *value)
{
    lo_pp_macro *macro;
    char *new_value;

    new_value = pp_strdup(value);

    if (!new_value)
        return -1;

    macro = find_macro(preprocessor, name);

    if (macro) {
        free(macro->value);
        macro->value = new_value;
        return 0;
    }

    macro = calloc(1, sizeof(*macro));

    if (!macro) {
        free(new_value);
        return -1;
    }

    macro->name = pp_strdup(name);

    if (!macro->name) {
        free(new_value);
        free(macro);
        return -1;
    }

    macro->value = new_value;

    macro->next = preprocessor->macros;
    preprocessor->macros = macro;

    return 0;
}

static void undef_macro(
    lo_preprocessor *preprocessor,
    const char *name)
{
    lo_pp_macro **link;

    link = &preprocessor->macros;

    while (*link) {
        lo_pp_macro *macro = *link;

        if (strcmp(macro->name, name) == 0) {
            *link = macro->next;

            free(macro->name);
            free(macro->value);
            free(macro);

            return;
        }

        link = &macro->next;
    }
}

/* ------------------------------------------------------------------ */
/* conditionals                                                        */
/* ------------------------------------------------------------------ */

static int conditional_active(
    pp_conditional *conditionals)
{
    if (!conditionals)
        return 1;

    return conditionals->active;
}

static int conditional_push(
    pp_context *context,
    int condition)
{
    pp_conditional *conditional;

    conditional = calloc(1, sizeof(*conditional));

    if (!conditional)
        return -1;

    conditional->parent_active =
        conditional_active(context->conditionals);

    conditional->active =
        conditional->parent_active && condition;

    conditional->branch_taken =
        conditional->active;

    conditional->next =
        context->conditionals;

    context->conditionals =
        conditional;

    return 0;
}

static int conditional_else(
    pp_context *context)
{
    pp_conditional *conditional;

    conditional = context->conditionals;

    if (!conditional)
        return -1;

    if (conditional->branch_taken)
        conditional->active = 0;
    else {
        conditional->active =
            conditional->parent_active;

        conditional->branch_taken =
            conditional->active;
    }

    return 0;
}

static int conditional_pop(
    pp_context *context)
{
    pp_conditional *conditional;

    conditional = context->conditionals;

    if (!conditional)
        return -1;

    context->conditionals =
        conditional->next;

    free(conditional);

    return 0;
}

static void conditional_free_all(
    pp_context *context)
{
    while (context->conditionals)
        conditional_pop(context);
}

/* ------------------------------------------------------------------ */
/* macro expansion                                                     */
/* ------------------------------------------------------------------ */

static int macro_disabled(
    lo_pp_macro **disabled,
    size_t count,
    lo_pp_macro *macro)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        if (disabled[i] == macro)
            return 1;
    }

    return 0;
}

static int expand_text(
    pp_context *context,
    const char *source,
    size_t length,
    lo_pp_macro **disabled,
    size_t disabled_count)
{
    size_t i = 0;

    while (i < length) {
        char c = source[i];

        /*
         * Strings and character constants are copied verbatim.
         */
        if (c == '"' || c == '\'') {
            char quote = c;
            size_t start = i;

            ++i;

            while (i < length) {
                if (source[i] == '\\') {
                    if (i + 1 < length)
                        i += 2;
                    else
                        ++i;

                    continue;
                }

                if (source[i] == quote) {
                    ++i;
                    break;
                }

                ++i;
            }

            if (buffer_append(
                    context->output,
                    source + start,
                    i - start) != 0)
                return pp_oom(context, 0);

            continue;
        }

        /*
         * Comments are copied verbatim.
         */
        if (c == '/' &&
            i + 1 < length &&
            source[i + 1] == '/') {

            if (buffer_append(
                    context->output,
                    source + i,
                    length - i) != 0)
                return pp_oom(context, 0);

            return 0;
        }

        if (c == '/' &&
            i + 1 < length &&
            source[i + 1] == '*') {

            size_t start = i;

            i += 2;

            while (i + 1 < length &&
                   !(source[i] == '*' &&
                     source[i + 1] == '/'))
                ++i;

            if (i + 1 < length)
                i += 2;
            else
                i = length;

            if (buffer_append(
                    context->output,
                    source + start,
                    i - start) != 0)
                return pp_oom(context, 0);

            continue;
        }

        /*
         * Identifier -> macro expansion.
         */
        if (is_identifier_start(c)) {
            size_t start = i;
            size_t name_length;
            char *name;
            lo_pp_macro *macro;

            ++i;

            while (i < length &&
                   is_identifier_char(source[i]))
                ++i;

            name_length = i - start;

            name = pp_strndup(
                source + start,
                name_length);

            if (!name)
                return pp_oom(context, 0);

            macro = find_macro(
                context->pp,
                name);

            if (macro &&
                !macro_disabled(
                    disabled,
                    disabled_count,
                    macro) &&
                disabled_count < PP_MAX_EXPAND_DEPTH) {

                lo_pp_macro *next_disabled[
                    PP_MAX_EXPAND_DEPTH
                ];

                memcpy(
                    next_disabled,
                    disabled,
                    disabled_count *
                    sizeof(*disabled));

                next_disabled[disabled_count] =
                    macro;

                if (expand_text(
                        context,
                        macro->value,
                        strlen(macro->value),
                        next_disabled,
                        disabled_count + 1) != 0) {

                    free(name);
                    return -1;
                }
            } else {
                if (buffer_append(
                        context->output,
                        source + start,
                        name_length) != 0) {

                    free(name);
                    return pp_oom(context, 0);
                }
            }

            free(name);

            continue;
        }

        /*
         * Normal character.
         */
        if (buffer_putc(
                context->output,
                c) != 0)
            return pp_oom(context, 0);

        ++i;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* file handling                                                       */
/* ------------------------------------------------------------------ */

static char *read_file(
    const char *path,
    size_t *length)
{
    FILE *file;
    long size;
    size_t read_count;
    char *buffer;

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

    buffer = malloc((size_t)size + 1);

    if (!buffer) {
        fclose(file);
        return NULL;
    }

    read_count =
        fread(buffer, 1, (size_t)size, file);

    fclose(file);

    if (read_count != (size_t)size) {
        free(buffer);
        return NULL;
    }

    buffer[read_count] = '\0';

    if (length)
        *length = read_count;

    return buffer;
}

static int file_exists(const char *path)
{
    FILE *file;

    file = fopen(path, "rb");

    if (!file)
        return 0;

    fclose(file);

    return 1;
}

static char *join_path(
    const char *directory,
    const char *name)
{
    size_t directory_length;
    size_t name_length;
    int separator;
    char *path;

    directory_length =
        strlen(directory);

    name_length =
        strlen(name);

    separator =
        directory_length > 0 &&
        directory[directory_length - 1] != '/';

    path = malloc(
        directory_length +
        (size_t)separator +
        name_length +
        1);

    if (!path)
        return NULL;

    memcpy(
        path,
        directory,
        directory_length);

    if (separator)
        path[directory_length++] = '/';

    memcpy(
        path + directory_length,
        name,
        name_length);

    path[
        directory_length +
        name_length
    ] = '\0';

    return path;
}

static char *source_directory(
    const char *source_file)
{
    const char *slash;
    size_t length;

    if (!source_file)
        return pp_strdup(".");

    slash = strrchr(source_file, '/');

    if (!slash)
        return pp_strdup(".");

    if (slash == source_file)
        return pp_strdup("/");

    length =
        (size_t)(slash - source_file);

    return pp_strndup(
        source_file,
        length);
}

/* ------------------------------------------------------------------ */
/* include search                                                      */
/* ------------------------------------------------------------------ */

static char *find_include(
    lo_preprocessor *preprocessor,
    const char *name,
    int angled)
{
    lo_pp_include_dir *directory;
    char *path;
    char *source_dir;

    /*
     * #include "file.h"
     *
     * Search the directory containing
     * the current source file first.
     */
    if (!angled &&
        preprocessor->source_file) {

        source_dir =
            source_directory(
                preprocessor->source_file);

        if (!source_dir)
            return NULL;

        path =
            join_path(
                source_dir,
                name);

        free(source_dir);

        if (!path)
            return NULL;

        if (file_exists(path))
            return path;

        free(path);
    }

    /*
     * -I directories.
     */
    for (directory =
             preprocessor->include_dirs;
         directory;
         directory =
             directory->next) {

        path =
            join_path(
                directory->path,
                name);

        if (!path)
            return NULL;

        if (file_exists(path))
            return path;

        free(path);
    }

    /*
     * Host system headers.
     *
     * We'll eventually make these
     * configurable through sysroot.
     */
    {
        static const char *system_dirs[] = {
            "/usr/local/include",
            "/usr/include",
            NULL
        };

        size_t i;

        for (i = 0;
             system_dirs[i];
             ++i) {

            path =
                join_path(
                    system_dirs[i],
                    name);

            if (!path)
                return NULL;

            if (file_exists(path))
                return path;

            free(path);
        }
    }

    return NULL;
}

/* ------------------------------------------------------------------ */
/* include processing                                                  */
/* ------------------------------------------------------------------ */

static int process_source(
    pp_context *context,
    const char *source,
    size_t length,
    const char *source_file,
    unsigned depth);

static int process_include(
    pp_context *context,
    const char *name,
    int angled,
    size_t line)
{
    char *include_path;
    char *included_source;
    char *old_source_file;
    size_t included_length;
    int result;

    if (context->include_depth >=
        PP_MAX_INCLUDE_DEPTH) {

        return pp_error(
            context,
            LO_PP_ERROR_INCLUDE_ERROR,
            "include nesting too deep",
            line,
            1);
    }

    include_path =
        find_include(
            context->pp,
            name,
            angled);

    if (!include_path) {
        return pp_error(
            context,
            LO_PP_ERROR_INCLUDE_NOT_FOUND,
            "include file not found",
            line,
            1);
    }

    included_source =
        read_file(
            include_path,
            &included_length);

    if (!included_source) {
        free(include_path);

        return pp_error(
            context,
            LO_PP_ERROR_INCLUDE_ERROR,
            "failed to read include file",
            line,
            1);
    }

    old_source_file =
        context->pp->source_file;

    context->pp->source_file =
        include_path;

    ++context->include_depth;

    result =
        process_source(
            context,
            included_source,
            included_length,
            include_path,
            context->include_depth);

    --context->include_depth;

    context->pp->source_file =
        old_source_file;

    free(included_source);
    free(include_path);

    return result;
}

/* ------------------------------------------------------------------ */
/* directive parsing                                                   */
/* ------------------------------------------------------------------ */

static void skip_spaces(
    const char **cursor,
    const char *end)
{
    while (*cursor < end &&
           is_space_char(**cursor))
        ++*cursor;
}

static int parse_identifier(
    const char **cursor,
    const char *end,
    char *name,
    size_t name_size)
{
    size_t length = 0;

    skip_spaces(cursor, end);

    if (*cursor >= end ||
        !is_identifier_start(**cursor))
        return -1;

    while (*cursor < end &&
           is_identifier_char(**cursor)) {

        if (length + 1 < name_size)
            name[length++] = **cursor;

        ++*cursor;
    }

    name[length] = '\0';

    return 0;
}

static int evaluate_condition(
    lo_preprocessor *preprocessor,
    const char *expression,
    size_t length)
{
    const char *cursor = expression;
    char *end;
    long value;
    char *expanded;
    size_t expanded_length;
    pp_buffer buffer = {0};
    lo_pp_macro *macro;
    char name[256];

    skip_spaces(
        &cursor,
        expression + length);

    /*
     * defined(NAME)
     */
    if (strncmp(cursor, "defined", 7) == 0) {
        cursor += 7;

        while (cursor < expression + length &&
               is_space_char(*cursor))
            ++cursor;

        if (cursor < expression + length &&
            *cursor == '(') {

            ++cursor;

            if (parse_identifier(
                    &cursor,
                    expression + length,
                    name,
                    sizeof(name)) != 0)
                return 0;

            while (cursor < expression + length &&
                   is_space_char(*cursor))
                ++cursor;

            if (cursor < expression + length &&
                *cursor == ')') {

                macro =
                    find_macro(
                        preprocessor,
                        name);

                return macro != NULL;
            }

            return 0;
        }

        if (parse_identifier(
                &cursor,
                expression + length,
                name,
                sizeof(name)) != 0)
            return 0;

        macro =
            find_macro(
                preprocessor,
                name);

        return macro != NULL;
    }

    /*
     * Expand simple macros before
     * evaluating integer constants.
     */
    {
        pp_context context;

        memset(&context, 0, sizeof(context));

        context.pp =
            preprocessor;

        context.output =
            &buffer;

        if (expand_text(
                &context,
                expression,
                length,
                NULL,
                0) != 0) {

            free(buffer.data);
            return 0;
        }
    }

    expanded =
        buffer.data;

    expanded_length =
        buffer.length;

    (void)expanded_length;

    if (!expanded)
        return 0;

    value =
        strtol(
            expanded,
            &end,
            0);

    while (*end &&
           isspace((unsigned char)*end))
        ++end;

    if (*end != '\0') {
        free(expanded);
        return 0;
    }

    free(expanded);

    return value != 0;
}

/* ------------------------------------------------------------------ */
/* line processing                                                     */
/* ------------------------------------------------------------------ */

static int process_source(
    pp_context *context,
    const char *source,
    size_t length,
    const char *source_file,
    unsigned depth)
{
    size_t offset = 0;
    size_t line = 1;
    pp_conditional *saved_conditionals;

    (void)source_file;
    (void)depth;

    saved_conditionals =
        context->conditionals;

    while (offset < length) {
        size_t line_start = offset;
        size_t line_end;
        size_t cursor;
        size_t logical_end;
        int directive = 0;

        /*
         * Find physical line ending.
         */
        while (offset < length &&
               source[offset] != '\n')
            ++offset;

        line_end = offset;

        /*
         * Handle backslash-newline continuation.
         */
        logical_end = line_end;

        while (logical_end > line_start &&
               source[logical_end - 1] == '\\') {

            if (offset < length)
                ++offset;

            while (offset < length &&
                   source[offset] != '\n')
                ++offset;

            logical_end = offset;
        }

        cursor = line_start;

        /*
         * Skip whitespace before '#'.
         */
        while (cursor < logical_end &&
               (source[cursor] == ' ' ||
                source[cursor] == '\t'))
            ++cursor;

        if (cursor < logical_end &&
            source[cursor] == '#') {

            const char *directive_start;
            const char *directive_end;
            size_t directive_length;
            const char *argument;
            size_t argument_length;
            char directive_name[64];

            directive = 1;

            ++cursor;

            while (cursor < logical_end &&
                   is_space_char(source[cursor]))
                ++cursor;

            directive_start = source + cursor;

            while (cursor < logical_end &&
                   is_identifier_char(source[cursor]))
                ++cursor;

            directive_end =
                source + cursor;

            directive_length =
                (size_t)(
                    directive_end -
                    directive_start);

            if (directive_length >=
                sizeof(directive_name))
                directive_length =
                    sizeof(directive_name) - 1;

            memcpy(
                directive_name,
                directive_start,
                directive_length);

            directive_name[
                directive_length
            ] = '\0';

            while (cursor < logical_end &&
                   is_space_char(source[cursor]))
                ++cursor;

            argument =
                source + cursor;

            argument_length =
                logical_end - cursor;

            if (strcmp(
                    directive_name,
                    "define") == 0) {

                char name[256];
                const char *value;
                size_t value_length;

                if (!conditional_active(
                        context->conditionals))
                    goto directive_done;

                if (parse_identifier(
                        &argument,
                        source + logical_end,
                        name,
                        sizeof(name)) != 0) {

                    pp_error(
                        context,
                        LO_PP_ERROR_INVALID_DEFINE,
                        "invalid macro definition",
                        line,
                        1);

                    goto fail;
                }

                /*
                 * Function-like macros are intentionally
                 * not supported yet.
                 */
                if (argument < source + logical_end &&
                    *argument == '(') {

                    pp_error(
                        context,
                        LO_PP_ERROR_INVALID_DEFINE,
                        "function-like macros are not supported yet",
                        line,
                        1);

                    goto fail;
                }

                while (argument <
                           source + logical_end &&
                       is_space_char(*argument))
                    ++argument;

                value = argument;

                value_length =
                    (size_t)(
                        source + logical_end -
                        value);

                while (value_length > 0 &&
                       is_space_char(
                           value[value_length - 1]))
                    --value_length;

                {
                    char *value_copy =
                        pp_strndup(
                            value,
                            value_length);

                    int result;

                    if (!value_copy) {
                        pp_oom(context, line);
                        goto fail;
                    }

                    result =
                        define_macro(
                            context->pp,
                            name,
                            value_copy);

                    free(value_copy);

                    if (result != 0) {
                        pp_oom(context, line);
                        goto fail;
                    }
                }
            }
            else if (strcmp(
                         directive_name,
                         "undef") == 0) {

                char name[256];

                if (!conditional_active(
                        context->conditionals))
                    goto directive_done;

                if (parse_identifier(
                        &argument,
                        source + logical_end,
                        name,
                        sizeof(name)) != 0) {

                    pp_error(
                        context,
                        LO_PP_ERROR_INVALID_UNDEF,
                        "invalid #undef",
                        line,
                        1);

                    goto fail;
                }

                undef_macro(
                    context->pp,
                    name);
            }
            else if (strcmp(
                         directive_name,
                         "include") == 0) {

                char name[4096];
                size_t name_length = 0;
                int angled;

                if (!conditional_active(
                        context->conditionals))
                    goto directive_done;

                while (argument <
                           source + logical_end &&
                       is_space_char(*argument))
                    ++argument;

                if (argument >=
                    source + logical_end) {

                    pp_error(
                        context,
                        LO_PP_ERROR_INCLUDE_ERROR,
                        "missing include file",
                        line,
                        1);

                    goto fail;
                }

                if (*argument == '<')
                    angled = 1;
                else if (*argument == '"')
                    angled = 0;
                else {
                    pp_error(
                        context,
                        LO_PP_ERROR_INCLUDE_ERROR,
                        "invalid include syntax",
                        line,
                        1);

                    goto fail;
                }

                {
                    char closing =
                        angled ? '>' : '"';

                    ++argument;

                    while (
                        argument <
                            source + logical_end &&
                        *argument != closing) {

                        if (name_length + 1 <
                            sizeof(name))
                            name[name_length++] =
                                *argument;

                        ++argument;
                    }

                    if (argument >=
                        source + logical_end) {

                        pp_error(
                            context,
                            LO_PP_ERROR_INCLUDE_ERROR,
                            "unterminated include",
                            line,
                            1);

                        goto fail;
                    }
                }

                name[name_length] = '\0';

                if (process_include(
                        context,
                        name,
                        angled,
                        line) != 0)
                    goto fail;
            }
            else if (strcmp(
                         directive_name,
                         "ifdef") == 0) {

                char name[256];
                int condition = 0;

                if (parse_identifier(
                        &argument,
                        source + logical_end,
                        name,
                        sizeof(name)) == 0) {

                    condition =
                        find_macro(
                            context->pp,
                            name) != NULL;
                }

                if (conditional_push(
                        context,
                        condition) != 0) {

                    pp_oom(context, line);
                    goto fail;
                }
            }
            else if (strcmp(
                         directive_name,
                         "ifndef") == 0) {

                char name[256];
                int condition = 0;

                if (parse_identifier(
                        &argument,
                        source + logical_end,
                        name,
                        sizeof(name)) == 0) {

                    condition =
                        find_macro(
                            context->pp,
                            name) == NULL;
                }

                if (conditional_push(
                        context,
                        condition) != 0) {

                    pp_oom(context, line);
                    goto fail;
                }
            }
            else if (strcmp(
                         directive_name,
                         "if") == 0) {

                int condition = 0;

                if (conditional_active(
                        context->conditionals)) {

                    condition =
                        evaluate_condition(
                            context->pp,
                            argument,
                            argument_length);
                }

                if (conditional_push(
                        context,
                        condition) != 0) {

                    pp_oom(context, line);
                    goto fail;
                }
            }
            else if (strcmp(
                         directive_name,
                         "else") == 0) {

                if (conditional_else(
                        context) != 0) {

                    pp_error(
                        context,
                        LO_PP_ERROR_INVALID_CONDITIONAL,
                        "unexpected #else",
                        line,
                        1);

                    goto fail;
                }
            }
            else if (strcmp(
                         directive_name,
                         "endif") == 0) {

                if (conditional_pop(
                        context) != 0) {

                    pp_error(
                        context,
                        LO_PP_ERROR_INVALID_CONDITIONAL,
                        "unexpected #endif",
                        line,
                        1);

                    goto fail;
                }
            }
            else if (directive_name[0] != '\0') {

                if (conditional_active(
                        context->conditionals)) {

                    pp_error(
                        context,
                        LO_PP_ERROR_INVALID_DIRECTIVE,
                        "unknown preprocessing directive",
                        line,
                        1);

                    goto fail;
                }
            }

directive_done:

            /*
             * Preserve newlines so lexer diagnostics
             * continue to use the original line numbers.
             */
            {
                size_t i;

                for (i = 0;
                     i < logical_end - line_start;
                     ++i) {

                    if (buffer_putc(
                            context->output,
                            '\n') != 0) {

                        pp_oom(context, line);
                        goto fail;
                    }
                }
            }
        }
        else if (conditional_active(
                     context->conditionals)) {

            if (expand_text(
                    context,
                    source + line_start,
                    logical_end - line_start,
                    NULL,
                    0) != 0)
                goto fail;

            if (logical_end < length) {
                if (buffer_putc(
                        context->output,
                        '\n') != 0) {

                    pp_oom(context, line);
                    goto fail;
                }
            }
        }
        else {
            /*
             * Keep newlines from disabled code.
             */
            if (logical_end < length) {
                if (buffer_putc(
                        context->output,
                        '\n') != 0) {

                    pp_oom(context, line);
                    goto fail;
                }
            }
        }

        if (logical_end < length)
            offset = logical_end + 1;
        else
            offset = logical_end;

        ++line;

        (void)directive;
    }

    if (context->conditionals !=
        saved_conditionals) {

        pp_error(
            context,
            LO_PP_ERROR_INVALID_CONDITIONAL,
            "unterminated conditional",
            line,
            1);

        goto fail;
    }

    return 0;

fail:
    while (context->conditionals !=
           saved_conditionals) {

        conditional_pop(context);
    }

    return -1;
}

/* ------------------------------------------------------------------ */
/* public API                                                          */
/* ------------------------------------------------------------------ */

void lo_preprocessor_init(
    lo_preprocessor *preprocessor,
    const char *source,
    size_t length)
{
    if (!preprocessor)
        return;

    memset(
        preprocessor,
        0,
        sizeof(*preprocessor));

    preprocessor->source = source;
    preprocessor->length = length;
}

int lo_pp_add_include_dir(
    lo_preprocessor *preprocessor,
    const char *path
)
{
    if (!preprocessor || !path || path[0] == '\0')
        return 0;

    lo_pp_include_dir *entry =
        calloc(1, sizeof(*entry));

    if (!entry)
        return 0;

    entry->path = malloc(strlen(path) + 1);

    if (!entry->path) {
        free(entry);
        return 0;
    }

    strcpy(entry->path, path);

    entry->next = NULL;

    if (!preprocessor->include_dirs) {
        preprocessor->include_dirs = entry;
        return 1;
    }

    lo_pp_include_dir *tail =
        preprocessor->include_dirs;

    while (tail->next)
        tail = tail->next;

    tail->next = entry;

    return 1;
}

int lo_pp_set_source_file(
    lo_preprocessor *preprocessor,
    const char *path
)
{
    if (!preprocessor || !path)
        return 0;

    char *copy = malloc(strlen(path) + 1);

    if (!copy)
        return 0;

    strcpy(copy, path);

    free(preprocessor->source_file);
    preprocessor->source_file = copy;

    return 1;
}

int lo_preprocessor_run(
    lo_preprocessor *preprocessor,
    char **output,
    size_t *output_length,
    lo_pp_error *error)
{
    pp_buffer buffer = {0};
    pp_context context;
    int result;

    if (output)
        *output = NULL;

    if (output_length)
        *output_length = 0;

    if (error) {
        error->kind =
            LO_PP_ERROR_NONE;

        error->message =
            NULL;

        error->line = 0;
        error->column = 0;
    }

    if (!preprocessor ||
        !output ||
        !output_length) {

        if (error) {
            error->kind =
                LO_PP_ERROR_INVALID_DIRECTIVE;

            error->message =
                "invalid arguments";
        }

        return -1;
    }

    memset(
        &context,
        0,
        sizeof(context));

    context.pp =
        preprocessor;

    context.output =
        &buffer;

    context.error =
        error;

    if (buffer_reserve(
            &buffer,
            preprocessor->length) != 0) {

        pp_oom(&context, 1);
        return -1;
    }

    buffer.data[0] = '\0';

    result =
        process_source(
            &context,
            preprocessor->source
                ? preprocessor->source
                : "",
            preprocessor->source
                ? preprocessor->length
                : 0,
            preprocessor->source_file,
            0);

    conditional_free_all(&context);

    if (result != 0) {
        free(buffer.data);
        return -1;
    }

    *output =
        buffer.data;

    *output_length =
        buffer.length;

    return 0;
}

void lo_preprocessor_free(
    lo_preprocessor *preprocessor)
{
    lo_pp_macro *macro;
    lo_pp_include_dir *directory;

    if (!preprocessor)
        return;

    while (preprocessor->macros) {
        macro =
            preprocessor->macros;

        preprocessor->macros =
            macro->next;

        free(macro->name);
        free(macro->value);
        free(macro);
    }

    while (preprocessor->include_dirs) {
        directory =
            preprocessor->include_dirs;

        preprocessor->include_dirs =
            directory->next;

        free(directory->path);
        free(directory);
    }

    free(preprocessor->source_file);

    preprocessor->source_file = NULL;
    preprocessor->source = NULL;
    preprocessor->length = 0;
}

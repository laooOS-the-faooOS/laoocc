#include "linker/linker.h"

#include <stdlib.h>
#include <string.h>

/* Command */

void lo_linker_command_init(lo_linker_command *command)
{
    if (!command)
        return;

    command->program = NULL;
    command->arguments = NULL;
    command->count = 0;
    command->capacity = 0;
}

void lo_linker_command_free(lo_linker_command *command)
{
    if (!command)
        return;

    free(command->arguments);

    command->program = NULL;
    command->arguments = NULL;
    command->count = 0;
    command->capacity = 0;
}

int lo_linker_command_push(lo_linker_command *command,
                           const char *argument)
{
    char **arguments;
    size_t capacity;

    if (!command || !argument)
        return -1;

    if (command->count == command->capacity) {
        capacity = command->capacity ? command->capacity * 2 : 8;

        if (capacity < command->capacity)
            return -1;

        arguments = realloc(command->arguments,
                            capacity * sizeof(*arguments));

        if (!arguments)
            return -1;

        command->arguments = arguments;
        command->capacity = capacity;
    }

    command->arguments[command->count++] = (char *)argument;

    return 0;
}

int lo_linker_command_push_many(lo_linker_command *command,
                                const char *const *arguments,
                                size_t count)
{
    size_t i;

    if (!command || (!arguments && count != 0))
        return -1;

    for (i = 0; i < count; i++) {
        if (lo_linker_command_push(command, arguments[i]) != 0)
            return -1;
    }

    return 0;
}

/* Options */

void lo_linker_options_init(lo_linker_options *options)
{
    if (!options)
        return;

    options->kind = LO_LINKER_UNKNOWN;
    options->shared = 0;
    options->pie = 0;
    options->static_link = 0;
    options->strip = 0;
    options->output = NULL;
    options->sysroot = NULL;
}

/* Detection */

const char *lo_linker_name(lo_linker_kind kind)
{
    switch (kind) {
    case LO_LINKER_LD:
        return "GNU ld";

    case LO_LINKER_LLD:
        return "LLVM lld";

    case LO_LINKER_MOLD:
        return "mold";

    default:
        return "unknown";
    }
}

const char *lo_linker_program(lo_linker_kind kind)
{
    switch (kind) {
    case LO_LINKER_LD:
        return "ld";

    case LO_LINKER_LLD:
        return "ld.lld";

    case LO_LINKER_MOLD:
        return "mold";

    default:
        return NULL;
    }
}

lo_linker_kind lo_linker_detect(const char *program)
{
    const char *name;

    if (!program)
        return LO_LINKER_UNKNOWN;

    name = strrchr(program, '/');

    if (name)
        name++;
    else
        name = program;

    if (strcmp(name, "ld") == 0 ||
        strcmp(name, "ld.bfd") == 0 ||
        strcmp(name, "ld.gold") == 0)
        return LO_LINKER_LD;

    if (strcmp(name, "ld.lld") == 0 ||
        strcmp(name, "lld") == 0)
        return LO_LINKER_LLD;

    if (strcmp(name, "mold") == 0 ||
        strcmp(name, "ld.mold") == 0)
        return LO_LINKER_MOLD;

    return LO_LINKER_UNKNOWN;
}

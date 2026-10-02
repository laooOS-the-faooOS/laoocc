#ifndef LAOOCC_LINKER_COMMAND_H
#define LAOOCC_LINKER_COMMAND_H

#include <stddef.h>

typedef struct lo_linker_command {
    const char *program;
    char      **arguments;
    size_t      count;
    size_t      capacity;
} lo_linker_command;

void lo_linker_command_init(lo_linker_command *command);

void lo_linker_command_free(lo_linker_command *command);

int lo_linker_command_push(lo_linker_command *command,
                           const char *argument);

int lo_linker_command_push_many(lo_linker_command *command,
                                const char *const *arguments,
                                size_t count);

#endif /* LAOOCC_LINKER_COMMAND_H */

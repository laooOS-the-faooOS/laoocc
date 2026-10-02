#ifndef LAOOCC_ERROR_HANDLER_H
#define LAOOCC_ERROR_HANDLER_H

#include <stddef.h>

#include "error/error.h"

typedef struct lo_error_handler {
    lo_error *errors;

    size_t error_count;
    size_t error_capacity;

    size_t warning_count;
    size_t note_count;
} lo_error_handler;

void lo_error_handler_init(
    lo_error_handler *handler
);

void lo_error_handler_free(
    lo_error_handler *handler
);

int lo_error_handler_add(
    lo_error_handler *handler,
    const lo_error *error
);

int lo_error_handler_error(
    lo_error_handler *handler,
    int code,
    lo_diagnostic_kind kind,
    const char *message
);

int lo_error_handler_warning(
    lo_error_handler *handler,
    lo_diagnostic_kind kind,
    const char *message
);

int lo_error_handler_note(
    lo_error_handler *handler,
    lo_diagnostic_kind kind,
    const char *message
);

int lo_error_handler_failed(
    const lo_error_handler *handler
);

size_t lo_error_handler_count(
    const lo_error_handler *handler
);

const lo_error *lo_error_handler_get(
    const lo_error_handler *handler,
    size_t index
);

#endif /* LAOOCC_ERROR_HANDLER_H */

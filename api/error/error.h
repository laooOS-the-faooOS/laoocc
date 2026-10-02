#ifndef LAOOCC_ERROR_ERROR_H
#define LAOOCC_ERROR_ERROR_H

#include <stddef.h>

#include "diagnostic/diagnostic.h"

typedef struct lo_error {
    lo_diagnostic diagnostic;

    int code;
} lo_error;

void lo_error_init(lo_error *error);

void lo_error_set(
    lo_error *error,
    int code,
    lo_diagnostic_severity severity,
    lo_diagnostic_kind kind,
    const char *message
);

int lo_error_failed(const lo_error *error);

#endif /* LAOOCC_ERROR_ERROR_H */

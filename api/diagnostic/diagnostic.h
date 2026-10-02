#ifndef LAOOCC_DIAGNOSTIC_DIAGNOSTIC_H
#define LAOOCC_DIAGNOSTIC_DIAGNOSTIC_H

#include <stddef.h>

#include "diagnostic/kind.h"
#include "diagnostic/location.h"
#include "diagnostic/severity.h"

typedef struct lo_diagnostic {
    lo_diagnostic_severity severity;
    lo_diagnostic_kind kind;

    lo_diagnostic_location location;

    const char *message;
} lo_diagnostic;

void lo_diagnostic_init(lo_diagnostic *diagnostic);

void lo_diagnostic_set(
    lo_diagnostic *diagnostic,
    lo_diagnostic_severity severity,
    lo_diagnostic_kind kind,
    const char *message
);

void lo_diagnostic_set_location(
    lo_diagnostic *diagnostic,
    const lo_diagnostic_location *location
);

#endif /* LAOOCC_DIAGNOSTIC_DIAGNOSTIC_H */

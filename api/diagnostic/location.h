#ifndef LAOOCC_DIAGNOSTIC_LOCATION_H
#define LAOOCC_DIAGNOSTIC_LOCATION_H

#include <stddef.h>

typedef struct lo_diagnostic_location {
    const char *file;

    size_t line;
    size_t column;

    size_t offset;
} lo_diagnostic_location;

void lo_diagnostic_location_init(
    lo_diagnostic_location *location
);

#endif /* LAOOCC_DIAGNOSTIC_LOCATION_H */

#ifndef LAOOCC_SOURCE_LOCATION_H
#define LAOOCC_SOURCE_LOCATION_H

#include <stddef.h>

typedef struct lo_source_location {
    size_t offset;
    size_t line;
    size_t column;
} lo_source_location;

void lo_source_location_init(
    lo_source_location *location
);

#endif /* LAOOCC_SOURCE_LOCATION_H */

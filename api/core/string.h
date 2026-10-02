#ifndef LAOOCC_CORE_STRING_H
#define LAOOCC_CORE_STRING_H

#include <stddef.h>

typedef struct lo_string {
    const char *data;
    size_t length;
} lo_string;

#define LO_STRING_EMPTY \
    ((lo_string){ NULL, 0 })

#define LO_STRING_LITERAL(string) \
    ((lo_string){ (string), sizeof(string) - 1 })

size_t lo_string_length(lo_string string);
int    lo_string_empty(lo_string string);
int    lo_string_compare(lo_string lhs, lo_string rhs);
int    lo_string_equal(lo_string lhs, lo_string rhs);

#endif /* LAOOCC_CORE_STRING_H */

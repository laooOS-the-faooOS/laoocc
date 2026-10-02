#ifndef LAOOCC_MUSL_ALLOC_H
#define LAOOCC_MUSL_ALLOC_H

#include <stddef.h>

void *lo_musl_alloc(size_t size);
void *lo_musl_calloc(size_t count, size_t size);
void *lo_musl_realloc(void *ptr, size_t size);
void  lo_musl_free(void *ptr);

#endif /* LAOOCC_MUSL_ALLOC_H */

#ifndef LAOOCC_CORE_MEMORY_H
#define LAOOCC_CORE_MEMORY_H

#include <stddef.h>

void *lo_mem_alloc(size_t size);
void *lo_mem_calloc(size_t count, size_t size);
void *lo_mem_realloc(void *ptr, size_t size);
void  lo_mem_free(void *ptr);

void *lo_mem_copy(void *dest, const void *src, size_t size);
void *lo_mem_move(void *dest, const void *src, size_t size);
void *lo_mem_set(void *dest, int value, size_t size);
int   lo_mem_compare(const void *lhs, const void *rhs, size_t size);

#endif /* LAOOCC_CORE_MEMORY_H */

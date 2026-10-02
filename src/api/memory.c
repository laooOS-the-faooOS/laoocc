#include "core/memory.h"

#include <stdlib.h>
#include <string.h>

void *lo_mem_alloc(size_t size)
{
    return malloc(size);
}

void *lo_mem_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

void *lo_mem_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void lo_mem_free(void *ptr)
{
    free(ptr);
}

void *lo_mem_copy(void *dest, const void *src, size_t size)
{
    return memcpy(dest, src, size);
}

void *lo_mem_move(void *dest, const void *src, size_t size)
{
    return memmove(dest, src, size);
}

void *lo_mem_set(void *dest, int value, size_t size)
{
    return memset(dest, value, size);
}

int lo_mem_compare(const void *lhs, const void *rhs, size_t size)
{
    return memcmp(lhs, rhs, size);
}

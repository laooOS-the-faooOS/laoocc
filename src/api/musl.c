#define _POSIX_C_SOURCE 200809L
#include "musl/musl.h"

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

void *lo_musl_alloc(size_t size)
{
    return malloc(size);
}

void *lo_musl_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

void *lo_musl_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void lo_musl_free(void *ptr)
{
    free(ptr);
}

int lo_musl_write(int fd, const void *data, size_t size)
{
    ssize_t result;

    result = write(fd, data, size);

    if (result < 0)
        return -1;

    return (int)result;
}

int lo_musl_read(int fd, void *data, size_t size)
{
    ssize_t result;

    result = read(fd, data, size);

    if (result < 0)
        return -1;

    return (int)result;
}

int lo_musl_printf(const char *format, ...)
{
    va_list args;
    int result;

    va_start(args, format);
    result = vprintf(format, args);
    va_end(args);

    return result;
}

int lo_musl_process_id(void)
{
    return (int)getpid();
}

int lo_musl_parent_process_id(void)
{
    return (int)getppid();
}

int lo_musl_process_exit(int status)
{
    exit(status);

    return status;
}

const char *lo_musl_getenv(const char *name)
{
    return getenv(name);
}

int lo_musl_setenv(const char *name,
                   const char *value,
                   int overwrite)
{
    return setenv(name, value, overwrite);
}

int lo_musl_unsetenv(const char *name)
{
    return unsetenv(name);
}

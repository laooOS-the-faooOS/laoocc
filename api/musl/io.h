#ifndef LAOOCC_MUSL_IO_H
#define LAOOCC_MUSL_IO_H

#include <stddef.h>
#include <stdio.h>

int lo_musl_write(int fd, const void *data, size_t size);

int lo_musl_read(int fd, void *data, size_t size);

int lo_musl_printf(const char *format, ...);

#endif /* LAOOCC_MUSL_IO_H */

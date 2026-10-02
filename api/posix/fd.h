#ifndef LAOOCC_POSIX_FD_H
#define LAOOCC_POSIX_FD_H

#include <stddef.h>

int lo_posix_read(int fd, void *buffer, size_t size);

int lo_posix_write(int fd, const void *buffer, size_t size);

int lo_posix_close(int fd);

int lo_posix_dup(int fd);

int lo_posix_dup2(int old_fd, int new_fd);

#endif /* LAOOCC_POSIX_FD_H */

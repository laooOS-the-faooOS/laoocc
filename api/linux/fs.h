#ifndef LAOOCC_LINUX_FS_H
#define LAOOCC_LINUX_FS_H

#include <stddef.h>

int lo_linux_openat(int dirfd,
                    const char *path,
                    int flags,
                    int mode);

int lo_linux_close(int fd);

int lo_linux_read(int fd,
                  void *buffer,
                  size_t size);

int lo_linux_write(int fd,
                   const void *buffer,
                   size_t size);

#endif /* LAOOCC_LINUX_FS_H */

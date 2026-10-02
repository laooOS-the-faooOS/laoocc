#ifndef LAOOCC_POSIX_FILE_H
#define LAOOCC_POSIX_FILE_H

#include <stddef.h>
#include <sys/types.h>

int lo_posix_open(const char *path, int flags, int mode);

int lo_posix_unlink(const char *path);

int lo_posix_mkdir(const char *path, int mode);

int lo_posix_rmdir(const char *path);

int lo_posix_rename(const char *old_path,
                    const char *new_path);

off_t lo_posix_seek(int fd, off_t offset, int whence);

#endif /* LAOOCC_POSIX_FILE_H */

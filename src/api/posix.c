#define _POSIX_C_SOURCE 200809L

#include "posix/posix.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* File descriptors */

int lo_posix_read(int fd, void *buffer, size_t size)
{
    ssize_t result;

    result = read(fd, buffer, size);

    if (result < 0)
        return -1;

    return (int)result;
}

int lo_posix_write(int fd, const void *buffer, size_t size)
{
    ssize_t result;

    result = write(fd, buffer, size);

    if (result < 0)
        return -1;

    return (int)result;
}

int lo_posix_close(int fd)
{
    return close(fd);
}

int lo_posix_dup(int fd)
{
    return dup(fd);
}

int lo_posix_dup2(int old_fd, int new_fd)
{
    return dup2(old_fd, new_fd);
}

/* Files */

int lo_posix_open(const char *path, int flags, int mode)
{
    return open(path, flags, mode);
}

int lo_posix_unlink(const char *path)
{
    return unlink(path);
}

int lo_posix_mkdir(const char *path, int mode)
{
    return mkdir(path, (mode_t)mode);
}

int lo_posix_rmdir(const char *path)
{
    return rmdir(path);
}

int lo_posix_rename(const char *old_path,
                    const char *new_path)
{
    return rename(old_path, new_path);
}

off_t lo_posix_seek(int fd, off_t offset, int whence)
{
    return lseek(fd, offset, whence);
}

/* Process */

pid_t lo_posix_getpid(void)
{
    return getpid();
}

pid_t lo_posix_getppid(void)
{
    return getppid();
}

pid_t lo_posix_fork(void)
{
    return fork();
}

int lo_posix_exec(const char *path, char *const argv[])
{
    return execv(path, argv);
}

int lo_posix_wait(pid_t pid, int *status)
{
    return (int)waitpid(pid, status, 0);
}

void lo_posix_exit(int status)
{
    _exit(status);
}

/* Time */

time_t lo_posix_time(void)
{
    return time(NULL);
}

int lo_posix_sleep(unsigned int seconds)
{
    return (int)sleep(seconds);
}

int lo_posix_nanosleep(const struct timespec *request,
                       struct timespec *remaining)
{
    return nanosleep(request, remaining);
}

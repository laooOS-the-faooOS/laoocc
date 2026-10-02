#define _GNU_SOURCE

#include "linux/linux.h"

#include <sys/syscall.h>
#include <unistd.h>

/* Syscalls */

long lo_linux_syscall0(long number)
{
    return syscall(number);
}

long lo_linux_syscall1(long number, long arg1)
{
    return syscall(number, arg1);
}

long lo_linux_syscall2(long number, long arg1, long arg2)
{
    return syscall(number, arg1, arg2);
}

long lo_linux_syscall3(long number,
                       long arg1,
                       long arg2,
                       long arg3)
{
    return syscall(number, arg1, arg2, arg3);
}

long lo_linux_syscall4(long number,
                       long arg1,
                       long arg2,
                       long arg3,
                       long arg4)
{
    return syscall(number, arg1, arg2, arg3, arg4);
}

long lo_linux_syscall5(long number,
                       long arg1,
                       long arg2,
                       long arg3,
                       long arg4,
                       long arg5)
{
    return syscall(number, arg1, arg2, arg3, arg4, arg5);
}

long lo_linux_syscall6(long number,
                       long arg1,
                       long arg2,
                       long arg3,
                       long arg4,
                       long arg5,
                       long arg6)
{
    return syscall(number, arg1, arg2, arg3,
                   arg4, arg5, arg6);
}

/* Process */

pid_t lo_linux_getpid(void)
{
    return getpid();
}

pid_t lo_linux_gettid(void)
{
    return (pid_t)syscall(SYS_gettid);
}

int lo_linux_exit(int status)
{
    syscall(SYS_exit, status);

    return status;
}

/* Filesystem */

int lo_linux_openat(int dirfd,
                    const char *path,
                    int flags,
                    int mode)
{
    return (int)syscall(SYS_openat,
                        dirfd,
                        path,
                        flags,
                        mode);
}

int lo_linux_close(int fd)
{
    return (int)syscall(SYS_close, fd);
}

int lo_linux_read(int fd,
                  void *buffer,
                  size_t size)
{
    return (int)syscall(SYS_read,
                        fd,
                        buffer,
                        size);
}

int lo_linux_write(int fd,
                   const void *buffer,
                   size_t size)
{
    return (int)syscall(SYS_write,
                        fd,
                        buffer,
                        size);
}

/* System */

long lo_linux_getuid(void)
{
    return syscall(SYS_getuid);
}

long lo_linux_getgid(void)
{
    return syscall(SYS_getgid);
}

long lo_linux_geteuid(void)
{
    return syscall(SYS_geteuid);
}

long lo_linux_getegid(void)
{
    return syscall(SYS_getegid);
}

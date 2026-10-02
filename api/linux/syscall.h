#ifndef LAOOCC_LINUX_SYSCALL_H
#define LAOOCC_LINUX_SYSCALL_H

#include <stdint.h>

long lo_linux_syscall0(long number);
long lo_linux_syscall1(long number, long arg1);
long lo_linux_syscall2(long number, long arg1, long arg2);
long lo_linux_syscall3(long number, long arg1, long arg2, long arg3);
long lo_linux_syscall4(long number, long arg1, long arg2,
                       long arg3, long arg4);
long lo_linux_syscall5(long number, long arg1, long arg2,
                       long arg3, long arg4, long arg5);
long lo_linux_syscall6(long number, long arg1, long arg2,
                       long arg3, long arg4, long arg5, long arg6);

#endif /* LAOOCC_LINUX_SYSCALL_H */

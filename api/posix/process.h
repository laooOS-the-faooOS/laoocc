#ifndef LAOOCC_POSIX_PROCESS_H
#define LAOOCC_POSIX_PROCESS_H

#include <sys/types.h>

pid_t lo_posix_getpid(void);

pid_t lo_posix_getppid(void);

pid_t lo_posix_fork(void);

int lo_posix_exec(const char *path, char *const argv[]);

int lo_posix_wait(pid_t pid, int *status);

void lo_posix_exit(int status);

#endif /* LAOOCC_POSIX_PROCESS_H */

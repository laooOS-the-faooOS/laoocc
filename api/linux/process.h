#ifndef LAOOCC_LINUX_PROCESS_H
#define LAOOCC_LINUX_PROCESS_H

#include <sys/types.h>

pid_t lo_linux_getpid(void);
pid_t lo_linux_gettid(void);

int lo_linux_exit(int status);

#endif /* LAOOCC_LINUX_PROCESS_H */

#ifndef LAOOCC_MUSL_PROCESS_H
#define LAOOCC_MUSL_PROCESS_H

#include <sys/types.h>

int lo_musl_process_id(void);

int lo_musl_parent_process_id(void);

int lo_musl_process_exit(int status);

#endif /* LAOOCC_MUSL_PROCESS_H */

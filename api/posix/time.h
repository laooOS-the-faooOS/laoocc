#ifndef LAOOCC_POSIX_TIME_H
#define LAOOCC_POSIX_TIME_H

#include <time.h>

time_t lo_posix_time(void);

int lo_posix_sleep(unsigned int seconds);

int lo_posix_nanosleep(const struct timespec *request,
                       struct timespec *remaining);

#endif /* LAOOCC_POSIX_TIME_H */

#ifndef LAOOCC_MUSL_SYSTEM_H
#define LAOOCC_MUSL_SYSTEM_H

const char *lo_musl_getenv(const char *name);

int lo_musl_setenv(const char *name,
                   const char *value,
                   int overwrite);

int lo_musl_unsetenv(const char *name);

#endif /* LAOOCC_MUSL_SYSTEM_H */

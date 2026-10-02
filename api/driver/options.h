#ifndef LAOOCC_DRIVER_OPTIONS_H
#define LAOOCC_DRIVER_OPTIONS_H

#include <stddef.h>

typedef struct lo_driver_options {
    int optimize;
    int debug;
    int verbose;
    int compile_only;
    int preprocess_only;

    const char *target;
    const char *sysroot;
    const char *output;

    const char *const *arguments;
    size_t argument_count;
} lo_driver_options;

void lo_driver_options_init(lo_driver_options *options);

#endif /* LAOOCC_DRIVER_OPTIONS_H */

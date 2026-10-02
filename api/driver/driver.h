#ifndef LAOOCC_DRIVER_DRIVER_H
#define LAOOCC_DRIVER_DRIVER_H

#include "driver/input.h"
#include "driver/output.h"
#include "driver/options.h"

typedef struct lo_driver {
    lo_driver_options options;
} lo_driver;

void lo_driver_init(lo_driver *driver);

int lo_driver_run(lo_driver *driver,
                  const lo_driver_input *input,
                  const lo_driver_output *output);

#endif /* LAOOCC_DRIVER_DRIVER_H */

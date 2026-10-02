#include "driver/driver.h"

#include <string.h>

/* Input */

void lo_driver_input_init(lo_driver_input *input)
{
    if (!input)
        return;

    input->path = NULL;
    input->kind = LO_INPUT_UNKNOWN;
}

/* Output */

void lo_driver_output_init(lo_driver_output *output)
{
    if (!output)
        return;

    output->path = NULL;
    output->kind = LO_OUTPUT_EXECUTABLE;
}

/* Options */

void lo_driver_options_init(lo_driver_options *options)
{
    if (!options)
        return;

    options->optimize = 0;
    options->debug = 0;
    options->verbose = 0;
    options->compile_only = 0;
    options->preprocess_only = 0;

    options->target = NULL;
    options->sysroot = NULL;
    options->output = NULL;

    options->arguments = NULL;
    options->argument_count = 0;
}

/* Driver */

void lo_driver_init(lo_driver *driver)
{
    if (!driver)
        return;

    lo_driver_options_init(&driver->options);
}

int lo_driver_run(lo_driver *driver,
                  const lo_driver_input *input,
                  const lo_driver_output *output)
{
    if (!driver || !input || !output || !input->path)
        return -1;

    /*
     * Driver pipeline will be implemented as the frontend/backend
     * APIs are added.
     *
     * Current stage:
     *
     *   input
     *     ↓
     *   driver
     *     ↓
     *   output
     */

    (void)output;
    (void)strlen(input->path);

    return -1;
}

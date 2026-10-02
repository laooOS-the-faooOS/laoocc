#ifndef LAOOCC_DRIVER_OUTPUT_H
#define LAOOCC_DRIVER_OUTPUT_H

typedef enum lo_output_kind {
    LO_OUTPUT_EXECUTABLE = 0,
    LO_OUTPUT_OBJECT,
    LO_OUTPUT_ASSEMBLY,
    LO_OUTPUT_PREPROCESS
} lo_output_kind;

typedef struct lo_driver_output {
    const char       *path;
    lo_output_kind    kind;
} lo_driver_output;

void lo_driver_output_init(lo_driver_output *output);

#endif /* LAOOCC_DRIVER_OUTPUT_H */

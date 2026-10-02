#ifndef LAOOCC_DRIVER_INPUT_H
#define LAOOCC_DRIVER_INPUT_H

typedef enum lo_input_kind {
    LO_INPUT_UNKNOWN = 0,
    LO_INPUT_C,
    LO_INPUT_OBJECT,
    LO_INPUT_LIBRARY
} lo_input_kind;

typedef struct lo_driver_input {
    const char      *path;
    lo_input_kind    kind;
} lo_driver_input;

void lo_driver_input_init(lo_driver_input *input);

#endif /* LAOOCC_DRIVER_INPUT_H */

#ifndef LAOOCC_IR_VALUE_H
#define LAOOCC_IR_VALUE_H

#include <stddef.h>
#include <stdint.h>

typedef enum lo_ir_value_kind {
    LO_IR_VALUE_UNKNOWN = 0,
    LO_IR_VALUE_CONSTANT,
    LO_IR_VALUE_REGISTER,
    LO_IR_VALUE_PARAMETER,
    LO_IR_VALUE_GLOBAL
} lo_ir_value_kind;

typedef struct lo_ir_value {
    lo_ir_value_kind kind;

    unsigned type;
    uint64_t integer;
    double floating;

    const char *name;
} lo_ir_value;

void lo_ir_value_init(lo_ir_value *value,
                      lo_ir_value_kind kind);

#endif /* LAOOCC_IR_VALUE_H */

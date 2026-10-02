#ifndef LAOOCC_IR_FUNCTION_H
#define LAOOCC_IR_FUNCTION_H

#include <stddef.h>

#include "ir/block.h"

typedef struct lo_ir_function {
    const char *name;

    lo_ir_block *blocks;
    size_t block_count;
    size_t block_capacity;
} lo_ir_function;

void lo_ir_function_init(lo_ir_function *function,
                         const char *name);

void lo_ir_function_free(lo_ir_function *function);

int lo_ir_function_add_block(lo_ir_function *function,
                             const lo_ir_block *block);

#endif /* LAOOCC_IR_FUNCTION_H */

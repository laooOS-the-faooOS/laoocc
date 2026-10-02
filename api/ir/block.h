#ifndef LAOOCC_IR_BLOCK_H
#define LAOOCC_IR_BLOCK_H

#include <stddef.h>

#include "ir/instruction.h"

typedef struct lo_ir_block {
    const char *name;

    lo_ir_instruction *instructions;
    size_t instruction_count;
    size_t instruction_capacity;
} lo_ir_block;

void lo_ir_block_init(lo_ir_block *block,
                      const char *name);

void lo_ir_block_free(lo_ir_block *block);

int lo_ir_block_add_instruction(lo_ir_block *block,
                                const lo_ir_instruction *instruction);

#endif /* LAOOCC_IR_BLOCK_H */

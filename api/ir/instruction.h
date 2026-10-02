#ifndef LAOOCC_IR_INSTRUCTION_H
#define LAOOCC_IR_INSTRUCTION_H

#include <stddef.h>

#include "ir/value.h"

typedef enum lo_ir_opcode {
    LO_IR_OP_UNKNOWN = 0,

    LO_IR_OP_NOP,

    LO_IR_OP_ADD,
    LO_IR_OP_SUB,
    LO_IR_OP_MUL,
    LO_IR_OP_DIV,
    LO_IR_OP_MOD,

    LO_IR_OP_AND,
    LO_IR_OP_OR,
    LO_IR_OP_XOR,
    LO_IR_OP_SHL,
    LO_IR_OP_SHR,

    LO_IR_OP_LOAD,
    LO_IR_OP_STORE,

    LO_IR_OP_ALLOCA,

    LO_IR_OP_CALL,

    LO_IR_OP_RETURN,

    LO_IR_OP_BRANCH,
    LO_IR_OP_CONDITIONAL_BRANCH,

    LO_IR_OP_COMPARE,

    LO_IR_OP_CAST
} lo_ir_opcode;

typedef struct lo_ir_instruction {
    lo_ir_opcode opcode;

    lo_ir_value result;

    lo_ir_value *operands;
    size_t operand_count;
    size_t operand_capacity;
} lo_ir_instruction;

void lo_ir_instruction_init(lo_ir_instruction *instruction,
                            lo_ir_opcode opcode);

void lo_ir_instruction_free(lo_ir_instruction *instruction);

int lo_ir_instruction_add_operand(lo_ir_instruction *instruction,
                                  const lo_ir_value *operand);

#endif /* LAOOCC_IR_INSTRUCTION_H */

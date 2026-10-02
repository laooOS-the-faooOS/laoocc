#include "ir/ir.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* IR Value */

void lo_ir_value_init(lo_ir_value *value,
                      lo_ir_value_kind kind)
{
    if (!value)
        return;

    value->kind = kind;
    value->type = 0;
    value->integer = 0;
    value->floating = 0.0;
    value->name = NULL;
}

/* IR Instruction */

void lo_ir_instruction_init(lo_ir_instruction *instruction,
                            lo_ir_opcode opcode)
{
    if (!instruction)
        return;

    instruction->opcode = opcode;
    lo_ir_value_init(&instruction->result, LO_IR_VALUE_UNKNOWN);

    instruction->operands = NULL;
    instruction->operand_count = 0;
    instruction->operand_capacity = 0;
}

void lo_ir_instruction_free(lo_ir_instruction *instruction)
{
    if (!instruction)
        return;

    free(instruction->operands);

    instruction->operands = NULL;
    instruction->operand_count = 0;
    instruction->operand_capacity = 0;
}

int lo_ir_instruction_add_operand(lo_ir_instruction *instruction,
                                  const lo_ir_value *operand)
{
    lo_ir_value *operands;
    size_t capacity;

    if (!instruction || !operand)
        return -1;

    if (instruction->operand_count == instruction->operand_capacity) {
        capacity = instruction->operand_capacity
                 ? instruction->operand_capacity * 2
                 : 4;

        if (capacity < instruction->operand_capacity)
            return -1;

        operands = realloc(instruction->operands,
                           capacity * sizeof(*operands));

        if (!operands)
            return -1;

        instruction->operands = operands;
        instruction->operand_capacity = capacity;
    }

    instruction->operands[instruction->operand_count++] = *operand;

    return 0;
}

/* IR Block */

void lo_ir_block_init(lo_ir_block *block,
                      const char *name)
{
    if (!block)
        return;

    block->name = name;

    block->instructions = NULL;
    block->instruction_count = 0;
    block->instruction_capacity = 0;
}

void lo_ir_block_free(lo_ir_block *block)
{
    size_t i;

    if (!block)
        return;

    for (i = 0; i < block->instruction_count; i++)
        lo_ir_instruction_free(&block->instructions[i]);

    free(block->instructions);

    block->instructions = NULL;
    block->instruction_count = 0;
    block->instruction_capacity = 0;
}

int lo_ir_block_add_instruction(lo_ir_block *block,
                                const lo_ir_instruction *instruction)
{
    lo_ir_instruction *instructions;
    size_t capacity;

    if (!block || !instruction)
        return -1;

    if (block->instruction_count == block->instruction_capacity) {
        capacity = block->instruction_capacity
                 ? block->instruction_capacity * 2
                 : 4;

        if (capacity < block->instruction_capacity)
            return -1;

        instructions = realloc(block->instructions,
                               capacity * sizeof(*instructions));

        if (!instructions)
            return -1;

        block->instructions = instructions;
        block->instruction_capacity = capacity;
    }

    block->instructions[block->instruction_count] = *instruction;

    /*
     * Transfer ownership of the operand array to the block.
     * The caller must not free the instruction afterward.
     */
    block->instructions[block->instruction_count].operands =
        instruction->operands;

    block->instructions[block->instruction_count].operand_count =
        instruction->operand_count;

    block->instructions[block->instruction_count].operand_capacity =
        instruction->operand_capacity;

    block->instruction_count++;

    return 0;
}

/* IR Function */

void lo_ir_function_init(lo_ir_function *function,
                         const char *name)
{
    if (!function)
        return;

    function->name = name;

    function->blocks = NULL;
    function->block_count = 0;
    function->block_capacity = 0;
}

void lo_ir_function_free(lo_ir_function *function)
{
    size_t i;

    if (!function)
        return;

    for (i = 0; i < function->block_count; i++)
        lo_ir_block_free(&function->blocks[i]);

    free(function->blocks);

    function->blocks = NULL;
    function->block_count = 0;
    function->block_capacity = 0;
}

int lo_ir_function_add_block(lo_ir_function *function,
                             const lo_ir_block *block)
{
    lo_ir_block *blocks;
    size_t capacity;

    if (!function || !block)
        return -1;

    if (function->block_count == function->block_capacity) {
        capacity = function->block_capacity
                 ? function->block_capacity * 2
                 : 4;

        if (capacity < function->block_capacity)
            return -1;

        blocks = realloc(function->blocks,
                          capacity * sizeof(*blocks));

        if (!blocks)
            return -1;

        function->blocks = blocks;
        function->block_capacity = capacity;
    }

    function->blocks[function->block_count] = *block;

    /*
     * Transfer ownership of the block's instruction array
     * to the function.
     */
    function->blocks[function->block_count].instructions =
        block->instructions;

    function->blocks[function->block_count].instruction_count =
        block->instruction_count;

    function->blocks[function->block_count].instruction_capacity =
        block->instruction_capacity;

    function->block_count++;

    return 0;
}

/* IR Module */

void lo_ir_module_init(lo_ir_module *module,
                       const char *name)
{
    if (!module)
        return;

    module->name = name;

    module->functions = NULL;
    module->function_count = 0;
    module->function_capacity = 0;
}

void lo_ir_module_free(lo_ir_module *module)
{
    size_t i;

    if (!module)
        return;

    for (i = 0; i < module->function_count; i++)
        lo_ir_function_free(&module->functions[i]);

    free(module->functions);

    module->functions = NULL;
    module->function_count = 0;
    module->function_capacity = 0;
}

int lo_ir_module_add_function(lo_ir_module *module,
                              const lo_ir_function *function)
{
    lo_ir_function *functions;
    size_t capacity;

    if (!module || !function)
        return -1;

    if (module->function_count == module->function_capacity) {
        capacity = module->function_capacity
                 ? module->function_capacity * 2
                 : 4;

        if (capacity < module->function_capacity)
            return -1;

        functions = realloc(module->functions,
                             capacity * sizeof(*functions));

        if (!functions)
            return -1;

        module->functions = functions;
        module->function_capacity = capacity;
    }

    module->functions[module->function_count] = *function;

    /*
     * Transfer ownership of the function's block array
     * to the module.
     */
    module->functions[module->function_count].blocks =
        function->blocks;

    module->functions[module->function_count].block_count =
        function->block_count;

    module->functions[module->function_count].block_capacity =
        function->block_capacity;

    module->function_count++;

    return 0;
}

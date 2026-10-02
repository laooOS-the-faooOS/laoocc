#ifndef LAOOCC_CODEGEN_CONTEXT_H
#define LAOOCC_CODEGEN_CONTEXT_H

#include "ir/ir.h"
#include "target/target.h"

typedef struct lo_codegen_context {
    const lo_ir_module *module;
    const lo_target    *target;

    void *output;
} lo_codegen_context;

void lo_codegen_context_init(lo_codegen_context *context);

#endif /* LAOOCC_CODEGEN_CONTEXT_H */

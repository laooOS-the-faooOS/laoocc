#ifndef LAOOCC_IR_MODULE_H
#define LAOOCC_IR_MODULE_H

#include <stddef.h>

#include "ir/function.h"

typedef struct lo_ir_module {
    const char *name;

    lo_ir_function *functions;
    size_t function_count;
    size_t function_capacity;
} lo_ir_module;

void lo_ir_module_init(lo_ir_module *module,
                       const char *name);

void lo_ir_module_free(lo_ir_module *module);

int lo_ir_module_add_function(lo_ir_module *module,
                              const lo_ir_function *function);

#endif /* LAOOCC_IR_MODULE_H */

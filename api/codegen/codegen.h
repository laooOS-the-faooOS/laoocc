#ifndef LAOOCC_CODEGEN_CODEGEN_H
#define LAOOCC_CODEGEN_CODEGEN_H

#include "codegen/context.h"
#include "codegen/emitter.h"
#include "codegen/options.h"

typedef struct lo_codegen {
    lo_codegen_context context;
    lo_codegen_options options;
    lo_codegen_emitter emitter;
} lo_codegen;

void lo_codegen_init(lo_codegen *codegen);

int lo_codegen_generate(lo_codegen *codegen);

const char *lo_codegen_data(const lo_codegen *codegen);
size_t lo_codegen_size(const lo_codegen *codegen);

#endif /* LAOOCC_CODEGEN_CODEGEN_H */

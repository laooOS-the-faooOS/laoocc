#ifndef LAOOCC_CODEGEN_EMITTER_H
#define LAOOCC_CODEGEN_EMITTER_H

#include <stddef.h>

typedef struct lo_codegen_emitter {
    char   *data;
    size_t  length;
    size_t  capacity;
} lo_codegen_emitter;

void lo_codegen_emitter_init(lo_codegen_emitter *emitter);
void lo_codegen_emitter_free(lo_codegen_emitter *emitter);

int lo_codegen_emitter_write(lo_codegen_emitter *emitter,
                             const char *data,
                             size_t length);

int lo_codegen_emitter_puts(lo_codegen_emitter *emitter,
                            const char *string);

int lo_codegen_emitter_putc(lo_codegen_emitter *emitter,
                            char character);

void lo_codegen_emitter_clear(lo_codegen_emitter *emitter);

#endif /* LAOOCC_CODEGEN_EMITTER_H */

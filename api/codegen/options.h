#ifndef LAOOCC_CODEGEN_OPTIONS_H
#define LAOOCC_CODEGEN_OPTIONS_H

typedef enum lo_codegen_output_kind {
    LO_CODEGEN_OUTPUT_ASSEMBLY = 0,
    LO_CODEGEN_OUTPUT_OBJECT
} lo_codegen_output_kind;

typedef struct lo_codegen_options {
    lo_codegen_output_kind output_kind;

    int optimize;
    int debug;
    int verbose;

    const char *output;
} lo_codegen_options;

void lo_codegen_options_init(lo_codegen_options *options);

#endif /* LAOOCC_CODEGEN_OPTIONS_H */

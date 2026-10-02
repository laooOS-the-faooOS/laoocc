#ifndef LAOOCC_LINKER_OPTIONS_H
#define LAOOCC_LINKER_OPTIONS_H

typedef enum lo_linker_kind {
    LO_LINKER_UNKNOWN = 0,
    LO_LINKER_LD,
    LO_LINKER_LLD,
    LO_LINKER_MOLD
} lo_linker_kind;

typedef struct lo_linker_options {
    lo_linker_kind kind;

    int shared;
    int pie;
    int static_link;
    int strip;

    const char *output;
    const char *sysroot;
} lo_linker_options;

void lo_linker_options_init(lo_linker_options *options);

#endif /* LAOOCC_LINKER_OPTIONS_H */

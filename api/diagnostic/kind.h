#ifndef LAOOCC_DIAGNOSTIC_KIND_H
#define LAOOCC_DIAGNOSTIC_KIND_H

typedef enum lo_diagnostic_kind {
    LO_DIAGNOSTIC_UNKNOWN = 0,

    LO_DIAGNOSTIC_LEXER,
    LO_DIAGNOSTIC_PARSER,
    LO_DIAGNOSTIC_SEMA,
    LO_DIAGNOSTIC_CODEGEN,
    LO_DIAGNOSTIC_DRIVER,

    LO_DIAGNOSTIC_INTERNAL,
    LO_DIAGNOSTIC_SYSTEM
} lo_diagnostic_kind;

const char *lo_diagnostic_kind_name(
    lo_diagnostic_kind kind
);

#endif /* LAOOCC_DIAGNOSTIC_KIND_H */

#ifndef LAOOCC_SEMA_DIAGNOSTIC_H
#define LAOOCC_SEMA_DIAGNOSTIC_H

typedef enum lo_sema_diagnostic_kind {
    LO_SEMA_DIAGNOSTIC_NONE = 0,
    LO_SEMA_DIAGNOSTIC_ERROR,
    LO_SEMA_DIAGNOSTIC_WARNING
} lo_sema_diagnostic_kind;

typedef struct lo_sema_diagnostic {
    lo_sema_diagnostic_kind kind;

    const char *message;

    unsigned line;
    unsigned column;
} lo_sema_diagnostic;

void lo_sema_diagnostic_init(
    lo_sema_diagnostic *diagnostic);

#endif /* LAOOCC_SEMA_DIAGNOSTIC_H */

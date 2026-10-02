#ifndef LAOOCC_DIAGNOSTIC_SEVERITY_H
#define LAOOCC_DIAGNOSTIC_SEVERITY_H

typedef enum lo_diagnostic_severity {
    LO_DIAGNOSTIC_NOTE = 0,
    LO_DIAGNOSTIC_WARNING,
    LO_DIAGNOSTIC_ERROR,
    LO_DIAGNOSTIC_FATAL
} lo_diagnostic_severity;

const char *lo_diagnostic_severity_name(
    lo_diagnostic_severity severity
);

#endif /* LAOOCC_DIAGNOSTIC_SEVERITY_H */

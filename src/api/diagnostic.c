#include "diagnostic/diagnostic_api.h"

const char *lo_diagnostic_severity_name(
    lo_diagnostic_severity severity)
{
    switch (severity) {
    case LO_DIAGNOSTIC_NOTE:
        return "note";

    case LO_DIAGNOSTIC_WARNING:
        return "warning";

    case LO_DIAGNOSTIC_ERROR:
        return "error";

    case LO_DIAGNOSTIC_FATAL:
        return "fatal";

    default:
        return "unknown";
    }
}

const char *lo_diagnostic_kind_name(
    lo_diagnostic_kind kind)
{
    switch (kind) {
    case LO_DIAGNOSTIC_LEXER:
        return "lexer";

    case LO_DIAGNOSTIC_PARSER:
        return "parser";

    case LO_DIAGNOSTIC_SEMA:
        return "sema";

    case LO_DIAGNOSTIC_CODEGEN:
        return "codegen";

    case LO_DIAGNOSTIC_DRIVER:
        return "driver";

    case LO_DIAGNOSTIC_INTERNAL:
        return "internal";

    case LO_DIAGNOSTIC_SYSTEM:
        return "system";

    default:
        return "unknown";
    }
}

void lo_diagnostic_location_init(
    lo_diagnostic_location *location)
{
    if (!location)
        return;

    location->file = NULL;
    location->line = 0;
    location->column = 0;
    location->offset = 0;
}

void lo_diagnostic_init(lo_diagnostic *diagnostic)
{
    if (!diagnostic)
        return;

    diagnostic->severity = LO_DIAGNOSTIC_NOTE;
    diagnostic->kind = LO_DIAGNOSTIC_UNKNOWN;
    diagnostic->message = NULL;

    lo_diagnostic_location_init(
        &diagnostic->location
    );
}

void lo_diagnostic_set(
    lo_diagnostic *diagnostic,
    lo_diagnostic_severity severity,
    lo_diagnostic_kind kind,
    const char *message)
{
    if (!diagnostic)
        return;

    diagnostic->severity = severity;
    diagnostic->kind = kind;
    diagnostic->message = message;
}

void lo_diagnostic_set_location(
    lo_diagnostic *diagnostic,
    const lo_diagnostic_location *location)
{
    if (!diagnostic || !location)
        return;

    diagnostic->location = *location;
}

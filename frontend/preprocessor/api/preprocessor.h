#ifndef LAOOCC_PREPROCESSOR_H
#define LAOOCC_PREPROCESSOR_H

#include <stddef.h>

#include "macro.h"
#include "conditional.h"
#include "directive.h"

typedef enum lo_pp_error_kind {
    LO_PP_ERROR_NONE = 0,
    LO_PP_ERROR_INVALID_DIRECTIVE,
    LO_PP_ERROR_INVALID_MACRO,
    LO_PP_ERROR_UNDEFINED_MACRO,
    LO_PP_ERROR_UNTERMINATED_CONDITIONAL,
    LO_PP_ERROR_UNEXPECTED_ELSE,
    LO_PP_ERROR_UNEXPECTED_ENDIF,
    LO_PP_ERROR_INCLUDE_NOT_FOUND
} lo_pp_error_kind;

typedef struct lo_pp_error {
    lo_pp_error_kind kind;
    const char *message;
    size_t line;
    size_t column;
} lo_pp_error;

typedef struct lo_preprocessor {
    const char *source;
    size_t length;

    size_t offset;
    size_t line;
    size_t column;

    lo_pp_macro *macros;
    size_t macro_count;
    size_t macro_capacity;

    lo_pp_conditional *conditionals;
    size_t conditional_count;
    size_t conditional_capacity;
} lo_preprocessor;

void lo_preprocessor_init(
    lo_preprocessor *preprocessor,
    const char *source,
    size_t length
);

int lo_preprocessor_run(
    lo_preprocessor *preprocessor,
    char **output,
    size_t *output_length,
    lo_pp_error *error
);

void lo_preprocessor_free(
    lo_preprocessor *preprocessor
);

const char *lo_pp_error_name(lo_pp_error_kind kind);

#endif

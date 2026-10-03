#ifndef LAOOCC_PREPROCESSOR_DIRECTIVE_H
#define LAOOCC_PREPROCESSOR_DIRECTIVE_H

typedef enum lo_pp_directive_kind {
    LO_PP_DIRECTIVE_UNKNOWN = 0,
    LO_PP_DIRECTIVE_DEFINE,
    LO_PP_DIRECTIVE_UNDEF,
    LO_PP_DIRECTIVE_INCLUDE,
    LO_PP_DIRECTIVE_IF,
    LO_PP_DIRECTIVE_IFDEF,
    LO_PP_DIRECTIVE_IFNDEF,
    LO_PP_DIRECTIVE_ELSE,
    LO_PP_DIRECTIVE_ENDIF
} lo_pp_directive_kind;

const char *lo_pp_directive_name(lo_pp_directive_kind kind);

#endif

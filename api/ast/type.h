#ifndef LAOOCC_AST_TYPE_H
#define LAOOCC_AST_TYPE_H

typedef enum lo_ast_type_kind {
    LO_AST_TYPE_UNKNOWN = 0,
    LO_AST_TYPE_VOID,
    LO_AST_TYPE_BOOL,
    LO_AST_TYPE_CHAR,
    LO_AST_TYPE_INT,
    LO_AST_TYPE_FLOAT,
    LO_AST_TYPE_DOUBLE,
    LO_AST_TYPE_POINTER,
    LO_AST_TYPE_ARRAY,
    LO_AST_TYPE_FUNCTION
} lo_ast_type_kind;

typedef struct lo_ast_type {
    lo_ast_type_kind kind;
    unsigned bits;
    unsigned is_signed;
} lo_ast_type;

void lo_ast_type_init(lo_ast_type *type,
                      lo_ast_type_kind kind);

#endif /* LAOOCC_AST_TYPE_H */

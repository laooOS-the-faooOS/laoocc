#ifndef LAOOCC_AST_TYPE_H
#define LAOOCC_AST_TYPE_H

#include <stddef.h>

typedef enum lo_ast_type_kind {
    LO_AST_TYPE_UNKNOWN = 0,
    LO_AST_TYPE_VOID,
    LO_AST_TYPE_BOOL,
    LO_AST_TYPE_CHAR,
    LO_AST_TYPE_SHORT,
    LO_AST_TYPE_INT,
    LO_AST_TYPE_LONG,
    LO_AST_TYPE_LONG_LONG,
    LO_AST_TYPE_FLOAT,
    LO_AST_TYPE_DOUBLE,
    LO_AST_TYPE_POINTER,
    LO_AST_TYPE_ARRAY,
    LO_AST_TYPE_FUNCTION,
    LO_AST_TYPE_STRUCT,
    LO_AST_TYPE_UNION,
    LO_AST_TYPE_ENUM,
    LO_AST_TYPE_TYPEDEF_NAME
} lo_ast_type_kind;

typedef struct lo_ast_type {
    lo_ast_type_kind kind;
    unsigned bits;
    unsigned is_signed;
    unsigned is_const;
    unsigned is_volatile;
    unsigned is_restrict;
    char *tag_or_typedef_name;
    struct lo_ast_type *subtype;
    size_t array_size;
    int has_array_size;
} lo_ast_type;

void lo_ast_type_init(lo_ast_type *type,
                      lo_ast_type_kind kind);

lo_ast_type *lo_ast_type_new(lo_ast_type_kind kind);

lo_ast_type *lo_ast_type_pointer_to(const lo_ast_type *base);

lo_ast_type *lo_ast_type_array_of(const lo_ast_type *element,
                                  size_t size,
                                  int has_size);

lo_ast_type *lo_ast_type_clone(const lo_ast_type *type);

void lo_ast_type_free_members(lo_ast_type *type);

void lo_ast_type_free(lo_ast_type *type);

const char *lo_ast_type_kind_name(lo_ast_type_kind kind);

#endif /* LAOOCC_AST_TYPE_H */

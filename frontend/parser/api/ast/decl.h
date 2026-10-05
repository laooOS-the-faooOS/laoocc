#ifndef LAOOCC_AST_DECL_H
#define LAOOCC_AST_DECL_H

#include "ast/node.h"
#include "ast/type.h"

typedef struct lo_ast_decl {
    lo_ast_node node;
    char *name;
    lo_ast_type type;
    unsigned is_const;
    unsigned is_static;
    unsigned is_extern;
    unsigned is_inline;
    unsigned is_typedef;
    unsigned is_auto;
    unsigned is_register;
    int is_function_definition;
} lo_ast_decl;

lo_ast_decl *lo_ast_decl_new(lo_ast_kind kind);

lo_ast_decl *lo_ast_var_decl_new(const char *name,
                                 size_t name_len,
                                 const lo_ast_type *type,
                                 lo_ast_node *init);

lo_ast_decl *lo_ast_func_decl_new(const char *name,
                                  size_t name_len,
                                  const lo_ast_type *return_type,
                                  int is_definition);

lo_ast_decl *lo_ast_param_decl_new(const char *name,
                                   size_t name_len,
                                   const lo_ast_type *type);

lo_ast_decl *lo_ast_struct_decl_new(const char *tag,
                                    size_t tag_len,
                                    int is_union,
                                    int is_definition);

lo_ast_decl *lo_ast_enum_decl_new(const char *tag,
                                  size_t tag_len,
                                  int is_definition);

lo_ast_decl *lo_ast_enumerator_new(const char *name,
                                   size_t name_len,
                                   lo_ast_node *value_expr);

#endif /* LAOOCC_AST_DECL_H */

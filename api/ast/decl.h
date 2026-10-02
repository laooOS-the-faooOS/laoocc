#ifndef LAOOCC_AST_DECL_H
#define LAOOCC_AST_DECL_H

#include "ast/node.h"
#include "ast/type.h"

typedef struct lo_ast_decl {
    lo_ast_node node;

    const char *name;
    lo_ast_type type;

    unsigned is_const;
    unsigned is_static;
    unsigned is_extern;
} lo_ast_decl;

#endif /* LAOOCC_AST_DECL_H */

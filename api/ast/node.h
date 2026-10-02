#ifndef LAOOCC_AST_NODE_H
#define LAOOCC_AST_NODE_H

#include <stddef.h>

typedef enum lo_ast_kind {
    LO_AST_UNKNOWN = 0,

    /* Translation unit */
    LO_AST_TRANSLATION_UNIT,

    /* Declarations */
    LO_AST_DECLARATION,
    LO_AST_FUNCTION_DECL,
    LO_AST_VARIABLE_DECL,
    LO_AST_PARAMETER_DECL,

    /* Statements */
    LO_AST_COMPOUND_STMT,
    LO_AST_EXPRESSION_STMT,
    LO_AST_IF_STMT,
    LO_AST_WHILE_STMT,
    LO_AST_FOR_STMT,
    LO_AST_RETURN_STMT,
    LO_AST_BREAK_STMT,
    LO_AST_CONTINUE_STMT,

    /* Expressions */
    LO_AST_LITERAL,
    LO_AST_IDENTIFIER,
    LO_AST_BINARY_EXPR,
    LO_AST_UNARY_EXPR,
    LO_AST_CALL_EXPR,
    LO_AST_ASSIGN_EXPR,
    LO_AST_CONDITIONAL_EXPR,

    /* Types */
    LO_AST_TYPE
} lo_ast_kind;

typedef struct lo_ast_node {
    lo_ast_kind kind;

    unsigned line;
    unsigned column;

    struct lo_ast_node *parent;

    struct lo_ast_node **children;
    size_t child_count;
    size_t child_capacity;
} lo_ast_node;

void lo_ast_node_init(lo_ast_node *node,
                      lo_ast_kind kind);

void lo_ast_node_free(lo_ast_node *node);

int lo_ast_node_add_child(lo_ast_node *parent,
                          lo_ast_node *child);

#endif /* LAOOCC_AST_NODE_H */

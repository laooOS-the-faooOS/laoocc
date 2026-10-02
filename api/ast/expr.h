#ifndef LAOOCC_AST_EXPR_H
#define LAOOCC_AST_EXPR_H

#include "ast/node.h"

typedef enum lo_ast_binary_op {
    LO_AST_BIN_UNKNOWN = 0,

    LO_AST_BIN_ADD,
    LO_AST_BIN_SUB,
    LO_AST_BIN_MUL,
    LO_AST_BIN_DIV,
    LO_AST_BIN_MOD,

    LO_AST_BIN_SHL,
    LO_AST_BIN_SHR,

    LO_AST_BIN_LT,
    LO_AST_BIN_LE,
    LO_AST_BIN_GT,
    LO_AST_BIN_GE,

    LO_AST_BIN_EQ,
    LO_AST_BIN_NE,

    LO_AST_BIN_BIT_AND,
    LO_AST_BIN_BIT_OR,
    LO_AST_BIN_BIT_XOR,

    LO_AST_BIN_LOGICAL_AND,
    LO_AST_BIN_LOGICAL_OR
} lo_ast_binary_op;

typedef enum lo_ast_unary_op {
    LO_AST_UNARY_UNKNOWN = 0,

    LO_AST_UNARY_PLUS,
    LO_AST_UNARY_MINUS,
    LO_AST_UNARY_NOT,
    LO_AST_UNARY_BIT_NOT,
    LO_AST_UNARY_ADDRESS,
    LO_AST_UNARY_DEREFERENCE
} lo_ast_unary_op;

typedef struct lo_ast_expr {
    lo_ast_node node;
} lo_ast_expr;

#endif /* LAOOCC_AST_EXPR_H */

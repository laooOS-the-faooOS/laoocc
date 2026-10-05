#ifndef LAOOCC_AST_EXPR_H
#define LAOOCC_AST_EXPR_H

#include "ast/node.h"
#include "ast/type.h"

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
    LO_AST_BIN_LOGICAL_OR,
    LO_AST_BIN_COMMA
} lo_ast_binary_op;

typedef enum lo_ast_assign_op {
    LO_AST_ASSIGN_UNKNOWN = 0,
    LO_AST_ASSIGN_EQUAL,
    LO_AST_ASSIGN_ADD,
    LO_AST_ASSIGN_SUB,
    LO_AST_ASSIGN_MUL,
    LO_AST_ASSIGN_DIV,
    LO_AST_ASSIGN_MOD,
    LO_AST_ASSIGN_SHL,
    LO_AST_ASSIGN_SHR,
    LO_AST_ASSIGN_AND,
    LO_AST_ASSIGN_XOR,
    LO_AST_ASSIGN_OR
} lo_ast_assign_op;

typedef enum lo_ast_unary_op {
    LO_AST_UNARY_UNKNOWN = 0,
    LO_AST_UNARY_PLUS,
    LO_AST_UNARY_MINUS,
    LO_AST_UNARY_NOT,
    LO_AST_UNARY_BIT_NOT,
    LO_AST_UNARY_ADDRESS,
    LO_AST_UNARY_DEREFERENCE,
    LO_AST_UNARY_PRE_INC,
    LO_AST_UNARY_PRE_DEC,
    LO_AST_UNARY_POST_INC,
    LO_AST_UNARY_POST_DEC,
    LO_AST_UNARY_SIZEOF
} lo_ast_unary_op;

typedef enum lo_ast_literal_kind {
    LO_AST_LITERAL_UNKNOWN = 0,
    LO_AST_LITERAL_NUMBER,
    LO_AST_LITERAL_CHARACTER,
    LO_AST_LITERAL_STRING
} lo_ast_literal_kind;

typedef struct lo_ast_expr {
    lo_ast_node node;
    lo_ast_binary_op binary_op;
    lo_ast_assign_op assign_op;
    lo_ast_unary_op unary_op;
    lo_ast_literal_kind literal_kind;
    char *string_value;
    size_t string_length;
    lo_ast_type cast_type;
    int is_arrow;
} lo_ast_expr;

lo_ast_expr *lo_ast_expr_new(lo_ast_kind kind);

lo_ast_expr *lo_ast_literal_new(lo_ast_literal_kind lkind,
                                const char *text,
                                size_t len);

lo_ast_expr *lo_ast_identifier_new(const char *name,
                                   size_t len);

lo_ast_expr *lo_ast_binary_new(lo_ast_binary_op op,
                               lo_ast_node *left,
                               lo_ast_node *right);

lo_ast_expr *lo_ast_assign_new(lo_ast_assign_op op,
                               lo_ast_node *target,
                               lo_ast_node *value);

lo_ast_expr *lo_ast_unary_new(lo_ast_unary_op op,
                              lo_ast_node *operand);

lo_ast_expr *lo_ast_call_new(lo_ast_node *callee);

lo_ast_expr *lo_ast_subscript_new(lo_ast_node *array_expr,
                                  lo_ast_node *index_expr);

lo_ast_expr *lo_ast_member_new(lo_ast_node *obj_expr,
                               const char *member,
                               size_t member_len,
                               int is_arrow);

lo_ast_expr *lo_ast_conditional_new(lo_ast_node *cond,
                                    lo_ast_node *true_expr,
                                    lo_ast_node *false_expr);

lo_ast_expr *lo_ast_cast_new(const lo_ast_type *type,
                             lo_ast_node *operand);

const char *lo_ast_binary_op_name(lo_ast_binary_op op);
const char *lo_ast_assign_op_name(lo_ast_assign_op op);
const char *lo_ast_unary_op_name(lo_ast_unary_op op);

#endif /* LAOOCC_AST_EXPR_H */

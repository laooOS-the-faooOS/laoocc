#ifndef LAOOCC_AST_STMT_H
#define LAOOCC_AST_STMT_H

#include "ast/node.h"

typedef struct lo_ast_stmt {
    lo_ast_node node;
    char *label_name;
} lo_ast_stmt;

lo_ast_stmt *lo_ast_stmt_new(lo_ast_kind kind);

lo_ast_stmt *lo_ast_compound_stmt_new(void);

lo_ast_stmt *lo_ast_expr_stmt_new(lo_ast_node *expr);

lo_ast_stmt *lo_ast_if_stmt_new(lo_ast_node *cond,
                                lo_ast_node *then_b,
                                lo_ast_node *else_b);

lo_ast_stmt *lo_ast_while_stmt_new(lo_ast_node *cond,
                                   lo_ast_node *body);

lo_ast_stmt *lo_ast_do_while_stmt_new(lo_ast_node *body,
                                      lo_ast_node *cond);

lo_ast_stmt *lo_ast_for_stmt_new(lo_ast_node *init,
                                 lo_ast_node *cond,
                                 lo_ast_node *step,
                                 lo_ast_node *body);

lo_ast_stmt *lo_ast_return_stmt_new(lo_ast_node *expr);

lo_ast_stmt *lo_ast_break_stmt_new(void);

lo_ast_stmt *lo_ast_continue_stmt_new(void);

lo_ast_stmt *lo_ast_switch_stmt_new(lo_ast_node *expr,
                                    lo_ast_node *body);

lo_ast_stmt *lo_ast_case_stmt_new(lo_ast_node *expr,
                                  lo_ast_node *body);

lo_ast_stmt *lo_ast_default_stmt_new(lo_ast_node *body);

lo_ast_stmt *lo_ast_goto_stmt_new(const char *label,
                                  size_t label_len);

lo_ast_stmt *lo_ast_label_stmt_new(const char *label,
                                   size_t label_len,
                                   lo_ast_node *stmt);

#endif /* LAOOCC_AST_STMT_H */

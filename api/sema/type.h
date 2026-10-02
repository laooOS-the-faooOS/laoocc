#ifndef LAOOCC_SEMA_TYPE_H
#define LAOOCC_SEMA_TYPE_H

#include "ast/type.h"

int lo_sema_type_equal(const lo_ast_type *lhs,
                       const lo_ast_type *rhs);

int lo_sema_type_compatible(const lo_ast_type *lhs,
                            const lo_ast_type *rhs);

int lo_sema_type_is_integer(const lo_ast_type *type);

int lo_sema_type_is_floating(const lo_ast_type *type);

int lo_sema_type_is_scalar(const lo_ast_type *type);

#endif /* LAOOCC_SEMA_TYPE_H */

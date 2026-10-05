#include "ast/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *lo_strdup_len(const char *s, size_t len)
{
    char *copy;
    if (!s)
        return NULL;
    copy = malloc(len + 1);
    if (!copy)
        return NULL;
    memcpy(copy, s, len);
    copy[len] = '\0';
    return copy;
}

/* ------------------------------------------------------------- Base Node */

void lo_ast_node_init(lo_ast_node *node, lo_ast_kind kind)
{
    if (!node)
        return;
    node->kind = kind;
    node->line = 0;
    node->column = 0;
    node->parent = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
}

void lo_ast_node_free(lo_ast_node *node)
{
    size_t i;
    if (!node)
        return;

    for (i = 0; i < node->child_count; i++) {
        if (node->children[i]) {
            lo_ast_node_free(node->children[i]);
            free(node->children[i]);
        }
    }
    free(node->children);
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;

    switch (node->kind) {
    case LO_AST_DECLARATION:
    case LO_AST_FUNCTION_DECL:
    case LO_AST_VARIABLE_DECL:
    case LO_AST_PARAMETER_DECL:
    case LO_AST_STRUCT_DECL:
    case LO_AST_UNION_DECL:
    case LO_AST_ENUM_DECL:
    case LO_AST_ENUMERATOR:
    case LO_AST_TYPEDEF_DECL: {
        lo_ast_decl *decl = (lo_ast_decl *)node;
        free(decl->name);
        decl->name = NULL;
        lo_ast_type_free_members(&decl->type);
        break;
    }
    case LO_AST_LITERAL:
    case LO_AST_IDENTIFIER:
    case LO_AST_BINARY_EXPR:
    case LO_AST_UNARY_EXPR:
    case LO_AST_CALL_EXPR:
    case LO_AST_ASSIGN_EXPR:
    case LO_AST_CONDITIONAL_EXPR:
    case LO_AST_CAST_EXPR:
    case LO_AST_SUBSCRIPT_EXPR:
    case LO_AST_MEMBER_EXPR:
    case LO_AST_INIT_LIST_EXPR: {
        lo_ast_expr *expr = (lo_ast_expr *)node;
        free(expr->string_value);
        expr->string_value = NULL;
        lo_ast_type_free_members(&expr->cast_type);
        break;
    }
    case LO_AST_GOTO_STMT:
    case LO_AST_LABEL_STMT: {
        lo_ast_stmt *stmt = (lo_ast_stmt *)node;
        free(stmt->label_name);
        stmt->label_name = NULL;
        break;
    }
    default:
        break;
    }

    node->parent = NULL;
}

int lo_ast_node_add_child(lo_ast_node *parent, lo_ast_node *child)
{
    lo_ast_node **children;
    size_t capacity;

    if (!parent || !child)
        return -1;

    if (parent->child_count == parent->child_capacity) {
        capacity = parent->child_capacity ? parent->child_capacity * 2 : 4;
        if (capacity < parent->child_capacity)
            return -1;
        children = realloc(parent->children, capacity * sizeof(*children));
        if (!children)
            return -1;
        parent->children = children;
        parent->child_capacity = capacity;
    }

    child->parent = parent;
    parent->children[parent->child_count++] = child;
    return 0;
}

const char *lo_ast_kind_name(lo_ast_kind kind)
{
    switch (kind) {
    case LO_AST_UNKNOWN: return "UNKNOWN";
    case LO_AST_TRANSLATION_UNIT: return "TRANSLATION_UNIT";
    case LO_AST_DECLARATION: return "DECLARATION";
    case LO_AST_FUNCTION_DECL: return "FUNCTION_DECL";
    case LO_AST_VARIABLE_DECL: return "VARIABLE_DECL";
    case LO_AST_PARAMETER_DECL: return "PARAMETER_DECL";
    case LO_AST_STRUCT_DECL: return "STRUCT_DECL";
    case LO_AST_UNION_DECL: return "UNION_DECL";
    case LO_AST_ENUM_DECL: return "ENUM_DECL";
    case LO_AST_ENUMERATOR: return "ENUMERATOR";
    case LO_AST_TYPEDEF_DECL: return "TYPEDEF_DECL";
    case LO_AST_COMPOUND_STMT: return "COMPOUND_STMT";
    case LO_AST_EXPRESSION_STMT: return "EXPRESSION_STMT";
    case LO_AST_IF_STMT: return "IF_STMT";
    case LO_AST_WHILE_STMT: return "WHILE_STMT";
    case LO_AST_DO_WHILE_STMT: return "DO_WHILE_STMT";
    case LO_AST_FOR_STMT: return "FOR_STMT";
    case LO_AST_RETURN_STMT: return "RETURN_STMT";
    case LO_AST_BREAK_STMT: return "BREAK_STMT";
    case LO_AST_CONTINUE_STMT: return "CONTINUE_STMT";
    case LO_AST_SWITCH_STMT: return "SWITCH_STMT";
    case LO_AST_CASE_STMT: return "CASE_STMT";
    case LO_AST_DEFAULT_STMT: return "DEFAULT_STMT";
    case LO_AST_GOTO_STMT: return "GOTO_STMT";
    case LO_AST_LABEL_STMT: return "LABEL_STMT";
    case LO_AST_LITERAL: return "LITERAL";
    case LO_AST_IDENTIFIER: return "IDENTIFIER";
    case LO_AST_BINARY_EXPR: return "BINARY_EXPR";
    case LO_AST_UNARY_EXPR: return "UNARY_EXPR";
    case LO_AST_CALL_EXPR: return "CALL_EXPR";
    case LO_AST_ASSIGN_EXPR: return "ASSIGN_EXPR";
    case LO_AST_CONDITIONAL_EXPR: return "CONDITIONAL_EXPR";
    case LO_AST_CAST_EXPR: return "CAST_EXPR";
    case LO_AST_SUBSCRIPT_EXPR: return "SUBSCRIPT_EXPR";
    case LO_AST_MEMBER_EXPR: return "MEMBER_EXPR";
    case LO_AST_INIT_LIST_EXPR: return "INIT_LIST_EXPR";
    case LO_AST_TYPE: return "TYPE";
    }
    return "UNKNOWN";
}

/* ------------------------------------------------------------- Type */

void lo_ast_type_init(lo_ast_type *type, lo_ast_type_kind kind)
{
    if (!type)
        return;
    memset(type, 0, sizeof(*type));
    type->kind = kind;
}

lo_ast_type *lo_ast_type_new(lo_ast_type_kind kind)
{
    lo_ast_type *type = malloc(sizeof(lo_ast_type));
    if (!type)
        return NULL;
    lo_ast_type_init(type, kind);
    return type;
}

lo_ast_type *lo_ast_type_clone(const lo_ast_type *type)
{
    lo_ast_type *copy;
    if (!type)
        return NULL;
    copy = malloc(sizeof(lo_ast_type));
    if (!copy)
        return NULL;
    *copy = *type;
    if (type->tag_or_typedef_name)
        copy->tag_or_typedef_name = lo_strdup_len(type->tag_or_typedef_name,
                                                   strlen(type->tag_or_typedef_name));
    if (type->subtype)
        copy->subtype = lo_ast_type_clone(type->subtype);
    return copy;
}

lo_ast_type *lo_ast_type_pointer_to(const lo_ast_type *base)
{
    lo_ast_type *ptr = lo_ast_type_new(LO_AST_TYPE_POINTER);
    if (!ptr)
        return NULL;
    if (base)
        ptr->subtype = lo_ast_type_clone(base);
    return ptr;
}

lo_ast_type *lo_ast_type_array_of(const lo_ast_type *element, size_t size, int has_size)
{
    lo_ast_type *arr = lo_ast_type_new(LO_AST_TYPE_ARRAY);
    if (!arr)
        return NULL;
    if (element)
        arr->subtype = lo_ast_type_clone(element);
    arr->array_size = size;
    arr->has_array_size = has_size;
    return arr;
}

void lo_ast_type_free_members(lo_ast_type *type)
{
    if (!type)
        return;
    free(type->tag_or_typedef_name);
    type->tag_or_typedef_name = NULL;
    if (type->subtype) {
        lo_ast_type_free(type->subtype);
        type->subtype = NULL;
    }
}

void lo_ast_type_free(lo_ast_type *type)
{
    if (!type)
        return;
    lo_ast_type_free_members(type);
    free(type);
}

const char *lo_ast_type_kind_name(lo_ast_type_kind kind)
{
    switch (kind) {
    case LO_AST_TYPE_UNKNOWN: return "unknown";
    case LO_AST_TYPE_VOID: return "void";
    case LO_AST_TYPE_BOOL: return "_Bool";
    case LO_AST_TYPE_CHAR: return "char";
    case LO_AST_TYPE_SHORT: return "short";
    case LO_AST_TYPE_INT: return "int";
    case LO_AST_TYPE_LONG: return "long";
    case LO_AST_TYPE_LONG_LONG: return "long long";
    case LO_AST_TYPE_FLOAT: return "float";
    case LO_AST_TYPE_DOUBLE: return "double";
    case LO_AST_TYPE_POINTER: return "pointer";
    case LO_AST_TYPE_ARRAY: return "array";
    case LO_AST_TYPE_FUNCTION: return "function";
    case LO_AST_TYPE_STRUCT: return "struct";
    case LO_AST_TYPE_UNION: return "union";
    case LO_AST_TYPE_ENUM: return "enum";
    case LO_AST_TYPE_TYPEDEF_NAME: return "typedef-name";
    }
    return "unknown";
}

/* ------------------------------------------------------------- Expression */

lo_ast_expr *lo_ast_expr_new(lo_ast_kind kind)
{
    lo_ast_expr *expr = calloc(1, sizeof(lo_ast_expr));
    if (!expr)
        return NULL;
    lo_ast_node_init(&expr->node, kind);
    return expr;
}

lo_ast_expr *lo_ast_literal_new(lo_ast_literal_kind lkind, const char *text, size_t len)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_LITERAL);
    if (!expr)
        return NULL;
    expr->literal_kind = lkind;
    expr->string_value = lo_strdup_len(text, len);
    expr->string_length = len;
    return expr;
}

lo_ast_expr *lo_ast_identifier_new(const char *name, size_t len)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_IDENTIFIER);
    if (!expr)
        return NULL;
    expr->string_value = lo_strdup_len(name, len);
    expr->string_length = len;
    return expr;
}

lo_ast_expr *lo_ast_binary_new(lo_ast_binary_op op, lo_ast_node *left, lo_ast_node *right)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_BINARY_EXPR);
    if (!expr)
        return NULL;
    expr->binary_op = op;
    if (left) lo_ast_node_add_child(&expr->node, left);
    if (right) lo_ast_node_add_child(&expr->node, right);
    return expr;
}

lo_ast_expr *lo_ast_assign_new(lo_ast_assign_op op, lo_ast_node *target, lo_ast_node *value)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_ASSIGN_EXPR);
    if (!expr)
        return NULL;
    expr->assign_op = op;
    if (target) lo_ast_node_add_child(&expr->node, target);
    if (value) lo_ast_node_add_child(&expr->node, value);
    return expr;
}

lo_ast_expr *lo_ast_unary_new(lo_ast_unary_op op, lo_ast_node *operand)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_UNARY_EXPR);
    if (!expr)
        return NULL;
    expr->unary_op = op;
    if (operand) lo_ast_node_add_child(&expr->node, operand);
    return expr;
}

lo_ast_expr *lo_ast_call_new(lo_ast_node *callee)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_CALL_EXPR);
    if (!expr)
        return NULL;
    if (callee) lo_ast_node_add_child(&expr->node, callee);
    return expr;
}

lo_ast_expr *lo_ast_subscript_new(lo_ast_node *array_expr, lo_ast_node *index_expr)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_SUBSCRIPT_EXPR);
    if (!expr)
        return NULL;
    if (array_expr) lo_ast_node_add_child(&expr->node, array_expr);
    if (index_expr) lo_ast_node_add_child(&expr->node, index_expr);
    return expr;
}

lo_ast_expr *lo_ast_member_new(lo_ast_node *obj_expr, const char *member, size_t member_len, int is_arrow)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_MEMBER_EXPR);
    if (!expr)
        return NULL;
    expr->is_arrow = is_arrow;
    expr->string_value = lo_strdup_len(member, member_len);
    expr->string_length = member_len;
    if (obj_expr) lo_ast_node_add_child(&expr->node, obj_expr);
    return expr;
}

lo_ast_expr *lo_ast_conditional_new(lo_ast_node *cond, lo_ast_node *true_expr, lo_ast_node *false_expr)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_CONDITIONAL_EXPR);
    if (!expr)
        return NULL;
    if (cond) lo_ast_node_add_child(&expr->node, cond);
    if (true_expr) lo_ast_node_add_child(&expr->node, true_expr);
    if (false_expr) lo_ast_node_add_child(&expr->node, false_expr);
    return expr;
}

lo_ast_expr *lo_ast_cast_new(const lo_ast_type *type, lo_ast_node *operand)
{
    lo_ast_expr *expr = lo_ast_expr_new(LO_AST_CAST_EXPR);
    if (!expr)
        return NULL;
    if (type) expr->cast_type = *type;
    if (operand) lo_ast_node_add_child(&expr->node, operand);
    return expr;
}

const char *lo_ast_binary_op_name(lo_ast_binary_op op)
{
    switch (op) {
    case LO_AST_BIN_UNKNOWN: return "?";
    case LO_AST_BIN_ADD: return "+";
    case LO_AST_BIN_SUB: return "-";
    case LO_AST_BIN_MUL: return "*";
    case LO_AST_BIN_DIV: return "/";
    case LO_AST_BIN_MOD: return "%";
    case LO_AST_BIN_SHL: return "<<";
    case LO_AST_BIN_SHR: return ">>";
    case LO_AST_BIN_LT: return "<";
    case LO_AST_BIN_LE: return "<=";
    case LO_AST_BIN_GT: return ">";
    case LO_AST_BIN_GE: return ">=";
    case LO_AST_BIN_EQ: return "==";
    case LO_AST_BIN_NE: return "!=";
    case LO_AST_BIN_BIT_AND: return "&";
    case LO_AST_BIN_BIT_OR: return "|";
    case LO_AST_BIN_BIT_XOR: return "^";
    case LO_AST_BIN_LOGICAL_AND: return "&&";
    case LO_AST_BIN_LOGICAL_OR: return "||";
    case LO_AST_BIN_COMMA: return ",";
    }
    return "?";
}

const char *lo_ast_assign_op_name(lo_ast_assign_op op)
{
    switch (op) {
    case LO_AST_ASSIGN_UNKNOWN: return "=";
    case LO_AST_ASSIGN_EQUAL: return "=";
    case LO_AST_ASSIGN_ADD: return "+=";
    case LO_AST_ASSIGN_SUB: return "-=";
    case LO_AST_ASSIGN_MUL: return "*=";
    case LO_AST_ASSIGN_DIV: return "/=";
    case LO_AST_ASSIGN_MOD: return "%=";
    case LO_AST_ASSIGN_SHL: return "<<=";
    case LO_AST_ASSIGN_SHR: return ">>=";
    case LO_AST_ASSIGN_AND: return "&=";
    case LO_AST_ASSIGN_XOR: return "^=";
    case LO_AST_ASSIGN_OR: return "|=";
    }
    return "=";
}

const char *lo_ast_unary_op_name(lo_ast_unary_op op)
{
    switch (op) {
    case LO_AST_UNARY_UNKNOWN: return "?";
    case LO_AST_UNARY_PLUS: return "+";
    case LO_AST_UNARY_MINUS: return "-";
    case LO_AST_UNARY_NOT: return "!";
    case LO_AST_UNARY_BIT_NOT: return "~";
    case LO_AST_UNARY_ADDRESS: return "&";
    case LO_AST_UNARY_DEREFERENCE: return "*";
    case LO_AST_UNARY_PRE_INC: return "++ (pre)";
    case LO_AST_UNARY_PRE_DEC: return "-- (pre)";
    case LO_AST_UNARY_POST_INC: return "++ (post)";
    case LO_AST_UNARY_POST_DEC: return "-- (post)";
    case LO_AST_UNARY_SIZEOF: return "sizeof";
    }
    return "?";
}

/* ------------------------------------------------------------- Statement */

lo_ast_stmt *lo_ast_stmt_new(lo_ast_kind kind)
{
    lo_ast_stmt *stmt = calloc(1, sizeof(lo_ast_stmt));
    if (!stmt)
        return NULL;
    lo_ast_node_init(&stmt->node, kind);
    return stmt;
}

lo_ast_stmt *lo_ast_compound_stmt_new(void)
{
    return lo_ast_stmt_new(LO_AST_COMPOUND_STMT);
}

lo_ast_stmt *lo_ast_expr_stmt_new(lo_ast_node *expr)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_EXPRESSION_STMT);
    if (!stmt)
        return NULL;
    if (expr) lo_ast_node_add_child(&stmt->node, expr);
    return stmt;
}

lo_ast_stmt *lo_ast_if_stmt_new(lo_ast_node *cond, lo_ast_node *then_b, lo_ast_node *else_b)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_IF_STMT);
    if (!stmt)
        return NULL;
    if (cond) lo_ast_node_add_child(&stmt->node, cond);
    if (then_b) lo_ast_node_add_child(&stmt->node, then_b);
    if (else_b) lo_ast_node_add_child(&stmt->node, else_b);
    return stmt;
}

lo_ast_stmt *lo_ast_while_stmt_new(lo_ast_node *cond, lo_ast_node *body)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_WHILE_STMT);
    if (!stmt)
        return NULL;
    if (cond) lo_ast_node_add_child(&stmt->node, cond);
    if (body) lo_ast_node_add_child(&stmt->node, body);
    return stmt;
}

lo_ast_stmt *lo_ast_do_while_stmt_new(lo_ast_node *body, lo_ast_node *cond)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_DO_WHILE_STMT);
    if (!stmt)
        return NULL;
    if (body) lo_ast_node_add_child(&stmt->node, body);
    if (cond) lo_ast_node_add_child(&stmt->node, cond);
    return stmt;
}

lo_ast_stmt *lo_ast_for_stmt_new(lo_ast_node *init, lo_ast_node *cond, lo_ast_node *step, lo_ast_node *body)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_FOR_STMT);
    if (!stmt)
        return NULL;
    /* Create empty node placeholder if child is NULL to maintain indices */
    lo_ast_node *empty = (lo_ast_node *)lo_ast_stmt_new(LO_AST_EXPRESSION_STMT);
    lo_ast_node_add_child(&stmt->node, init ? init : empty);
    if (!init) { /* empty is adopted */ }

    empty = (lo_ast_node *)lo_ast_stmt_new(LO_AST_EXPRESSION_STMT);
    lo_ast_node_add_child(&stmt->node, cond ? cond : empty);

    empty = (lo_ast_node *)lo_ast_stmt_new(LO_AST_EXPRESSION_STMT);
    lo_ast_node_add_child(&stmt->node, step ? step : empty);

    if (body) lo_ast_node_add_child(&stmt->node, body);
    return stmt;
}

lo_ast_stmt *lo_ast_return_stmt_new(lo_ast_node *expr)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_RETURN_STMT);
    if (!stmt)
        return NULL;
    if (expr) lo_ast_node_add_child(&stmt->node, expr);
    return stmt;
}

lo_ast_stmt *lo_ast_break_stmt_new(void)
{
    return lo_ast_stmt_new(LO_AST_BREAK_STMT);
}

lo_ast_stmt *lo_ast_continue_stmt_new(void)
{
    return lo_ast_stmt_new(LO_AST_CONTINUE_STMT);
}

lo_ast_stmt *lo_ast_switch_stmt_new(lo_ast_node *expr, lo_ast_node *body)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_SWITCH_STMT);
    if (!stmt)
        return NULL;
    if (expr) lo_ast_node_add_child(&stmt->node, expr);
    if (body) lo_ast_node_add_child(&stmt->node, body);
    return stmt;
}

lo_ast_stmt *lo_ast_case_stmt_new(lo_ast_node *expr, lo_ast_node *body)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_CASE_STMT);
    if (!stmt)
        return NULL;
    if (expr) lo_ast_node_add_child(&stmt->node, expr);
    if (body) lo_ast_node_add_child(&stmt->node, body);
    return stmt;
}

lo_ast_stmt *lo_ast_default_stmt_new(lo_ast_node *body)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_DEFAULT_STMT);
    if (!stmt)
        return NULL;
    if (body) lo_ast_node_add_child(&stmt->node, body);
    return stmt;
}

lo_ast_stmt *lo_ast_goto_stmt_new(const char *label, size_t label_len)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_GOTO_STMT);
    if (!stmt)
        return NULL;
    stmt->label_name = lo_strdup_len(label, label_len);
    return stmt;
}

lo_ast_stmt *lo_ast_label_stmt_new(const char *label, size_t label_len, lo_ast_node *substmt)
{
    lo_ast_stmt *stmt = lo_ast_stmt_new(LO_AST_LABEL_STMT);
    if (!stmt)
        return NULL;
    stmt->label_name = lo_strdup_len(label, label_len);
    if (substmt) lo_ast_node_add_child(&stmt->node, substmt);
    return stmt;
}

/* ------------------------------------------------------------- Declaration */

lo_ast_decl *lo_ast_decl_new(lo_ast_kind kind)
{
    lo_ast_decl *decl = calloc(1, sizeof(lo_ast_decl));
    if (!decl)
        return NULL;
    lo_ast_node_init(&decl->node, kind);
    return decl;
}

lo_ast_decl *lo_ast_var_decl_new(const char *name, size_t name_len, const lo_ast_type *type, lo_ast_node *init)
{
    lo_ast_decl *decl = lo_ast_decl_new(LO_AST_VARIABLE_DECL);
    if (!decl)
        return NULL;
    decl->name = lo_strdup_len(name, name_len);
    if (type) decl->type = *type;
    if (init) lo_ast_node_add_child(&decl->node, init);
    return decl;
}

lo_ast_decl *lo_ast_func_decl_new(const char *name, size_t name_len, const lo_ast_type *return_type, int is_definition)
{
    lo_ast_decl *decl = lo_ast_decl_new(LO_AST_FUNCTION_DECL);
    if (!decl)
        return NULL;
    decl->name = lo_strdup_len(name, name_len);
    if (return_type) decl->type = *return_type;
    decl->is_function_definition = is_definition;
    return decl;
}

lo_ast_decl *lo_ast_param_decl_new(const char *name, size_t name_len, const lo_ast_type *type)
{
    lo_ast_decl *decl = lo_ast_decl_new(LO_AST_PARAMETER_DECL);
    if (!decl)
        return NULL;
    if (name) decl->name = lo_strdup_len(name, name_len);
    if (type) decl->type = *type;
    return decl;
}

lo_ast_decl *lo_ast_struct_decl_new(const char *tag, size_t tag_len, int is_union, int is_definition)
{
    lo_ast_decl *decl = lo_ast_decl_new(is_union ? LO_AST_UNION_DECL : LO_AST_STRUCT_DECL);
    if (!decl)
        return NULL;
    if (tag) decl->name = lo_strdup_len(tag, tag_len);
    decl->is_function_definition = is_definition; /* re-used as has_body flag */
    return decl;
}

lo_ast_decl *lo_ast_enum_decl_new(const char *tag, size_t tag_len, int is_definition)
{
    lo_ast_decl *decl = lo_ast_decl_new(LO_AST_ENUM_DECL);
    if (!decl)
        return NULL;
    if (tag) decl->name = lo_strdup_len(tag, tag_len);
    decl->is_function_definition = is_definition;
    return decl;
}

lo_ast_decl *lo_ast_enumerator_new(const char *name, size_t name_len, lo_ast_node *value_expr)
{
    lo_ast_decl *decl = lo_ast_decl_new(LO_AST_ENUMERATOR);
    if (!decl)
        return NULL;
    decl->name = lo_strdup_len(name, name_len);
    if (value_expr) lo_ast_node_add_child(&decl->node, value_expr);
    return decl;
}

/* ------------------------------------------------------------- AST Dump */

static void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
        printf("  ");
}

static void print_type(const lo_ast_type *type)
{
    if (!type) return;
    if (type->is_const) printf("const ");
    if (type->is_volatile) printf("volatile ");
    if (type->is_restrict) printf("restrict ");

    switch (type->kind) {
    case LO_AST_TYPE_POINTER:
        print_type(type->subtype);
        printf("*");
        break;
    case LO_AST_TYPE_ARRAY:
        print_type(type->subtype);
        if (type->has_array_size)
            printf("[%zu]", type->array_size);
        else
            printf("[]");
        break;
    case LO_AST_TYPE_STRUCT:
        printf("struct %s", type->tag_or_typedef_name ? type->tag_or_typedef_name : "(anonymous)");
        break;
    case LO_AST_TYPE_UNION:
        printf("union %s", type->tag_or_typedef_name ? type->tag_or_typedef_name : "(anonymous)");
        break;
    case LO_AST_TYPE_ENUM:
        printf("enum %s", type->tag_or_typedef_name ? type->tag_or_typedef_name : "(anonymous)");
        break;
    case LO_AST_TYPE_TYPEDEF_NAME:
        printf("%s", type->tag_or_typedef_name ? type->tag_or_typedef_name : "(typedef)");
        break;
    default:
        printf("%s", lo_ast_type_kind_name(type->kind));
        break;
    }
}

void lo_ast_dump(const lo_ast_node *node, int indent)
{
    if (!node)
        return;

    print_indent(indent);
    printf("%s", lo_ast_kind_name(node->kind));

    switch (node->kind) {
    case LO_AST_FUNCTION_DECL: {
        const lo_ast_decl *decl = (const lo_ast_decl *)node;
        printf(" name=%s type=", decl->name ? decl->name : "(anonymous)");
        print_type(&decl->type);
        if (decl->is_static) printf(" [static]");
        if (decl->is_extern) printf(" [extern]");
        if (decl->is_inline) printf(" [inline]");
        if (!decl->is_function_definition) printf(" [prototype]");
        break;
    }
    case LO_AST_VARIABLE_DECL:
    case LO_AST_PARAMETER_DECL:
    case LO_AST_TYPEDEF_DECL: {
        const lo_ast_decl *decl = (const lo_ast_decl *)node;
        printf(" name=%s type=", decl->name ? decl->name : "(anonymous)");
        print_type(&decl->type);
        if (decl->is_static) printf(" [static]");
        if (decl->is_extern) printf(" [extern]");
        if (decl->is_typedef) printf(" [typedef]");
        break;
    }
    case LO_AST_STRUCT_DECL:
    case LO_AST_UNION_DECL:
    case LO_AST_ENUM_DECL: {
        const lo_ast_decl *decl = (const lo_ast_decl *)node;
        if (decl->name) printf(" tag=%s", decl->name);
        break;
    }
    case LO_AST_ENUMERATOR: {
        const lo_ast_decl *decl = (const lo_ast_decl *)node;
        if (decl->name) printf(" name=%s", decl->name);
        break;
    }
    case LO_AST_LITERAL: {
        const lo_ast_expr *expr = (const lo_ast_expr *)node;
        printf(" %s", expr->string_value ? expr->string_value : "");
        break;
    }
    case LO_AST_IDENTIFIER: {
        const lo_ast_expr *expr = (const lo_ast_expr *)node;
        printf(" %s", expr->string_value ? expr->string_value : "");
        break;
    }
    case LO_AST_BINARY_EXPR: {
        const lo_ast_expr *expr = (const lo_ast_expr *)node;
        printf(" op='%s'", lo_ast_binary_op_name(expr->binary_op));
        break;
    }
    case LO_AST_ASSIGN_EXPR: {
        const lo_ast_expr *expr = (const lo_ast_expr *)node;
        printf(" op='%s'", lo_ast_assign_op_name(expr->assign_op));
        break;
    }
    case LO_AST_UNARY_EXPR: {
        const lo_ast_expr *expr = (const lo_ast_expr *)node;
        printf(" op='%s'", lo_ast_unary_op_name(expr->unary_op));
        break;
    }
    case LO_AST_MEMBER_EXPR: {
        const lo_ast_expr *expr = (const lo_ast_expr *)node;
        printf(" %s%s", expr->is_arrow ? "->" : ".", expr->string_value ? expr->string_value : "");
        break;
    }
    case LO_AST_CAST_EXPR: {
        const lo_ast_expr *expr = (const lo_ast_expr *)node;
        printf(" target_type=");
        print_type(&expr->cast_type);
        break;
    }
    case LO_AST_GOTO_STMT:
    case LO_AST_LABEL_STMT: {
        const lo_ast_stmt *stmt = (const lo_ast_stmt *)node;
        if (stmt->label_name) printf(" label=%s", stmt->label_name);
        break;
    }
    default:
        break;
    }
    putchar('\n');

    for (size_t i = 0; i < node->child_count; i++) {
        lo_ast_dump(node->children[i], indent + 1);
    }
}

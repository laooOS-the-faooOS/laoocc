#include "ast/ast.h"

#include <stdlib.h>

/* AST Node */

void lo_ast_node_init(lo_ast_node *node,
                      lo_ast_kind kind)
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
    node->parent = NULL;
}

int lo_ast_node_add_child(lo_ast_node *parent,
                          lo_ast_node *child)
{
    lo_ast_node **children;
    size_t capacity;

    if (!parent || !child)
        return -1;

    if (parent->child_count == parent->child_capacity) {
        capacity = parent->child_capacity ?
                   parent->child_capacity * 2 : 4;

        if (capacity < parent->child_capacity)
            return -1;

        children = realloc(parent->children,
                            capacity * sizeof(*children));

        if (!children)
            return -1;

        parent->children = children;
        parent->child_capacity = capacity;
    }

    child->parent = parent;
    parent->children[parent->child_count++] = child;

    return 0;
}

/* AST Type */

void lo_ast_type_init(lo_ast_type *type,
                      lo_ast_type_kind kind)
{
    if (!type)
        return;

    type->kind = kind;
    type->bits = 0;
    type->is_signed = 0;
}

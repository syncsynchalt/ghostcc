#pragma once
#include "lex.h"

typedef enum {
    NODE_OTHER,
    NODE_INT,
    NODE_FLT,
    NODE_STR,
    NODE_CHA,

    NODE_LIST,
    NODE_SUBSCRIPT,
    NODE_CAST,
    NODE_ID_LIST,
    NODE_INIT_LIST,
    NODE_DECL_LIST,
    NODE_DECL_SPECIFIERS,
    NODE_DECLARE,
    NODE_STRUCT_MEMBERS,
    NODE_TRANSLATION_UNIT,
    NODE_COMPOUND_STATEMENT,
    NODE_BITFIELD,
    NODE_PARAM_LIST,
    NODE_FUNCTION,

    NODE_ABSTRACT,
    NODE_ABSTRACT_TYPE,
} node_type;

typedef struct _ast_node {
    node_type type;
    token_type tok_type;

    char *s;
    int ival;
    double fval;
    unsigned char cval;

    struct _ast_node *left;
    struct _ast_node *right;

    struct _ast_node **list;
    size_t list_len;
} ast_node;

extern ast_node *make_ast_node(const token t, ast_node *left, ast_node *right);
extern void add_to_ast_list(ast_node *node, ast_node *list_member);
extern void free_ast_node(ast_node *node);

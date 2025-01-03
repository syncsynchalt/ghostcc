#pragma once
#include "lex.h"

typedef enum {
    NODE_OTHER,
    NODE_INT,
    NODE_FLT,
    NODE_STR,
    NODE_CHA,
} node_type;

typedef struct _ast_node {
    struct _ast_node *prev; // linked list used for alloc/free tracking
    struct _ast_node *next;
    node_type node_type;
    token_type token_type;

    char *s;
    int ival;
    double fval;
    unsigned char cval;

    struct _ast_node *left;
    struct _ast_node *right;
} ast_node;

extern ast_node *make_ast_node(const token_state *tok, ast_node *left, ast_node *right);

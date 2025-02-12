#pragma once

#include "ast.h"

typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLT,
    TYPE_STRUCT,
    TYPE_PTR,
} type_t;

extern type_t read_type(const ast_node *type_decl);

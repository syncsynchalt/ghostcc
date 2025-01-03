#pragma once
#include "ast.h"

extern ast_node *pp_parse(const char *s);

#define AST_RESULT_TYPE_INT 0
#define AST_RESULT_TYPE_FLT 1
#define AST_RESULT_TYPE_STR 2
typedef struct {
  int type;
  int ival;
  double fval;
  char *sval;
} ast_result;

extern ast_result pp_resolve_ast(const ast_node *node);

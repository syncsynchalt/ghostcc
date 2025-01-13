#pragma once
#include "ast.h"
#include "defs.h"

/**
 * Given a preprocessor expression, parse into an abstract syntax tree.
 *
 * @example
 * Given input: "1 && (0 ? 3 : 4)"
 * Result (in pseudo-sexpr): (AND 1 (? 0 (: 3 4)))
 *
 * @param s preprocessor expression to parse
 * @param defs the known #defines, used to resolve the `defined()` pseudo-macro
 * @return top node of an abstract syntax tree
 */
extern ast_node *string_to_ast(const char *s, const defines *defs);

#define AST_RESULT_TYPE_INT 0
#define AST_RESULT_TYPE_FLT 1
#define AST_RESULT_TYPE_STR 2
typedef struct {
  int type;
  int ival;
  double fval;
  char *sval;
} ast_result;

/**
 * Resolve an abstract syntax tree to a result
 *
 * @param node head of an abstract syntax tree to resolve
 * @return an int, float, or string result
 */
extern ast_result resolve_ast(const ast_node *node);

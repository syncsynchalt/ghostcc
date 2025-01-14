#pragma once

#define YYSTYPE ast_node *
#include "ast.h"
#include "defs.h"

/// reset the parser
extern void reset_parser(const char *target, const defines *defs);
// xxx todo ^^^ this should take pp_parse_target and pp_parse_defs as input
/// get the next token from pp_parse_target
extern int yylex(void);
/// the return from yylex() (expected by yacc parser)
extern ast_node *yylval;
/// the root of the AST after calling yyparse()
extern ast_node *pp_parse_result;

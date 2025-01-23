#pragma once

#include "ast.h"
#define YYSTYPE ast_node *

/// get the next token from cc_parse_target
extern int yylex(void);
/// the return from yylex() (expected by yacc parser)
extern ast_node *yylval;
/// the root of the AST after calling yyparse()
extern ast_node *cc_parse_result;

/// if a typedef is fine in this AST node, add it to the list of type names for future lexical tokens
extern void check_for_typedef(const ast_node *node);

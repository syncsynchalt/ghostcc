#pragma once

#define YYSTYPE ast_node *
#include "ast.h"
#include "defs.h"

/// the string to parse against the grammar
extern char *pp_parse_target;
/// the #define list currently active
extern defines const *pp_parse_defs;
/// reset the parser
extern void reset_parser();
/// get the next token from pp_parse_target
extern int yylex(void);
/// the return from yylex() (expected by yacc parser)
extern ast_node *yylval;
/// the root of the AST after calling yyparse()
extern ast_node *pp_parse_result;

/// the line being parsed (used in error message)
extern char const *pp_parse_line;
/// the current into into the line being parsed (used in error message)
extern size_t pp_parse_line_index;

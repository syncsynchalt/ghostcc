#pragma once

#define YYSTYPE ast_node *
#include "ast.h"
#include "defs.h"

extern char *pp_parse_target;
extern defines const *pp_parse_defs;
extern void reset_parser();
extern int yylex(void);
extern ast_node *yylval;
extern ast_node *pp_parse_result;

extern char const *pp_parse_line;
extern size_t pp_parse_line_index;

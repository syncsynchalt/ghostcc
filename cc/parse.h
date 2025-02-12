#pragma once
#include <stdio.h>
#include "ast.h"

extern FILE *cc_input;
extern const char *cc_filename;
extern int cc_lineno;

extern ast_node *parse_ast(const char *file, FILE *in);

extern int yyparse();

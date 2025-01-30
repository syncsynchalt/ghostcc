#pragma once
#include <stdio.h>
#include "ast.h"

extern FILE *cc_input;
extern const char *cc_filename;
extern int cc_lineno;

extern int process_file(const char *startfile, FILE *in, FILE *out);
extern ast_node *parse_ast(const char *file, FILE *in);

extern int yyparse();

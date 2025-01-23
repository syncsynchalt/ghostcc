#pragma once
#include <stdio.h>

extern FILE *cc_input;
extern char *cc_filename;
extern int cc_lineno;

extern int process_file(char *startfile, FILE *in, FILE *out);

extern int yyparse();

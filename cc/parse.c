#include <stdio.h>
#include "parse.h"
#include "ast.h"
#include "cc_lex.h"

FILE *cc_input = NULL;
char *cc_filename = NULL;
int cc_lineno = 0;

int process_file(char *startfile, FILE *in, FILE *out)
{
    cc_input = in;
    cc_filename = startfile;
    cc_lineno = 0;

    yyparse();
    ast_node *ast = cc_parse_result;
    return 0;
}

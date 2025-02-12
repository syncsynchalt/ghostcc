#include <stdio.h>
#include "parse.h"
#include "ast.h"
#include "cc_lex.h"

FILE *cc_input = NULL;
const char *cc_filename = NULL;
int cc_lineno = 0;

ast_node *parse_ast(const char *file, FILE *in)
{
    cc_input = in;
    cc_filename = file;
    cc_lineno = 0;

    yyparse();
    return cc_parse_result;
}

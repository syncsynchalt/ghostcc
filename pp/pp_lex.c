#include <stdio.h>
#include <string.h>
#include "pp_lex.h"
#include "pp_macro.h"
#include "pp_toker.h"
#include "subst.h"

ast_node *pp_parse_result;

static token_state lex_ts;
static const char *pp_parse_target;
static defines const *pp_parse_defs;

// set for use in error message
static char const *pp_parse_line;
static size_t pp_parse_line_index;

static char *scratch;
static size_t scratch_len;

void reset_parser(const char *target, const defines *defs)
{
    memset(&lex_ts, 0, sizeof(lex_ts));
    pp_parse_target = target;
    pp_parse_defs = defs;
    free(scratch);
    scratch = NULL;
    scratch_len = 0;
}

/**
 * Two-pass lexer process:
 *
 * - On first call, iterate over the string doing #define replacements and resolving `defined()` pseudo-macro,
 *   saving the result in `scratch`
 * - On the first and successive calls return the next token from `scratch`
 *
 * Use reset_parser to reset this process for a new string.
 */
int yylex(void)
{
    if (!scratch) {
        scratch = subst_tokens(pp_parse_target, pp_parse_defs);
        scratch_len = strlen(scratch);
        set_token_string(&lex_ts, scratch);
    }
    token t;
    do {
        t = get_token(&lex_ts);
    } while (t.type == TOK_WS);

    if (t.type == EOF) {
        yylval = NULL;
        return -1;
    }

    pp_parse_line = lex_ts.line;
    pp_parse_line_index = lex_ts.ind - strlen(lex_ts.last.tok);
    pp_parse_line_index = pp_parse_line_index < 0 ? 0 : pp_parse_line_index;
    yylval = make_ast_node(t, NULL, NULL);
    return t.type;
}

void yyerror(const char *s)
{
    fprintf(stderr, "%s\n", s);
    int i;
    fprintf(stderr, "line: %s\n     ", pp_parse_line);
    for (i = 0; i <= pp_parse_line_index; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
    exit(1);
}

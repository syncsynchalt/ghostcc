#include <string.h>
#include <stdio.h>
#include "pp_lex.h"
#include "pp_macro.h"
#include "lex.h"
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
        scratch = subst_tokens(pp_parse_target, pp_parse_defs, NULL);
        scratch_len = strlen(scratch);
    }
    do {
        if (lex_ts.ind >= scratch_len) {
            yylval = NULL;
            return -1;
        }
        get_token(scratch, scratch_len, &lex_ts);
    } while (lex_ts.type == TOK_WS);

    pp_parse_line = lex_ts._line;
    pp_parse_line_index = lex_ts.pos;
    yylval = make_ast_node(&lex_ts, NULL, NULL);
    return lex_ts.type;
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

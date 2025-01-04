#include <string.h>
#include "pp_lex.h"
#include "lex.h"
#include <stdio.h>

static token_state s;
char *pp_parse_target;
defines const *pp_parse_defs;
size_t pp_parse_target_len;
ast_node *pp_parse_result;
char const *pp_parse_line;
size_t pp_parse_line_index;

char *scratch;
size_t scratch_len;

void reset_parser(void)
{
    memset(&s, 0, sizeof(s));
    pp_parse_target = NULL;
    pp_parse_target_len = 0;
    scratch = NULL;
    scratch_len = 0;
}

static void add_to_scratch(const char *w, size_t *ind)
{
    if (*ind + strlen(w) >= scratch_len) {
        scratch_len *= 2;
        scratch = realloc(scratch, scratch_len);
    }
    strcpy(scratch + *ind, w);
    *ind += strlen(w);
}

void subst_tokens(void)
{
    token_state ts = {0};

    if (scratch) {
        free(scratch);
        scratch = NULL;
    }
    pp_parse_target_len = strlen(pp_parse_target);
    scratch_len = 2 * pp_parse_target_len;
    scratch = malloc(scratch_len + 1);

    size_t ind = 0;
    const def *d = NULL;
    while (ts.ind < pp_parse_target_len) {
        get_token(pp_parse_target, strlen(pp_parse_target), &ts);
        if ((d = defines_get(pp_parse_defs, ts.tok))) {
            // todo handle macros
            int i = 0;
            while (d->replace && d->replace[i]) {
                add_to_scratch(d->replace[i++], &ind);
            }
        } else {
            add_to_scratch(ts.tok, &ind);
        }
    }
    scratch_len = strlen(scratch);
}

int yylex(void)
{
    if (!scratch) {
        subst_tokens();
    }
    do {
        if (s.ind >= scratch_len) {
            yylval = NULL;
            return -1;
        }
        get_token(scratch, scratch_len, &s);
    } while (s.type == TOK_WS);

    pp_parse_line = s._line;
    pp_parse_line_index = s.pos;
    yylval = make_ast_node(&s, NULL, NULL);
    return s.type;
}

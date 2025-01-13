#include <string.h>
#include <stdio.h>
#include "pp_lex.h"
#include "pp_macro.h"
#include "lex.h"
#include "die.h"

static token_state s_;
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
    memset(&s_, 0, sizeof(s_));
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

void handle_defined(token_state *ts, size_t *scratch_ind)
{
    int found_parens = 0;

    while (ts->ind < pp_parse_target_len) {
        get_token(pp_parse_target, strlen(pp_parse_target), ts);
        if (ts->type == TOK_WS) {
            continue;
        }
        if (ts->type == '(') {
            found_parens = 1;
            continue;
        }
        const int found = defines_get(pp_parse_defs, ts->tok) != NULL;
        add_to_scratch(found ? "1" : "0", scratch_ind);
        break;
    }

    while (found_parens && ts->ind < pp_parse_target_len) {
        get_token(pp_parse_target, strlen(pp_parse_target), ts);
        if (ts->type == TOK_WS) {
            continue;
        }
        if (ts->type == ')') {
            found_parens = 0;
            continue;
        }
        break;
    }

    if (found_parens) {
        die("Unexpected %s while looking for closing parens",
            ts->ind < pp_parse_target_len ? ts->tok : "end-of-line");
    }
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
    scratch = calloc(1, scratch_len + 1);

    size_t ind = 0;
    const def *d = NULL;
    while (ts.ind < pp_parse_target_len) {
        get_token(pp_parse_target, strlen(pp_parse_target), &ts);
        if ((d = defines_get(pp_parse_defs, ts.tok))) {
            if (d->args) {
                handle_macro(d, &ts, &scratch, &ind, &scratch_len);
            } else {
                int i = 0;
                while (d->replace && d->replace[i]) {
                    add_to_scratch(d->replace[i++], &ind);
                }
            }
        } else if (strcmp(ts.tok, "defined") == 0) {
            // special case: handle the `defined(SYMBOL)` pseudo-macro
            handle_defined(&ts, &ind);
        } else {
            add_to_scratch(ts.tok, &ind);
        }
    }
    scratch_len = strlen(scratch);
}

/**
 * Two-pass lexer process:
 *
 * - On first call, iterate over the string doing #define replacements and resolving `defined()` pseudo-macro,
 *   saving the result in `scratch`
 * - On the first and successive calls return the next token from `scratch`
 */
int yylex(void)
{
    if (!scratch) {
        subst_tokens();
    }
    do {
        if (s_.ind >= scratch_len) {
            yylval = NULL;
            return -1;
        }
        get_token(scratch, scratch_len, &s_);
    } while (s_.type == TOK_WS);

    pp_parse_line = s_._line;
    pp_parse_line_index = s_.pos;
    yylval = make_ast_node(&s_, NULL, NULL);
    return s_.type;
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

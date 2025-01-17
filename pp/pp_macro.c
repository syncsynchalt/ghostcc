#include "pp_macro.h"

#include <common.h>
#include <ctype.h>
#include <string.h>
#include "die.h"
#include "lex.h"
#include "str.h"
#include "subst.h"

static token skip_ws(token_state *ts)
{
    for (;;) {
        const token t = get_token(ts);
        if (t.type != TOK_WS) {
            return t;
        }
    }
}

static void trim_trailing_whitespace(str_t *str)
{
    while (str->end && isspace(str->s[str->end-1])) {
        str->s[--str->end] = '\0';
    }
}

static str_t read_arg(token t, token_state *ts)
{
    str_t result = {0};
    int parens = 0;
    for (;;) {
        if (t.type == '(') {
            parens++;
        }
        if (TOKEN_STATE_DONE(ts) || (parens == 0 && (t.type == ',' || t.type == ')'))) {
            break;
        }
        if (parens && t.type == ')') {
            parens--;
        }
        add_to_str(&result, t.tok);
        t = get_token(ts);
    }

    return result;
}

void handle_macro(const def *d, const defines *defs, token_state *ts, str_t *out)
{
    char **extra_args = NULL;
    int extra_args_num = 0;

    // ensure we're looking at a macro
    // xxx todo don't use ind directly
    size_t check_ind = ts->ind;
    check_ind += strspn(ts->line + check_ind, WHITESPACE);
    if (ts->line[check_ind] != '(') {
        // Mis-fire, this is not a macro call. Substitute the string as-is.
        add_to_str(out, d->name);
        return;
    }

    // make room for tracking replacement value of each arg
    int num_args;
    for (num_args = 0; d->args[num_args]; ++num_args) {}
    char **vals = malloc(num_args * sizeof(char *));

    if (num_args && strcmp(d->args[num_args-1], "...") == 0) {
        num_args--;
        extra_args = calloc(1, (extra_args_num + 5) * sizeof(*extra_args));
    }

    token t = skip_ws(ts); // skip to '('

    // assign replacement value of each arg
    int arg;
    for (arg = 0; arg < num_args; ++arg) {
        t = skip_ws(ts);
        str_t tmp = read_arg(t, ts);
        trim_trailing_whitespace(&tmp);
        vals[arg] = subst_tokens(tmp.s, defs, NULL);
        free_str(&tmp);
        if (!vals[arg]) {
            die("Missing arg %d in macro %s", arg+1, d->name);
        }
        if (ts->last.type != ',' && ts->last.type != ')') {
            die("Unexpected end of macro %s after arg %d", d->name, arg+1);
        }
        if (ts->last.type == ')' && arg+1 != num_args) {
            die("Unexpected end of args in macro %s", d->name);
        }
    }

    if (extra_args && ts->last.type != ')') {
        t = skip_ws(ts);
        for (;;) {
            if (extra_args_num && extra_args_num % 5 == 0) {
                extra_args = realloc(extra_args, (extra_args_num + 5) * sizeof(*extra_args));
            }
            const str_t tmp = read_arg(t, ts);
            extra_args[extra_args_num++] = subst_tokens(tmp.s, defs, NULL);
            free_str(&tmp);
            if (ts->last.type == ')' || TOKEN_STATE_DONE(ts)) {
                break;
            }
            extra_args[extra_args_num++] = strdup(ts->last.tok);
            t = get_token(ts);
        }
    }

    if (ts->last.type != ')') {
        if (TOKEN_STATE_DONE(ts)) {
            die("Unexpected end of args in macro %s", d->name);
        }
        die("Unexpected args past %d in macro %s", arg + extra_args_num, d->name);
    }

    // perform macro replacement
    int i = 0, j = 0, k = 0;
    char *substituted_replace = subst_tokens(d->replace, defs, d->name);
    token_state sub_ts = {0};
    set_token_string(&sub_ts, substituted_replace);
    while (!TOKEN_STATE_DONE(&sub_ts)) {
        const token tt = get_token(&sub_ts);
        const char *w = tt.tok;
        int matched = -1;

        // handle the "#" operator
        if (strcmp(w, "#") == 0) {
            skip_ws(&sub_ts);
            const char *ww = sub_ts.last.tok;
            // ensure next token is a macro arg
            for (j = 0; j < num_args; ++j) {
                if (strcmp(ww, d->args[j]) == 0) {
                    matched = j;
                }
            }
            if (matched < 0) {
                die("String operator not followed by arg");
            }

            // quote the arg replacement value and add it to output
            char *www = quote_str(vals[matched]);
            add_to_str(out, www);
            free(www);
            continue;
        }

        // handle the "##" operator
        if (strcmp(w, "##") == 0) {
            trim_trailing_whitespace(out);

            skip_ws(&sub_ts);
            const char *ww = sub_ts.last.tok;
            // check if next token is an arg
            for (j = 0; j < num_args; ++j) {
                if (strcmp(ww, d->args[j]) == 0) {
                    matched = j;
                }
            }
            if (matched >= 0) {
                // next token is a macro arg, add it to output (concatenated with previous output)
                add_to_str(out, vals[matched]);
            } else {
                // otherwise add the next token as-is (concatenated with previous output)
                add_to_str(out, ww);
            }

            // todo - "The resulting token is available for further macro replacement"
            continue;
        }

        // check if this is an arg name
        for (j = 0; j < num_args; ++j) {
            if (strcmp(w, d->args[j]) == 0) {
                matched = j;
            }
        }

        if (matched >= 0) {
            // if token matched against an arg name, print arg's replacement value
            add_to_str(out, vals[matched]);
        } else if (extra_args && strcmp(w, "__VA_ARGS__") == 0) {
            // if token is "__VA_ARGS__", print the extra args (comma separated)
            for (k = 0; k < extra_args_num; ++k) {
                add_to_str(out, extra_args[k]);
            }
        } else {
            // else pass the token along as-is
            add_to_str(out, w);
        }
    }
    free(substituted_replace);

    // free allocated memory
    for (num_args = 0; d->args[num_args]; ++num_args) {
        free(vals[num_args]);
    }
    free(vals);
    for (i = 0; extra_args && i < extra_args_num; ++i) {
        free(extra_args[i]);
    }
    free(extra_args);
}

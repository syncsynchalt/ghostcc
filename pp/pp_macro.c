#include "pp_macro.h"

#include <common.h>
#include <ctype.h>
#include <string.h>
#include "die.h"
#include "lex.h"
#include "str.h"
#include "subst.h"

static int skip_ws(token_state *s)
{
    while (!LINE_DONE(s, s->_line)) {
        get_token(s->_line, s->_line_len, s);
        if (s->type != TOK_WS) {
            break;
        }
    }
    return !LINE_DONE(s, s->_line);
}

static void trim_trailing_whitespace(str_t *str)
{
    while (str->end && isspace(str->s[str->end-1])) {
        str->s[--str->end] = '\0';
    }
}

static str_t read_arg(token_state *s)
{
    str_t result = {0};
    int parens = 0;
    for (;;) {
        if (s->type == '(') {
            parens++;
        }
        if (LINE_DONE(s, s->_line) || (parens == 0 && (s->type == ',' || s->type == ')'))) {
            break;
        }
        if (parens && s->type == ')') {
            parens--;
        }
        add_to_str(&result, s->tok);
        get_token(s->_line, s->_line_len, s);
    }

    return result;
}

void handle_macro(const def *d, const defines *defs, token_state *s, str_t *out)
{
    char **extra_args = NULL;
    int extra_args_num = 0;

    // ensure we're looking at a macro
    size_t check_ind = s->ind;
    check_ind += strspn(s->_line + check_ind, WHITESPACE);
    if (s->_line[check_ind] != '(') {
        // Mis-fire, this is not a macro call. Substitute the string as-is.
        add_to_str(out, s->tok);
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

    skip_ws(s); // skip to '('

    // assign replacement value of each arg
    int arg;
    for (arg = 0; arg < num_args; ++arg) {
        skip_ws(s);
        str_t tmp = read_arg(s);
        trim_trailing_whitespace(&tmp);
        vals[arg] = subst_tokens(tmp.s, defs, NULL);
        free_str(&tmp);
        if (!vals[arg]) {
            die("Missing arg %d in macro %s", arg+1, d->name);
        }
        if (s->type != ',' && s->type != ')') {
            die("Unexpected end of macro %s after arg %d", d->name, arg+1);
        }
        if (s->type == ')' && arg+1 != num_args) {
            die("Unexpected end of args in macro %s", d->name);
        }
    }

    if (extra_args && s->type != ')') {
        skip_ws(s);
        for (;;) {
            if (extra_args_num && extra_args_num % 5 == 0) {
                extra_args = realloc(extra_args, (extra_args_num + 5) * sizeof(*extra_args));
            }
            const str_t tmp = read_arg(s);
            extra_args[extra_args_num++] = subst_tokens(tmp.s, defs, NULL);
            free_str(&tmp);
            if (s->type == ')' || LINE_DONE(s, s->_line)) {
                break;
            }
            extra_args[extra_args_num++] = strdup(s->tok);
            get_token(s->_line, s->_line_len, s);
        }
    }

    if (s->type != ')') {
        die("Unexpected args past %d in macro %s", arg + extra_args_num, d->name);
    }

    // perform macro replacement
    int i = 0, j = 0, k = 0;
    char *substituted_replace = subst_tokens(d->replace, defs, d->name);
    const size_t sr_len = strlen(substituted_replace);
    token_state sub_ts = {0};
    while (!LINE_DONE(&sub_ts, substituted_replace)) {
        get_token(substituted_replace, sr_len, &sub_ts);
        const char *w = sub_ts.tok;
        int matched = -1;

        // handle the "#" operator
        if (strcmp(w, "#") == 0) {
            skip_ws(&sub_ts);
            const char *ww = sub_ts.tok;
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
            const char *ww = sub_ts.tok;
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

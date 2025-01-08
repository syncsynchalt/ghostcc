#include "pp_macro.h"
#include <string.h>
#include <ctype.h>
#include "die.h"
#include "lex.h"

static void add_to_out(const char *s, char **out, size_t *ind, size_t *sz)
{
    const size_t slen = strlen(s);
    while (slen + *ind >= *sz) {
        *sz = *sz ? 128 : *sz * 2;
        *out = realloc(*out, *sz);
    }
    strcpy(*out + *ind, s);
    *ind += slen;
}

static int skip_ws(token_state *s)
{
    while (s->ind < s->_line_len) {
        get_token(s->_line, s->_line_len, s);
        if (s->type != TOK_WS) {
            break;
        }
    }
    return s->ind < s->_line_len;
}

void handle_macro(const def *d, token_state *s, char **out, size_t *ind, size_t *sz)
{
    char **extra_args = NULL;
    int extra_args_num = 0;

    // make room for tracking replacement value of each arg
    int num_args;
    for (num_args = 0; d->args[num_args]; ++num_args) {}
    char **vals = malloc(num_args * sizeof(char *));

    if (num_args && strcmp(d->args[num_args-1], "...") == 0) {
        num_args--;
        extra_args = calloc(1, (extra_args_num + 5) * sizeof(*extra_args));
    }

    // assign replacement value of each arg
    skip_ws(s);
    if (s->type != '(') {
        die("Macro args for macro %s missing", d->name);
    }
    skip_ws(s);
    int arg = 0;
    for (arg = 0; arg < num_args; ++arg) {
        vals[arg] = strdup(s->tok);
        skip_ws(s);
        if (s->type != ',' && s->type != ')') {
            die("Unexpected token %s in macro %s arg %d. line: %s", s->tok, d->name, arg, s->_line);
        }
        if (s->type == ')' && arg+1 != num_args) {
            die("Unexpected end of args in macro %s. line: %s", d->name, s->_line);
        }
        if (s->type != ')') {
            skip_ws(s);
        }
    }

    if (extra_args && s->type != ')') {
        for (;;) {
            if (extra_args_num && extra_args_num % 5 == 0) {
                extra_args = realloc(extra_args, (extra_args_num + 5) * sizeof(*extra_args));
            }
            extra_args[extra_args_num++] = strdup(s->tok);
            skip_ws(s);
            if (s->type == ')' || s->ind >= s->_line_len) {
                break;
            }
            if (s->type != ',') {
                die("Unexpected token %s in macro %s arg %d. line: %s",
                    s->tok, d->name, arg + extra_args_num, s->_line);
            }
            skip_ws(s);
        }
    }

    if (s->type != ')') {
        die("Unexpected args past %d in macro %s. line: %s", arg + extra_args_num, d->name, s->_line);
    }

    // perform macro replacement
    int matched;
    int i = 0, j = 0, k = 0;
    for (i = 0; d->replace[i]; i++) {
        const char *w = d->replace[i];
        char *ww, *www;
        matched = -1;

        // handle the "#" operator
        if (strcmp(w, "#") == 0) {
            // find next non-space token
            for (k = i+1; d->replace[k]; k++) {
                if (!isspace(d->replace[k][0])) {
                    break;
                }
            }
            ww = d->replace[k];
            if (!ww) {
                die("String operator not followed by arg");
            }
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
            www = quote_str(vals[matched]);
            add_to_out(www, out, ind, sz);
            free(www);
            i = k;
            continue;
        }

        // handle the "##" operator
        // todo - "The resulting token is available for further macro replacement"
        if (strcmp(w, "##") == 0) {
            // delete trailing whitespace in output
            while (ind > 0 && isspace((*out)[*ind-1])) {
                (*ind)--;
            }

            // find next non-space token
            for (k = i+1; d->replace[k]; k++) {
                if (!isspace(d->replace[k][0])) {
                    break;
                }
            }
            ww = d->replace[k];
            if (!ww) {
                die("Concat operator not followed by arg");
            }

            // check if next token is an arg
            for (j = 0; j < num_args; ++j) {
                if (strcmp(ww, d->args[j]) == 0) {
                    matched = j;
                }
            }
            if (matched >= 0) {
                // next token is a macro arg, add it to output (concatenated with previous output)
                add_to_out(vals[matched], out, ind, sz);
            } else {
                // otherwise add the next token as-is (concatenated with previous output)
                add_to_out(ww, out, ind, sz);
            }
            i = k;
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
            add_to_out(vals[matched], out, ind, sz);
        } else if (extra_args && strcmp(w, "__VA_ARGS__") == 0) {
            // if token is "__VA_ARGS__", print the extra args (comma separated)
            for (k = 0; k < extra_args_num; ++k) {
                add_to_out(extra_args[k], out, ind, sz);
                if (k+1 < extra_args_num) {
                    add_to_out(", ", out, ind, sz);
                }
            }
        } else {
            // else pass the token along as-is
            add_to_out(w, out, ind, sz);
        }
    }

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

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

static int find_match_index(const char *lookup, char **options)
{
    int i;
    for (i = 0; options[i]; i++) {
        if (strcmp(lookup, options[i]) == 0) {
            return i;
        }
    }
    return -1;
}

static char **make_token_list(const char *s)
{
    size_t token_count = 0;
    char **tokens = malloc(sizeof(char *));
    token_state ts;
    set_token_string(&ts, s);
    while (!TOKEN_STATE_DONE(&ts)) {
        const token t = get_token(&ts);
        if (t.type != EOF) {
            if (token_count % 10 == 0) {
                tokens = realloc(tokens, (token_count + 11) * sizeof(char *));
            }
            tokens[token_count++] = strdup(t.tok);
        }
    }
    tokens[token_count] = NULL;
    return tokens;
}

static void free_string_list(char **tokens)
{
    if (!tokens) {
        return;
    }
    int i;
    for (i = 0; tokens[i]; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

static char *stringify(const char *replacement, char **args, char **params, char **expanded_params)
{
    token_state ts;
    set_token_string(&ts, replacement);
    str_t result = {0};

    while (!TOKEN_STATE_DONE(&ts)) {
        token t = get_token(&ts);
        if (t.type == '#') {
            // find next token, after whitespace
            do {
                t = get_token(&ts);
            } while (t.type == TOK_WS);
            const int ind = find_match_index(t.tok, args);
            if (ind < 0) {
                die("Couldn't find param after # operator");
            }
            char *tmp = quote_str(params[ind]);
            add_to_str(&result, tmp);
            free(tmp);
        } else {
            add_to_str(&result, t.tok);
        }
    }
    return result.s;
}

#define NOT_WHITESPACE(tok) (*(tok + strspn(tok, WHITESPACE)))

static char *concatenate(const char *replacement, char **args, char **params, char **expanded_params)
{
    char **tokens = make_token_list(replacement);
    int last_non_ws = -1;
    int next_non_ws = -1;
    int i, j, k;
    for (i = 0; tokens[i]; i++) {
        if (strcmp(tokens[i], "##") == 0) {
            if (last_non_ws < 0) {
                die("No token found before ## operator");
            }
            for (j = i; tokens[j]; j++) {
                if (NOT_WHITESPACE(tokens[j])) {
                    next_non_ws = j;
                }
            }
            if (next_non_ws < 0) {
                die("No token found after ## operator");
            }
            str_t concat_value = {0};
            int ind = find_match_index(tokens[last_non_ws], args);
            if (ind >= 0) {
                add_to_str(&concat_value, params[ind]);
            } else {
                add_to_str(&concat_value, tokens[last_non_ws]);
            }
            ind = find_match_index(tokens[next_non_ws], args);
            if (ind >= 0) {
                add_to_str(&concat_value, params[ind]);
            } else {
                add_to_str(&concat_value, tokens[next_non_ws]);
            }

            // replace tokens from last...next with concat_value
            for (j = last_non_ws; j <= next_non_ws; j++) {
                free(tokens[j]);
            }
            tokens[last_non_ws] = concat_value.s;
            for (j = last_non_ws + 1, k = next_non_ws + 1;; j++, k++) {
                tokens[j] = tokens[k];
                if (!tokens[k]) {
                    break;
                }
            }
        } else if (NOT_WHITESPACE(tokens[i])) {
            last_non_ws = i;
        }
    }

    str_t result = {0};
    for (i = 0; tokens[i]; i++) {
        add_to_str(&result, tokens[i]);
    }
    free_string_list(tokens);
    return result.s;
}

static int handle_object_macro(const def *d, const defines *defs, token_state *ts, str_t *out)
{
    char *s = subst_tokens(d->replace, defs, d->name);
    add_to_str(out, s);
    free(s);
    return 1;
}

int handle_macro(const def *d, const defines *defs, token_state *ts, str_t *out)
{
    char **extra_args = NULL;
    int extra_args_num = 0;

    if (!d->args) {
        // an "object-like" macro vs a "function-like" macro
        return handle_object_macro(d, defs, ts, out);
    }

    // ensure we're looking at a macro
    str_t check_keep = {0};
    token t = get_token(ts);
    while (t.type == TOK_WS) {
        add_to_str(&check_keep, t.tok);
    }
    if (t.type != '(') {
        // Mis-fire, this is not a macro call. Substitute the string as-is.
        if (check_keep.s) {
            push_back_token_data(ts, check_keep.s);
        }
        free_str(&check_keep);
        add_to_str(out, d->name);
        return 0;
    }
    free_str(&check_keep);

    // get the count of args
    int num_args;
    for (num_args = 0; d->args[num_args]; ++num_args) {}
    if (num_args && strcmp(d->args[num_args-1], "...") == 0) {
        num_args--;
        extra_args = calloc(1, (extra_args_num + 6) * sizeof(*extra_args));
    }

    // make the list of params (before macro-replacement)
    char **params = malloc((num_args + 1) * sizeof(char *));
    int arg;
    for (arg = 0; arg < num_args; ++arg) {
        t = skip_ws(ts);
        str_t tmp = read_arg(t, ts);
        if (!tmp.s) {
            die("Missing arg %d in macro %s", arg+1, d->name);
        }
        trim_trailing_whitespace(&tmp);
        params[arg] = tmp.s;
        if (ts->last.type != ',' && ts->last.type != ')') {
            die("Unexpected end of macro %s after arg %d", d->name, arg+1);
        }
        if (ts->last.type == ')' && arg+1 != num_args) {
            die("Unexpected end of args in macro %s", d->name);
        }
    }
    params[num_args] = NULL;

    // make the list of macro-expanded params
    char **expanded_params = malloc((num_args + 1) * sizeof(char *));
    for (arg = 0; params[arg]; ++arg) {
        expanded_params[arg] = subst_tokens(params[arg], defs, d->name);
    }
    expanded_params[num_args] = NULL;

    if (extra_args && ts->last.type != ')') {
        t = skip_ws(ts);
        for (;;) {
            if (extra_args_num && extra_args_num % 5 == 0) {
                extra_args = realloc(extra_args, (extra_args_num + 6) * sizeof(*extra_args));
            }
            const str_t tmp = read_arg(t, ts);
            extra_args[extra_args_num++] = tmp.s;
            if (ts->last.type == ')' || TOKEN_STATE_DONE(ts)) {
                break;
            }
            extra_args[extra_args_num++] = strdup(ts->last.tok);
            t = get_token(ts);
        }
        extra_args[extra_args_num] = NULL;
    }

    if (ts->last.type != ')') {
        if (TOKEN_STATE_DONE(ts)) {
            die("Unexpected end of args in macro %s", d->name);
        }
        die("Unexpected args past %d in macro %s", arg + extra_args_num, d->name);
    }

    // stringify and concatenate
    char *stringified = stringify(d->replace, d->args, params, expanded_params);
    char *concatenated = concatenate(stringified, d->args, params, expanded_params);
    free(stringified);

    // perform arg replacement
    int i = 0;
    token_state sub_ts = {0};
    str_t interim_result = {0};
    set_token_string(&sub_ts, concatenated);
    while (!TOKEN_STATE_DONE(&sub_ts)) {
        const token tt = get_token(&sub_ts);
        const char *w = tt.tok;
        const int matched = find_match_index(tt.tok, d->args);
        if (matched >= 0) {
            // if token matched against an arg name, print arg's replacement value
            add_to_str(&interim_result, expanded_params[matched]);
        } else if (extra_args && strcmp(w, "__VA_ARGS__") == 0) {
            // if token is "__VA_ARGS__", print the extra args (comma separated)
            for (i = 0; i < extra_args_num; ++i) {
                add_to_str(&interim_result, extra_args[i]);
            }
        } else {
            // else pass the token along as-is
            add_to_str(&interim_result, w);
        }
    }

    char *s = subst_tokens(interim_result.s, defs, d->name);
    free_str(&interim_result);
    add_to_str(out, s);
    free(s);

    // free allocated memory
    free(concatenated);
    free_string_list(params);
    free_string_list(expanded_params);
    free_string_list(extra_args);
    return 1;
}

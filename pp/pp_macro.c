#include "pp_macro.h"
#include <string.h>
#include "die.h"

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
    char *free_buf = NULL;
    size_t free_buf_ind = 0;
    size_t free_buf_sz = 0;
    if (!out) {
        // alloc our own work buffer
        free_buf_sz = 128;
        free_buf = malloc(free_buf_sz);
        out = &free_buf;
        ind = &free_buf_ind;
        sz = &free_buf_sz;
    }

    // make room for tracking replacement value of each arg
    int num_args;
    for (num_args = 0; d->args[num_args]; ++num_args) {}
    char **vals = malloc(num_args * sizeof(char *));

    // todo - handle `...`

    // assign replacement value of each arg
    skip_ws(s);
    if (s->type != '(') {
        die("Macro args for macro %s missing", d->name); exit(1);
    }
    skip_ws(s);
    int i = 0;
    for (i = 0; i < num_args; ++i) {
        vals[i] = strdup(s->tok);
        skip_ws(s);
        if (s->type != ',' && s->type != ')') {
            die("Unexpected token %s in macro %s arg %d. line: %s", s->tok, d->name, i, s->_line); exit(1);
        }
        if (s->type == ')' && i+1 != num_args) {
            die("Unexpected end of args in macro %s. line: %s", d->name, s->_line); exit(1);
        }
        if (s->type != ')') {
            skip_ws(s);
        }
    }
    if (s->type != ')') {
        die("Unexpected args past %d in macro %s. line: %s", i, d->name, s->_line); exit(1);
    }

    // perform macro replacement
    int matched;
    int j = 0;
    for (i = 0; d->replace[i]; i++) {
        const char *w = d->replace[i];
        matched = -1;
        // check if this is an arg name
        for (j = 0; j < num_args; ++j) {
            if (strcmp(w, d->args[j]) == 0) {
                matched = j;
            }
        }
        // if it's an arg print replacement value, else print token as-is
        if (matched >= 0) {
            add_to_out(vals[matched], out, ind, sz);
        } else {
            add_to_out(w, out, ind, sz);
        }
    }

    // free allocated memory
    for (num_args = 0; d->args[num_args]; ++num_args) {
        free(vals[num_args]);
    }
    free(vals);
    free(free_buf);
}

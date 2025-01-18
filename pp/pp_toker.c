#include "pp_toker.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"

#include "lex.h"
#include "die.h"

static void read_full_directive(token_state *ts);

void set_token_string(token_state *ts, const char *s)
{
    ts->line = s;
    ts->ind = 0;
    ts->end = strlen(ts->line);
    ts->line_sz = -1; // not memory-managed
    ts->f = NULL;
    ts->filename = NULL;
    ts->line_is_directive = 0;
    strcpy(ts->token_buf, "");
    ts->unget_ind = 0;
    ts->unget_buf = NULL;
}

void set_token_file(token_state *ts, FILE *f, const char *filename)
{
    ts->line_sz = 128;
    ts->line = calloc(1, ts->line_sz);
    ts->ind = 0;
    ts->end = 0;
    ts->f = f;
    ts->filename = filename;
    ts->line_is_directive = 0;
    strcpy(ts->token_buf, "");
    ts->unget_ind = 0;
    ts->unget_buf = NULL;
}

static void read_line_from_file(token_state *ts)
{
    TOKEN_STATE_CHECK_ALLOC(ts);
    if (getline((char **)&ts->line, &ts->line_sz, ts->f) < 0) {
        if (feof(ts->f)) {
            ts->end = ts->ind = 0;
            return;
        }
        die("reading input file: %s", strerror(errno));
    }
    current_lineno++;
    current_line = ts->line;
    ts->end = strlen(ts->line);
    ts->ind = 0;
    if (ts->line[strspn(ts->line, " \t")] == '#') {
        ts->line_is_directive = 1;
        read_full_directive(ts);
    } else {
        ts->line_is_directive = 0;
    }
}

static int token_string_getc(token_state *ts)
{
    if (ts->unget_ind) {
        const char c = ts->unget_buf[--ts->unget_ind];
        if (!ts->unget_ind) {
            free(ts->unget_buf);
        }
        return c;
    }

    if (ts->ind >= ts->end) {
        if (ts->f) {
            if (feof(ts->f)) {
                return EOF;
            }
            read_line_from_file(ts);
        } else {
            return EOF;
        }
    }
    return ts->line[ts->ind++];
}

static int token_string_ungetc(int c, token_state *ts)
{
    if (c == EOF) {
        return EOF;
    }
    if (ts->ind && ts->line[ts->ind - 1] == c) {
        ts->ind--;
    } else {
        const char buf[2] = { c, '\0' };
        push_back_token_data(ts, buf);
    }
    return 0;
}

token get_token(token_state *ts)
{
    ts->last = read_token((int (*)(void *))token_string_getc, (int (*)(int, void *))token_string_ungetc,
                         ts, ts->token_buf);
    return ts->last;
}

void push_back_token_data(token_state *ts, const char *data)
{
    if (!data) {
        return;
    }

    const size_t len = strlen(data);
    if (ts->unget_ind) {
        ts->unget_buf = realloc(ts->unget_buf, ts->unget_ind + len + 1);
    } else {
        ts->unget_buf = malloc(len + 1);
    }
    int i;
    for (i = len-1; i >= 0; i--) {
        ts->unget_buf[ts->unget_ind++] = data[i];
    }
    ts->unget_buf[ts->unget_ind] = '\0';
}

static int directive_continues(const token_state *ts)
{
    const char *last_bs = strrchr(ts->line, '\\');
    return last_bs && *(last_bs + 1 + strspn(last_bs + 1, "\r\n")) == '\0';
}

static void read_full_directive(token_state *ts)
{
    int multiline = 0;
    while (directive_continues(ts)) {
        char *last_bs = strrchr(ts->line, '\\');
        *last_bs = '\0';
        ts->end = last_bs - ts->line;

        char *l = NULL;
        size_t ll = 0;
        if (getline(&l, &ll, ts->f) < 0) {
            break;
        }
        current_lineno++;
        multiline++;
        if (ts->line_sz < strlen(ts->line) + strlen(l) + 1) {
            TOKEN_STATE_CHECK_ALLOC(ts);
            ts->line_sz = strlen(ts->line) + strlen(l) + 1;
            ts->line = realloc((char *)ts->line, ts->line_sz);
        }
        strcpy((char *)ts->line + ts->end, l);
        ts->end += strlen(l);
        free(l);
    }
    while (ts->end && strchr("\r\n", ts->line[ts->end-1])) {
        TOKEN_STATE_CHECK_ALLOC(ts);
        ((char *)ts->line)[--ts->end] = '\0';
    }
    if (multiline && output_active && output) {
        fprintf(output, "# %d \"%s\"\n", current_lineno, current_file);
    }
}

void skip_line(token_state *ts)
{
    if (ts->f) {
        read_line_from_file(ts);
    } else {
        ts->ind = ts->end;
    }
}

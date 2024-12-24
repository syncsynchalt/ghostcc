#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "die.h"
#include "lex.h"

static void process_directive(const char *line, size_t line_len, FILE *in, FILE *out, const defines *defs,
                              const char * const *include_paths);
static void process_tokens(const char *line, size_t line_len, FILE *out, const defines *defs);

void parse(FILE *in, FILE *out, const defines *defs, const char * const *include_paths)
{
    char *line = NULL;
    size_t linecap;
    int len;

    while ((len = getline(&line, &linecap, in)) > 0) {
        if (line[0] == '#') {
            process_directive(line, len, in, out, defs, include_paths);
        } else {
            process_tokens(line, len, out, defs);
        }
    }
    if (ferror(in)) {
        die("reading input file");
    }
}

static int resolve_include_path(char *filename, const size_t len, const char * const *include_paths)
{
    char buf[256];
    int i;
    if (filename[0] == '/' && access(filename, F_OK) == 0) {
        return 1;
    }
    for (i = 0; include_paths[i]; i++) {
        snprintf(buf, sizeof(buf), "%s/%s", include_paths[i], filename);
        if (access(buf, F_OK) == 0) {
            snprintf(filename, len, "%s", buf);
            return 1;
        }
    }
    return 0;
}

static void process_include(const char *line, const size_t line_len, FILE *out, const defines *defs,
                            const char * const *include_paths)
{
    token_state s = {};
    char filename[256] = {};

    get_token(line, line_len, &s);
    if (s.type == TOK_STR) {
        if (!decode_str(s.tok, filename, sizeof(filename))) {
            die("Unable to decode filename %s", s.tok);
        }
    } else if (s.type == TOK_LT) {
        for (;;) {
            const int eol = get_token(line, line_len, &s);
            if (s.type == TOK_GT) {
                break;
            }
            snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename), "%s", s.tok);
            if (eol) {
                return die("#define unterminated filename %s", line);
            }
        }
    } else {
        return die("#define unrecognized filename %s", line);
    }

    if (!resolve_include_path(filename, sizeof(filename), include_paths)) {
        return die("file %s not found in include paths", filename);
    }
    FILE *in = fopen(filename, "r");
    parse(in, out, defs, include_paths);
}

static int line_continues(const char *line)
{
    const char *last_bs = strrchr(line, '\\');
    return last_bs && *(last_bs+1 + strspn(last_bs+1, "\r\n")) == '\0';
}

static void strip_continue(char *line)
{
    char *last_bs = strrchr(line, '\\');
    if (last_bs) {
        *last_bs = '\0';
    }
}

static void process_define(const char *line, const size_t line_len, FILE *in, const defines *defs)
{
    char *name = NULL;
    char *args = NULL;
    char *replace = NULL;

    token_state s = {};
    get_token(line, line_len, &s);
    if (s.type != TOK_ID && s.type != TOK_KEYWORD) {
        return die("#define must be an identifier: %s", line);
    }
    name = strdup(s.tok);

    const char *p = line + s.ind;
    if (*p == '(') {
        const char *q = strchr(p, ')');
        if (!q) {
            return die("#define missing closing parens: %s", line);
        }
        args = malloc(q - p);
        strncpy(args, p + 1, q - p - 1);
    }
    const int len = strspn(p, " \t");
    p += len;

    replace = strdup(p);
    while (line_continues(replace)) {
        strip_continue(replace);
        char *l = NULL;
        size_t ll = 0;
        if (getline(&l, &ll, in) < 0) {
            break;
        }
        replace = realloc(replace, strlen(replace) + strlen(l) + 1);
        strcat(replace, l);
        free(l);
    }
    while (strlen(replace) && replace[strlen(replace)-1] == '\r' || replace[strlen(replace)-1] == '\n') {
        replace[strlen(replace)-1] = '\0';
    }

    defines_add(defs, name, args, replace);
    free(name);
    free(args);
    free(replace);
}

static void process_directive(const char *line, const size_t line_len, FILE *in, FILE *out, const defines *defs,
                              const char * const *include_paths)
{
    char cmd[64];
    const char *w = line + 1;
    w += strspn(w, " \t");
    const int len = strspn(w, LOWER);
    snprintf(cmd, sizeof(cmd), "%.*s", len, w);
    w += len;
    w += strspn(w, " \t");

    if (strcmp(cmd, "include") == 0) {
        process_include(w, strlen(w), out, defs, include_paths);
    } else if (strcmp(cmd, "define") == 0) {
        process_define(w, strlen(w), in, defs);
    } else {
        fprintf(stderr, "Warning: unrecognized directive %s\n", cmd);
    }
}

static void process_tokens(const char *line, const size_t line_len, FILE *out, const defines *defs)
{
    token_state s = {};
    int eol = 0;
    while (!eol) {
        eol = get_token(line, line_len, &s);
        const def *d = defines_get(defs, s.tok);
        if (d) {
            // todo handle macros
            int i = 0;
            while (d->replace && d->replace[i]) {
                fprintf(out, "%s", d->replace[i++]);
            }
        } else {
            fprintf(out, "%s", s.tok);
        }
    }
}

#include "preprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ast.h"
#include "common.h"
#include "die.h"
#include "lex.h"
#include "pp_ast.h"
#include "pp_macro.h"

static void process_directive(const char *line, size_t line_len, FILE *in, parse_state *state);
static void process_tokens(const char *line, size_t line_len, parse_state *state);

// ReSharper disable CppParameterMayBeConstPtrOrRef

void parse(const char *filename, FILE *in, FILE *out, parse_state *existing_state)
{
    char *line = NULL;
    size_t linecap;
    int len;

    parse_state ps = {0};
    ps.defs = existing_state ? existing_state->defs : NULL;
    ps.include_paths = existing_state ? existing_state->include_paths : NULL;
    ps.out = out;
    ps.once_filenames =
            existing_state && existing_state->once_filenames ? existing_state->once_filenames : hashmap_init(32);
    ps.current_filename = filename;

    while ((len = getline(&line, &linecap, in)) > 0) {
        current_lineno++;
        current_line = line;
        if (line[0] == '#') {
            process_directive(line, len, in, &ps);
        } else {
            if (OUTPUT_VISIBLE(&ps)) {
                LINE_RESET(&ps.ts);
                process_tokens(line, len, &ps);
            }
        }
    }
    current_line = NULL;
    if (ferror(in)) {
        die("reading input file");
    }

    if (ps.top_if) {
        die("Missing #endif in %s", filename);
    }
}

static int resolve_include_path(char *filename, const size_t len, const char *const *include_paths)
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

static int resolve_condition(const char *condition, parse_state *state)
{
    const ast_node *node = pp_parse(condition, state->defs);
    const ast_result pp_result = pp_resolve_ast(node);
    switch (pp_result.type) {
        case AST_RESULT_TYPE_INT:
        default:
            return !!pp_result.ival;
        case AST_RESULT_TYPE_FLT:
            return !!pp_result.fval;
        case AST_RESULT_TYPE_STR:
            return !!strlen(pp_result.sval);
    }
}

static int line_continues(const char *line)
{
    const char *last_bs = strrchr(line, '\\');
    return last_bs && *(last_bs + 1 + strspn(last_bs + 1, "\r\n")) == '\0';
}

static void strip_continue(const char *line)
{
    char *last_bs = strrchr(line, '\\');
    if (last_bs) {
        *last_bs = '\0';
    }
}

static char *read_full_line(const char *line, FILE *in)
{
    char *result;
    result = strdup(line);
    while (line_continues(result)) {
        strip_continue(result);
        char *l = NULL;
        size_t ll = 0;
        if (getline(&l, &ll, in) < 0) {
            break;
        }
        result = realloc(result, strlen(result) + strlen(l) + 1);
        strcat(result, l);
        free(l);
    }
    while (strlen(result) && result[strlen(result) - 1] == '\r' || result[strlen(result) - 1] == '\n') {
        result[strlen(result) - 1] = '\0';
    }
    return result;
}

static void add_if_frame(parse_state *state)
{
    struct if_frame *new_if_frame = calloc(1, sizeof *new_if_frame);
    new_if_frame->masked = state->top_if && !state->top_if->is_true;
    new_if_frame->next = state->top_if;
    state->top_if = new_if_frame;
}

static void mark_true(parse_state *state, const int truth)
{
    state->top_if->is_true = 0;
    if (!state->top_if->truth_found) {
        state->top_if->is_true = truth;
        state->top_if->truth_found = truth;
    }
}

static void handle_ifdef(const char *line, const size_t line_len, parse_state *state, const int invert)
{
    if (state->top_if->masked) {
        return;
    }
    token_state s = {};
    get_token(line, line_len, &s);
    const int defined = defines_get(state->defs, s.tok) ? 1 : 0;
    mark_true(state, invert ? !defined : defined);
}

static void handle_if(const char *line, FILE *in, parse_state *state)
{
    if (state->top_if->masked) {
        return;
    }
    const char *condition = read_full_line(line, in);
    mark_true(state, resolve_condition(condition, state));
}

static void handle_else(parse_state *state)
{
    if (state->top_if->masked) {
        return;
    }
    mark_true(state, 1);
}

static void handle_endif(parse_state *state)
{
    if (!state->top_if) {
        die("#endif unmatched");
    }
    struct if_frame *i = state->top_if;
    state->top_if = state->top_if->next;
    free(i);
}

static void handle_include(const char *line, const size_t line_len, parse_state *state)
{
    token_state s = {};
    char filename[256] = {};

    get_token(line, line_len, &s);
    if (s.type == TOK_STR) {
        if (!decode_str(s.tok, filename, sizeof(filename))) {
            die("Unable to decode filename %s", s.tok);
        }
    } else if (s.type == '<') {
        for (;;) {
            const int cont = get_token(line, line_len, &s);
            if (s.type == '>') {
                break;
            }
            snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename), "%s", s.tok);
            if (!cont) {
                die("#define unterminated filename %s", line);
            }
        }
    } else {
        die("#define unrecognized filename %s", line);
    }

    if (!resolve_include_path(filename, sizeof(filename), state->include_paths)) {
        die("file %s not found in include paths", filename);
    }
    const char *existing_file = current_file;
    const int existing_line = current_lineno;
    FILE *in = fopen(filename, "r");
    // ReSharper disable once CppDFALocalValueEscapesFunction
    current_file = filename;
    current_lineno = 0;
    parse(filename, in, state->out, state);
    current_file = existing_file;
    current_lineno = existing_line;
}

int span_parens(const char *p)
{
    int i = 0;
    int parens_count = 0;
    for (i = 0; p[i] != '\0'; i++) {
        if (p[i] == '(') {
            parens_count++;
        }
        if (p[i] == ')') {
            parens_count--;
            if (parens_count == 0) {
                return i;
            }
        }
    }
    return i;
}

static void handle_define(const char *line, const size_t line_len, FILE *in, const parse_state *state)
{
    char *name = NULL;
    char *args = NULL;

    token_state s = {};
    get_token(line, line_len, &s);
    if (s.type != TOK_ID && !IS_KEYWORD(s.type)) {
        die("#define must be an identifier: %s", line);
    }
    name = strdup(s.tok);

    const char *p = line + s.ind;
    if (*p == '(') {
        // todo handle parens in macro args
        const char *q = p + span_parens(p);
        if (*q != ')') {
            die("#define missing closing parens: %s", line);
        }
        args = malloc(q - p);
        strncpy(args, p + 1, q - p - 1);
        p = q + 1;
    }
    p += strspn(p, " \t");

    char *full_line = read_full_line(p, in);

    defines_add(state->defs, name, args, full_line);
    free(name);
    free(args);
    free(full_line);
}

static void handle_pragma(const char *line, FILE *in, const parse_state *state)
{
    char buf[16];
    const char *end = line + strcspn(line, WHITESPACE);
    snprintf(buf, sizeof(buf), "%.*s", (int) (end - line), line);
    if (strcmp(buf, "once") == 0) {
        if (hashmap_get(state->once_filenames, state->current_filename)) {
            fseek(in, 0, SEEK_END);
        } else {
            const char *key = strdup(state->current_filename);
            hashmap_add(state->once_filenames, key, key);
        }
    } else {
        fprintf(stderr, "Warning: unrecognized pragma %s\n", buf);
    }
}

static void process_directive(const char *line, const size_t line_len, FILE *in, parse_state *state)
{
    char cmd[64];
    const char *w = line + 1;
    w += strspn(w, " \t");
    const int len = strspn(w, LOWER);
    snprintf(cmd, sizeof(cmd), "%.*s", len, w);
    w += len;
    w += strspn(w, " \t");

    if (strcmp(cmd, "ifdef") == 0) {
        add_if_frame(state);
        handle_ifdef(w, strlen(w), state, 0);
    } else if (strcmp(cmd, "ifndef") == 0) {
        add_if_frame(state);
        handle_ifdef(w, strlen(w), state, 1);
    } else if (strcmp(cmd, "endif") == 0) {
        handle_endif(state);
    } else if (strcmp(cmd, "include") == 0) {
        if (OUTPUT_VISIBLE(state)) {
            handle_include(w, strlen(w), state);
        }
    } else if (strcmp(cmd, "define") == 0) {
        if (OUTPUT_VISIBLE(state)) {
            handle_define(w, strlen(w), in, state);
        }
    } else if (strcmp(cmd, "if") == 0) {
        add_if_frame(state);
        handle_if(w, in, state);
    } else if (strcmp(cmd, "elif") == 0) {
        handle_if(w, in, state);
    } else if (strcmp(cmd, "else") == 0) {
        handle_else(state);
    } else if (strcmp(cmd, "pragma") == 0) {
        handle_pragma(w, in, state);
    } else if (OUTPUT_VISIBLE(state)) {
        fprintf(stderr, "Warning: unrecognized directive %s\n", cmd);
    }
}

static void process_tokens(const char *line, const size_t line_len, parse_state *state)
{
    token_state *s = &state->ts;

    int cont = 1;
    while (cont) {
        cont = get_token(line, line_len, s);
        const def *d = defines_get(state->defs, s->tok);
        if (d) {
            if (d->args) {
                size_t ind = 0;
                size_t sz = 128;
                char *buf = calloc(1, sz);
                handle_macro(d, s, &buf, &ind, &sz);
                fprintf(state->out, "%s", buf);
                free(buf);
            } else {
                int i = 0;
                while (d->replace && d->replace[i]) {
                    fprintf(state->out, "%s", d->replace[i++]);
                }
            }
        } else {
            fprintf(state->out, "%s", s->tok);
        }
    }
}

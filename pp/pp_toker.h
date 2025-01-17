#pragma once

#include "lex.h"
#include <stdio.h>

typedef struct {
    const char *line;
    size_t ind;
    size_t end;
    size_t line_sz;
    token last;
    char token_buf[TOKEN_BUF_SZ];
    FILE *f;
    const char *filename;
    int line_is_directive;
} token_state;

#define TOKEN_STATE_DONE(tss) ((tss)->ind >= (tss)->end)
#define TOKEN_STATE_DIRECTIVE(tss) ((tss)->line_is_directive)
#define TOKEN_STATE_CHECK_ALLOC(tss) if ((tss)->line_sz < 0) die("Can't alloc static line")
#define DIRECTIVE_SIGIL '\0'

/// set up the token state to consume given string
extern void set_token_string(token_state *ts, const char *s);

/// set up the token state to consume given file
extern void set_token_file(token_state *ts, FILE *f, const char *filename);

/// clear the directive line and mark the directive as handled
extern void clear_directive(token_state *ts);

/// discard the current line and grab the next
extern void skip_line(token_state *ts);

extern token get_token(token_state *ts);

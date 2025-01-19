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

    size_t unget_ind;
    char *unget_buf;

} token_state;

#define TOKEN_STATE_DONE(tss) ((tss)->ind >= (tss)->end && !(tss)->unget_ind)
#define TOKEN_STATE_DIRECTIVE(tss) ((tss)->line_is_directive)
#define TOKEN_STATE_CHECK_ALLOC(tss) if ((tss)->line_sz < 0) die("Can't alloc static line")
#define TOKEN_STATE_READING_IGNORED(tss) ((tss)->unget_ind != 0)

/// set up the token state to consume given string
extern void set_token_string(token_state *ts, const char *s);

/// set up the token state to consume given file
extern void set_token_file(token_state *ts, FILE *f, const char *filename);

/// clear the directive line and mark the directive as handled
extern void clear_directive(token_state *ts);

/// discard the current line and grab the next
extern void skip_line(token_state *ts);

/// get the next token from the token stream (file or string)
extern token get_token(token_state *ts);

/// add data to the head of the token stream (i.e. for un-getting a token)
extern void push_back_token_data(token_state *ts, const char *data);
// xxx test this

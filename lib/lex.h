#pragma once

#include <stdlib.h>
#include "token.h"

typedef struct {
    token_type type; ///< parsed token type
    char *tok; ///< parsed token as string
    size_t pos; ///< offset into line that token starts at
    size_t ind; ///< start of next token

    int _tok_max;
    const char *_line;
    size_t _line_len;
    int _comment_level;
} token_state;

#define LINE_RESET(ts) do { (ts)->_line = 0; (ts)->_line_len = 0; (ts)->ind = 0; } while(0)
#define LINE_DONE(ts, line) ((ts)->_line == line && (ts)->ind >= (ts)->_line_len)

/**
 * get the next token from the given line
 *
 * @param line line of text currently being processed
 * @param line_len the length of line
 * @param token parsing state, allocated by caller and updated by get_token. Resets automatically every time `line`
 * changes.
 * @return 0 if end of line, else 1
 */
extern int get_token(const char *line, size_t line_len, token_state *token);

/**
 * Decode c-style double-quoted strings.
 * @param s string, including quotes.
 * @param out output buffer
 * @param len length of output buffer
 * @return pointer to out, or NULL on decode failure.
 */
extern char *decode_str(const char *s, char *out, size_t len);

/**
 * Encode c-style double-quoted string.
 * @param s input string
 * @return pointer to quoted string, must be free'd.
 */
extern char *quote_str(const char *s);

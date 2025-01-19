#pragma once

#include <stdlib.h>
#include "token.h"

typedef struct {
    token_type type; ///< parsed token type
    char *tok; ///< parsed token as string
} token;

#define TOKEN_BUF_SZ 256
extern token read_token(int (*getch)(void *param), int (*ungetch)(int c, void *param), void *param, char *token_buf);

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

#pragma once

#include "defs.h"
#include "lex.h"
#include "str.h"

/**
 * Resolve a #define macro.
 *
 * Given lexer state at the start of the macro, resolve macro arguments and perform
 * macro replacement, putting the result in `out`.
 *
 * @param d the #define macro in question
 * @param defs all known #defines
 * @param s the lexer state (includes the string we are parsing and current position)
 * @param out[in,out] the output string to add result to
 */
extern void handle_macro(const def *d, const defines *defs, token_state *s, str_t *out);

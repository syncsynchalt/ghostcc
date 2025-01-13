#pragma once

#include "defs.h"
#include "lex.h"
#include <stdlib.h>

/**
 * Resolve a #define macro.
 *
 * Given lexer state at the start of the macro, resolve macro arguments and perform
 * macro replacement, putting the result in `out`.
 *
 * @param d the #define macro
 * @param s the lexer state (includes the string we are parsing and current position)
 * @param out[in,out] the output to add result to (realloc'ed if necessary)
 * @param ind[in,out] the index of written output
 * @param sz[in,out] the allocated size of `out`
 */
extern void handle_macro(const def *d, token_state *s, char **out, size_t *ind, size_t *sz);

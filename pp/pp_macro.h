#pragma once

#include "defs.h"
#include "lex.h"
#include <stdlib.h>

extern void handle_macro(const def *d, token_state *s, char **out, size_t *ind, size_t *sz);

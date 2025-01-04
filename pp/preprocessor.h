#pragma once

#include <stdio.h>
#include "defs.h"

extern void parse(FILE *in, FILE *out, const defines *defs, const char * const *include_paths);

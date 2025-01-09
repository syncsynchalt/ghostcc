#pragma once

#include <stdio.h>
#include "defs.h"
#include "lex.h"

typedef struct {
  int if_level;
  int mask_level;
  defines *defs;
  const char * const *include_paths;
  FILE *out;
  const char *current_filename;
  hashmap *once_filenames;
  token_state ts;
} parse_state;

extern void parse(const char *filename, FILE *in, FILE *out, parse_state *existing_state);

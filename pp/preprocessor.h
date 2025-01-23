#pragma once

#include <stdio.h>
#include "defs.h"
#include "pp_toker.h"

struct if_frame {
  struct if_frame *next;
  int masked; // is the parent #if false
  int truth_found; // has any #if at this level been true?
  int is_true; // is the current #if at this level true?
};

typedef struct {
  struct if_frame *top_if;
  defines *defs;
  const char **include_paths;
  FILE *out;
  const char *current_filename;
  hashmap *once_filenames;
  token_state ts;
} parse_state;

extern void process_file(const char *filename, FILE *in, FILE *out, parse_state *existing_state);

#pragma once

#include <stdio.h>

extern void add_to_data_section(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
extern void print_data_section(FILE *out);

#pragma once

#define WHITESPACE " \t\v\r\n"
#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER "abcdefghijklmnopqrstuvwxyz"
#define NUMERIC "0123456789"
#define HEXNUM "0123456789abcdefABCDEF"

extern const char *current_file;
extern int current_lineno;
extern const char *current_line;

#include <stdio.h>
extern FILE *output;
extern int output_active;

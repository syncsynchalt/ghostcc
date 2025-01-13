#include "die.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

void die(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    printf("\n");
    fflush(stdout);
    fprintf(stderr, "\n\n\n---\nFatal error: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    if (current_line) {
        fprintf(stderr, "   at %s:%d\n", current_file, current_lineno);
        fprintf(stderr, "   line: %s\n", current_line);
    }
    exit(1);
}

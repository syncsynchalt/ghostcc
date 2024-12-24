#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void die(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, msg, ap);
    exit(1);
}

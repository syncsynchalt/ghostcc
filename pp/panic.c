#include <stdio.h>
#include <stdlib.h>

void panic(const char *reason)
{
    fprintf(stderr, "panic: %s\n", reason);
    exit(1);
}

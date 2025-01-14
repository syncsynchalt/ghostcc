#include "str.h"
#include <string.h>
#include <stdlib.h>

void add_to_str(str_t *str, const char *s)
{
    const unsigned int len = strlen(s);
    while (str->end + len >= str->sz) {
        str->sz = str->sz ? 2*str->sz : 128;
        str->s = realloc(str->s, str->sz);
    }
    strcpy(str->s + str->end, s);
    str->end += len;
}

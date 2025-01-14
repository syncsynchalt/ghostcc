#pragma once

typedef struct {
    char *s; ///< string result (must be freed by call to free())
    unsigned int end; ///< current end of string
    unsigned int sz; ///< size of allocated string memory
} str_t;

extern void add_to_str(str_t *str, const char *s);

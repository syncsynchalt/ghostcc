#pragma once

#include "hashmap.h"

typedef struct {
    char *name;
    char **args;
    char **replace;
} def;

typedef struct {
    hashmap *h;
} defines;

extern defines *defines_init(void);
extern void defines_destroy(defines *defs);
extern void defines_add(const defines *defs, const char *name, const char *args, const char *replace);
extern const def *defines_get(const defines *defs, const char *name);

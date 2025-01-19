#pragma once

#include "hashmap.h"

typedef struct {
    char *name; ///< the name of the #define
    char **args; ///< macro function arguments (or null for non-macro)
    char *replace; ///< replacement string for the #define (or null if no replacement)
    int ignored;
} def;

typedef struct {
    hashmap *h;
} defines;

/// initialize a list of defines
extern defines *defines_init(void);

/// destroy a list of defines
extern void defines_destroy(defines *defs);

/**
 * add a define to the list
 *
 * @param defs list of defines to add to
 * @param name the name of the define
 * @param[optional] args the define macro args, as a single string, without enclosing parens: "bar, bux"
 * @param[optional] replace the define replacement, as a single string
 */
extern void defines_add(const defines *defs, const char *name, const char *args, const char *replace);

/// get a define from the list, unless ignored flag is set
extern def *defines_get(const defines *defs, const char *name);

/// remove a define from the list
extern int defines_remove(const defines *defs, const char *name);

typedef struct {
    def **ignored;
    size_t count;
} ignore_list;

extern void clear_ignore_list(ignore_list *l);
extern void add_to_ignore_list(ignore_list *l, def *d);

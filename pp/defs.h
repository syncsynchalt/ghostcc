#ifndef INC_DEFS_H__
#define INC_DEFS_H__

typedef struct _def {
    char *name;
    char **args;
    char **replace;
} def;

typedef struct _defines {
    int cur;
    int max;
    def *defs;
} defines;

extern defines *init_defines(void);
extern void add_define(defines *defs, const char *name, const char *args, const char *replace);

#endif

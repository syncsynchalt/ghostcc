#include <gtest/gtest.h>
extern "C" {
#include "../defs.h"
}

TEST(DefsTest, InitDestroy)
{
    defines *d = defines_init();
    defines_destroy(d);
}

TEST(DefsTest, AddToken)
{
    defines *defs = defines_init();
    defines_add(defs, "FOO", NULL, NULL);
    const def *d = defines_get(defs, "FOO");
    ASSERT_TRUE(d);
    ASSERT_STREQ("FOO", d->name);
    ASSERT_EQ(NULL, d->args);
    ASSERT_EQ(NULL, d->replace);
    defines_destroy(defs);
}

TEST(DefsTest, DISABLED_AddSubst)
{
    defines *defs = defines_init();
    defines_add(defs, "FOO", NULL, "a b c");
    const def *d = defines_get(defs, "FOO");
    ASSERT_TRUE(d);
    ASSERT_STREQ("FOO", d->name);
    ASSERT_EQ(NULL, d->args);
    const char *expected[] = {"a", "b", "c", NULL};
    for (int i = 0; i < sizeof(expected) / sizeof(*expected); i++) {
        ASSERT_STREQ(expected[i], d->replace[i]);
    }
    defines_destroy(defs);
}

TEST(DefsTest, DISABLED_AddMacro)
{
    defines *defs = defines_init();
    defines_add(defs, "FOO", "a, b, c", "aa = a; bb = b;");
    const def *d = defines_get(defs, "FOO");
    ASSERT_TRUE(d);
    ASSERT_STREQ("FOO", d->name);
    const char *e_args[] = {"a", "b", "c", NULL};
    for (int i = 0; i < sizeof(e_args) / sizeof(*e_args); i++) {
        ASSERT_STREQ(e_args[i], d->args[i]);
    }
    const char *e_replace[] = {"aa", " ", "=", " ", "a", ";", "bb", " ", "=", " ", "b", ";", NULL};
    for (int i = 0; i < sizeof(e_replace) / sizeof(*e_replace); i++) {
        ASSERT_STREQ(e_replace[i], d->replace[i]);
    }

    defines_destroy(defs);
}

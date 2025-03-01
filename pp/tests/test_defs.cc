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

TEST(DefsTest, AddSubst)
{
    defines *defs = defines_init();
    defines_add(defs, "FOO", NULL, "a = b + c->d");
    const def *d = defines_get(defs, "FOO");
    ASSERT_TRUE(d);
    ASSERT_STREQ("FOO", d->name);
    ASSERT_EQ(NULL, d->args);
    ASSERT_STREQ("a = b + c->d", d->replace);
    defines_destroy(defs);
}

static std::vector<std::string> make_vector(const char * const *arr)
{
    std::vector<std::string> v;
    int i = 0;
    for (i = 0; arr[i]; i++) {
        v.emplace_back(arr[i]);
    }
    return v;
}

TEST(DefsTest, AddMacro)
{
    defines *defs = defines_init();
    defines_add(defs, "FOO", "a, b, c", "aa = a; bb = b;");
    const def *d = defines_get(defs, "FOO");
    ASSERT_TRUE(d);
    ASSERT_STREQ("FOO", d->name);
    const char *e_args[] = {"a", "b", "c", NULL};
    ASSERT_EQ(make_vector(e_args), make_vector(d->args));
    ASSERT_STREQ("aa = a; bb = b;", d->replace);

    defines_destroy(defs);
}

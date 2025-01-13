#include <gtest/gtest.h>
#include <fstream>
#include "helper.h"

TEST(IfTest, Simple)
{
    const auto output = run_parser(R"(
#if 0
 foo
#else
 bar
#endif
)");
    EXPECT_EQ("\n bar\n", output);
}

TEST(IfTest, Complicated)
{
    const auto output = run_parser(R"(
#if (((3 + 4 * 2 << 3 >> 2 >> 1 & 7) = 3))
 foo
#else
 bar
#endif
)");
    EXPECT_EQ("\n foo\n", output);
}

TEST(IfTest, Elif)
{
    const auto output = run_parser(R"(
#if 0
 foo
#elif 1 == 2
 bar
#elif 2 > 1
 baz
#else
 bux
#endif
)");
    EXPECT_EQ("\n baz\n", output);
}

TEST(IfTest, Nested)
{
    const auto output = run_parser(R"(
# if 0
 foo
#  if 1
 bar
#  else
 baz
#  endif
 fred
# else
 barney
# endif
)");
    EXPECT_EQ("\n barney\n", output);
}

TEST(IfTest, Macros)
{
    const auto output = run_parser(R"(
#define FOO 123
#if FOO == 123
 fred
#else
 barney
#endif
)");
    EXPECT_EQ("\n fred\n", output);
}

TEST(IfTest, Defined)
{
    const auto output = run_parser(R"(
#define FOO 123
#define BAR
#define BAZ
#if defined FOO
 foo
#endif
#if defined(BAR)
 bar
#endif
#if defined ( FRED ) || ( defined ( BAZ ) && defined BAR)
 fred or baz
#endif
)");
    EXPECT_EQ("\n foo\n bar\n fred or baz\n", output);
}

TEST(IfTest, DefinedShortcut)
{
    const auto output = run_parser(R"(
#define FOO 1
#if defined(FOO) && FOO
 foo
#elif defined(BAR) && BAR
 bar
#endif
 baz
)");
    EXPECT_EQ("\n foo\n baz\n", output);
}

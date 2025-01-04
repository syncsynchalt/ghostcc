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

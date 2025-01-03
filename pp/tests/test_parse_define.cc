#include <gtest/gtest.h>
#include <fstream>
#include "helper.h"

TEST(DefinesTest, Simple)
{
    const auto output = run_parser(R"(
#define a 123
#define b 234
a b c
)");
    EXPECT_EQ("\n123 234 c\n", output);
}

TEST(DefinesTest, EmptyToken)
{
    const auto output = run_parser(R"(
#define a
#define b
a b c
)");
    EXPECT_EQ("\n  c\n", output);
}

TEST(DefinesTest, MultiLine)
{
    const auto output = run_parser(R"(
#define a x y\
z 1 2 3
a to z
)");
    EXPECT_EQ("\nx yz 1 2 3 to z\n", output);
}

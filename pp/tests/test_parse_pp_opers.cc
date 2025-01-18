#include <gtest/gtest.h>
#include "helper.h"

TEST(OpersTest, StringOper)
{
    const auto output = run_parser(R"(
#define A(a, b)  a = #b
 A(x, 1)
)");
    EXPECT_EQ("\n x = \"1\"\n", output);
}

TEST(OpersTest, ConcatOperTwoParams)
{
    const auto output = run_parser(R"(
#define A(a, b)  a ## b
 A(x, 1)
)");
    EXPECT_EQ("\n x1\n", output);
}

TEST(OpersTest, ConcatOperFirstParam)
{
    const auto output = run_parser(R"(
#define A(a)  a ## 3
 A(x)
)");
    EXPECT_EQ("\n x3\n", output);
}

TEST(OpersTest, ConcatOperSecondParam)
{
    const auto output = run_parser(R"(
#define A(a)  3 ## a
 A(x)
)");
    EXPECT_EQ("\n 3x\n", output);
}

TEST(OpersTest, ConcatOperNeitherParam)
{
    const auto output = run_parser(R"(
#define A(a)  3 ## 4
 A(x)
)");
    EXPECT_EQ("\n 34\n", output);
}

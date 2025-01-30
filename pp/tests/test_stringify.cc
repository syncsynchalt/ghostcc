#include <gtest/gtest.h>
#include "pp_helper.h"

TEST(StringifyTest, Basic) {
    const auto output = run_parser(R"(
#define STR(x) #x

STR(foo)
)");
    ASSERT_EQ("\n\n\"foo\"\n"s, output);
}

TEST(StringifyTest, StringifyMacroWrong)
{
    const auto output = run_parser(R"(
#define STR(x) #x

STR(__LINE__)
)");
    ASSERT_EQ("\n\n\"__LINE__\"\n"s, output);
}

TEST(StringifyTest, StringifyMacroRight)
{
    const auto output = run_parser(R"(
#define STR(x) #x
#define xSTR(x) STR(x)

xSTR(__LINE__)
)");
    ASSERT_EQ("\n\n\"5\"\n"s, output);
}

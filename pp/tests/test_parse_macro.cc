#include <gtest/gtest.h>
#include "helper.h"

TEST(MacroTest, Macro)
{
    auto output = run_parser(R"(
#define A(a, b, c)  a = b + c
 foo A(1, 2, 3)
)");
    EXPECT_EQ("\n foo 1 = 2 + 3\n", output);

    output = run_parser(R"(
#define A(  a  ,b,  c)  a = b + c
 foo A(   1  ,2, 3    )
)");
    EXPECT_EQ("\n foo 1 = 2 + 3\n", output);
}

TEST(MacroTest, MacroDeath)
{
    EXPECT_DEATH({
        run_parser(R"(
#define A(a, b, c) a = b + c
 foo A(1)
)");
    }, "Unexpected end of args in macro A.");

    EXPECT_DEATH({
    run_parser(R"(
#define A(a, b, c) a = b + c
 foo A(1, 2, 3, 4)
)");
    }, "Unexpected args past 3 in macro A.");
}

TEST(MacroTest, MacroArgParens)
{
    const auto output = run_parser(R"(
#define A(a, (ignore) b, (special(thing)) c) \
a = b + c
 A(1, 2, 3)
)");
    EXPECT_EQ("\n 1 = 2 + 3\n", output);

    EXPECT_DEATH({
        run_parser(R"(
#define A(a, (ignore b, (special) c) a = b + c
 A(1, 2, 3)
)");
        }, "#define missing closing parens:");
}

TEST(MacroTest, MacroVarArg)
{
    const auto output = run_parser(R"(
#define A(a, ...) printf(a, __VA_ARGS__);
 A("foo %d %d", 1, 2)
)");
    EXPECT_EQ(R"(
 printf("foo %d %d", 1, 2);
)", output);
}

TEST(MacroTest, MacroVarArgEmpty)
{
    const auto output = run_parser(R"(
#define A(a, ...) printf(a, __VA_ARGS__);
 A("foo %d %d")
)");
    EXPECT_EQ(R"(
 printf("foo %d %d", );
)", output); // sic
}

TEST(MacroTest, DISABLED_ArgReplace)
{
    const auto output = run_parser(R"(
#define A(x, y) x
#define B(x, y) x
a = A(B(1, 2), 3)
)");
    EXPECT_EQ(R"(
a = 1
)", output); // sic
}

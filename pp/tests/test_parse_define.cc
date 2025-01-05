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

TEST(DefinesTest, Macro)
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

TEST(DefinesTest, MacroDeath)
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

TEST(DefinesTest, MacroArgParens)
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

TEST(DefinesTest, MacroVarArg)
{
    const auto output = run_parser(R"(
#define A(a, ...) printf(a, __VA_ARGS__);
 A("foo %d %d", 1, 2)
)");
    EXPECT_EQ(R"(
 printf("foo %d %d", 1, 2);
)", output);
}

TEST(DefinesTest, MacroVarArgEmpty)
{
    const auto output = run_parser(R"(
#define A(a, ...) printf(a, __VA_ARGS__);
 A("foo %d %d")
)");
    EXPECT_EQ(R"(
 printf("foo %d %d", );
)", output); // sic
}

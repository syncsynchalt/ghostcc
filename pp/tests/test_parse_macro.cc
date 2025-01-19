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

TEST(MacroTest, ArgReplace)
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

TEST(MacroTest, ArgReplaceExtras)
{
    const auto output = run_parser(R"(
#define ID(x) x
#define A(x, ...) x __VA_ARGS__
a = A(1, ID(2))
)");
    EXPECT_EQ(R"(
a = 1 2
)", output); // sic
}

TEST(MacroTest, DISABLED_Testing)
{
    const auto output = run_parser(R"(
         #define x    3
         #define f(a) f(x * (a))
         #undef  x
         #define x    2
         #define g    f
         #define z    z[0]
         #define h    g(~
         #define m(a) a(w)
         #define w    0,1
         #define t(a) a

         t(t(g)(0) + t)(1);
)");
    const auto expect = R"(

         f(2 * (0)) + t(1);
)";
    EXPECT_EQ(expect, output);
}

TEST(MacroTest, DISABLED_xxx)
{
    // C89 spec 3.8.3.5 part 1
    const auto output = run_parser(R"(
         #define x    3
         #define f(a) f(x * (a))
         #undef  x
         #define x    2
         #define g    f
         #define z    z[0]
         #define h    g(~
         #define m(a) a(w)
         #define w    0,1
         #define t(a) a

         t(t(g)(0) + t)(1);
)");
    const auto expect = R"(

         f(2 * (0)) + t(1);
)";
    EXPECT_EQ(expect, output);
}

TEST(MacroTest, DISABLED_SpecTest1)
{
    // C89 spec 3.8.3.5 part 1
    const auto output = run_parser(R"(
         #define x    3
         #define f(a) f(x * (a))
         #undef  x
         #define x    2
         #define g    f
         #define z    z[0]
         #define h    g(~
         #define m(a) a(w)
         #define w    0,1
         #define t(a) a

         f(y+1) + f(f(z)) % t(t(g)(0) + t)(1);
         g(x+(3,4)-w) | h 5) & m(f)^m(m);
)");
    const auto expect = R"(

         f(2 * (y+1)) + f(2 * (f(2 * (z[0])))) % f(2 * (0)) + t(1);
         f(2 * (2+(3,4)-0,1)) | f(2 * (~ 5)) & f(2 * (0,1))^m(0,1);
)";
    EXPECT_EQ(expect, output);
}

TEST(MacroTest, SpecTest2)
{
    // C89 spec 3.8.3.5 part 2
    const auto output = run_parser(R"(
         #define str(s)      # s
         #define xstr(s)     str(s)
         #define debug(s, t) printf("x" # s "= %d, x" # t "= %s", \
                                             x ## s, x ## t)
         #define INCFILE(n)  vers ## n /* from previous #include example */
         #define glue(a, b)  a ## b
         #define xglue(a, b) glue(a, b)
         #define HIGHLOW     "hello"
         #define LOW         LOW ", world"

         debug(1, 2);
         fputs(str(strncmp("abc\0d", "abc", '\4')  /* this goes away */ == 0) str(: @\n), s);
         include xstr(INCFILE(2).h)
         glue(HIGH, LOW);
         xglue(HIGH, LOW)
)");
    const auto expect = R"(
         printf("x" "1" "= %d, x" "2" "= %s", x1, x2);
         fputs("strncmp(\"abc\\0d\", \"abc\", '\\4')    == 0" ": @\n", s);
         include "vers2.h"   (after macro replacement, before file access)
         "hello";
         "hello" ", world"
)";
    EXPECT_EQ(expect, output);
}

#include <gtest/gtest.h>
#include <fstream>
#include "helper.h"

TEST(IfDefTest, Simple)
{
    const auto output = run_parser(R"(
#define a
#define b 123
#ifdef a
aa
#ifdef b
bb
#endif
#endif
cc
)");
    EXPECT_EQ("\naa\nbb\ncc\n", output);
}

TEST(IfDefTest, NotDef)
{
    const auto output = run_parser(R"(
#ifdef a
aa
#ifdef b
bb
#endif
#endif
cc
)");
    EXPECT_EQ("\ncc\n", output);
}

TEST(IfDefTest, Mixed)
{
    const auto output = run_parser(R"(
#define b
#ifdef a
aa
#endif
#ifdef b
bb
#endif
cc
)");
    EXPECT_EQ("\nbb\ncc\n", output);
}

TEST(IfDefTest, DeepNest)
{
    const auto output = run_parser(R"(
#define a
#define c
#ifdef a
aa
#ifdef b
bb
#ifdef c
cc
#ifdef d
dd
#endif // dd
#endif // cc
#endif // bb
#endif // aa
ee
)");
    EXPECT_EQ("\naa\nee\n", output);
}

TEST(IfDefTest, IfNotDefined)
{
    const auto output = run_parser(R"(
#define a
#ifndef a
aa
#endif
#ifndef bb
bb
#endif
)");
    EXPECT_EQ("\nbb\n", output);
}

TEST(IfDefTest, Undef)
{
    const auto output = run_parser(R"(
#define a 1
#define b 2
#undef a
#ifdef a
aa
#endif
#ifdef b
bb
#endif
cc
)");
    EXPECT_EQ("\nbb\ncc\n", output);
}

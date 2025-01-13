#include <gtest/gtest.h>
#include "helper.h"
extern "C" {
#include "preprocessor.h"
}

TEST(ParseTest, CommentReal)
{
    const auto contents = R"(/*
 * This comment contains tokens that the lexer will choke on if it
 * attempts to read this as a non-comment, such as the string that
 * follows: (the 'License').
 */
foo
)";

    const auto infile = write_file(contents);
    const auto outfile = write_file("");
    FileDeleter fd1(infile), fd2(outfile);
    FILE *fin = fopen(infile.c_str(), "r");
    FILE *fout = fopen(outfile.c_str(), "w");
    parse(infile.c_str(), fin, fout, NULL);
    fclose(fin);
    fclose(fout);
    EXPECT_EQ(" \nfoo\n", read_file(outfile));
}

TEST(TestParse, TernaryShortCircuit)
{
    const auto output = run_parser(R"(
#define FOO
#if defined(FOO) ? 1 : AVOID_ME
 foo
#endif
#if defined(BAR) ? AVOID_ME : 0
 bar
#endif
)");
    EXPECT_EQ("\n foo\n", output);
}

TEST(TestParse, HasInclude)
{
    const auto output = run_parser(R"(
#if __has_include(<foo.h>)
foo
#elif __has_include("bar.h")
bar
#endif
)");
    EXPECT_EQ("\n", output);
}

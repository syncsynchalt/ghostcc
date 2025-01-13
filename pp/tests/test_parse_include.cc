#include <gtest/gtest.h>
#include <fstream>
#include "helper.h"
extern "C" {
#include "preprocessor.h"
}

TEST(IncludesTest, Simple)
{
    const auto file1 = write_file("a b c\n");
    FileDeleter f1(file1);
    const auto file2 = write_file("d e f\n");
    FileDeleter f2(file1);
    const auto input = "#include \"" + file1 + "\"\n#include <" + file2 + ">\n xyz \n";
    const auto output = run_parser(input);
    EXPECT_EQ("a b c\nd e f\n xyz \n", output);
}

TEST(IncludesTest, MissingEndif)
{
    const auto file1 = write_file(R"(
#if 0
foo
#else
)");
    FileDeleter f1(file1);
    const auto input = "#include \"" + file1 + "\"\n";
    EXPECT_DEATH({
        const auto output = run_parser(input);
    }, "Missing #endif in");
}

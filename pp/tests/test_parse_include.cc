#include <gtest/gtest.h>
#include <fstream>
#include "helper.h"
extern "C" {
#include "preprocessor.h"
}

TEST(IncludesTest, Simple)
{
    const auto file1 = write_file("a b c\n");
    const auto file2 = write_file("d e f\n");
    const auto input = "#include \"" + file1 + "\"\n#include <" + file2 + ">\n xyz \n";
    const auto output = run_parser(input);
    EXPECT_EQ("a b c\nd e f\n xyz \n", output);
}

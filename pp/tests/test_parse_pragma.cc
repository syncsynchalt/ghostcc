#include <gtest/gtest.h>
#include <fstream>
#include "pp_helper.h"

TEST(PragmaTest, Once)
{
    const auto file1 = write_file(R"(
#pragma once
 foo
)");
    FileDeleter f1(file1);
    std::string contents2 = R"(
#pragma once
 bar
#include "XXX"
 baz
)";
    contents2 = contents2.replace(contents2.find("XXX"), 3, file1);
    const auto file2 = write_file(contents2);
    FileDeleter f2(file2);

    std::string contents3 = R"(
#include "XXX1"
#include "XXX2"
 outer
)";
    contents3 = contents3.replace(contents3.find("XXX1"), 4, file1);
    contents3 = contents3.replace(contents3.find("XXX2"), 4, file2);

    const auto output = run_parser(contents3);
    EXPECT_EQ("\n\n foo\n\n bar\n\n baz\n outer\n", output);
}

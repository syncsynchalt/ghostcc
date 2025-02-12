#include <gtest/gtest.h>
#include "cc_helper.h"

TEST(SimpleProgs, Simplest)
{
    EXPECT_EQ("", compile_and_run(R"(
int main(void)
{
    return 2 + 1;
}
)", 3));
}

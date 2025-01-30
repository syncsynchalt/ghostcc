#include <gtest/gtest.h>
#include "cc_helper.h"

TEST(TypedefTest, RegisterTypeName)
{
    const auto ast = run_grammar(R"(
typedef int foo;
int main()
{
    int a = 1;
    foo b = 2;
}
)");
    EXPECT_EQ("\n", print_ast(ast));
}

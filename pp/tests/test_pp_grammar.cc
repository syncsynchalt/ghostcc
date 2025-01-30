#include <gtest/gtest.h>
#include "pp_helper.h"
extern "C" {
#include "../pp_lex.h"
#include "../pp_ast.h"
extern int yyparse(void);
}

ast_node *process_file(const std::string &s)
{
    reset_parser(s.c_str(), NULL);
    yyparse();
    return pp_parse_result;
}

TEST(GrammarTest, TestExhaustive)
{
    const auto r = process_file("1 + 2");
    ASSERT_TRUE(r);
    ASSERT_EQ(NODE_OTHER, r->type);
    ASSERT_EQ('+', r->tok_type);
    ASSERT_TRUE(r->left);
    ASSERT_TRUE(r->right);
    ASSERT_EQ(NODE_INT, r->left->type);
    ASSERT_EQ(1, r->left->ival);
    ASSERT_EQ(NODE_INT, r->right->type);
    ASSERT_EQ(2, r->right->ival);
}

TEST(GrammarTest, TestParser)
{
    auto r = process_file("1 * 2");
    ASSERT_EQ("(1*2)", print_ast(r));

    r = process_file("3 + (2 * 1)");
    ASSERT_EQ("(3+(2*1))", print_ast(r));
}

TEST(GrammarTest, TestTernary)
{
    auto r = process_file("1 ? 2 : 3+4");
    ASSERT_EQ("(1?(2:(3+4)))", print_ast(r));

    r = process_file("1 ? 2 ? 6 : 7 : 3 ? 4 : 5");
    ASSERT_EQ("(1?((2?(6:7)):(3?(4:5))))", print_ast(r));
}

TEST(GrammarParseTest, TestIntResolution)
{
    auto r = resolve_ast(process_file("1 + 2"));
    EXPECT_EQ(3, r.ival);
    r = resolve_ast(process_file("3 + (2 * 1)"));
    EXPECT_EQ(5, r.ival);
    r = resolve_ast(process_file("3 * (0x3 << 1)"));
    EXPECT_EQ(18, r.ival);
    r = resolve_ast(process_file("1 ? 2 : 3"));
    EXPECT_EQ(2, r.ival);
    r = resolve_ast(process_file("2 || 3"));
    EXPECT_EQ(1, r.ival);
}

TEST(GrammarParseTest, TestFltResolution)
{
    auto r = resolve_ast(process_file("1 + 2.3"));
    EXPECT_EQ(AST_RESULT_TYPE_FLT, r.type);
    EXPECT_EQ(3.3, r.fval);
    r = resolve_ast(process_file("1.0 + 2.3"));
    EXPECT_EQ(AST_RESULT_TYPE_FLT, r.type);
    EXPECT_EQ(3.3, r.fval);
    r = resolve_ast(process_file("4.0 << 2"));
    EXPECT_EQ(AST_RESULT_TYPE_INT, r.type);
    EXPECT_EQ(16, r.ival);
    r = resolve_ast(process_file("3.2 / 2.0"));
    EXPECT_EQ(AST_RESULT_TYPE_FLT, r.type);
    EXPECT_EQ(1.6, r.fval);
    r = resolve_ast(process_file("3.2 / 2"));
    EXPECT_EQ(AST_RESULT_TYPE_FLT, r.type);
    EXPECT_EQ(1.6, r.fval);
    r = resolve_ast(process_file("1 && 2.0"));
    EXPECT_EQ(AST_RESULT_TYPE_INT, r.type);
    EXPECT_EQ(1, r.ival);
}

TEST(GrammarParseTest, TestStrResolution)
{
    auto r = resolve_ast(process_file(R"("foo" = "bar")"));
    EXPECT_EQ(AST_RESULT_TYPE_INT, r.type);
    EXPECT_EQ(0, r.ival);
    r = resolve_ast(process_file(R"("foo" = "foo")"));
    EXPECT_EQ(AST_RESULT_TYPE_INT, r.type);
    EXPECT_EQ(1, r.ival);
    r = resolve_ast(process_file(R"("foo" == "foo")"));
    EXPECT_EQ(AST_RESULT_TYPE_INT, r.type);
    EXPECT_EQ(1, r.ival);
}

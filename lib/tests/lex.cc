#include <gtest/gtest.h>
extern "C" {
#include "../lex.h"
}



static void assert_token(const char *l, token_state &s, const token_type type, const std::string &token,
                         const bool eol = false, const int pos = -1)
{
    ASSERT_EQ(eol ? 1 : 0, get_token(l, strlen(l), &s));
    ASSERT_EQ(type, s.type);
    ASSERT_EQ(token, s.tok);
    if (pos >= 0) {
        ASSERT_EQ(pos, s.pos);
    }
}

TEST(LexTest, Empty)
{
    auto line = "";
    token_state s = {};
    assert_token(line, s, TOK_ERR, "", true, 0);

    line = "\n";
    assert_token(line, s, TOK_WS, "\n", true, 0);
    assert_token(line, s, TOK_ERR, "", true, 1);
}

TEST(LexTest, Identifier)
{
    const auto line = "abc";
    token_state s = {};
    assert_token(line, s, TOK_ID, "abc", true, 0);
}

TEST(LexTest, Keyword)
{
    const auto line = "abc auto \t def\r\n";
    token_state s = {};
    assert_token(line, s, TOK_ID, "abc", false, 0);
    assert_token(line, s, TOK_WS, " ", false, 3);
    assert_token(line, s, TOK_KEYWORD, "auto", false, 4);
    assert_token(line, s, TOK_WS, " \t ", false, 8);
    assert_token(line, s, TOK_ID, "def", false, 11);
    assert_token(line, s, TOK_WS, "\r\n", true, 14);
}

TEST(LexTest, Illegal)
{
    auto line = "foo élan";
    token_state s = {};
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_ERR, "é");
    assert_token(line, s, TOK_ID, "lan", true);

    line = "foo @ bar";
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_ERR, "@");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_ID, "bar", true);
}

TEST(LexTest, LineReset)
{
    auto line = "foo\n";
    token_state s = {};
    assert_token(line, s, TOK_ID, "foo", false, 0);
    assert_token(line, s, TOK_WS, "\n", true, 3);
    line = "bar baz\n";
    assert_token(line, s, TOK_ID, "bar", false, 0);
    assert_token(line, s, TOK_WS, " ", false, 3);
    assert_token(line, s, TOK_ID, "baz", false, 4);
    assert_token(line, s, TOK_WS, "\n", true, 7);
}

TEST(LexTest, Numbers)
{
    const auto line = "-2 -2.3e-10 123 234.0 13e4 10u 10U 11l 13f 14.0F 1.2e10l";
    token_state s = {};

    assert_token(line, s, TOK_MINUS, "-");
    assert_token(line, s, TOK_NUM, "2");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_MINUS, "-");
    assert_token(line, s, TOK_NUM, "2.3e-10");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "123");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "234.0");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "13e4");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "10u");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "10U");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "11l");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "13f");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "14.0F");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_NUM, "1.2e10l", true);
}

TEST(LexTest, Constants)
{
    const auto line = R"("bar" "bux\"bix" 'a' '\0' '\'')";
    token_state s = {};

    assert_token(line, s, TOK_STR, R"("bar")");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_STR, R"("bux\"bix")");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_CHA, "'a'");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_CHA, R"('\0')");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_CHA, R"('\'')", true);
}

TEST(LexTest, ConstantsIllegal)
{
    token_state s = {};
    EXPECT_DEATH({
        const auto line = R"("bar)";
        get_token(line, strlen(line), &s);
    }, "unterminated quote");
    EXPECT_DEATH({
        const auto line = R"("bar\")";
        get_token(line, strlen(line), &s);
    }, "unterminated quote");
    EXPECT_DEATH({
        const auto line = R"('aa')";
        get_token(line, strlen(line), &s);
    }, "unterminated character");
    EXPECT_DEATH({
        const auto line = R"('\')";
        get_token(line, strlen(line), &s);
    }, "unterminated character");
}

TEST(LexTest, Singles)
{
    const auto line = "{([id])},:;~?";
    token_state s = {};
    assert_token(line, s, TOK_LBRACE, "{");
    assert_token(line, s, TOK_LPAREN, "(");
    assert_token(line, s, TOK_LBRACK, "[");
    assert_token(line, s, TOK_ID, "id");
    assert_token(line, s, TOK_RBRACK, "]");
    assert_token(line, s, TOK_RPAREN, ")");
    assert_token(line, s, TOK_RBRACE, "}");
    assert_token(line, s, TOK_COMMA, ",");
    assert_token(line, s, TOK_COLON, ":");
    assert_token(line, s, TOK_SEMI, ";");
    assert_token(line, s, TOK_BIT_FLIP, "~");
    assert_token(line, s, TOK_QUESTION, "?", true);
}

TEST(LexTest, Multis)
{
    auto line = ".->...-+/+=-=";
    token_state s = {};
    assert_token(line, s, TOK_DOT, ".");
    assert_token(line, s, TOK_ARROW, "->");
    assert_token(line, s, TOK_DOTDOTDOT, "...");
    assert_token(line, s, TOK_MINUS, "-");
    assert_token(line, s, TOK_PLUS, "+");
    assert_token(line, s, TOK_DIV, "/");
    assert_token(line, s, TOK_ASSIGN_PLUS, "+=");
    assert_token(line, s, TOK_ASSIGN_MINUS, "-=", true);

    line = "*=/=%=&=|=^=";
    assert_token(line, s, TOK_ASSIGN_MULT, "*=");
    assert_token(line, s, TOK_ASSIGN_DIV, "/=");
    assert_token(line, s, TOK_ASSIGN_MOD, "%=");
    assert_token(line, s, TOK_ASSIGN_AND, "&=");
    assert_token(line, s, TOK_ASSIGN_OR, "|=");
    assert_token(line, s, TOK_ASSIGN_XOR, "^=", true);

    line = "<<>><<=>>=<=>=<>";
    assert_token(line, s, TOK_LSHIFT, "<<");
    assert_token(line, s, TOK_RSHIFT, ">>");
    assert_token(line, s, TOK_ASSIGN_LSHIFT, "<<=");
    assert_token(line, s, TOK_ASSIGN_RSHIFT, ">>=");
    assert_token(line, s, TOK_LE, "<=");
    assert_token(line, s, TOK_GE, ">=");
    assert_token(line, s, TOK_LT, "<");
    assert_token(line, s, TOK_GT, ">", true);

    line = "*%&|^ =";
    assert_token(line, s, TOK_MULT, "*");
    assert_token(line, s, TOK_MOD, "%");
    assert_token(line, s, TOK_AND, "&");
    assert_token(line, s, TOK_OR, "|");
    assert_token(line, s, TOK_XOR, "^");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_ASSIGN, "=", true);

    line = "&&||!!===++--";
    assert_token(line, s, TOK_LOGICAL_AND, "&&");
    assert_token(line, s, TOK_LOGICAL_OR, "||");
    assert_token(line, s, TOK_BANG, "!");
    assert_token(line, s, TOK_NE, "!=");
    assert_token(line, s, TOK_EQ, "==");
    assert_token(line, s, TOK_PLUSPLUS, "++");
    assert_token(line, s, TOK_MINUSMINUS, "--", true);
}

TEST(LexTest, PreProcessor)
{
    auto line = "foo##bar";
    token_state s = {};
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_PP_COMBINE, "##");
    assert_token(line, s, TOK_ID, "bar", true);

    line = "foo #bar";
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_PP_STR, "#");
    assert_token(line, s, TOK_ID, "bar", true);

    line = "foo // comment\n";
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_LINE_COMMENT, "// comment\n", true);

    line = R"(foo \)";
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_PP_CONTINUE, "\\", true);

    line = R"(foo\)" "\n";
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_PP_CONTINUE, "\\\n", true);
}

TEST(LexTest, Comments)
{
    auto line = "foo /* bar";
    token_state s = {};
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_COMMENT, " ", true);
    ASSERT_EQ(s._comment_level, 1);

    line = " baz */ bux";
    assert_token(line, s, TOK_COMMENT, " ");
    assert_token(line, s, TOK_WS, " ");
    assert_token(line, s, TOK_ID, "bux", true);
}

TEST(LexTest, NestedComment)
{
    auto line = "foo/* bar /* bux */ cash */money";
    token_state s = {};
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_COMMENT, " ");
    assert_token(line, s, TOK_ID, "money", true);

    line = "foo/* bar /*/ bux */ cash */money";
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_COMMENT, " ");
    assert_token(line, s, TOK_ID, "money", true);
}

TEST(LexTest, NonCToken)
{
    auto line = "foo@bar";
    token_state s = {};
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_ERR, "@");
    assert_token(line, s, TOK_ID, "bar", true);

    line = "foo@@bar";
    assert_token(line, s, TOK_ID, "foo");
    assert_token(line, s, TOK_ERR, "@@");
    assert_token(line, s, TOK_ID, "bar", true);
}

TEST(LexTest, DecodeStr)
{
    char buf[512];
    EXPECT_STREQ("foo", decode_str(R"("foo")", buf, sizeof(buf)));
    EXPECT_STREQ("foo\b baz", decode_str(R"("foo\b baz")", buf, sizeof(buf)));
    EXPECT_STREQ("foo\n baz", decode_str(R"("foo\n baz")", buf, sizeof(buf)));
    EXPECT_STREQ("foo z baz", decode_str(R"("foo \z baz")", buf, sizeof(buf)));
    EXPECT_STREQ("foo \\ baz", decode_str(R"("foo \\ baz")", buf, sizeof(buf)));
    EXPECT_STREQ("foo A baz", decode_str(R"("foo \101 baz")", buf, sizeof(buf)));
    EXPECT_STREQ("foo A baz", decode_str(R"("foo \x41 baz")", buf, sizeof(buf)));
    EXPECT_STREQ("foo X41 baz", decode_str(R"("foo \X41 baz")", buf, sizeof(buf)));
}

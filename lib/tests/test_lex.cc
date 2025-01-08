#include <gtest/gtest.h>
extern "C" {
#include "../lex.h"
}


static void assert_token(const int line, const char *l, token_state &s, const int type, const std::string &token,
                         const bool eol = false, const int pos = -1)
{
    const std::string at = "(at " __FILE__ ":" + std::to_string(line) + ")";
    ASSERT_EQ(eol ? 0 : 1, get_token(l, strlen(l), &s)) << at;
    ASSERT_EQ(type, s.type) << at;
    ASSERT_EQ(token, s.tok) << at;
    if (pos >= 0) {
        ASSERT_EQ(pos, s.pos) << at;
    }
}

TEST(LexTest, Empty)
{
    auto line = "";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ERR, "", true, 0);

    line = "\n";
    assert_token(__LINE__, line, s, TOK_WS, "\n", true, 0);
    assert_token(__LINE__, line, s, TOK_ERR, "", true, 1);
}

TEST(LexTest, Identifier)
{
    const auto line = "abc";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "abc", true, 0);
}

TEST(LexTest, Keyword)
{
    const auto line = "abc auto \t def\r\n";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "abc", false, 0);
    assert_token(__LINE__, line, s, TOK_WS, " ", false, 3);
    assert_token(__LINE__, line, s, TOK_KW_AUTO, "auto", false, 4);
    assert_token(__LINE__, line, s, TOK_WS, " \t ", false, 8);
    assert_token(__LINE__, line, s, TOK_ID, "def", false, 11);
    assert_token(__LINE__, line, s, TOK_WS, "\r\n", true, 14);
}

TEST(LexTest, Illegal)
{
    auto line = "foo élan";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_ERR, "é");
    assert_token(__LINE__, line, s, TOK_ID, "lan", true);

    line = "foo @ bar";
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_ERR, "@");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_ID, "bar", true);
}

TEST(LexTest, LineReset)
{
    auto line = "foo\n";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "foo", false, 0);
    assert_token(__LINE__, line, s, TOK_WS, "\n", true, 3);
    line = "bar baz\n";
    assert_token(__LINE__, line, s, TOK_ID, "bar", false, 0);
    assert_token(__LINE__, line, s, TOK_WS, " ", false, 3);
    assert_token(__LINE__, line, s, TOK_ID, "baz", false, 4);
    assert_token(__LINE__, line, s, TOK_WS, "\n", true, 7);
}

TEST(LexTest, Numbers)
{
    const auto line = "-2 -2.3e-10 123 234.0 13e4 10u 10U 11l 13f 14.0F 1.2e10l";
    token_state s = {};

    assert_token(__LINE__, line, s, '-', "-");
    assert_token(__LINE__, line, s, TOK_NUM, "2");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, '-', "-");
    assert_token(__LINE__, line, s, TOK_NUM, "2.3e-10");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "123");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "234.0");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "13e4");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "10u");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "10U");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "11l");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "13f");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "14.0F");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_NUM, "1.2e10l", true);
}

TEST(LexTest, Constants)
{
    const auto line = R"("bar" "bux\"bix" 'a' '\0' '\'')";
    token_state s = {};

    assert_token(__LINE__, line, s, TOK_STR, R"("bar")");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_STR, R"("bux\"bix")");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_CHA, "'a'");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_CHA, R"('\0')");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_CHA, R"('\'')", true);
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
    assert_token(__LINE__, line, s, '{', "{");
    assert_token(__LINE__, line, s, '(', "(");
    assert_token(__LINE__, line, s, '[', "[");
    assert_token(__LINE__, line, s, TOK_ID, "id");
    assert_token(__LINE__, line, s, ']', "]");
    assert_token(__LINE__, line, s, ')', ")");
    assert_token(__LINE__, line, s, '}', "}");
    assert_token(__LINE__, line, s, ',', ",");
    assert_token(__LINE__, line, s, ':', ":");
    assert_token(__LINE__, line, s, ';', ";");
    assert_token(__LINE__, line, s, '~', "~");
    assert_token(__LINE__, line, s, '?', "?", true);
}

TEST(LexTest, Multis)
{
    auto line = ".->...-+/+=-=";
    token_state s = {};
    assert_token(__LINE__, line, s, '.', ".");
    assert_token(__LINE__, line, s, TOK_PTR_OP, "->");
    assert_token(__LINE__, line, s, TOK_ELLIPSIS, "...");
    assert_token(__LINE__, line, s, '-', "-");
    assert_token(__LINE__, line, s, '+', "+");
    assert_token(__LINE__, line, s, '/', "/");
    assert_token(__LINE__, line, s, TOK_ADD_ASSIGN, "+=");
    assert_token(__LINE__, line, s, TOK_SUB_ASSIGN, "-=", true);

    line = "*=/=%=&=|=^=";
    assert_token(__LINE__, line, s, TOK_MUL_ASSIGN, "*=");
    assert_token(__LINE__, line, s, TOK_DIV_ASSIGN, "/=");
    assert_token(__LINE__, line, s, TOK_MOD_ASSIGN, "%=");
    assert_token(__LINE__, line, s, TOK_AND_ASSIGN, "&=");
    assert_token(__LINE__, line, s, TOK_OR_ASSIGN, "|=");
    assert_token(__LINE__, line, s, TOK_XOR_ASSIGN, "^=", true);

    line = "<<>><<=>>=<=>=<>";
    assert_token(__LINE__, line, s, TOK_LEFT_OP, "<<");
    assert_token(__LINE__, line, s, TOK_RIGHT_OP, ">>");
    assert_token(__LINE__, line, s, TOK_LEFT_ASSIGN, "<<=");
    assert_token(__LINE__, line, s, TOK_RIGHT_ASSIGN, ">>=");
    assert_token(__LINE__, line, s, TOK_LE_OP, "<=");
    assert_token(__LINE__, line, s, TOK_GE_OP, ">=");
    assert_token(__LINE__, line, s, '<', "<");
    assert_token(__LINE__, line, s, '>', ">", true);

    line = "*%&|^ =";
    assert_token(__LINE__, line, s, '*', "*");
    assert_token(__LINE__, line, s, '%', "%");
    assert_token(__LINE__, line, s, '&', "&");
    assert_token(__LINE__, line, s, '|', "|");
    assert_token(__LINE__, line, s, '^', "^");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, '=', "=", true);

    line = "&&||!!===++--";
    assert_token(__LINE__, line, s, TOK_AND_OP, "&&");
    assert_token(__LINE__, line, s, TOK_OR_OP, "||");
    assert_token(__LINE__, line, s, '!', "!");
    assert_token(__LINE__, line, s, TOK_NE_OP, "!=");
    assert_token(__LINE__, line, s, TOK_EQ_OP, "==");
    assert_token(__LINE__, line, s, TOK_INC_OP, "++");
    assert_token(__LINE__, line, s, TOK_DEC_OP, "--", true);
}

TEST(LexTest, PreProcessor)
{
    auto line = "foo##bar";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_PP_COMBINE, "##");
    assert_token(__LINE__, line, s, TOK_ID, "bar", true);

    line = "foo #bar";
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_PP_STR, "#");
    assert_token(__LINE__, line, s, TOK_ID, "bar", true);

    line = "foo // comment\n";
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_LINE_COMMENT, "// comment\n", true);

    line = R"(foo \)";
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_PP_CONTINUE, "\\", true);

    line = R"(foo\)" "\n";
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_PP_CONTINUE, "\\\n", true);
}

TEST(LexTest, Comments)
{
    auto line = "foo /* bar";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_COMMENT, " ", true);
    ASSERT_EQ(s._comment_level, 1);

    line = " baz */ bux";
    assert_token(__LINE__, line, s, TOK_COMMENT, " ");
    assert_token(__LINE__, line, s, TOK_WS, " ");
    assert_token(__LINE__, line, s, TOK_ID, "bux", true);
}

TEST(LexTest, NestedComment)
{
    auto line = "foo/* bar /* bux */ cash */money";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_COMMENT, " ");
    assert_token(__LINE__, line, s, TOK_ID, "money", true);

    line = "foo/* bar /*/ bux */ cash */money";
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_COMMENT, " ");
    assert_token(__LINE__, line, s, TOK_ID, "money", true);
}

TEST(LexTest, NonCToken)
{
    auto line = "foo@bar";
    token_state s = {};
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_ERR, "@");
    assert_token(__LINE__, line, s, TOK_ID, "bar", true);

    line = "foo@@bar";
    assert_token(__LINE__, line, s, TOK_ID, "foo");
    assert_token(__LINE__, line, s, TOK_ERR, "@@");
    assert_token(__LINE__, line, s, TOK_ID, "bar", true);
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

TEST(LexTest, QuoteStr)
{
    char *p = quote_str("foo bar baz");
    EXPECT_STREQ("\"foo bar baz\"", p);
    free(p);
    p = quote_str("foo\n bar\n baz");
    EXPECT_STREQ("\"foo\\n bar\\n baz\"", p);
    free(p);
    p = quote_str("all specials: \a\b\f\n\r\t\v\\\'\"\?\x7f\x81\xff");
    EXPECT_STREQ("\"all specials: \\a\\b\\f\\n\\r\\t\\v\\\\'\\\"?\\x7f\\x81\\xff\"", p);
    free(p);
}

TEST(LexTest, RoundTrip)
{
    const auto input = std::string("all specials: \a\b\f\n\r\t\v\\\'\"\?\x7f\x81\xff");
    char *p = quote_str(input.c_str());
    char buf[512];
    decode_str(p, buf, sizeof(buf));
    EXPECT_EQ(input, buf);
    free(p);
}

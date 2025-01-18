#include <gtest/gtest.h>
extern "C" {
#include "../pp_toker.h"
}

static void assert_token(const int line, token_state *ts, const int type, const std::string &token,
                         const bool eol = false)
{
    const std::string at = "(at " __FILE__ ":" + std::to_string(line) + ")";
    const auto t = get_token(ts);
    ASSERT_EQ(type, t.type) << at;
    ASSERT_EQ(token, t.tok) << at;
    if (eol) {
        ASSERT_TRUE(TOKEN_STATE_DONE(ts)) << at;
    }
}

TEST(LexTest, Empty)
{
    token_state ts;

    auto line = "";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, -1, "", true);

    line = "\n";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_WS, "\n", true);
    assert_token(__LINE__, &ts, -1, "", true);
}

TEST(LexTest, Identifier)
{
    const auto line = "abc";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "abc", true);
}

TEST(LexTest, Keyword)
{
    const auto line = "abc auto \t def\r\n";
    token_state ts;
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "abc", false);
    assert_token(__LINE__, &ts, TOK_WS, " ", false);
    assert_token(__LINE__, &ts, TOK_KW_AUTO, "auto", false);
    assert_token(__LINE__, &ts, TOK_WS, " \t ", false);
    assert_token(__LINE__, &ts, TOK_ID, "def", false);
    assert_token(__LINE__, &ts, TOK_WS, "\r\n", true);
}

TEST(LexTest, Illegal)
{
    token_state ts;
    {
        const auto line = "foo élan";
        set_token_string(&ts, line);
        assert_token(__LINE__, &ts, TOK_ID, "foo");
        assert_token(__LINE__, &ts, TOK_WS, " ");
        assert_token(__LINE__, &ts, TOK_ERR, "é");
        assert_token(__LINE__, &ts, TOK_ID, "lan", true);
    }

    {
        const auto line = "foo @ bar";
        set_token_string(&ts, line);
        assert_token(__LINE__, &ts, TOK_ID, "foo");
        assert_token(__LINE__, &ts, TOK_WS, " ");
        assert_token(__LINE__, &ts, TOK_ERR, "@");
        assert_token(__LINE__, &ts, TOK_WS, " ");
        assert_token(__LINE__, &ts, TOK_ID, "bar", true);
    }
}

TEST(LexTest, LineReset)
{
    token_state ts;
    {
        const auto line = "foo\n";
        set_token_string(&ts, line);
        assert_token(__LINE__, &ts, TOK_ID, "foo", false);
        assert_token(__LINE__, &ts, TOK_WS, "\n", true);
    }
    {
        const auto line = "bar baz\n";
        set_token_string(&ts, line);
        assert_token(__LINE__, &ts, TOK_ID, "bar");
        assert_token(__LINE__, &ts, TOK_WS, " ");
        assert_token(__LINE__, &ts, TOK_ID, "baz");
        assert_token(__LINE__, &ts, TOK_WS, "\n", true);
    }
}

TEST(LexTest, Numbers)
{
    const auto line = "-2 -2.3e-10 123 234.0 13e4 10u 10U 11l 13f 14.0F 1.2e10l";
    token_state ts;
    set_token_string(&ts, line);

    assert_token(__LINE__, &ts, '-', "-");
    assert_token(__LINE__, &ts, TOK_NUM, "2");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, '-', "-");
    assert_token(__LINE__, &ts, TOK_NUM, "2.3e-10");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "123");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "234.0");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "13e4");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "10u");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "10U");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "11l");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "13f");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "14.0F");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_NUM, "1.2e10l", true);
}

TEST(LexTest, Constants)
{
    const auto line = R"("bar" "bux\"bix" 'a' '\0' '\'')";
    token_state ts = {};
    set_token_string(&ts, line);

    assert_token(__LINE__, &ts, TOK_STR, R"("bar")");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_STR, R"("bux\"bix")");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_CHA, "'a'");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_CHA, R"('\0')");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_CHA, R"('\'')", true);
}

TEST(LexTest, ConstantsIllegal)
{
    EXPECT_DEATH(
            {
                const auto line = R"("bar)";
                token_state ts = {};
                set_token_string(&ts, line);
                get_token(&ts);
            },
            "unterminated quote");
    EXPECT_DEATH(
            {
                const auto line = R"("bar\")";
                token_state ts = {};
                set_token_string(&ts, line);
                get_token(&ts);
            },
            "unterminated quote");
    EXPECT_DEATH(
            {
                const auto line = R"('aa')";
                token_state ts = {};
                set_token_string(&ts, line);
                get_token(&ts);
            },
            "unterminated character");
    EXPECT_DEATH(
            {
                const auto line = R"('\')";
                token_state ts = {};
                set_token_string(&ts, line);
                get_token(&ts);
            },
            "unterminated character");
}

TEST(LexTest, Singles)
{
    const auto line = "{([id])},:;~?";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, '{', "{");
    assert_token(__LINE__, &ts, '(', "(");
    assert_token(__LINE__, &ts, '[', "[");
    assert_token(__LINE__, &ts, TOK_ID, "id");
    assert_token(__LINE__, &ts, ']', "]");
    assert_token(__LINE__, &ts, ')', ")");
    assert_token(__LINE__, &ts, '}', "}");
    assert_token(__LINE__, &ts, ',', ",");
    assert_token(__LINE__, &ts, ':', ":");
    assert_token(__LINE__, &ts, ';', ";");
    assert_token(__LINE__, &ts, '~', "~");
    assert_token(__LINE__, &ts, '?', "?", true);
}

TEST(LexTest, Multis)
{
    auto line = ".->...-+/+=-=";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, '.', ".");
    assert_token(__LINE__, &ts, TOK_PTR_OP, "->");
    assert_token(__LINE__, &ts, TOK_ELLIPSIS, "...");
    assert_token(__LINE__, &ts, '-', "-");
    assert_token(__LINE__, &ts, '+', "+");
    assert_token(__LINE__, &ts, '/', "/");
    assert_token(__LINE__, &ts, TOK_ADD_ASSIGN, "+=");
    assert_token(__LINE__, &ts, TOK_SUB_ASSIGN, "-=", true);

    line = "*=/=%=&=|=^=";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_MUL_ASSIGN, "*=");
    assert_token(__LINE__, &ts, TOK_DIV_ASSIGN, "/=");
    assert_token(__LINE__, &ts, TOK_MOD_ASSIGN, "%=");
    assert_token(__LINE__, &ts, TOK_AND_ASSIGN, "&=");
    assert_token(__LINE__, &ts, TOK_OR_ASSIGN, "|=");
    assert_token(__LINE__, &ts, TOK_XOR_ASSIGN, "^=", true);

    line = "<<>><<=>>=<=>=<>";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_LEFT_OP, "<<");
    assert_token(__LINE__, &ts, TOK_RIGHT_OP, ">>");
    assert_token(__LINE__, &ts, TOK_LEFT_ASSIGN, "<<=");
    assert_token(__LINE__, &ts, TOK_RIGHT_ASSIGN, ">>=");
    assert_token(__LINE__, &ts, TOK_LE_OP, "<=");
    assert_token(__LINE__, &ts, TOK_GE_OP, ">=");
    assert_token(__LINE__, &ts, '<', "<");
    assert_token(__LINE__, &ts, '>', ">", true);

    line = "*%&|^ =";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, '*', "*");
    assert_token(__LINE__, &ts, '%', "%");
    assert_token(__LINE__, &ts, '&', "&");
    assert_token(__LINE__, &ts, '|', "|");
    assert_token(__LINE__, &ts, '^', "^");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, '=', "=", true);

    line = "&&||!!===++--";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_AND_OP, "&&");
    assert_token(__LINE__, &ts, TOK_OR_OP, "||");
    assert_token(__LINE__, &ts, '!', "!");
    assert_token(__LINE__, &ts, TOK_NE_OP, "!=");
    assert_token(__LINE__, &ts, TOK_EQ_OP, "==");
    assert_token(__LINE__, &ts, TOK_INC_OP, "++");
    assert_token(__LINE__, &ts, TOK_DEC_OP, "--", true);
}

TEST(LexTest, PreProcessor)
{
    auto line = "foo##bar";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_PP_COMBINE, "##");
    assert_token(__LINE__, &ts, TOK_ID, "bar", true);

    line = "foo #bar";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, '#', "#");
    assert_token(__LINE__, &ts, TOK_ID, "bar", true);

    line = "foo // comment\n";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_LINE_COMMENT, " ");
    assert_token(__LINE__, &ts, TOK_WS, "\n");

    line = R"(foo \)";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, '\\', "\\", true);

    line = "foo\\\n";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_PP_CONTINUE, "\\\n", true);

    line = "foo\\\r\n";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_PP_CONTINUE, "\\\r\n", true);
}

TEST(LexTest, Comments)
{
    const auto line = "foo /* bar baz */ bux";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_COMMENT, " ");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_ID, "bux", true);
}

TEST(LexTest, NestedComment)
{
    auto line = "foo/* bar /* bux */ cash */money";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_COMMENT, " ");
    assert_token(__LINE__, &ts, TOK_ID, "money", true);

    line = "foo/* bar /*/ bux */ cash */money";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_COMMENT, " ");
    assert_token(__LINE__, &ts, TOK_ID, "money", true);
}

TEST(LexTest, NonCToken)
{
    auto line = "foo@bar";
    token_state ts = {};
    {
        set_token_string(&ts, line);
        assert_token(__LINE__, &ts, TOK_ID, "foo");
        assert_token(__LINE__, &ts, TOK_ERR, "@");
        assert_token(__LINE__, &ts, TOK_ID, "bar", true);
    }

    line = "foo@@bar";
    {
        set_token_string(&ts, line);
        assert_token(__LINE__, &ts, TOK_ID, "foo");
        assert_token(__LINE__, &ts, TOK_ERR, "@@");
        assert_token(__LINE__, &ts, TOK_ID, "bar", true);
    }
}

TEST(LexTest, CppNamespaces)
{
    auto line = "bar::baz bux";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "bar::baz");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_ID, "bux", true);

    line = "bar:baz";
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "bar");
    assert_token(__LINE__, &ts, ':', ":");
    assert_token(__LINE__, &ts, TOK_ID, "baz", true);
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

TEST(LexTest, PushbackData)
{
    const auto line = "foo bar baz";
    token_state ts = {};
    set_token_string(&ts, line);
    assert_token(__LINE__, &ts, TOK_ID, "foo");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    push_back_token_data(&ts, "buzz_");
    assert_token(__LINE__, &ts, TOK_ID, "buzz_bar");
    assert_token(__LINE__, &ts, TOK_WS, " ");
    assert_token(__LINE__, &ts, TOK_ID, "baz", true);
}

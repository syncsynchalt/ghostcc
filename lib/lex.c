#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "lex.h"

// map of ASCII character to what token type they might start
static token_type tok_start[128] = {
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_WS,  TOK_WS,  TOK_WS,  TOK_ERR, TOK_WS,  TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_WS,  '!',     TOK_STR, TOK_PP_STR, TOK_ERR, '%',  '&',     TOK_CHA,
    '(',     ')',     '*',     '+',     ',',     '-',     '.',     '/',
    TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM,
    TOK_NUM, TOK_NUM, ':',     ';',     '<',     '=',     '>',     '?',
    TOK_ERR, TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  '[', TOK_PP_CONTINUE, ']', '^',     TOK_ID,
    TOK_ERR, TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  '{',     '|',     '}',     '~',     TOK_ERR,
};

static char *keywords[] = {"auto",     "break",  "case",     "char",   "const",    "continue", "default",
                           "do",       "double", "else",     "enum",   "extern",   "float",    "for",
                           "goto",     "if",     "int",      "long",   "register", "return",   "short",
                           "signed",   "sizeof", "static",   "struct", "switch",   "typedef",  "union",
                           "unsigned", "void",   "volatile", "while",  NULL};

static int copy_token(token_state *token, const token_type type, size_t len)
{
    if (len + 1 > token->_tok_max) {
        token->_tok_max = len + 1;
        token->tok = realloc(token->tok, token->_tok_max);
    }
    memcpy(token->tok, token->_line + token->ind, len);
    token->tok[len] = '\0';
    token->type = type;
    token->pos = token->ind;
    token->ind += len;
    return token->ind < token->_line_len;
}

static int die_at(const token_state *token, const size_t ind, const char *msg)
{
    fprintf(stderr, "ERR: %s\n", msg);
    fprintf(stderr, "     %s", token->_line);
    if (token->_line[strlen(token->_line) - 1] != '\n') {
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "     ");
    int i;
    for (i = 0; i < ind; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
    exit(1);
}

static int eat_err(token_state *token)
{
    size_t i;
    for (i = token->ind; i < token->_line_len - 1; i++) {
        if (!token->_line[i]) {
            break;
        }
        if ((unsigned char) token->_line[i] < 0x7f && tok_start[token->_line[i]] != TOK_ERR) {
            break;
        }
    }
    return copy_token(token, TOK_ERR, i - token->ind);
}

static int eat_comment(token_state *token)
{
    size_t i;
    for (i = token->ind; i < token->_line_len - 1; i++) {
        if (token->_line[i] == '*' && token->_line[i + 1] == '/') {
            token->_comment_level--;
            if (!token->_comment_level) {
                copy_token(token, TOK_COMMENT, 1);
                // collapse the comment to a single space
                strcpy(token->tok, " ");
                token->ind = i + 2;
                return token->ind < token->_line_len;
            }
        } else if (token->_line[i] == '/' && token->_line[i + 1] == '*') {
            token->_comment_level++;
            i += 2;
        }
    }
    copy_token(token, TOK_COMMENT, 1);
    // not worth preserving
    strcpy(token->tok, "");
    token->ind = token->_line_len;
    return 0; // end of line
}

int get_token(const char *line, const size_t line_len, token_state *token)
{
    int len;
    size_t i;

    if (line && token->_line != line) {
        LINE_RESET(token);
        token->_line = line;
        token->_line_len = line_len;
    }

    if (token->_comment_level) {
        return eat_comment(token);
    }

    const char *p = token->_line + token->ind;
    int line_cont;

    token_type type;
    if ((unsigned char) token->_line[token->ind] > 127) {
        return eat_err(token);
    }

    switch ((int) (type = tok_start[token->_line[token->ind]])) {
        case TOK_ERR:
            // unrecognized symbol
            return eat_err(token);

        case TOK_WS:
            // whitespace
            len = strspn(token->_line + token->ind, WHITESPACE);
            return copy_token(token, TOK_WS, len);

        case TOK_ID:
            // identifier (variable name, argument name, function name, ...)
            len = strspn(p, UPPER LOWER NUMERIC "_");
            while (p[len] == ':' && p[len+1] == ':') {
                // special case for C++ namespace::separators that exist in system headers
                len += 2;
                len += strspn(p+len, UPPER LOWER NUMERIC "_");
            }
            line_cont = copy_token(token, TOK_ID, len);
            for (i = 0; keywords[i]; i++) {
                // keywords such as "auto", "continue", "return", ...
                if (strcmp(token->tok, keywords[i]) == 0) {
                    token->type = 600 + i;
                }
            }
            return line_cont;

        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ',':
        case '~':
        case ':':
        case ';':
        case '?':
            // single-character token
            copy_token(token, type, 1);
            line_cont = token->ind < token->_line_len;
            return line_cont;

        case TOK_NUM:
            // numeric constant
            if (strncmp(p, "0x", 2) == 0 || strncmp(p, "0X", 2) == 0) {
                // 0xABCDEF
                len = 2;
                len += strspn(p + len, HEXNUM);
                len += strspn(p + len, "uUlLfF");
            } else {
                // 123, 0123, 1.23, 1.2e3, 123u, 123L, 12.3f, 12.3F, etc
                len = strspn(p, NUMERIC);
                len += strspn(p + len, ".");
                len += strspn(p + len, NUMERIC);
                if (p[len] == 'e') {
                    len += 1;
                    if (p[len] == '-') {
                        len++;
                    }
                    len += strspn(p + len, NUMERIC);
                }
                len += strspn(p + len, "uUlLfF");
            }
            return copy_token(token, TOK_NUM, len);

        case TOK_STR:
            // string constant: "foo", "\"bar\"", ...
            p = p + 1;
            while (*p && *p != '"') {
                if (*p == '\\') {
                    p++;
                }
                p++;
            }
            if (*p != '"') {
                return die_at(token, p - (token->_line + token->ind), "unterminated quote");
            }
            return copy_token(token, TOK_STR, p - (token->_line + token->ind) + 1);

        case TOK_CHA:
            // character constant: 'a', '\0', '\b', ...
            if (*(p + 1) == '\\') {
                len = 4;
            } else {
                len = 3;
            }
            if (*(p + len - 1) != '\'') {
                return die_at(token, p - token->_line, "unterminated character");
            }
            return copy_token(token, TOK_CHA, len);

        case TOK_PP_STR: {
            if (p[1] == '#') {
                // preprocessor combine tokens: "##"
                return copy_token(token, TOK_PP_COMBINE, 2);
            }
            // preprocessor stringify token: "#"
            return copy_token(token, TOK_PP_STR, 1);
        }

        case TOK_PP_CONTINUE:
            // preprocessor line continuation: ending a line with backslash
            len = strspn(p + 1, "\r\n");
            if (token->ind + 1 + len != token->_line_len) {
                return die_at(token, token->ind, "backslash (not at end of line)");
            }
            return copy_token(token, TOK_PP_CONTINUE, token->_line_len - token->ind);

        case '+':
            if (p[1] == '+') {
                // ++
                return copy_token(token, TOK_INC_OP, 2);
            }
            if (p[1] == '=') {
                // +=
                return copy_token(token, TOK_ADD_ASSIGN, 2);
            }
            // +
            return copy_token(token, '+', 1);

        case '-':
            if (p[1] == '-') {
                // --
                return copy_token(token, TOK_DEC_OP, 2);
            }
            if (p[1] == '>') {
                // ->
                return copy_token(token, TOK_PTR_OP, 2);
            }
            if (p[1] == '=') {
                // -=
                return copy_token(token, TOK_SUB_ASSIGN, 2);
            }
            // -
            return copy_token(token, '-', 1);

        case '!':
            if (p[1] == '=') {
                // !=
                return copy_token(token, TOK_NE_OP, 2);
            }
            // !
            return copy_token(token, '!', 1);

        case '&':
            if (p[1] == '&') {
                // &&
                return copy_token(token, TOK_AND_OP, 2);
            }
            if (p[1] == '=') {
                // &=
                return copy_token(token, TOK_AND_ASSIGN, 2);
            }
            // &
            return copy_token(token, '&', 1);

        case '|':
            if (p[1] == '|') {
                // ||
                return copy_token(token, TOK_OR_OP, 2);
            }
            if (p[1] == '=') {
                // |=
                return copy_token(token, TOK_OR_ASSIGN, 2);
            }
            // |
            return copy_token(token, '|', 1);

        case '%':
            if (p[1] == '=') {
                // %=
                return copy_token(token, TOK_MOD_ASSIGN, 2);
            }
            // %
            return copy_token(token, '%', 1);

        case '^':
            if (p[1] == '=') {
                // ^=
                return copy_token(token, TOK_XOR_ASSIGN, 2);
            }
            // ^
            return copy_token(token, '^', 1);

        case '*':
            if (p[1] == '=') {
                // *=
                return copy_token(token, TOK_MUL_ASSIGN, 2);
            }
            // *
            return copy_token(token, '*', 1);

        case '/':
            if (p[1] == '*') {
                return eat_comment(token);
            }
            if (p[1] == '/') {
                return copy_token(token, TOK_LINE_COMMENT, token->_line_len - token->ind);
            }
            if (p[1] == '=') {
                // /=
                return copy_token(token, TOK_DIV_ASSIGN, 2);
            }
            // /
            return copy_token(token, '/', 1);

        case '<':
            if (p[1] == '<') {
                if (p[2] == '=') {
                    // <<=
                    return copy_token(token, TOK_LEFT_ASSIGN, 3);
                }
                // <<
                return copy_token(token, TOK_LEFT_OP, 2);
            }
            if (p[1] == '=') {
                // <=
                return copy_token(token, TOK_LE_OP, 2);
            }
            // <
            return copy_token(token, '<', 1);

        case '>':
            if (p[1] == '>') {
                if (p[2] == '=') {
                    // >>=
                    return copy_token(token, TOK_RIGHT_ASSIGN, 3);
                }
                // >>
                return copy_token(token, TOK_RIGHT_OP, 2);
            }
            if (p[1] == '=') {
                // >=
                return copy_token(token, TOK_GE_OP, 2);
            }
            // >
            return copy_token(token, '>', 1);

        case '=':
            if (p[1] == '=') {
                // ==
                return copy_token(token, TOK_EQ_OP, 2);
            }
            // =
            return copy_token(token, '=', 1);

        case '.':
            if (p[1] == '.' && p[2] == '.') {
                // ...
                return copy_token(token, TOK_ELLIPSIS, 3);
            }
            // .
            return copy_token(token, '.', 1);

        default:
            return die_at(token, token->ind, "unmapped character");
    }
}

static char *letter_map = "       abtnvfr";

char *decode_str(const char *s, char *out, const size_t len)
{
    if (*s != '"') {
        return NULL;
    }
    char *q = out;
    const char *p = s + 1;
    for (; *p != '"' && q - out < len; p++, q++) {
        if (*p == '\\') {
            const char *lookup = strchr(letter_map, p[1]);
            if (p[1] && lookup) {
                *q = (char)(lookup - letter_map);
                p++;
            } else if (strspn(p + 1, "01234567") >= 3) {
                char buf[4];
                snprintf(buf, sizeof(buf), "%.3s", p + 1);
                *q = strtol(buf, NULL, 8);
                p += 3;
            } else if (p[1] == 'x' && strspn(p + 2, HEXNUM) >= 2) {
                char buf[3];
                snprintf(buf, sizeof(buf), "%.2s", p + 2);
                *q = strtol(buf, NULL, 16);
                p += 3;
            } else {
                *q = p[1];
                p++;
            }
        } else {
            *q = *p;
        }
    }
    if (*p != '"') {
        return NULL;
    }
    return out;
}

char *quote_str(const char *s)
{
    const size_t slen = strlen(s);
    char *ret = calloc(1, slen * 4 + 3);
    size_t i, j = 0;

    ret[j++] = '"';
    for (i = 0; i < slen; i++) {
        switch (s[i]) {
            case '\a':
            case '\b':
            case '\t':
            case '\n':
            case '\v':
            case '\f':
            case '\r':
                sprintf(ret + j, "\\%c", letter_map[(int)s[i]]);
                j += 2;
                break;
            case '\"':
            case '\\':
                sprintf(ret + j, "\\%c", s[i]);
                j += 2;
                break;
            default:
                if (!isprint(s[i])) {
                    sprintf(ret + j, "\\x%02x", (unsigned char) s[i]);
                    j += 4;
                    break;
                }
                ret[j++] = s[i];
                break;
        }
    }
    ret[j] = '"';
    return ret;
}

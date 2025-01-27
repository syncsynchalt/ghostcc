#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "lex.h"
#include "die.h"

// map of ASCII character to what token type they might start
static token_type tok_start[128] = {
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_WS,  '\n',    TOK_WS,  TOK_ERR, TOK_WS,  TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_WS,  '!',     TOK_STR, '#', TOK_ERR, '%',  '&',     TOK_CHA,
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

// characters that are legal in a non-hex int/float token (0-9, ".", "e", "E", "-", "UuLlFf")
static int is_valid_num_char[128] = {
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
};

static char *keywords[] = {"auto",     "break",  "case",     "char",   "const",    "continue", "default",
                           "do",       "double", "else",     "enum",   "extern",   "float",    "for",
                           "goto",     "if",     "int",      "long",   "register", "return",   "short",
                           "signed",   "sizeof", "static",   "struct", "switch",   "typedef",  "union",
                           "unsigned", "void",   "volatile", "while",  NULL};

static void eat_comment(int (*getch)(void *), void *param)
{
    int comment_level = 1;
    for (;;) {
        // xxx todo need to recognize and ignore directives
        int c = getch(param);
        if (c == EOF) {
            die("EOF while looking for end of comment");
        }
        if (c == '/') {
            c = getch(param);
            if (c == '*') {
                comment_level++;
            }
        } else if (c == '*') {
            c = getch(param);
            if (c == '/') {
                comment_level--;
                if (comment_level == 0) {
                    return;
                }
            }
        }
    }
}

#define add_token_buf_chr(c) \
    do { \
        if (token_buf_ind >= TOKEN_BUF_SZ) die("token too long"); \
        if (c != EOF) token_buf[token_buf_ind++] = c; \
        token_buf[token_buf_ind] = '\0'; \
    } while (0)

static token eat_err(int (*getch)(void *), int (*ungetch)(int, void *), void *param, char *token_buf)
{
    size_t token_buf_ind = strlen(token_buf);
    int c;
    for (;;) {
        c = getch(param);
        if (c == EOF || ((unsigned char)c < 128 && tok_start[c] != TOK_ERR)) {
            break;
        }
        add_token_buf_chr(c);
    }
    ungetch(c, param);

    token t;
    t.type = TOK_ERR;
    t.tok = token_buf;
    return t;
}

token read_token(int (*getch)(void *), int (*ungetch)(int c, void *param), void *param, char *token_buf)
{
    size_t i;
    token t;

    t.tok = token_buf;
    strcpy(token_buf, "");
    size_t token_buf_ind = 0;

    int c = getch(param);

    if (c == EOF) {
        t.type = TOK_EOF;
        return t;
    }

    if ((unsigned char)c >= 128) {
        add_token_buf_chr(c);
        return eat_err(getch, ungetch, param, token_buf);
    }

    add_token_buf_chr(c);
    switch ((int) (t.type = tok_start[c])) {
        case TOK_ERR:
            // unrecognized symbol
            return eat_err(getch, ungetch, param, token_buf);

        case '\n':
            // whitespace
            t.type = TOK_WS;
            break;

        case TOK_WS:
            // whitespace
            for (;;) {
                c = getch(param);
                if (isspace(c)) {
                    add_token_buf_chr(c);
                    if (c == '\n') {
                        break;
                    }
                } else {
                    ungetch(c, param);
                    break;
                }
            }
            break;

        case TOK_ID:
            // identifier (variable name, argument name, function name, ...)
            for (;;) {
                c = getch(param);
                if (isalnum(c) || c == '_') {
                    add_token_buf_chr(c);
                } else if (c == ':') {
                    c = getch(param);
                    if (c == ':') {
                        add_token_buf_chr(':');
                        add_token_buf_chr(':');
                    } else {
                        ungetch(c, param);
                        ungetch(':', param);
                        break;
                    }
                } else {
                    ungetch(c, param);
                    break;
                }
            }

            for (i = 0; keywords[i]; i++) {
                // keywords such as "auto", "continue", "return", ...
                if (strcmp(t.tok, keywords[i]) == 0) {
                    t.type = 600 + i;
                }
            }
            break;

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
            break;

        case TOK_NUM:
            // numeric constant
            if (c == '0') {
                c = getch(param);
                if (c == 'x' || c == 'X') {
                    add_token_buf_chr(c);
                    c = getch(param);
                    while (isxdigit(c) || c == 'u' || c == 'U' || c == 'l' || c == 'L') {
                        add_token_buf_chr(c);
                        c = getch(param);
                    }
                } else {
                    while (is_valid_num_char[c & 0x7f]) {
                        add_token_buf_chr(c);
                        c = getch(param);
                    }
                }
            } else {
                c = getch(param);
                while (is_valid_num_char[c & 0x7f]) {
                    add_token_buf_chr(c);
                    c = getch(param);
                }
            }
            ungetch(c, param);
            break;

        case TOK_STR:
            for (;;) {
                c = getch(param);
                if (c == EOF) {
                    die("unterminated quote");
                }
                add_token_buf_chr(c);
                if (c == '"') {
                    break;
                }
                if (c == '\\') {
                    c = getch(param);
                    add_token_buf_chr(c);
                }
            }
            break;

        case TOK_CHA:
            // character constant: 'a', '\0', '\b', ...
            c = getch(param);
            if (c == EOF) {
                die("unterminated character constant");
            }
            add_token_buf_chr(c);
            if (c == '\\') {
                c = getch(param);
                add_token_buf_chr(c);
            }
            c = getch(param);
            if (c != '\'') {
                die("unterminated character constant");
            }
            add_token_buf_chr(c);
            break;

        case '#':
            // preprocessor tokens, "#" or "##"
            c = getch(param);
            if (c == '#') {
                add_token_buf_chr(c);
                t.type = TOK_PP_COMBINE;
            } else {
                ungetch(c, param);
                t.type = '#';
            }
            break;

        case TOK_PP_CONTINUE:
            // preprocessor line continuation: ending a line with backslash
            c = getch(param);
            if (c == '\r' || c == '\n') {
                add_token_buf_chr(c);
                if (c == '\r') {
                    c = getch(param);
                    add_token_buf_chr(c);
                }
                t.type = TOK_PP_CONTINUE;
            } else {
                t.type = '\\';
                ungetch(c, param);
            }
            break;

        case '+':
            c = getch(param);
            if (c == '+') {
                add_token_buf_chr(c);
                t.type = TOK_INC_OP;
            } else if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_ADD_ASSIGN;
            } else {
                ungetch(c, param);
            }
            break;

        case '-':
            c = getch(param);
            if (c == '-') {
                add_token_buf_chr(c);
                t.type = TOK_DEC_OP;
            } else if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_SUB_ASSIGN;
            } else if (c == '>') {
                add_token_buf_chr(c);
                t.type = TOK_PTR_OP;
            } else {
                ungetch(c, param);
            }
            break;

        case '!':
            c = getch(param);
            if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_NE_OP;
            } else {
                ungetch(c, param);
            }
            break;

        case '&':
            c = getch(param);
            if (c == '&') {
                add_token_buf_chr(c);
                t.type = TOK_AND_OP;
            } else if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_AND_ASSIGN;
            } else {
                ungetch(c, param);
            }
            break;

        case '|':
            c = getch(param);
            if (c == '|') {
                add_token_buf_chr(c);
                t.type = TOK_OR_OP;
            } else if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_OR_ASSIGN;
            } else {
                ungetch(c, param);
            }
            break;

        case '%':
            c = getch(param);
            if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_MOD_ASSIGN;
            } else {
                ungetch(c, param);
            }
            break;

        case '^':
            c = getch(param);
            if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_XOR_ASSIGN;
            } else {
                ungetch(c, param);
            }
            break;

        case '*':
            c = getch(param);
            if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_MUL_ASSIGN;
            } else {
                ungetch(c, param);
            }
            break;

        case '/':
            c = getch(param);
            if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_DIV_ASSIGN;
            } else if (c == '/') {
                for (;;) {
                    c = getch(param);
                    if (c == '\r' || c == '\n') {
                        ungetch(c, param);
                        break;
                    }
                }
                t.type = TOK_LINE_COMMENT;
                strcpy(token_buf, " ");
            } else if (c == '*') {
                eat_comment(getch, param);
                strcpy(token_buf, " ");
                t.type = TOK_COMMENT;
            } else {
                ungetch(c, param);
            }
            break;

        case '<':
            c = getch(param);
            if (c == '<') {
                add_token_buf_chr(c);
                t.type = TOK_LEFT_OP;
                c = getch(param);
                if (c == '=') {
                    add_token_buf_chr(c);
                    t.type = TOK_LEFT_ASSIGN;
                } else {
                    ungetch(c, param);
                }
            } else if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_LE_OP;
            } else {
                ungetch(c, param);
            }
            break;

        case '>':
            c = getch(param);
            if (c == '>') {
                add_token_buf_chr(c);
                t.type = TOK_RIGHT_OP;
                c = getch(param);
                if (c == '=') {
                    add_token_buf_chr(c);
                    t.type = TOK_RIGHT_ASSIGN;
                } else {
                    ungetch(c, param);
                }
            } else if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_GE_OP;
            } else {
                ungetch(c, param);
            }
            break;

        case '=':
            c = getch(param);
            if (c == '=') {
                add_token_buf_chr(c);
                t.type = TOK_EQ_OP;
            } else {
                ungetch(c, param);
            }
            break;

        case '.':
            c = getch(param);
            if (c == '.') {
                add_token_buf_chr(c);
                if (getch(param) != '.') {
                    die("Incomplete ellipsis (double dot)");
                }
                add_token_buf_chr('.');
                t.type = TOK_ELLIPSIS;
            } else {
                ungetch(c, param);
            }
            break;

        default:
            die("unmapped character %c", c);
    }
    t.tok = token_buf;
    return t;
}

static const char *letter_map = "       abtnvfr";

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
    *q = '\0';
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

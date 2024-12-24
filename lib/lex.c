#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lex.h"
#include "common.h"

// map of ASCII character to what token type they might start
static token_type tok_start[128] = {
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_WS,  TOK_WS,  TOK_WS,  TOK_ERR, TOK_WS,  TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR, TOK_ERR,
    TOK_WS, TOK_BANG, TOK_STR, TOK_PP_STR, TOK_ERR, TOK_MOD, TOK_AND, TOK_CHA,
    TOK_LPAREN, TOK_RPAREN, TOK_MULT, TOK_PLUS, TOK_COMMA, TOK_MINUS, TOK_DOT, TOK_DIV,
    TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM, TOK_NUM,
    TOK_NUM, TOK_NUM, TOK_COLON, TOK_SEMI, TOK_LT, TOK_ASSIGN, TOK_GT, TOK_QUESTION,
    TOK_ERR, TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_LBRACK, TOK_PP_CONTINUE, TOK_RBRACK, TOK_XOR, TOK_ID,
    TOK_ERR, TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,  TOK_ID,
    TOK_ID,  TOK_ID,  TOK_ID,  TOK_LBRACE, TOK_OR, TOK_RBRACE, TOK_BIT_FLIP, TOK_ERR,
};

static char *keywords[] = {
    "auto", "double", "int", "struct", "break", "else", "long", "switch",
    "case", "enum", "register", "typedef", "char", "extern", "return",
    "union", "const", "float", "short", "unsigned", "continue", "for",
    "signed", "void", "default", "goto", "sizeof", "volatile", "do",
    "if", "static", "while", NULL
};

static int copy_token(token_state *token, token_type type, size_t len)
{
    if (len+1 > token->_tok_max) {
        token->_tok_max = len + 1;
        token->tok = realloc(token->tok, token->_tok_max);
    }
    if (type == TOK_COMMENT) {
        strcpy(token->tok, " ");
    } else {
        memcpy(token->tok, token->_line + token->ind, len);
        token->tok[len] = '\0';
    }
    token->type = type;
    token->pos = token->ind;
    token->ind += len;
    return token->ind >= token->_line_len;
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

static int eat_err(const char *line, const size_t line_len, token_state *token)
{
    size_t i;
    for (i = token->ind; i < line_len - 1; i++) {
        if (!line[i]) {
            break;
        }
        if ((unsigned char)line[i] < 0x7f && tok_start[line[i]] != TOK_ERR) {
            break;
        }
    }
    return copy_token(token, TOK_ERR, i - token->ind);
}

static int eat_comment(const char *line, const size_t line_len, token_state *token)
{
    size_t i;
    for (i = token->ind; i < line_len - 1; i++) {
        if (line[i] == '*' && line[i+1] == '/') {
            token->_comment_level--;
            if (!token->_comment_level) {
                copy_token(token, TOK_COMMENT, 1);
                token->ind = i + 2;
                return token->ind >= line_len;
            }
        } else if (line[i] == '/' && line[i+1] == '*') {
            token->_comment_level++;
            i += 2;
        }
    }
    copy_token(token, TOK_COMMENT, 1);
    token->ind = line_len;
    return 1; // end of line
}

int get_token(const char *line, const size_t line_len, token_state *token)
{
    int len;
    size_t i;

    if (token->_line != line) {
        // reset for new line
        token->_line = line;
        token->_line_len = line_len;
        token->ind = 0;
    }

    if (token->_comment_level) {
        return eat_comment(line, line_len, token);
    }

    const char *p = line + token->ind;
    int line_done;

    token_type type;
    if ((unsigned char)line[token->ind] > 127) {
        return eat_err(line, line_len, token);
    }

    switch ((type = tok_start[line[token->ind]])) {
    case TOK_ERR:
        // unrecognized symbol
        return eat_err(line, line_len, token);

    case TOK_WS:
        // whitespace
        len = strspn(line + token->ind, WHITESPACE);
        return copy_token(token, TOK_WS, len);

    case TOK_ID:
        // identifier (variable name, argument name, function name, ...)
        len = strspn(p, UPPER LOWER NUMERIC "_");
        line_done = copy_token(token, TOK_ID, len);
        for (i = 0; keywords[i]; i++) {
            // keywords such as "auto", "continue", "return"
            if (strcmp(token->tok, keywords[i]) == 0) {
                token->type = TOK_KEYWORD;
            }
        }
        return line_done;

    case TOK_LPAREN: case TOK_RPAREN:
    case TOK_LBRACK: case TOK_RBRACK:
    case TOK_LBRACE: case TOK_RBRACE:
    case TOK_COMMA: case TOK_BIT_FLIP:
    case TOK_COLON: case TOK_SEMI:
    case TOK_QUESTION:
        // single-character token
        // "(", ")", "{", ";", "?", ":", ...
        copy_token(token, type, 1);
        line_done = token->ind >= line_len;
        return line_done;

    case TOK_NUM:
        // numeric constant
        if (strncmp(p, "0x", 2) == 0 || strncmp(p, "0X", 2) == 0) {
            // 0xABCDEF
            len = 2;
            len += strspn(p + len, HEXNUM);
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
            return die_at(token, p - (line + token->ind), "unterminated quote");
        }
        return copy_token(token, TOK_STR, p - (line + token->ind) + 1);

    case TOK_CHA:
        // character constant: 'a', '\0', '\b', ...
        if (*(p + 1) == '\\') {
            len = 4;
        } else {
            len = 3;
        }
        if (*(p + len - 1) != '\'') {
            return die_at(token, p - line, "unterminated character");
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
        if (token->ind + 1 + len != line_len) {
            return die_at(token, token->ind, "backslash (not at end of line)");
        }
        return copy_token(token, TOK_PP_CONTINUE, line_len - token->ind);

    case TOK_PLUS:
        if (p[1] == '+') {
            // ++
            return copy_token(token, TOK_PLUSPLUS, 2);
        }
        if (p[1] == '=') {
            // +=
            return copy_token(token, TOK_ASSIGN_PLUS, 2);
        }
        // +
        return copy_token(token, TOK_PLUS, 1);

    case TOK_MINUS:
        if (p[1] == '-') {
            // --
            return copy_token(token, TOK_MINUSMINUS, 2);
        }
        if (p[1] == '>') {
            // ->
            return copy_token(token, TOK_ARROW, 2);
        }
        if (p[1] == '=') {
            // -=
            return copy_token(token, TOK_ASSIGN_MINUS, 2);
        }
        // -
        return copy_token(token, TOK_MINUS, 1);

    case TOK_BANG:
        if (p[1] == '=') {
            // !=
            return copy_token(token, TOK_NE, 2);
        }
        // !
        return copy_token(token, TOK_BANG, 1);

    case TOK_AND:
        if (p[1] == '&') {
            // &&
            return copy_token(token, TOK_LOGICAL_AND, 2);
        }
        if (p[1] == '=') {
            // &=
            return copy_token(token, TOK_ASSIGN_AND, 2);
        }
        // &
        return copy_token(token, TOK_AND, 1);

    case TOK_OR:
        if (p[1] == '|') {
            // ||
            return copy_token(token, TOK_LOGICAL_OR, 2);
        }
        if (p[1] == '=') {
            // |=
            return copy_token(token, TOK_ASSIGN_OR, 2);
        }
        // |
        return copy_token(token, TOK_OR, 1);

    case TOK_MOD:
        if (p[1] == '=') {
            // %=
            return copy_token(token, TOK_ASSIGN_MOD, 2);
        }
        // %
        return copy_token(token, TOK_MOD, 1);

    case TOK_XOR:
        if (p[1] == '=') {
            // ^=
            return copy_token(token, TOK_ASSIGN_XOR, 2);
        }
        // ^
        return copy_token(token, TOK_XOR, 1);

    case TOK_MULT:
        if (p[1] == '=') {
            // *=
            return copy_token(token, TOK_ASSIGN_MULT, 2);
        }
        // *
        return copy_token(token, TOK_MULT, 1);

    case TOK_DIV:
        if (p[1] == '*') {
            token->_comment_level++;
            token->ind += 2;
            return eat_comment(line, line_len, token);
        }
        if (p[1] == '/') {
            return copy_token(token, TOK_LINE_COMMENT, line_len - token->ind);
        }
        if (p[1] == '=') {
            // /=
            return copy_token(token, TOK_ASSIGN_DIV, 2);
        }
        // /
        return copy_token(token, TOK_DIV, 1);

    case TOK_LT:
        if (p[1] == '<') {
            if (p[2] == '=') {
                // <<=
                return copy_token(token, TOK_ASSIGN_LSHIFT, 3);
            }
            // <<
            return copy_token(token, TOK_LSHIFT, 2);
        }
        if (p[1] == '=') {
            // <=
            return copy_token(token, TOK_LE, 2);
        }
        // <
        return copy_token(token, TOK_LT, 1);

    case TOK_GT:
        if (p[1] == '>') {
            if (p[2] == '=') {
                // >>=
                return copy_token(token, TOK_ASSIGN_RSHIFT, 3);
            }
            // >>
            return copy_token(token, TOK_RSHIFT, 2);
        }
        if (p[1] == '=') {
            // >=
            return copy_token(token, TOK_GE, 2);
        }
        // >
        return copy_token(token, TOK_GT, 1);

    case TOK_ASSIGN:
        if (p[1] == '=') {
            // ==
            return copy_token(token, TOK_EQ, 2);
        }
        // =
        return copy_token(token, TOK_ASSIGN, 1);

    case TOK_DOT:
            if (p[1] == '.' && p[2] == '.') {
                // ...
                return copy_token(token, TOK_DOTDOTDOT, 3);
            }
        // .
        return copy_token(token, TOK_DOT, 1);

    default:
        return die_at(token, token->ind, "unmapped character");
    }
}

char *decode_str(const char *s, char *out, const size_t len)
{
    if (*s != '"') {
        return NULL;
    }
    char *q = out;
    const char *p = s + 1;
    for (; *p != '"' && q - out < len; p++, q++) {
        if (*p == '\\') {
            if (p[1] == 'a') { *q = '\a'; }
            else if (p[1] == 'b') { *q = '\b'; p++; }
            else if (p[1] == 'f') { *q = '\f'; p++; }
            else if (p[1] == 'n') { *q = '\n'; p++; }
            else if (p[1] == 'r') { *q = '\r'; p++; }
            else if (p[1] == 't') { *q = '\t'; p++; }
            else if (p[1] == 'v') { *q = '\v'; p++; }
            else if (strspn(p+1, "01234567") >= 3) {
                char buf[4];
                snprintf(buf, sizeof(buf), "%.3s", p+1);
                *q = strtol(buf, NULL, 8);
                p += 3;
            } else if (p[1] == 'x' && strspn(p+2, HEXNUM) >= 2) {
                char buf[3];
                snprintf(buf, sizeof(buf), "%.2s", p+2);
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

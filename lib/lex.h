#ifndef INC_LEX_H
#define INC_LEX_H


typedef enum {
    TOK_WS = 1, TOK_KEYWORD, TOK_ID,
    TOK_NUM, TOK_STR, TOK_CHA,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE, TOK_LBRACK, TOK_RBRACK,
    TOK_DOT, TOK_DOTDOTDOT, TOK_ARROW, TOK_COMMA,

    TOK_PLUS, TOK_MINUS, TOK_MULT, TOK_DIV, TOK_LSHIFT, TOK_RSHIFT,
    TOK_MOD, TOK_AND, TOK_OR, TOK_XOR,
    TOK_ASSIGN,
    TOK_ASSIGN_PLUS, TOK_ASSIGN_MINUS, TOK_ASSIGN_MULT, TOK_ASSIGN_DIV, TOK_ASSIGN_LSHIFT, TOK_ASSIGN_RSHIFT,
    TOK_ASSIGN_MOD, TOK_ASSIGN_AND, TOK_ASSIGN_OR, TOK_ASSIGN_XOR,

    TOK_BIT_FLIP,
    TOK_LOGICAL_AND, TOK_LOGICAL_OR,
    TOK_QUESTION, TOK_COLON, TOK_SEMI, TOK_BANG,
    TOK_EQ, TOK_NE, TOK_LE, TOK_LT, TOK_GE, TOK_GT,
    TOK_PLUSPLUS, TOK_MINUSMINUS,

    // pre-processor only
    TOK_PP_STR, TOK_PP_COMBINE, TOK_PP_CONTINUE,
    TOK_COMMENT, TOK_LINE_COMMENT,

    TOK_ERR,
} token_type;

typedef struct {
    token_type type; //< parsed token type
    char *tok;       //< parsed token as string
    size_t pos;      //< offset into line that token starts at

    int _tok_max;
    const char *_line;
    size_t _line_len;
    size_t _i;
    int _comment_level;
} token_state;

/*
 * get the next token from the given line
 *
 * token - parsing state, allocated by caller and updated by get_token.
 *         Resets automatically every time `line` changes.
 */
extern int get_token(const char *line, size_t line_len, token_state *token);

#endif

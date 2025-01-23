#pragma once

typedef enum {
    TOK_WS = 500,
    TOK_KW_AUTO = 600,
    TOK_KW_BREAK,
    TOK_KW_CASE,
    TOK_KW_CHAR,
    TOK_KW_CONST,
    TOK_KW_CONTINUE,
    TOK_KW_DEFAULT,
    TOK_KW_DO,
    TOK_KW_DOUBLE,
    TOK_KW_ELSE,
    TOK_KW_ENUM,
    TOK_KW_EXTERN,
    TOK_KW_FLOAT,
    TOK_KW_FOR,
    TOK_KW_GOTO,
    TOK_KW_IF,
    TOK_KW_INT,
    TOK_KW_LONG,
    TOK_KW_REGISTER,
    TOK_KW_RETURN,
    TOK_KW_SHORT,
    TOK_KW_SIGNED,
    TOK_KW_SIZEOF,
    TOK_KW_STATIC,
    TOK_KW_STRUCT,
    TOK_KW_SWITCH,
    TOK_KW_TYPEDEF,
    TOK_KW_UNION,
    TOK_KW_UNSIGNED,
    TOK_KW_VOID,
    TOK_KW_VOLATILE,
    TOK_KW_WHILE,

    TOK_ID = 700,
    TOK_NUM, // literal
    TOK_STR, // literal
    TOK_CHA, // literal

    TOK_ELLIPSIS,
    TOK_PTR_OP,
    TOK_LEFT_OP,
    TOK_RIGHT_OP,

    TOK_AND_OP,
    TOK_OR_OP,
    TOK_EQ_OP,
    TOK_NE_OP,
    TOK_LE_OP,
    TOK_GE_OP,
    TOK_INC_OP,
    TOK_DEC_OP,

    TOK_ADD_ASSIGN = 800,
    TOK_SUB_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_LEFT_ASSIGN,
    TOK_RIGHT_ASSIGN,
    TOK_MOD_ASSIGN,
    TOK_AND_ASSIGN,
    TOK_OR_ASSIGN,
    TOK_XOR_ASSIGN,

    TOK_TYPE_NAME,

    // pre-processor only
    TOK_PP_COMBINE = 900,
    TOK_PP_CONTINUE,
    TOK_COMMENT,
    TOK_LINE_COMMENT,

    TOK_ERR = 1000,
    TOK_EOF = -1,
} token_type;

#define IS_KEYWORD(t) (t >= 600 && t < 700)
#define IS_LITERAL(t) (t == TOK_CHA || t == TOK_STR || t == TOK_NUM)
#define IS_GROUP(t) (t == TOK_LPAREN)
#define IS_RVALUE(t) (IS_LITERAL(t) || IS_GROUP(t) || t == TOK_KW || t == TOK_ID)

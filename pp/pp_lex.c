#include <string.h>
#include "pp_lex.h"
#include "lex.h"
#include <stdio.h>

static token_state s;
char *pp_parse_target;
size_t pp_parse_target_len;
ast_node *pp_parse_result;
char const *pp_parse_line;
size_t pp_parse_line_index;

void reset_parser(void)
{
    memset(&s, 0, sizeof(s));
    pp_parse_target = NULL;
    pp_parse_target_len = 0;
}

int yylex(void)
{
    if (!pp_parse_target_len) {
        pp_parse_target_len = strlen(pp_parse_target);
    }
    do {
        if (s.ind >= pp_parse_target_len) {
            yylval = NULL;
            return -1;
        }
        get_token(pp_parse_target, strlen(pp_parse_target), &s);
    } while (s.type == TOK_WS);

    pp_parse_line = s._line;
    pp_parse_line_index = s.pos;
    yylval = make_ast_node(&s, NULL, NULL);
    return s.type;
}

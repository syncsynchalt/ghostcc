#include <string.h>
#include <ctype.h>
#include "preprocessor.h"
#include "pp_ast.h"
#include "pp_lex.h"
#include "die.h"

extern int yyparse();

ast_node *pp_parse(const char *s, defines *defs)
{
    reset_parser();
    pp_parse_target = (char *)s;
    pp_parse_defs = defs;
    yyparse();
    return pp_parse_result;
}

ast_result pp_resolve_ast(const ast_node *node)
{
    ast_result r = {0};

    switch (node->node_type) {
        case NODE_CHA:
            r.ival = node->cval;
        return r;
        case NODE_INT:
            r.ival = node->ival;
        return r;
        case NODE_FLT:
            r.type = AST_RESULT_TYPE_FLT;
        r.fval = node->fval;
        r.ival = node->fval;
        return r;
        case NODE_STR:
            r.type = AST_RESULT_TYPE_STR;
        r.sval = node->s;
        r.ival = 0;
        return r;
        default:
            break;
    }

    ast_result r1 = {0}, r2 = {0};
    if (node->left) {
        r1 = pp_resolve_ast(node->left);
    }
    if (node->right) {
        r2 = pp_resolve_ast(node->right);
    }
#define V(res) (res.type == AST_RESULT_TYPE_FLT ? res.fval : res.ival)
#define VR(val) \
    if (r1.type == AST_RESULT_TYPE_FLT || r2.type == AST_RESULT_TYPE_FLT) { \
        r.type = AST_RESULT_TYPE_FLT; \
        r.ival = val; \
        r.fval = val; \
    } else { \
        r.ival = val; \
    }
    switch ((int)node->token_type) {
        case '+':
            VR(V(r1) + V(r2))
            return r;
        case '-':
            VR(V(r1) - V(r2))
            return r;
        case '*':
            VR(V(r1) * V(r2))
            return r;
        case '/':
            VR(V(r1) / V(r2))
            return r;
        case '^':
            r.ival = r1.ival ^ r2.ival;
            return r;
        case '%':
            VR(V(r1) / V(r2))
            return r;
        case '&':
            r.ival = r1.ival & r2.ival;
            return r;
        case TOK_RIGHT_OP:
            r.ival = r1.ival >> r2.ival;
            return r;
        case TOK_LEFT_OP:
            r.ival = r1.ival << r2.ival;
            return r;
        case TOK_AND_OP:
            r.ival = V(r1) && V(r2);
            return r;
        case TOK_OR_OP:
            r.ival = V(r1) || V(r2);
            return r;
        case TOK_EQ_OP:
        case '=':
            if (r1.type == AST_RESULT_TYPE_STR && r2.type == AST_RESULT_TYPE_STR) {
                r.ival = strcmp(r1.sval, r2.sval) == 0;
            } else {
                r.ival = V(r1) == V(r2);
            }
            return r;
        case ':':
            // ignore, this is always a right-node child of '?' and is handled below
            return r;
        case '?':
            if (!node->right || !node->right->left || !node->right->right) {
                die("Missing nodes in ternary");
                exit(1);
            }
            if (r1.type == AST_RESULT_TYPE_FLT) {
                return pp_resolve_ast(r1.fval ? node->right->left : node->right->right);
            } else {
                return pp_resolve_ast(r1.ival ? node->right->left : node->right->right);
            }
        default:
            die("Didn't recognize token type %d (%c / %s)",
                node->token_type,
                isprint(node->token_type) ? node->token_type : '?',
                node->s);
            exit(1);
    }
}

void yyerror(const char *s)
{
    fprintf(stderr, "%s\n", s);
    int i;
    fprintf(stderr, "line: %s\n     ", pp_parse_line);
    for (i = 0; i <= pp_parse_line_index; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
    exit(1);
}

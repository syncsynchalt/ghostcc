#include "pp_ast.h"
#include <ctype.h>
#include <string.h>
#include "die.h"
#include "pp_lex.h"
#include "preprocessor.h"

extern int yyparse();

ast_node *string_to_ast(const char *s, const defines *defs)
{
    reset_parser();
    pp_parse_target = (char *) s;
    pp_parse_defs = defs;
    yyparse();
    return pp_parse_result;
}

ast_result resolve_ast(const ast_node *node)
{
    ast_result r = {0};
    if (!node) {
        return r;
    }

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
#define V(res) (res.type == AST_RESULT_TYPE_FLT ? res.fval : res.ival)
#define VR(val)                                                                                                        \
    if (r1.type == AST_RESULT_TYPE_FLT || r2.type == AST_RESULT_TYPE_FLT) {                                            \
        r.type = AST_RESULT_TYPE_FLT;                                                                                  \
        r.ival = val;                                                                                                  \
        r.fval = val;                                                                                                  \
    } else {                                                                                                           \
        r.ival = val;                                                                                                  \
    }

    // handle the short-circuit operators specially
    switch ((int)node->token_type) {
        case TOK_AND_OP:
            r.ival = 0;
            r1 = resolve_ast(node->left);
            if (V(r1)) {
                r2 = resolve_ast(node->right);
                r.ival = V(r1) && V(r2);
            }
            return r;
        case TOK_OR_OP:
            r1 = resolve_ast(node->left);
            r.ival = V(r1) ? 1 : 0;
            if (!V(r1)) {
                r2 = resolve_ast(node->right);
                r.ival = V(r1) || V(r2);
            }
            return r;
        case ':':
            // this is always a right-node child of '?' and is handled below
            die("Unexpected : token in AST resolver");
        case '?':
            if (!node->left || !node->right || !node->right->left || !node->right->right) {
                die("Missing nodes in ternary");
            }
            r1 = resolve_ast(node->left);
            return resolve_ast(V(r1) ? node->right->left : node->right->right);
        default:
            break;
    }

    r1 = resolve_ast(node->left);
    r2 = resolve_ast(node->right);
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
        case '!':
            r.ival = V(r1) ? 0 : 1;
            return r;
        case TOK_RIGHT_OP:
            r.ival = r1.ival >> r2.ival;
            return r;
        case TOK_LEFT_OP:
            r.ival = r1.ival << r2.ival;
            return r;
        case TOK_EQ_OP:
        case '=':
            if (r1.type == AST_RESULT_TYPE_STR && r2.type == AST_RESULT_TYPE_STR) {
                r.ival = strcmp(r1.sval, r2.sval) == 0;
            } else {
                r.ival = V(r1) == V(r2);
            }
            return r;
        case '<':
            r.ival = V(r1) < V(r2) ? 1 : 0;
            return r;
        case '>':
            r.ival = V(r1) > V(r2) ? 1 : 0;
            return r;
        case TOK_LE_OP:
            r.ival = V(r1) <= V(r2) ? 1 : 0;
            return r;
        case TOK_GE_OP:
            r.ival = V(r1) >= V(r2) ? 1 : 0;
            return r;
        case TOK_ID:
            fprintf(stderr, "Warning: ignoring unrecognized identifier %s\n", node->s);
            r.ival = 0;
            return r;

        default:
            die("Didn't recognize token type 0x%x / %d (as char: %c, as str: %s)", node->token_type, node->token_type,
                isprint(node->token_type) ? node->token_type : ' ', node->s);
    }
}

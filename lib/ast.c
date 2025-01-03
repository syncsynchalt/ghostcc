#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ast_node *make_ast_node(const token_state *tok, ast_node *left, ast_node *right)
{
    char buf[256];

    // todo - track so we can free
    ast_node *node = calloc(1, sizeof *node);
    node->left = left;
    node->right = right;

    node->token_type = tok ? tok->type : 0;
    if (IS_KEYWORD(tok->type) || tok->type == TOK_ID) {
        node->s = strdup(tok->tok);
        node->node_type = NODE_OTHER;
    } else switch ((int)tok->type) {
        case TOK_NUM:
            snprintf(buf, sizeof(buf), "%s", tok->tok);
            // todo - handle type suffixes
            if (strchr(tok->tok, '.') || strchr(tok->tok, 'e') || strchr(tok->tok, 'E')) {
                node->node_type = NODE_FLT;
                node->fval = strtod(buf, NULL);
            } else {
                node->node_type = NODE_INT;
                node->ival = strtol(tok->tok, NULL, 0);
            }
            break;
        case TOK_STR:
            node->s = malloc(strlen(tok->tok) + 1);
            decode_str(tok->tok, node->s, strlen(tok->tok) + 1);
            node->node_type = NODE_STR;
            break;
        case TOK_CHA:
            node->cval = *tok->tok;
            node->node_type = NODE_CHA;
            break;
        default:
            node->node_type = NODE_OTHER;
            break;
    }
    return node;
}

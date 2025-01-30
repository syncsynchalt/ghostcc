#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ast_node *make_ast_node(const token t, ast_node *left, ast_node *right)
{
    char buf[256];

    // todo - track so we can free
    ast_node *node = calloc(1, sizeof *node);
    node->left = left;
    node->right = right;

    node->tok_type = t.type;
    if (IS_KEYWORD(t.type) || t.type == TOK_ID) {
        node->s = strdup(t.tok);
        node->type = NODE_OTHER;
    } else switch ((int)t.type) {
        case TOK_NUM:
            snprintf(buf, sizeof(buf), "%s", t.tok);
            // todo - handle type suffixes
            if (strchr(t.tok, '.') || strchr(t.tok, 'e') || strchr(t.tok, 'E')) {
                node->type = NODE_FLT;
                node->fval = strtod(buf, NULL);
            } else {
                node->type = NODE_INT;
                node->ival = strtol(t.tok, NULL, 0);
            }
            break;
        case TOK_STR:
            node->s = malloc(strlen(t.tok) + 1);
            decode_str(t.tok, node->s, strlen(t.tok) + 1);
            node->type = NODE_STR;
            break;
        case TOK_CHA:
            node->cval = t.tok[1] == '\\' ? t.tok[2] : t.tok[1];
            node->type = NODE_CHA;
            break;
        default:
            node->type = NODE_OTHER;
            break;
    }
    return node;
}

void add_to_ast_list(ast_node *node, ast_node *list_member)
{
    if (node->list_len % 5 == 0) {
        node->list = realloc(node->list, (node->list_len + 6) * sizeof(ast_node *));
    }
    node->list[node->list_len++] = list_member;
}

void free_ast_node(ast_node *node)
{
    free(node->s);
    free(node);
}

int contains_token(const ast_node *node, token_type t)
{
    if (!node) {
        return 0;
    }
    return (node->left && contains_token(node->left, t)) ||
        (node->right && contains_token(node->right, t)) ||
        node->tok_type == t;
}

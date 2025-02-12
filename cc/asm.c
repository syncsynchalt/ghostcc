#include "asm.h"
#include "typing.h"
#include "die.h"

static void print_globals(const ast_node *ast, FILE *out);
static void handle_function(const ast_node *type, const ast_node *name, const ast_node *statements, FILE *out);
static void resolve_statement(int reg, const ast_node *s, FILE *out);

void read_ast(const ast_node *ast, FILE *out)
{
    if (ast->type != NODE_TRANSLATION_UNIT) {
        die("Not a translation unit");
    }

    print_globals(ast, out);
    fprintf(out, "\n");

    int i;
    for (i = 0; ast->list && ast->list[i]; i++) {
        const ast_node *item = ast->list[i];
        if (item->type == NODE_DECL) {
            // todo - implement
        }
        else if (item->type == NODE_FUNCTION) {
            const ast_node *ret = item->left;
            const ast_node *nam = item->right;
            const ast_node *statements = item->list[0];
            handle_function(ret, nam, statements, out);
        }
    }

}

static void print_globals(const ast_node *ast, FILE *out)
{
    int printed_global = 0;

    int i;
    for (i = 0; ast->list && ast->list[i]; i++) {
        const ast_node *item = ast->list[i];
        if (item->type == NODE_FUNCTION) {
            if (!printed_global) {
                fprintf(out, ".global ");
                printed_global = 1;
            } else {
                fprintf(out, ", ");
            }

            const ast_node *nam = item->right;
            fprintf(out, "_%s", nam->left->s);
        }
        if (printed_global) {
            fprintf(out, "\n");
        }
    }
}

static void handle_return(const ast_node *type_info, const ast_node *statement, FILE *out)
{
    type_t t = read_type(type_info);

    if (t == TYPE_VOID) {
        if (statement->left) {
            die("Return value in void function");
        }
        fprintf(out, "    ret\n\n");
    } else {
        if (!statement->left) {
            die("No return value in non-void function");
        }
        resolve_statement(9, statement->left, out);
        fprintf(out, "    mov x0, x9\n");
        fprintf(out, "    ret\n\n");
    }
}

static void handle_function(const ast_node *type, const ast_node *name, const ast_node *statements, FILE *out)
{
    fprintf(out, "_%s:\n", name->left->s);
    int i;
    for (i = 0; statements->list[i]; i++) {
        const ast_node *s = statements->list[i];
        if (s->tok_type == TOK_KW_RETURN) {
            handle_return(type, s, out);
        } else {
            die("xxx not handled");
        }
    }
}

static void resolve_statement(const int reg, const ast_node *s, FILE *out)
{
    switch ((int)s->tok_type) {
        case TOK_NUM:
            if (s->type == TYPE_INT) {
                if (s->ival <= 0xffff) {
                    fprintf(out, "    mov x%d, #0x%lx\n", reg, s->ival);
                } else {
                    die("xxx");
                }
            } else {
                die("xxx");
            }
            break;
        case '+':
            resolve_statement(9, s->left, out);
            resolve_statement(10, s->right, out);
            fprintf(out, "    add x%d, x9, x10\n", reg);
            break;
        default:
            die("xxx");
    }
}

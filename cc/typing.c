#include "typing.h"
#include "assert.h"

type_t read_type(const ast_node *type_decl)
{
    switch (type_decl->tok_type) {
        case TOK_KW_INT:
            return TYPE_INT;
        default:
            // todo
            assert("not handled" == NULL);
            exit(1);
    }
}

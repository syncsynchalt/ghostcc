#include "helper.h"
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>
extern "C" {
#include "../../lib/die.h"
}

std::string write_file(const std::string &contents)
{
    // ReSharper disable once CppDeprecatedEntity
    const char *filename = tmpnam(NULL);
    FILE *f = fopen(filename, "w");
    assert(f);
    fwrite(contents.c_str(), 1, contents.size(), f);
    fclose(f);
    return filename;
}

std::string read_file(const std::string &filename)
{
    std::ifstream ifs(filename);
    std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    return content;
}

std::string print_ast_node_type(const ast_node *node)
{
    switch (node->type) {
        case NODE_LIST: return "(nt:list)";
        case NODE_SUBSCRIPT: return "(nt:subscript)";
        case NODE_CAST: return "(nt:cast)";
        case NODE_ID_LIST: return "(nt:id_list)";
        case NODE_INIT_LIST: return "(nt:init_list)";
        case NODE_DECL_LIST: return "(nt:decl_list)";
        case NODE_DECL_SPECIFIERS: return "(nt:decl_specifiers)";
        case NODE_STRUCT_MEMBERS: return "(nt:struct_members)";
        case NODE_TRANSLATION_UNIT: return "(nt:translation_unit)";
        case NODE_COMPOUND_STATEMENT: return "(nt:compound_statement)";
        case NODE_BITFIELD: return "(nt:bitfield)";
        case NODE_PARAM_LIST: return "(nt:param_list)";
        case NODE_FUNCTION: return "(nt:function)";
        case NODE_ABSTRACT: return "(nt:abstract)";
        case NODE_ABSTRACT_TYPE: return "(nt:abstract_type)";
        case NODE_OTHER:
        case NODE_INT:
        case NODE_FLT:
        case NODE_STR:
        case NODE_CHA:
            return "";
        default:
            die("Unknown node type %d", node->type);
    }
}

std::string print_ast_node(const ast_node *node)
{
    std::string result;
        switch (node->tok_type) {
        case TOK_WS: result += ' '; break;
        case TOK_KW_AUTO: result += "auto"; break;
        case TOK_KW_BREAK: result += "break"; break;
        case TOK_KW_CASE: result += "case"; break;
        case TOK_KW_CHAR: result += "char"; break;
        case TOK_KW_CONST: result += "const"; break;
        case TOK_KW_CONTINUE: result += "continue"; break;
        case TOK_KW_DEFAULT: result += "default"; break;
        case TOK_KW_DO: result += "do"; break;
        case TOK_KW_DOUBLE: result += "double"; break;
        case TOK_KW_ELSE: result += "else"; break;
        case TOK_KW_ENUM: result += "enum"; break;
        case TOK_KW_EXTERN: result += "extern"; break;
        case TOK_KW_FLOAT: result += "float"; break;
        case TOK_KW_FOR: result += "for"; break;
        case TOK_KW_GOTO: result += "goto"; break;
        case TOK_KW_IF: result += "if"; break;
        case TOK_KW_INT: result += "int"; break;
        case TOK_KW_LONG: result += "long"; break;
        case TOK_KW_REGISTER: result += "register"; break;
        case TOK_KW_RETURN: result += "return"; break;
        case TOK_KW_SHORT: result += "short"; break;
        case TOK_KW_SIGNED: result += "signed"; break;
        case TOK_KW_SIZEOF: result += "sizeof"; break;
        case TOK_KW_STATIC: result += "static"; break;
        case TOK_KW_STRUCT: result += "struct"; break;
        case TOK_KW_SWITCH: result += "switch"; break;
        case TOK_KW_TYPEDEF: result += "typedef"; break;
        case TOK_KW_UNION: result += "union"; break;
        case TOK_KW_UNSIGNED: result += "unsigned"; break;
        case TOK_KW_VOID: result += "void"; break;
        case TOK_KW_VOLATILE: result += "volatile"; break;
        case TOK_KW_WHILE: result += "while"; break;
        case TOK_ID: result += node->s; break;
        case TOK_ELLIPSIS: result += "..."; break;
        case TOK_PTR_OP: result += "->"; break;
        case TOK_LEFT_OP: result += "<<"; break;
        case TOK_RIGHT_OP: result += ">>"; break;
        case TOK_AND_OP: result += "&&"; break;
        case TOK_OR_OP: result += "||"; break;
        case TOK_EQ_OP: result += "=="; break;
        case TOK_NE_OP: result += "!="; break;
        case TOK_LE_OP: result += "<="; break;
        case TOK_GE_OP: result += ">="; break;
        case TOK_INC_OP: result += "++"; break;
        case TOK_DEC_OP: result += "--"; break;
        case TOK_ADD_ASSIGN: result += "+="; break;
        case TOK_SUB_ASSIGN: result += "-="; break;
        case TOK_MUL_ASSIGN: result += "*="; break;
        case TOK_DIV_ASSIGN: result += "/="; break;
        case TOK_LEFT_ASSIGN: result += "<<="; break;
        case TOK_RIGHT_ASSIGN: result += ">>="; break;
        case TOK_MOD_ASSIGN: result += "%="; break;
        case TOK_AND_ASSIGN: result += "&="; break;
        case TOK_OR_ASSIGN: result += "|="; break;
        case TOK_XOR_ASSIGN: result += "^="; break;
        case TOK_PP_COMBINE: result += "##"; break;
        case TOK_PP_CONTINUE: result += "\\\n"; break;
        case TOK_COMMENT: result += "/*comment*/"; break;
        case TOK_LINE_COMMENT: result += "//line-comment\n"; break;
        default: result += (char)node->tok_type; break;
    }

    if (node->list_len) {
        result += "[";
        for (auto i = 0; i < node->list_len; i++) {
            result += print_ast_node(node->list[i]);
        }
        result += "]";
    }
    return result;
}

std::string print_ast_inner(const ast_node *node, int indent)
{
    auto result = std::string();

    if (node->type == NODE_INT) {
        return std::to_string(node->ival);
    } else if (node->type == NODE_FLT) {
        return std::to_string(node->fval);
    } else if (node->type == NODE_CHA) {
        if (!isprint(node->cval)) {
            return "'?'"s;
        }
        if (node->cval == '"' || node->cval == '\'') {
            return "'\\"s + (char)node->cval + "'";
        }
        return "'"s + (char)node->cval + "'";
    } else if (node->type == NODE_STR) {
        const std::stringstream ss(node->s);
        return ss.str();
    }

    result += "(";
    if (node->left) {
        result += print_ast(node->left);
    }

    result += print_ast_node_type(node);
    result += print_ast_node(node);

    if (node->right) {
        result += print_ast(node->right);
    }

    result += ")";
    return result;
}

std::string print_ast(const ast_node *node)
{
    return print_ast_inner(node, 0);
}

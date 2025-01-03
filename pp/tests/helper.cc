#include "helper.h"
#include <cassert>
#include <fstream>
#include <string>
extern "C" {
#include "preprocessor.h"
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

std::string run_parser(const std::string &input, const std::string &extra_include_path)
{
    const auto infile = write_file(input);
    return run_parser_on_file(infile, extra_include_path);
}

std::string run_parser_on_file(const std::string &filename, const std::string &extra_include_path)
{
    const auto defs = defines_init();
    const char *includes[3] = {"/usr/include", NULL};
    if (!extra_include_path.empty()) {
        includes[1] = extra_include_path.c_str();
        includes[2] = NULL;
    }
    const auto outfile = write_file("");
    FILE *in = fopen(filename.c_str(), "r");
    FILE *out = fopen(outfile.c_str(), "w");
    parse(in, out, defs, includes);
    fclose(out);
    fclose(in);
    return read_file(outfile);
}

std::string print_ast(const ast_node *node)
{
    auto result = std::string();

    if (node->node_type == NODE_INT) {
        return std::to_string(node->ival);
    } else if (node->node_type == NODE_FLT) {
        return std::to_string(node->fval);
    } else if (node->node_type == NODE_CHA) {
        // todo - quote correctly
        return std::string() + "'" + (char)node->cval + "'";
    } else if (node->node_type == NODE_STR) {
        // todo - quote correctly
        return std::string() + "\"" + node->s + "\"";
    }

    result += "(";
    if (node->left) {
        result += print_ast(node->left);
    }

    switch (node->token_type) {
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
        case TOK_PP_STR: result += "#"; break;
        case TOK_PP_COMBINE: result += "##"; break;
        case TOK_PP_CONTINUE: result += "\\\n"; break;
        case TOK_COMMENT: result += "/*comment*/"; break;
        case TOK_LINE_COMMENT: result += "//line-comment\n"; break;
        default: result += (char)node->token_type; break;
    }

    if (node->right) {
        result += print_ast(node->right);
    }
    result += ")";
    return result;
}

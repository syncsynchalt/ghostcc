#include "cc_helper.h"
#include <iostream>
extern "C" {
#include "parse.h"
}

ast_node *run_grammar(const std::string &input)
{
    const auto infile = write_file(input);
    FileDeleter fd(infile);
    return run_grammar_on_file(infile);
}

ast_node *run_grammar_on_file(const std::string &filename)
{
    FILE *f = fopen(filename.c_str(), "r");
    if (!f) {
        std::cerr << "Failed to open file " << filename << std::endl;
        return nullptr;
    }
    ast_node *ast = parse_ast(filename.c_str(), f);
    fclose(f);
    return ast;
}

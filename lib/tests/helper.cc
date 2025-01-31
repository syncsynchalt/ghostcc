#include "helper.h"
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>
extern "C" {
#include "die.h"
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

std::string PrintAst(const ast_node *node)
{
    char *s = print_ast(node);
    std::string ss(s);
    free(s);
    return "\n" + ss;
}

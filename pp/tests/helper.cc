#include "helper.h"
#include <cassert>
#include <fstream>
#include <string>
extern "C" {
#include "parse.h"
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

#include "pp_helper.h"
#include "../../lib/tests/helper.h"
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>
extern "C" {
#include "preprocessor.h"
}
using namespace std::string_literals;

std::string run_parser(const std::string &input, const std::string &extra_include_path)
{
    const auto infile = write_file(input);
    FileDeleter fd(infile);
    return run_parser_on_file(infile, extra_include_path);
}

std::string strip_line_hints(std::string s)
{
    size_t ind;
    while (s[0] == '#') {
        s = s.replace(0, s.find('\n') + 1, "");
    }
    while ((ind = s.find("\n#")) != std::string::npos) {
        s = s.replace(ind+1, s.find('\n', ind+1) - ind, "");
    }
    return s;
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
    FileDeleter fd(outfile);
    FILE *in = fopen(filename.c_str(), "r");
    FILE *out = fopen(outfile.c_str(), "w");
    parse_state state = {0};
    state.defs = defs;
    state.include_paths = includes;
    process_file(filename.c_str(), in, out, &state);
    fclose(out);
    fclose(in);
    return strip_line_hints(read_file(outfile));
}

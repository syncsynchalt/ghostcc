#pragma once

#include "../../lib/tests/helper.h"
#include <string>
using namespace std::literals::string_literals;

extern std::string strip_line_hints(std::string s);
extern std::string run_parser(const std::string &input, const std::string &extra_include_path = "");
extern std::string run_parser_on_file(const std::string &filename, const std::string &extra_include_path = "");

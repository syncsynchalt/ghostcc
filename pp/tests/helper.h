#pragma once

#include <string>

extern std::string write_file(const std::string &contents);
extern std::string read_file(const std::string &filename);
extern std::string run_parser(const std::string &input, const std::string &extra_include_path = "");
extern std::string run_parser_on_file(const std::string &filename, const std::string &extra_include_path = "");

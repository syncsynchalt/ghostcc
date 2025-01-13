#pragma once

#include <string>
#include <unistd.h>

extern std::string write_file(const std::string &contents);
extern std::string read_file(const std::string &filename);
extern std::string strip_line_hints(std::string s);
extern std::string run_parser(const std::string &input, const std::string &extra_include_path = "");
extern std::string run_parser_on_file(const std::string &filename, const std::string &extra_include_path = "");

class FileDeleter {
  const std::string _filename;
public:
  FileDeleter(const FileDeleter &) = delete;
  explicit FileDeleter(const std::string &filename) : _filename(filename) {}
  ~FileDeleter() { unlink(_filename.c_str()); }
};

#include "../../lib/ast.h"
extern std::string print_ast(const ast_node *node);

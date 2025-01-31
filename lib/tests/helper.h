#pragma once

#include <string>
#include <unistd.h>
extern "C" {
#include "ast.h"
}
using namespace std::literals::string_literals;

extern std::string write_file(const std::string &contents);
extern std::string read_file(const std::string &filename);

class FileDeleter {
  const std::string _filename;
public:
  FileDeleter(const FileDeleter &) = delete;
  explicit FileDeleter(const std::string &filename) : _filename(filename) {}
  ~FileDeleter() { unlink(_filename.c_str()); }
};

inline bool string_contains(const std::string &s, const std::string &sub) { return s.find(sub) != std::string::npos; }
inline bool string_ends_with(const std::string &s, const std::string &sub)
{
  return s.size() >= sub.size() && s.compare(s.size() - sub.size(), sub.size(), sub) == 0;
}

extern std::string PrintAst(const ast_node *node);

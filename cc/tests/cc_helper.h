#pragma once

#include "../../lib/tests/helper.h"
#include <string>
extern "C" {
#include "ast.h"
}

using namespace std::literals::string_literals;

extern ast_node *run_grammar(const std::string &input);
extern ast_node *run_grammar_on_file(const std::string &filename);

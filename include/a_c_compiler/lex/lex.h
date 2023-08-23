#pragma once
#include <filesystem>
#include <vector>

namespace a_c_compiler {

namespace fs = std::filesystem;

enum token : long int {
#define CHAR_TOKEN(TOK, LIT) TOK = LIT,
#define WHITESPACE_TOKEN(TOK, LIT) TOK = LIT,
#define TOKEN(TOK, INTVAL) TOK = INTVAL,
#include "tokens.inl.h"
#undef WHITESPACE_TOKEN
#undef CHAR_TOKEN
#undef TOKEN
};


struct file_offset_info {
  size_t lineno, column;
};

void dump_tokens(std::vector<std::tuple<token, file_offset_info>> const& toks);

std::string lexed_id(size_t index);
int lexed_numeric_literal(size_t index);
std::string lexed_string_literal(size_t index);

std::vector<std::tuple<token, file_offset_info>> lex(fs::path const &source_file);
} /* namespace a_c_compiler */

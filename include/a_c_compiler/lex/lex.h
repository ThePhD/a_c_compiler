#pragma once
#include <filesystem>
#include <vector>

namespace a_c_compiler {

namespace fs = std::filesystem;

enum token : long int {
#define CHAR_TOKEN(TOK, LIT) TOK = LIT,
#define TOKEN(TOK, INTVAL) TOK = INTVAL,
#include "tokens.inl.h"
#undef CHAR_TOKEN
#undef TOKEN
};


struct file_offset_info {
  size_t lineno, column;
};

using token_vector = std::vector<std::tuple<token, file_offset_info>>;
void dump_tokens(token_vector const& toks);

std::string lexed_id(size_t index);
std::string lexed_numeric_literal(size_t index);
std::string lexed_string_literal(size_t index);

token_vector lex(fs::path const &source_file);
} /* namespace a_c_compiler */

#pragma once

#include <filesystem>
#include <vector>
#include <cstdint>

namespace a_c_compiler {

	namespace fs = std::filesystem;

	enum token_id : int32_t {
#define CHAR_TOKEN(TOK, INTVAL) TOK = INTVAL,
#define KEYWORD_TOKEN(TOK, INTVAL, KEYWORD) TOK = INTVAL,
#define TOKEN(TOK, INTVAL) TOK = INTVAL,
#include "tokens.inl.h"
#undef CHAR_TOKEN
#undef KEYWORD_TOKEN
#undef TOKEN
	};


	struct file_offset_info {
		size_t lineno, column;
	};

	struct token {
		token_id id;
		file_offset_info source_location;
	};

	using token_vector = std::vector<token>;
	void dump_tokens(token_vector const& toks);

	std::string_view lexed_id(size_t index);
	std::string_view lexed_numeric_literal(size_t index);
	std::string_view lexed_string_literal(size_t index);

	token_vector lex(fs::path const& source_file);
} /* namespace a_c_compiler */

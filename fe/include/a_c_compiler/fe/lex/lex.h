// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include <a_c_compiler/options/global_options.h>
#include <a_c_compiler/fe/reporting/diagnostic_handles.h>
#include <a_c_compiler/fe/reporting/logger.h>

#include <filesystem>
#include <vector>
#include <cstdint>
#include <iosfwd>

namespace a_c_compiler {

	namespace fs = std::filesystem;

	enum token_id : int32_t {
#define CHAR_TOKEN(TOK, INTVAL) TOK = INTVAL,
#define KEYWORD_TOKEN(TOK, INTVAL, KEYWORD) TOK = INTVAL,
#define TOKEN(TOK, INTVAL) TOK = INTVAL,
#include <a_c_compiler/fe/lex/tokens.inl.h>
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
	void dump_tokens_into(token_vector const& toks, std::ostream& output_stream) noexcept;
	void dump_tokens(token_vector const& toks) noexcept;

	std::string_view lexed_id(size_t index) noexcept;
	std::string_view lexed_numeric_literal(size_t index) noexcept;
	std::string_view lexed_string_literal(size_t index) noexcept;

	token_vector lex(fs::path const& source_file, const global_options& global_opts,
	     diagnostic_handles& diag_handles) noexcept;
} /* namespace a_c_compiler */

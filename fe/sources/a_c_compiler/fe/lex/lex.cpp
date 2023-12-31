// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#include <a_c_compiler/fe/lex/lex.h>
#include <a_c_compiler/fe/reporting/diagnostic_handles.h>

#include <a_c_compiler/version.h>
#include <ztd/idk/assert.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <ostream>

/* Place lexed literals and identifiers in these vectors for the parser to
 * access later. Leave numeric literals as strings becaues it is the parser's
 * job to figure out what type the literal should be parsed to. */
static std::vector<std::string> lexed_numeric_literals;
static std::vector<std::string> lexed_string_literals;
static std::vector<std::string> lexed_ids;

namespace a_c_compiler {

	/* Maps tokens to offsets into source files */
	static std::vector<file_offset_info> token_source_info;

	file_offset_info source_info_for_token(size_t tok_idx) {
		return token_source_info[tok_idx];
	}

	void dump_tokens_into(token_vector const& toks, std::ostream& output_stream) noexcept {
		size_t id_idx = 0, numlit_idx = 0, strlit_idx = 0;
		static constexpr size_t width = 15;
		output_stream << std::setw(width) << "line:column"
		              << " | token\n";
		for (auto [t, foi] : toks) {
			std::stringstream ss;
			ss << foi.lineno << ":" << foi.column;
			output_stream << std::setw(width) << ss.str() << " | ";
			switch (t) {

#define CHAR_TOKEN(TOK, LIT)     \
	case TOK:                   \
		output_stream << #TOK; \
		break;

#define KEYWORD_TOKEN(TOK, LIT, KEYWORD) \
	case TOK:                           \
		output_stream << #TOK;         \
		break;

#include <a_c_compiler/fe/lex/tokens.inl.h>
#undef CHAR_TOKEN
#undef KEYWORD_TOKEN

			case tok_block_comment:
				output_stream << "tok_block_comment";
				break;

			case tok_line_comment:
				output_stream << "tok_line_comment";
				break;

			case tok_newline:
				output_stream << "tok_newline";
				break;

			case tok_tab:
				output_stream << "tok_tab";
				break;

			case tok_id:
				output_stream << "tok_id: " << lexed_id(id_idx++);
				break;

			case tok_num_literal:
				output_stream << "tok_num_literal: " << lexed_numeric_literal(numlit_idx++);
				break;

			case tok_str_literal:
				output_stream << "str_literal: " << lexed_string_literal(strlit_idx++);
				break;

			case tok_pp_embed:
				output_stream << "pp_embed_literal";
				break;

			default:
				ZTD_ASSERT_MESSAGE("Got invalid token", false);
			}
			output_stream << "\n";
		}
	}

	void dump_tokens(token_vector const& toks) noexcept {
		dump_tokens_into(toks, std::cout);
	}

	std::string_view lexed_numeric_literal(size_t index) noexcept {
		return lexed_numeric_literals[index].data();
	}
	std::string_view lexed_id(size_t index) noexcept {
		return lexed_ids[index].data();
	}
	std::string_view lexed_string_literal(size_t index) noexcept {
		return lexed_string_literals[index].data();
	}

	token_vector lex(fs::path const& source_file, const global_options& global_opts,
	     diagnostic_handles& diag_handles) noexcept {
		token_vector toks;
		toks.reserve(2048);

		FILE* fp =
#if ZTD_IS_ON(ZTD_LIBVCXX)
		     _wfopen(source_file.c_str(), L"r")
#else
		     std::fopen(source_file.c_str(), "r")
#endif
		     ;
		ZTD_ASSERT_MESSAGE("Couldn't open file", fp);
		char c;
		size_t lineno = 0, column = 0;

		/* Get next character while tracking source location info */
		auto getc = [&]() {
			c = fgetc(fp);
			if (c == '\n') {
				lineno++;
				column = 0;
			}
			else {
				column++;
			}
			return c;
		};

		c = std::fgetc(fp);
		while (c != EOF) {
			switch (c) {
			case ' ':
				break;

			case '\t':
				// toks.push_back({ tok_tab, { lineno, column } });
				break;

				/* Handle comments */
			case '/': {
				file_offset_info foi { lineno, column };
				getc();
				/* Line comment */
				if (c == '/') {
					while (c != '\n') {
						getc();
					}
					toks.push_back({ tok_line_comment, foi });
					break;
				}

				/* Block comment */
				else if (c == '*') {
					while (true) {
						if (getc() == '*' && getc() == '/') {
							toks.push_back({ tok_block_comment, foi });
							break;
						}
						ZTD_ASSERT_MESSAGE("unterminated block comment", c != EOF);
					}
					break;
				}
				else {
					toks.push_back({ tok_forward_slash, foi });
				}
			}

			/* Char-like tokens */
#define CHAR_TOKEN(TOK, LIT) case LIT:
#include <a_c_compiler/fe/lex/tokens.inl.h>
				toks.push_back({ (token_id)c, { lineno, column } });
				break;
#undef CHAR_TOKEN

			case '\n':
				toks.push_back({ tok_newline, { lineno, column } });
				break;

			case '"': {
				toks.push_back({ tok_str_literal, { lineno, column } });
				std::string lit = "";
				while (getc() != '"') {
					lit += c;
				}
				lexed_string_literals.push_back(lit);
			} break;

				/* Numeric literals */
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.': {
				file_offset_info foi { lineno, column };
				std::string lit = "";
				lit += c;
				while (std::isdigit(getc()) || c == '.') {
					lit += c;
				}

				/* numeric literal type suffixes */
				if (c == 'f' or c == 'd') {
					lit += c;
				}
				else {
					ungetc(c, fp);
				}

				lexed_numeric_literals.push_back(lit);
				toks.push_back({ tok_num_literal, foi });
				break;
			}
			}

			/* Identifier */
			if (std::isalpha(c) or c == '_') {
				file_offset_info foi { lineno, column };
				std::string lit = "";
				lit += c;

				while (std::isalnum(c = fgetc(fp)) or c == '_') {
					lit += c;
				}
				std::ungetc(c, fp);
				if (false) { }
				// if it matches a keyword's spelling, it's a keyword
#define KEYWORD_TOKEN(TOK, INTVAL, KEYWORD) \
	else if (lit == #KEYWORD) {            \
		toks.push_back({ TOK, foi });     \
	}
#include <a_c_compiler/fe/lex/tokens.inl.h>
#undef KEYWORD_TOKEN
				else {
					lexed_ids.push_back(lit);
					toks.push_back({ tok_id, foi });
				}
			}
			getc();
		}
		return toks;
	}

} // namespace a_c_compiler

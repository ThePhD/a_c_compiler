// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#include "parse.h"
#include "parser_diagnostic_reporter.h"
#include "parser_diagnostic.h"

#include <expected>
#include <optional>
#include <utility>

namespace a_c_compiler {

	struct parser {
		using next_token_t = std::expected<std::reference_wrapper<const token>,
		     std::reference_wrapper<const parser_diagnostic>>;

		std::size_t m_toks_index;
		token_vector const& m_toks;
		parser_diagnostic_reporter& m_reporter;

		/* Which ident are we operating on? This allows us to get a handle to the
		 * string representation of an id. */
		std::size_t id_index = 0;

		constexpr parser(std::size_t toks_index, token_vector const& toks,
		     parser_diagnostic_reporter& reporter) noexcept
		: m_toks_index(toks_index), m_toks(toks), m_reporter(reporter) {
		}

		const token& current_token() noexcept {
			const token& target_token = m_toks[m_toks_index];
			return target_token;
		}

		next_token_t get_next_token() noexcept {
			++m_toks_index;
			if (m_toks_index == m_toks.size()) {
				return std::unexpected(parser_err::out_of_tokens);
			}
			const token& target_token = m_toks[m_toks_index];
			return target_token;
		}

		std::optional<std::vector<attribute>> parse_attributes() noexcept {
			return {};
		}

		std::string_view next_id_value() noexcept {
			id_index++;
			return current_id_value();
		}

		std::string_view current_id_value() const noexcept {
			return lexed_id(id_index);
		}

#define KEYWORD_TOKEN(TOK, INTVAL, KEYWORD)                                                        \
	bool parse_##KEYWORD(translation_unit& tu) {                                                  \
		auto const& tok = current_token();                                                       \
		m_reporter.report(parser_err::unimplemented_keyword, "", tok.source_location, #KEYWORD); \
		return false;                                                                            \
	}
#include "tokens.inl.h"
#undef KEYWORD_TOKEN

		bool parse_attribute_specifier_sequence(translation_unit& tu, function_definition& fd) {
			return false;
		}

		bool parse_storage_class_specifier(translation_unit& tu, function_definition& fd) {
			switch (current_token().id) {
			case tok_keyword_auto:
				// TODO: was this intentionally left out of the scs enum?
				assert(false && "unsupported storage class specifier");
			case tok_keyword_static:
				fd.declaration.t.data().specifiers |= storage_class_specifier::scs_static;
				break;
			case tok_keyword_extern:
				fd.declaration.t.data().specifiers |= storage_class_specifier::scs_extern;
				break;
			case tok_keyword_constexpr:
				fd.declaration.t.data().specifiers |= storage_class_specifier::scs_constexpr;
				break;
			case tok_keyword_register:
				fd.declaration.t.data().specifiers |= storage_class_specifier::scs_register;
				break;
			case tok_keyword_thread_local:
				fd.declaration.t.data().specifiers |= storage_class_specifier::scs_thread_local;
				break;
			case tok_keyword_typedef:
				fd.declaration.t.data().specifiers |= storage_class_specifier::scs_typedef;
				break;
			default:
				return false;
			}
			get_next_token();
			return true;
		}

		void merge_type_categories(function_definition& fd, type_category tc) {
#define TYPE_SPECIFIER_MERGE_RULE(BASETYPE, TYPESPEC, NEWTYPE)   \
	if (fd.declaration.t.data().category == type_category::BASETYPE \
	     && tc == type_category::TYPESPEC) {                    \
		fd.declaration.t.data().category = type_category::NEWTYPE; \
		return;                                                \
	}
			TYPE_SPECIFIER_MERGE_RULE(tc_long, tc_double, tc_longdouble);
			TYPE_SPECIFIER_MERGE_RULE(tc_long, tc_longdouble, tc_longlongdouble);
			TYPE_SPECIFIER_MERGE_RULE(tc_long, tc_int, tc_long);
			TYPE_SPECIFIER_MERGE_RULE(tc_longlong, tc_int, tc_longlong);
#undef TYPE_SPECIFIER_MERGE_RULE
			/* If none of the rules match, just assign the new type to the function's
			 * type category */
			fd.declaration.t.data().category = tc;
		}

		void merge_type_categories(function_definition& fd, type_modifier tm) {
			assert(fd.declaration.t.data().modifier == type_modifier::tm_none
			     && "unexpected multiple type modifiers");
			fd.declaration.t.data().modifier = tm;
		}

		bool parse_type_specifier(translation_unit& tu, function_definition& fd) {
			switch (current_token().id) {
			case tok_keyword_void:
				fd.declaration.t.data().category = type_category::tc_void;
				break;
			case tok_keyword_char:
				fd.declaration.t.data().category = type_category::tc_char;
				break;
			case tok_keyword_bool:
				fd.declaration.t.data().category = type_category::tc_bool;
				break;
			case tok_keyword_short:
				fd.declaration.t.data().category = type_category::tc_short;
				break;
			case tok_keyword_int:
				merge_type_categories(fd, type_category::tc_int);
				break;
			case tok_keyword_long:
				merge_type_categories(fd, type_category::tc_long);
				break;
			case tok_keyword_float:
				merge_type_categories(fd, type_category::tc_float);
				break;
			case tok_keyword_double:
				merge_type_categories(fd, type_category::tc_double);
				break;
			case tok_keyword_signed:
				merge_type_categories(fd, type_modifier::tm_signed);
				break;
			case tok_keyword_unsigned:
				merge_type_categories(fd, type_modifier::tm_unsigned);
				break;
			case tok_keyword__BitInt:
			case tok_keyword__Complex:
				// case tok_keyword__Decimal32: TODO
				// case tok_keyword__Decimal64: TODO
				// case tok_keyword__Decimal128: TODO
				assert(false && "unsupported type specifier");
				return false;
				// TODO: atomic-type-specifier
				// TODO: struct-or-union-specifier
				// TODO: enum-specifier
				// TODO: typedef-name
				// TODO: typeof-specifier
			}
			return true;
		}

		bool parse_type_qualifier(translation_unit& tu, function_definition& fd) {
			return false;
		}

		bool parse_alignment_specifier(translation_unit& tu, function_definition& fd) {
			return false;
		}

		/*
		 * type_specifier_qualifier ::= type-specifier | type-qualifier | alignment-specifier
		 */
		bool parse_type_specifier_qualifier(translation_unit& tu, function_definition& fd) {
			return false;
		}

		bool parse_function_specifier(translation_unit& tu, function_definition& fd) {
			return false;
		}

		/*
		 * declaration-specifier ::=
		 *    storage-class-specifier
		 *    | type-specifier-qualifier
		 *    | function-specifier
		 */
		bool parse_declaration_specifier(translation_unit& tu, function_definition& fd) {
			if (parse_storage_class_specifier(tu, fd))
				return true;

			if (parse_type_specifier_qualifier(tu, fd))
				return true;

			if (parse_function_specifier(tu, fd))
				return true;

			return false;
		}

		/*
		 * declaration-specifiers ::=
		 *    declaration-specifier attribute-specifier-sequence?
		 *    | declaration-specifier declaration-specifiers
		 */
		bool parse_declaration_specifiers(translation_unit& tu, function_definition& fd) {
			if (!parse_declaration_specifier(tu, fd))
				return false;

			if (parse_attribute_specifier_sequence(tu, fd))
				return true;

			while (parse_declaration_specifier(tu, fd)) {
				continue;
			}

			return true;
		}

		bool parse_declarator(translation_unit& tu, function_definition& fd) {
			return false;
		}

		bool parse_function_body(translation_unit& tu, function_definition& fd) {
			return false;
		}

		/*
		 * function-definition ::=
		 *    attribute-specifier-sequence? declaration-specifiers declarator function-body
		 */
		bool parse_function_definition(translation_unit& tu) {
			function_definition fd;

			/* attr specs are optional, so don't bail if we don't find any */
			parse_attribute_specifier_sequence(tu, fd);

			if (!parse_declaration_specifiers(tu, fd))
				return false;

			if (!parse_declarator(tu, fd))
				return false;

			if (!parse_function_body(tu, fd))
				return false;

			return true;
		}

		bool parse_declaration(translation_unit& tu) {
			return false;
		}

		/*
		 * \brief Attempt to parse a declaration
		 * \returns true if declaration parse was successful.
		 *
		 * external-declaration ::= function-definition | declaration
		 */
		bool parse_external_declaration(translation_unit& tu) noexcept {
			if (m_toks.empty()) {
				return false;
			}

			if (parse_function_definition(tu))
				return true;

			if (parse_declaration(tu))
				return true;

			/* Keep track of first and last token when searching for a declaration.
			 * Figure out the details later. */
			auto const& start_token = current_token();

			/* To determine if we're working with a var decl or a function decl, we
			 * must first try to parse an ident token. */
			for (;;) {
				const auto tok = this->current_token();
				switch (tok.id) {
#define KEYWORD_TOKEN(TOK, INTVAL, KEYWORD) \
	case TOK:                              \
		return parse_##KEYWORD(tu);
#include "tokens.inl.h"
#undef KEYWORD_TOKEN

				default:
					// unrecognized token: report and bail!
					m_reporter.report(
					     parser_err::unrecognized_token, "", tok.source_location, (int)tok.id);
					return false;
				}
				auto maybe_err = get_next_token();
				if (!maybe_err.has_value()) {
					const auto wrapped_err = maybe_err.error();
					m_reporter.report(wrapped_err, "", tok.source_location);
					break;
				}
			}

			return false;
		}

		translation_unit parse_translation_unit() noexcept {
			translation_unit tu {};
			while (parse_external_declaration(tu)) {
				continue;
			}
			return tu;
		}
	};



	ast_module parse(token_vector const& toks) noexcept {
		parser_diagnostic_reporter reporter {};
		parser p(0, toks, reporter);
		ast_module mod { p.parse_translation_unit() };
		return mod;
	}

} /* namespace a_c_compiler */

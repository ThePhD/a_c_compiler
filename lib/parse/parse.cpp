// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#include "parse.h"
#include "logger.h"
#include "feature_flag.h"
#include "parser_diagnostic_reporter.h"
#include "parser_diagnostic.h"

#include <expected>
#include <optional>
#include <utility>

#define DEBUGGING() get_feature_flag(1, 0x1)
#define DEBUG(FORMATSTR, ...)                                                  \
	if (DEBUGGING()) {                                                        \
		logger::indent();                                                    \
		std::fprintf(stderr, "parser:%s:" FORMATSTR, __func__, __VA_ARGS__); \
	}
#define DEBUGS(DEBUGSTR)                                         \
	if (DEBUGGING()) {                                          \
		logger::indent();                                      \
		std::fprintf(stderr, "parser:%s:" DEBUGSTR, __func__); \
	}
#define ENTER_PARSE_FUNCTION()                                       \
	scope_logger current_scope_logger(__func__, [&]() {             \
		auto loc = this -> current_token().source_location;        \
		std::fprintf(stderr, ":%zu:%zu:", loc.lineno, loc.column); \
	});

namespace a_c_compiler {

	struct parser {
		using next_token_t = std::expected<std::reference_wrapper<const token>,
		     std::reference_wrapper<const parser_diagnostic>>;

		std::size_t m_toks_index;
		std::size_t m_last_identifier_string_index = 0;
		std::vector<std::size_t> m_token_history_stack;
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

		void unget_token() noexcept {
			ZTD_ASSERT_MESSAGE(
			     "Cannot unget token at beginning of token stream", m_toks_index > 0);
			m_toks_index--;
		}

		void eat_token(token_id expected_token) noexcept {
			auto maybe_tok = get_next_token();
			assert(maybe_tok && "expected token");
			token got_token = maybe_tok.value();
			assert(got_token.id == expected_token && "got unexpected token");
		}

		void pop_token_index() {
			m_toks_index = m_token_history_stack.back();
			m_token_history_stack.pop_back();
		}

		void push_token_index() {
			m_token_history_stack.push_back(m_toks_index);
		}

		token find_first_of(std::vector<token_id> toks) {
			push_token_index();
			while (std::find(toks.begin(), toks.end(), current_token().id) == toks.end()) {
				if (!get_next_token().has_value()) {
					break;
				}
			}
			token found_tok = current_token();
			pop_token_index();
			return found_tok;
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
			ENTER_PARSE_FUNCTION();
			return false;
		}

		bool parse_storage_class_specifier(
		     translation_unit& tu, function_definition& fd, type ty) {
			ENTER_PARSE_FUNCTION();
			switch (current_token().id) {
			case tok_keyword_auto:
				// TODO: was this intentionally left out of the scs enum?
				assert(false && "unsupported storage class specifier");
			case tok_keyword_static:
				ty.data().specifiers |= storage_class_specifier::scs_static;
				break;
			case tok_keyword_extern:
				ty.data().specifiers |= storage_class_specifier::scs_extern;
				break;
			case tok_keyword_constexpr:
				ty.data().specifiers |= storage_class_specifier::scs_constexpr;
				break;
			case tok_keyword_register:
				ty.data().specifiers |= storage_class_specifier::scs_register;
				break;
			case tok_keyword_thread_local:
				ty.data().specifiers |= storage_class_specifier::scs_thread_local;
				break;
			case tok_keyword_typedef:
				ty.data().specifiers |= storage_class_specifier::scs_typedef;
				break;
			default:
				return false;
			}
			get_next_token();
			return true;
		}

		void merge_type_categories(type ty, type_category tc) {
			/* If the two type categories are given as type specifiers, they may need
			 * to be merged. E.g. long int and long do not need to be merged, we can
			 * just take the former. long double however must be merged together into
			 * the long double type category. */
#define TYPE_SPECIFIER_MERGE_RULE(BASETYPE, NEWTYPESPEC, NEWTYPE)                             \
	if (ty.data().category == type_category::BASETYPE && tc == type_category::NEWTYPESPEC) { \
		ty.data().category = type_category::NEWTYPE;                                        \
		return;                                                                             \
	}
			TYPE_SPECIFIER_MERGE_RULE(tc_long, tc_double, tc_longdouble);
			TYPE_SPECIFIER_MERGE_RULE(tc_long, tc_longdouble, tc_longlongdouble);
			TYPE_SPECIFIER_MERGE_RULE(tc_long, tc_int, tc_long);
			TYPE_SPECIFIER_MERGE_RULE(tc_longlong, tc_int, tc_longlong);
#undef TYPE_SPECIFIER_MERGE_RULE
			/* If none of the rules match, just assign the new type to the function's
			 * type category */
			ty.data().category = tc;
		}

		void merge_type_categories(type ty, type_modifier tm) {
			assert(ty.data().modifier == type_modifier::tm_none
			     && "unexpected multiple type modifiers");
			ty.data().modifier = tm;
		}

		bool parse_type_specifier(translation_unit& tu, function_definition& fd, type ty) {
			ENTER_PARSE_FUNCTION();
			switch (current_token().id) {
			case tok_keyword_void:
				ty.data().category = type_category::tc_void;
				break;
			case tok_keyword_char:
				ty.data().category = type_category::tc_char;
				break;
			case tok_keyword_bool:
				ty.data().category = type_category::tc_bool;
				break;
			case tok_keyword_short:
				ty.data().category = type_category::tc_short;
				break;
			case tok_keyword_int:
				merge_type_categories(ty, type_category::tc_int);
				break;
			case tok_keyword_long:
				merge_type_categories(ty, type_category::tc_long);
				break;
			case tok_keyword_float:
				merge_type_categories(ty, type_category::tc_float);
				break;
			case tok_keyword_double:
				merge_type_categories(ty, type_category::tc_double);
				break;
			case tok_keyword_signed:
				merge_type_categories(ty, type_modifier::tm_signed);
				break;
			case tok_keyword_unsigned:
				merge_type_categories(ty, type_modifier::tm_unsigned);
				break;
			case tok_keyword__BitInt:
			case tok_keyword__Complex:
				// case tok_keyword__Decimal32: TODO
				// case tok_keyword__Decimal64: TODO
				// case tok_keyword__Decimal128: TODO
				assert(false && "unsupported type specifier");
				// TODO: atomic-type-specifier
				// TODO: struct-or-union-specifier
				// TODO: enum-specifier
				// TODO: typedef-name
				// TODO: typeof-specifier
			default:
				return false;
			}
			get_next_token();
			return true;
		}

		bool parse_type_qualifier(translation_unit& tu, function_definition& fd, type ty) {
			ENTER_PARSE_FUNCTION();
			return false;
		}

		bool parse_alignment_specifier(translation_unit& tu, function_definition& fd, type ty) {
			ENTER_PARSE_FUNCTION();
			return false;
		}

		/*
		 * type_specifier_qualifier ::= type-specifier | type-qualifier | alignment-specifier
		 */
		bool parse_type_specifier_qualifier(
		     translation_unit& tu, function_definition& fd, type ty) {
			ENTER_PARSE_FUNCTION();
			if (parse_type_specifier(tu, fd, ty))
				return true;
			if (parse_type_qualifier(tu, fd, ty))
				return true;
			if (parse_alignment_specifier(tu, fd, ty))
				return true;
			return false;
		}

		bool parse_function_specifier(translation_unit& tu, function_definition& fd) {
			ENTER_PARSE_FUNCTION();
			switch (current_token().id) {
			case tok_keyword_inline:
				fd.declaration.funcspecs |= function_specifier::funcspec_inline;
				break;
			case tok_keyword__Noreturn:
				fd.declaration.funcspecs |= function_specifier::funcspec__Noreturn;
				break;
			default:
				return false;
			}
			get_next_token();
			return true;
		}

		/*
		 * declaration-specifier ::=
		 *    storage-class-specifier
		 *    | type-specifier-qualifier
		 *    | function-specifier
		 */
		bool parse_declaration_specifier(translation_unit& tu, function_definition& fd, type ty) {
			ENTER_PARSE_FUNCTION();
			if (parse_storage_class_specifier(tu, fd, ty))
				return true;

			if (parse_type_specifier_qualifier(tu, fd, ty))
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
		bool parse_declaration_specifiers(
		     translation_unit& tu, function_definition& fd, type ty) {
			ENTER_PARSE_FUNCTION();
			while (parse_declaration_specifier(tu, fd, ty)) {
				// parse_attribute_specifier_sequence(tu, fd, ty);
			}

			/* empty declspecs is valid */
			return true;
		}

		/*
		 *
		 */
		bool parse_type_qualifier_list(translation_unit tu, function_definition fd) {
			ENTER_PARSE_FUNCTION();
			return false;
		}

		/*
		 *
		 */
		bool parse_array_declarator(translation_unit tu, function_definition fd) {
			ENTER_PARSE_FUNCTION();
			return false;
		}

		bool parse_parameter_type_list(
		     translation_unit tu, function_definition fd, std::vector<type>& typelist) {
			ENTER_PARSE_FUNCTION();
			return false;
		}

		/*
		 * function-declarator ::=
		 *      direct-declarator ( parameter-type-list? )
		 */
		bool parse_function_declarator(translation_unit tu, function_definition fd) {
			ENTER_PARSE_FUNCTION();
			std::string funcname;
			push_token_index();
      // should be parse_declarator
			// if (!parse_declarator(tu, fd)) {
      if (!parse_identifier(tu, fd, funcname)) {
				pop_token_index();
				return false;
			}
			if (current_token().id != tok_l_paren) {
				pop_token_index();
				return false;
			}
			get_next_token();
			std::vector<type> typelist;
			if (!parse_parameter_type_list(tu, fd, typelist)) {
				pop_token_index();
				return false;
			}
			eat_token(tok_r_paren);
			return true;
		}


		/*
		 * pointer ::=
		 *    * attribute-specifier-sequence? type-qualifier-list?
		 *    | * attribute-specifier-sequence? type-qualifier-list? pointer
		 */
		bool parse_pointer(translation_unit& tu, function_definition& fd) {
			ENTER_PARSE_FUNCTION();
			if (current_token().id != tok_asterisk)
				return false;
			while (current_token().id == tok_asterisk) {
				// parse_attribute_specifier_sequence(tu, fd);
				parse_type_qualifier_list(tu, fd);
				get_next_token();
			}
			return true;
		}

		bool parse_identifier(translation_unit& tu, function_definition& fd, std::string& idval) {
			ENTER_PARSE_FUNCTION();
			switch (current_token().id) {
			case tok_id:
				idval = lexed_id(m_last_identifier_string_index++);
				break;
			case tok_keyword_int:
				idval = "int";
				break;
			default:
				return false;
			}
			get_next_token();
			return true;
		}

		/*
		 * direct-declarator ::=
		 *    identifier attribute-specifier-sequence?
		 *    | ( declarator )
		 *    | array-declarator attribute-specifier-sequence?
		 *    | function-declarator attribute-specifier-sequence?
		 */
		bool parse_direct_declarator(translation_unit& tu, function_definition& fd) {
			ENTER_PARSE_FUNCTION();
			std::string idval;

			// If we find lparen before the next '{' or ';', it's probably a function
			// declarator
			const token t = find_first_of({ tok_l_paren, tok_l_curly_bracket, tok_semicolon });
			if (t.id == tok_l_paren && parse_function_declarator(tu, fd)) {
				// parse_attribute_specifier_sequence(tu, fd);
				return true;
			}

			if (parse_identifier(tu, fd, idval)) {
				// parse_attribute_specifier_sequence(tu, fd);
				return true;
			}

			if (current_token().id == tok_l_paren) {
				get_next_token();
				if (!parse_declarator(tu, fd)) {
					unget_token();
					return false;
				}
				eat_token(tok_r_paren);
			}

			if (parse_array_declarator(tu, fd)) {
				// parse_attribute_specifier_sequence(tu, fd);
				return true;
			}

			return false;
		}

		/*
		 * declarator ::= pointer? direct-declarator
		 */
		bool parse_declarator(translation_unit& tu, function_definition& fd) {
			ENTER_PARSE_FUNCTION();
			parse_pointer(tu, fd);
			if (!parse_direct_declarator(tu, fd))
				return false;
			get_next_token();
			return true;
		}

		bool parse_function_body(translation_unit& tu, function_definition& fd) {
			ENTER_PARSE_FUNCTION();
			return false;
		}

		/*
		 * function-definition ::=
		 *    attribute-specifier-sequence? declaration-specifiers declarator function-body
		 */
		bool parse_function_definition(translation_unit& tu) {
			ENTER_PARSE_FUNCTION();
			function_definition fd;
			fd.declaration.t = type_data::get_new_type();

			// parse_attribute_specifier_sequence(tu, fd);

			type return_type = type_data::get_new_type();
			if (!parse_declaration_specifiers(tu, fd, return_type))
				return false;

			if (!parse_declarator(tu, fd))
				return false;

			if (!parse_function_body(tu, fd))
				return false;

			return true;
		}

		/*
		 *
		 */
		bool parse_declaration(translation_unit& tu) {
			ENTER_PARSE_FUNCTION();
			return false;
		}

		/*
		 * \brief Attempt to parse a declaration
		 * \returns true if declaration parse was successful.
		 *
		 * external-declaration ::= function-definition | declaration
		 */
		bool parse_external_declaration(translation_unit& tu) noexcept {
			ENTER_PARSE_FUNCTION();
			if (m_toks.empty()) {
				return false;
			}

			push_token_index();
			if (parse_function_definition(tu))
				return true;
			pop_token_index();

			push_token_index();
			if (parse_declaration(tu))
				return true;
			pop_token_index();

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

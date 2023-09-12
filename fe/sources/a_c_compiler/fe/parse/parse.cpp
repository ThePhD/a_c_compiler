// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#include <a_c_compiler/options/global_options.h>
#include <a_c_compiler/fe/parse/parse.h>
#include <a_c_compiler/fe/reporting/logger.h>
#include <a_c_compiler/fe/parse/parser_diagnostic_reporter.h>
#include <a_c_compiler/fe/parse/parser_diagnostic.h>

#include <expected>
#include <optional>
#include <utility>

#define DEBUGGING() this->global_opts.get_feature_flag(1, 0x1)
#define DEBUG(FORMATSTR, ...)                                                                 \
	if (DEBUGGING()) {                                                                       \
		this->m_logger.indent();                                                            \
		std::fprintf(                                                                       \
		     this->m_logger.c_out_handle(), "parser:%s:" FORMATSTR, __func__, __VA_ARGS__); \
	}
#define DEBUGS(DEBUGSTR)                                                                \
	if (DEBUGGING()) {                                                                 \
		this->m_logger.indent();                                                      \
		std::fprintf(this->m_logger.c_out_handle(), "parser:%s:" DEBUGSTR, __func__); \
	}
#define ENTER_PARSE_FUNCTION()                                                                   \
	scope_logger current_scope_logger(                                                          \
	     __func__,                                                                              \
	     [&]() {                                                                                \
		     auto loc = this->current_token().source_location;                                 \
		     std::fprintf(this->m_logger.c_out_handle(), ":%zu:%zu:", loc.lineno, loc.column); \
	     },                                                                                     \
	     this->m_logger);

namespace a_c_compiler {

	struct parser {
		using next_token_t = std::expected<std::reference_wrapper<const token>,
		     std::reference_wrapper<const parser_diagnostic>>;

		std::size_t m_toks_index;
		std::size_t m_last_identifier_string_index = 0;
		std::vector<std::size_t> m_token_history_stack;
		token_vector const& m_toks;
		parser_diagnostic_reporter& m_reporter;
		const global_options& m_global_opts;
		logger& m_logger;

		/* Which ident are we operating on? This allows us to get a handle to the
		 * string representation of an id. */
		std::size_t id_index = 0;

		constexpr parser(std::size_t toks_index, token_vector const& toks,
		     parser_diagnostic_reporter& reporter, const global_options& global_opts,
		     logger& logs) noexcept
		: m_toks_index(toks_index)
		, m_toks(toks)
		, m_reporter(reporter)
		, m_global_opts(global_opts)
		, m_logger(logs) {
		}

		const token& current_token() noexcept {
			const token& target_token = m_toks[m_toks_index];
			return target_token;
		}

		next_token_t peek_token(std::size_t peek_by = 1) noexcept {
			size_t current_toks_index = m_toks_index;
			if (m_toks.size() - current_toks_index < peek_by) {
				return std::unexpected(parser_err::out_of_tokens);
			}
			const token& target_token = m_toks[m_toks_index + peek_by];
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

		void advance_token_index(std::size_t advance_by = 1) noexcept {
			ZTD_ASSERT_MESSAGE("Cannot advance beyond end of stream",
			     advance_by < (m_toks.size() - m_toks_index));
			m_toks_index += advance_by;
		}

		void recede_token_index(std::size_t recede_by = 1) noexcept {
			ZTD_ASSERT_MESSAGE(
			     "Cannot recede past beginning of token stream", recede_by > m_toks_index);
			m_toks_index -= recede_by;
		}

		void unget_token() noexcept {
			ZTD_ASSERT_MESSAGE(
			     "Cannot unget token at beginning of token stream", m_toks_index > 0);
			m_toks_index--;
		}

		void eat_token(token_id expected_token) noexcept {
			auto maybe_tok = get_next_token();
			ZTD_ASSERT_MESSAGE("expected token", maybe_tok);
			token got_token = maybe_tok.value();
			ZTD_ASSERT_MESSAGE("got unexpected token", got_token.id == expected_token);
		}

		void pop_token_index() {
			m_toks_index = m_token_history_stack.back();
			m_token_history_stack.pop_back();
		}

		void push_token_index() {
			m_token_history_stack.push_back(m_toks_index);
		}

		bool has_more_tokens() noexcept {
			return m_toks_index < m_toks.size();
		}

		token find_first_of(const std::vector<token_id>& toks) noexcept {
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
#include <a_c_compiler/fe/lex/tokens.inl.h>
#undef KEYWORD_TOKEN

		using maybe_attribute_t
		     = std::expected<attribute, std::reference_wrapper<const parser_diagnostic>>;

		enum class balanced_token_seq_behavior {
			normal         = 0b00,
			ignore_initial = 0b01,
			ignore_all     = 0b10
		};

		enum class balanced_delimeter {
			parenthesis    = 0,
			square_bracket = 1,
			curly_bracket  = 2,
		};

		struct seen_delimeter {
			size_t& count;
			balanced_delimeter delimeter;
		};

		struct balanced_token_delimeters {
			size_t parentheses;
			size_t curly_brackets;
			size_t square_brackets;
			std::optional<balanced_delimeter> last_seen;

			[[nodiscard]] constexpr bool unclosed_delimeters() const noexcept {
				return last_seen.has_value();
			}
		};

		template <typename OnToken>
		balanced_token_delimeters consume_balanced_token_sequence(OnToken&& on_token,
		     balanced_token_seq_behavior behavior,
		     balanced_delimeter initial_delimeter = balanced_delimeter::parenthesis) noexcept {
			balanced_token_delimeters delimeters = {};
			std::vector<balanced_delimeter> delimeter_stack(
			     &initial_delimeter, &initial_delimeter + 1);

			for (; delimeters.unclosed_delimeters();) {
				if (!has_more_tokens()) {
					break;
				}
				// keep going until the sequence is terminated
				const token& tok                  = current_token();
				const auto maybe_manage_delimeter = [&tok, &delimeter_stack,
				                                         &delimeters]() noexcept {
					switch (tok.id) {
					case tok_l_paren:
						delimeter_stack.push_back(balanced_delimeter::parenthesis);
						++delimeters.parentheses;
						break;
					case tok_l_square_bracket:
						delimeter_stack.push_back(balanced_delimeter::square_bracket);
						++delimeters.square_brackets;
						break;
					case tok_l_curly_bracket:
						delimeter_stack.push_back(balanced_delimeter::curly_bracket);
						++delimeters.curly_brackets;
						break;
					case tok_r_paren:
						if (delimeter_stack.back() != balanced_delimeter::parenthesis) {
							return;
						}
						delimeter_stack.pop_back();
						--delimeters.parentheses;
						break;
					case tok_r_square_bracket:
						if (delimeter_stack.back() != balanced_delimeter::square_bracket) {
							return;
						}
						delimeter_stack.pop_back();
						--delimeters.square_brackets;
						break;
					case tok_r_curly_bracket:
						if (delimeter_stack.back() != balanced_delimeter::curly_bracket) {
							return;
						}
						delimeter_stack.pop_back();
						--delimeters.curly_brackets;
						break;
					default:
						break;
					}
				};
				maybe_manage_delimeter();
				if (behavior == balanced_token_seq_behavior::ignore_initial
				     && delimeter_stack.size() == 0) {
					break;
				}
				if (!on_token(tok)) {
					break;
				}
			}
			if (!delimeter_stack.empty()) {
				delimeters.last_seen = delimeter_stack.back();
			}
			return delimeters;
		}

		maybe_attribute_t parse_attribute() noexcept {
			// `attribute-token`
			// TODO: store attribute token names
			attribute attr {};
			const token& first_token = current_token();
			if (first_token.id != tok_id) {
				// failure
				return std::unexpected(parser_err::expected_attribute_identifier);
			}
			attr.tokens.push_back(first_token);
			advance_token_index(1);
			constexpr const auto next_token_is_colon = [](const next_token_t& maybe_next_tok) {
				return maybe_next_tok.has_value() && maybe_next_tok->get().id == tok_colon;
			};
			for (; current_token().id == tok_colon && next_token_is_colon(peek_token());) {
				advance_token_index(2);
				const token& expecting_id_tok = current_token();
				if (expecting_id_tok.id != tok_id) {
					// failure
					return std::unexpected(parser_err::expected_attribute_identifier);
				}
				attr.tokens.push_back(expecting_id_tok);
				advance_token_index(1);
			}
			if (current_token().id == tok_l_paren) {
				// expect attribute arguments are this point, and consume a balanced
				// token sequence, started with
				// `( balanced-token-seq )`
				// consume all attribute arguments
				attr.tokens.push_back(current_token());
				const auto on_token = [&attr](const token& tok) noexcept {
					attr.tokens.push_back(tok);
					return true;
				};
				advance_token_index(1);
				balanced_token_delimeters delimeters = consume_balanced_token_sequence(on_token,
				     balanced_token_seq_behavior::ignore_initial,
				     balanced_delimeter::parenthesis);
				if (delimeters.unclosed_delimeters()) {
					const token& stop_token = current_token();
					m_reporter.report(parser_err::unbalanced_token_sequence, "",
					     stop_token.source_location, (char)stop_token.id);
				}
			}
			return attr;
		}

		size_t parse_attribute_list(std::vector<attribute>& attributes) noexcept {
			size_t number_of_successfully_parsed_attributes = 0;
			// loop until double r square bracket
			for (;;) {
				const token& tok = current_token();
				switch (tok.id) {
				case tok_comma:
					// this means we COULD get another attribute; just loop around
					break;
				case tok_r_square_bracket: {
					auto maybe_expected_second_r_square_bracket = peek_token();
					if (!maybe_expected_second_r_square_bracket.has_value()) {
						return number_of_successfully_parsed_attributes; // there are no more
						                                                 // tokens? No more
						                                                 // attributes.
					}
					const token& expected_second_r_square_bracket
					     = *maybe_expected_second_r_square_bracket;
					if (expected_second_r_square_bracket.id == tok_r_square_bracket) {
						// we are finished and we can leave.
						return number_of_successfully_parsed_attributes;
					}
				} break;
				default: {
					auto maybe_current_attribute = parse_attribute();
					if (maybe_current_attribute.has_value()) {
						attributes.push_back(std::move(*maybe_current_attribute));
						++number_of_successfully_parsed_attributes;
					}
				} break;
				}
			}
			return true;
		}

		size_t parse_attribute_specifier_sequence(
		     translation_unit& tu, std::vector<attribute>& attributes) noexcept {
			ENTER_PARSE_FUNCTION();
			std::size_t number_of_successfully_parsed_attribute_specifiers = 0;
			for (;;) {
				auto maybe_expected_second_l_square_bracket = peek_token();
				const token& expected_l_square_bracket      = current_token();
				if (expected_l_square_bracket.id != tok_l_square_bracket
				     || !maybe_expected_second_l_square_bracket.has_value()) {
					return number_of_successfully_parsed_attribute_specifiers;
				}
				const token& expected_second_l_square_bracket
				     = *maybe_expected_second_l_square_bracket;
				if (expected_second_l_square_bracket.id != tok_l_square_bracket) {
					return number_of_successfully_parsed_attribute_specifiers;
				}
				advance_token_index(2);
				size_t successfully_parsed_attributes = parse_attribute_list(attributes);
				number_of_successfully_parsed_attribute_specifiers
				     += successfully_parsed_attributes;
				eat_token(tok_r_square_bracket);
			}

			return number_of_successfully_parsed_attribute_specifiers;
		}

		bool parse_storage_class_specifier(
		     translation_unit& tu, function_definition& fd, type ty) noexcept {
			ENTER_PARSE_FUNCTION();
			switch (current_token().id) {
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
			case tok_keyword_auto:
				// While the C standard says this is a Storage Class Specifier, that is
				// literally just a Temporary Stop Gap™ input by the C Committee
				// because we did not have enough time to write the rules out for
				// it being a proper type versus storage class specifier. It should allow for
				// both, provided it has `auto auto` when we want to use it to make a
				// block-scope variable whose type is automatically deduced.
				//
				// Otherwise, `auto` by itself means type deduction unless there's a real type
				// in the type name at some point.
				ty.data().specifiers |= storage_class_specifier::scs_auto;
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

			parse_attribute_specifier_sequence(tu, fd.declaration.attributes);

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
#include <a_c_compiler/fe/lex/tokens.inl.h>
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



	ast_module parse(
	     token_vector const& toks, const global_options& global_opts, logger& logs) noexcept {
		parser_diagnostic_reporter reporter {};
		parser p(0, toks, reporter, global_opts, logs);
		ast_module mod { p.parse_translation_unit() };
		return mod;
	}

} /* namespace a_c_compiler */

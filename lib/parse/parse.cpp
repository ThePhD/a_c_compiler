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

		void parse_declaration(translation_unit& tu) noexcept {
			if (m_toks.empty()) {
				return;
			}
			for (;;) {
				const auto tok = this->current_token();
				switch (tok.id) {
				default:
					// unrecognized token: report and bail!
					m_reporter.report(
					     parser_err::unrecognized_token, "", tok.source_location, (int)tok.id);
					return;
				}
				auto maybe_err = get_next_token();
				if (!maybe_err.has_value()) {
					const auto wrapped_err = maybe_err.error();
					m_reporter.report(wrapped_err, "", tok.source_location);
					break;
				}
			}
		}

		translation_unit parse_translation_unit() noexcept {
			translation_unit tu {};
			this->parse_declaration(tu);
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

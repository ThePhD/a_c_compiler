#pragma once

#include <string_view>

namespace a_c_compiler {
	enum class parser_diagnostic_id {
		out_of_tokens,
		unrecognized_token,
	};

	struct parser_diagnostic {
		parser_diagnostic_id id;
		std::string_view format;
	};

	namespace parser_err {
		inline constexpr const parser_diagnostic out_of_tokens {parser_diagnostic_id::out_of_tokens, "out of tokens"};
		inline constexpr const parser_diagnostic unrecognized_token {parser_diagnostic_id::unrecognized_token, "unrecognized token {}"};
	}
}
	
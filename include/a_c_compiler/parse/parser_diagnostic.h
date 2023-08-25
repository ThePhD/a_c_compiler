#pragma once

#include <string_view>

namespace a_c_compiler {
	enum class parser_diagnostic_id {
#define DIAGNOSTIC(SRC_NAME, FMT_STRING) SRC_NAME,
#include "parser_diagnostic.inl.h"
#undef DIAGNOSTIC
	};

	struct parser_diagnostic {
		parser_diagnostic_id id;
		std::string_view format;
	};

	namespace parser_err {
#define DIAGNOSTIC(SRC_NAME, FMT_STRING) inline constexpr const parser_diagnostic SRC_NAME {parser_diagnostic_id::SRC_NAME, FMT_STRING};
#include "parser_diagnostic.inl.h"
#undef DIAGNOSTIC
	}
}
	

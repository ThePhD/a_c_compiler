// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include "lex.h"
#include "ast_module.h"
#include "parser_diagnostic.h"

namespace a_c_compiler {

	struct parser_diagnostic_reporter {
		template <typename... FmtArgs>
		void report(parser_diagnostic const& diagnostic, std::string_view file_name,
		     file_offset_info const& source_location, FmtArgs&&... format_args) noexcept;
	};

} /* namespace a_c_compiler */


#include "parser_diagnostic_reporter.template.h"

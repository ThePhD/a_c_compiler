// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include <a_c_compiler/fe/lex/lex.h>
#include <a_c_compiler/fe/parse/parser_diagnostic.h>
#include <a_c_compiler/fe/reporting/diagnostic_handles.h>

namespace a_c_compiler {

	struct parser_diagnostic_reporter {
		constexpr parser_diagnostic_reporter(diagnostic_handles& handles) noexcept
		: m_handles(handles) {
		}

		[[nodiscard]] diagnostic_handles& handles() noexcept {
			return this->m_handles;
		}

		template <typename... FmtArgs>
		void report(parser_diagnostic const& diagnostic, std::string_view file_name,
		     file_offset_info const& source_location, FmtArgs&&... format_args) noexcept;

	private:
		diagnostic_handles& m_handles;
	};

} /* namespace a_c_compiler */


#include "parser_diagnostic_reporter.template.h"

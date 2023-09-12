// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include "parser_diagnostic_reporter.h"

#include "parser_diagnostic.h"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <utility>
#include <iostream>

namespace a_c_compiler {
	template <typename... FmtArgs>
	void parser_diagnostic_reporter::report(parser_diagnostic const& diagnostic,
	     std::string_view file_name, file_offset_info const& source_Location,
	     FmtArgs&&... format_args) noexcept {
		file_name = file_name.size() ? file_name : "<source file>";
		fmt::print(this->m_handles.error_handle(), "{} ({}, {})\n❌ ", file_name,
		     source_Location.lineno, source_Location.column);
		fmt::vprint(this->m_handles.error_handle(), diagnostic.format,
		     fmt::make_format_args(format_args...));
		fmt::print(this->m_handles.error_handle(), "\n");
	}
} // namespace a_c_compiler

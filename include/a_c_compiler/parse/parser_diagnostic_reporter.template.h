#include "parser_diagnostic_reporter.h"

#include "parser_diagnostic.h"

#include <print>
#include <utility>
#include <iostream>

namespace a_c_compiler {
	template <typename... FmtArgs>
	void parser_diagnostic_reporter::report(
	     parser_diagnostic const& diagnostic, std::string_view file_name, file_offset_info const& source_Location, FmtArgs&&... format_args) noexcept {
		std::print(std::cerr, "{} ({}, {})\n❌ ", file_name, source_Location.lineno, source_Location.column);
		std::vprint_unicode(std::cerr, diagnostic.format, std::make_format_args(std::forward<FmtArgs>(format_args)...));
		std::print(std::cerr, "\n");
	}
} // namespace a_c_compiler

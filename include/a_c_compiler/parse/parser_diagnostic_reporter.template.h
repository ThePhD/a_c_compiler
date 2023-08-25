#include "parser_diagnostic_reporter.h"

#include "parser_diagnostic.h"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <utility>
#include <iostream>

namespace a_c_compiler {
	template <typename... FmtArgs>
	void parser_diagnostic_reporter::report(
	     parser_diagnostic const& diagnostic, std::string_view file_name, file_offset_info const& source_Location, FmtArgs&&... format_args) noexcept {
    file_name = file_name.size() ? file_name : "<source file>";
		fmt::print(std::cerr, "{} ({}, {})\n❌ ", file_name, source_Location.lineno, source_Location.column);
		fmt::vprint(std::cerr, diagnostic.format, fmt::make_format_args(format_args...));
		fmt::print(std::cerr, "\n");
	}
} // namespace a_c_compiler

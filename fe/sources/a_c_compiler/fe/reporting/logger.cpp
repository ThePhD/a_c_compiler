// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //


#include <a_c_compiler/fe/reporting/logger.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <utility>
#include <iostream>

namespace a_c_compiler {

	void logger::incr_indent() noexcept {
		++this->m_indent_width;
	}

	void logger::decr_indent() noexcept {
		--this->m_indent_width;
	}

	void logger::indent() noexcept {
		for (std::size_t i = 0; i < m_indent_level; i++)
			for (std::size_t j = 0; j < m_indent_width; j++)
				fmt::print(this->handle(), "| ");
	}

	scope_logger::scope_logger(std::string scope_name,
	     std::function<void(logger&)>&& entry_callback, logger& target,
	     std::optional<std::string_view> logfile) noexcept
	: m_logger(target), logfile(std::move(logfile)), scope_name(std::move(scope_name)) {
		// TODO: logfile handling
		m_logger.indent();
		fmt::print(m_logger.handle(), "{}", scope_name);
		entry_callback(m_logger);
		fmt::println(m_logger.handle(), "\n");
		m_logger.incr_indent();
	}

	scope_logger::~scope_logger() noexcept {
		m_logger.decr_indent();
		// logger::indent();
		// fprintf(stderr, "%s\n", scope_name.c_str());
	}

	void scope_logger::indent() noexcept {
		m_logger.indent();
	}

} // namespace a_c_compiler

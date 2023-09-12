// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include <cstdio>
#include <optional>
#include <cstddef>
#include <string>
#include <string_view>
#include <functional>
#include <iosfwd>

namespace a_c_compiler {

	struct log_collection { };

	struct logger {
		inline static constexpr const std::size_t default_indent_width = 1;

		constexpr logger(std::ostream& arg_stream, FILE* arg_c_stream,
		     std::size_t arg_indent_width = default_indent_width) noexcept
		: m_stream(arg_stream)
		, m_c_stream(arg_c_stream)
		, m_indent_level(0)
		, m_indent_width(arg_indent_width) {
		}

		void indent() noexcept;
		void incr_indent() noexcept;
		void decr_indent() noexcept;

		[[nodiscard]] constexpr FILE* c_handle() const noexcept {
			return m_c_stream;
		}

		[[nodiscard]] constexpr std::ostream& handle() const noexcept {
			return m_stream;
		}

		[[nodiscard]] constexpr std::size_t indent_width() const noexcept {
			return m_indent_width;
		}

		[[nodiscard]] constexpr std::size_t indent_level() const noexcept {
			return m_indent_level;
		}

	private:
		std::ostream& m_stream;
		FILE* m_c_stream;
		std::size_t m_indent_level;
		std::size_t m_indent_width;
	};

	struct scope_logger {
		scope_logger(std::string scope_name, std::function<void(logger&)>&& entry_callback,
		     logger& target, std::optional<std::string_view> logfile = std::nullopt) noexcept;

		~scope_logger() noexcept;

	private:
		logger& m_logger;
		std::string scope_name;
		std::optional<std::string_view> logfile;

		void indent() noexcept;
	};
} // namespace a_c_compiler

// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <functional>

namespace a_c_compiler {

	struct logger {

		logger(std::size_t arg_indent_width) noexcept;

		constexpr logger(FILE* arg_c_err_stream, FILE* arg_c_out_stream,
		     std::size_t arg_indent_width) noexcept
		: m_c_err_stream(arg_c_err_stream), m_indent_level(0), m_indent_width(arg_indent_width) {
		}

		void indent() noexcept;
		void incr_indent() noexcept;
		void decr_indent() noexcept;

		[[nodiscard]] constexpr FILE* c_err_handle() const noexcept {
			return m_c_err_stream;
		}

		[[nodiscard]] constexpr FILE* c_out_handle() const noexcept {
			return m_c_out_stream;
		}

		[[nodiscard]] constexpr std::size_t indent_width() const noexcept {
			return m_indent_width;
		}

	private:
		FILE* m_c_err_stream;
		FILE* m_c_out_stream;
		std::size_t m_indent_level;
		std::size_t m_indent_width;
	};

	struct scope_logger {
		scope_logger(std::string scope_name, std::function<void()>&& entry_callback,
		     logger& target, std::optional<std::string_view> logfile = std::nullopt) noexcept;

		~scope_logger() noexcept;

	private:
		logger& m_logger;
		std::string scope_name;
		std::optional<std::string_view> logfile;

		void indent() noexcept;
	};
} // namespace a_c_compiler

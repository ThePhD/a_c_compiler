// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include <a_c_compiler/fe/reporting/logger.h>

#include <cstdio>
#include <iosfwd>

namespace a_c_compiler {

	struct diagnostic_handles {
		diagnostic_handles() noexcept;
		diagnostic_handles(std::ostream& stdout_handle, FILE* arg_c_stdout_handle,
		     std::ostream& stderr_handle, FILE* arg_c_stderr_handle,
		     std::optional<FILE*> arg_c_maybe_error_handle   = std::nullopt,
		     std::ostream* maybe_error_handle                = nullptr,
		     std::optional<FILE*> arg_c_maybe_warning_handle = std::nullopt,
		     std::ostream* maybe_warning_handle              = nullptr,
		     std::optional<FILE*> arg_c_maybe_debug_handle   = std::nullopt,
		     std::ostream* maybe_debug_handle                = nullptr,
		     std::optional<FILE*> arg_c_maybe_info_handle    = std::nullopt,
		     std::ostream* maybe_info_handle                 = nullptr) noexcept;

		[[nodiscard]] constexpr FILE* c_stdout_handle() const noexcept {
			return this->m_c_stdout_handle;
		}

		[[nodiscard]] constexpr FILE* c_stderr_handle() const noexcept {
			return this->m_c_stderr_handle;
		}

		[[nodiscard]] constexpr FILE* c_error_handle() const noexcept {
			return this->m_maybe_c_error_handle ? this->m_maybe_c_error_handle.value()
			                                    : this->c_stderr_handle();
		}

		[[nodiscard]] constexpr FILE* c_warning_handle() const noexcept {
			return this->m_maybe_c_warning_handle ? this->m_maybe_c_warning_handle.value()
			                                      : this->c_stderr_handle();
		}

		[[nodiscard]] constexpr FILE* c_debug_handle() const noexcept {
			return this->m_maybe_c_debug_handle ? this->m_maybe_c_debug_handle.value()
			                                    : this->c_stderr_handle();
		}

		[[nodiscard]] constexpr FILE* c_info_handle() const noexcept {
			return this->m_maybe_c_info_handle ? this->m_maybe_c_info_handle.value()
			                                   : this->c_stderr_handle();
		}

		[[nodiscard]] constexpr std::ostream& stdout_handle() const noexcept {
			return this->m_stdout_handle;
		}

		[[nodiscard]] constexpr std::ostream& stderr_handle() const noexcept {
			return this->m_stderr_handle;
		}

		[[nodiscard]] constexpr std::ostream& error_handle() const noexcept {
			return this->m_maybe_error_handle ? *this->m_maybe_error_handle
			                                  : this->stderr_handle();
		}

		[[nodiscard]] constexpr std::ostream& warning_handle() const noexcept {
			return this->m_maybe_warning_handle ? *this->m_maybe_warning_handle
			                                    : this->stderr_handle();
		}

		[[nodiscard]] constexpr std::ostream& debug_handle() const noexcept {
			return this->m_maybe_debug_handle ? *this->m_maybe_debug_handle
			                                  : this->stderr_handle();
		}

		[[nodiscard]] constexpr std::ostream& info_handle() const noexcept {
			return this->m_maybe_info_handle ? *this->m_maybe_info_handle
			                                 : this->stderr_handle();
		}

	private:
		FILE* m_c_stdout_handle;
		FILE* m_c_stderr_handle;
		std::ostream& m_stdout_handle;
		std::ostream& m_stderr_handle;

		std::optional<FILE*> m_maybe_c_error_handle;
		std::optional<FILE*> m_maybe_c_warning_handle;
		std::optional<FILE*> m_maybe_c_debug_handle;
		std::optional<FILE*> m_maybe_c_info_handle;
		std::ostream* m_maybe_error_handle;
		std::ostream* m_maybe_warning_handle;
		std::ostream* m_maybe_debug_handle;
		std::ostream* m_maybe_info_handle;
	};
} // namespace a_c_compiler

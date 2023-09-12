#include <a_c_compiler/fe/reporting/diagnostic_handles.h>

#include <iostream>

namespace a_c_compiler {
	diagnostic_handles::diagnostic_handles() noexcept
	: diagnostic_handles(std::cout, stdout, std::cerr, stderr) {
	}

	diagnostic_handles::diagnostic_handles(std::ostream& arg_stdout_handle,
	     FILE* arg_c_stdout_handle, std::ostream& arg_stderr_handle, FILE* arg_c_stderr_handle,
	     std::optional<FILE*> arg_c_maybe_error_handle, std::ostream* arg_maybe_error_handle,
	     std::optional<FILE*> arg_c_maybe_warning_handle, std::ostream* arg_maybe_warning_handle,
	     std::optional<FILE*> arg_c_maybe_debug_handle, std::ostream* arg_maybe_debug_handle,
	     std::optional<FILE*> arg_c_maybe_info_handle,
	     std::ostream* arg_maybe_info_handle) noexcept
	: m_stdout_handle(arg_stdout_handle)
	, m_c_stdout_handle(arg_c_stdout_handle)
	, m_stderr_handle(arg_stderr_handle)
	, m_c_stderr_handle(arg_c_stderr_handle)
	, m_maybe_c_error_handle(std::move(arg_c_maybe_error_handle))
	, m_maybe_c_warning_handle(std::move(arg_c_maybe_warning_handle))
	, m_maybe_c_debug_handle(std::move(arg_c_maybe_debug_handle))
	, m_maybe_c_info_handle(std::move(arg_c_maybe_info_handle))
	, m_maybe_error_handle(std::move(arg_maybe_error_handle))
	, m_maybe_warning_handle(std::move(arg_maybe_warning_handle))
	, m_maybe_debug_handle(std::move(arg_maybe_debug_handle))
	, m_maybe_info_handle(std::move(arg_maybe_info_handle)) {
	}
} // namespace a_c_compiler

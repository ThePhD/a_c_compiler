# =============================================================================
#
# a_c_compiler
#
# ============================================================================>

cmake_minimum_required(VERSION 3.25)

include(FetchContent)
FetchContent_Declare(ztd.cmake
	GIT_REPOSITORY https://github.com/soasis/cmake
	GIT_TAG main)
FetchContent_MakeAvailable(ztd.cmake)
set(CMAKE_PROJECT_INCLUDE ${ZTD_CMAKE_PROJECT_PRELUDE})

project(a_c_compiler
	VERSION 0.0.0
	DESCRIPTION "A C Compiler"
	LANGUAGES CXX)

# # # Top-Level Directories
# Check if this is the top-level project or not
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
	set(A_C_COMPILER_IS_TOP_LEVEL_PROJECT YES)
else()
	set(A_C_COMPILER_IS_TOP_LEVEL_PROJECT NO)
endif()

# Modify bad flags / change defaults if we are the top level
if(A_C_COMPILER_IS_TOP_LEVEL_PROJECT)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}/lib")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}/bin")

	if(NOT DEFINED CMAKE_CXX_STANDARD)
		set(CMAKE_CXX_STANDARD 26)
	endif()

	if(NOT DEFINED CMAKE_C_STANDARD)
		set(CMAKE_C_STANDARD 23)
	endif()

	set(CMAKE_OBJECT_PATH_MAX 1024)

	if(A_C_COMPILER_BENCHMARKS OR A_C_COMPILER_EXAMPLES OR A_C_COMPILER_TESTS OR A_C_COMPILER_SCRATCH)
		# normal flags
		check_compiler_flag(disable-permissive MSVC /permissive- GCC -pedantic)
		check_compiler_flag(utf8-literal-encoding MSVC /execution-charset:utf-8 GCC -fexec-charset=utf-8)
		check_compiler_flag(utf8-source-encoding MSVC /source-charset:utf-8 GCC -finput-charset=utf-8)
		check_compiler_flag(extra-constexpr-depth MSVC /constexpr:depth2147483647 GCC -fconstexpr-depth=2147483647 Clang -fconstexpr-depth=2147483647 LANGUAGES CXX)
		check_compiler_flag(extra-constexpr-steps MSVC /constexpr:steps2147483647 GCC -fconstexpr-ops-limit=2147483647 Clang -fconstexpr-steps=2147483647 LANGUAGES CXX)
		check_compiler_flag(template-debugging-mode GCC -ftemplate-backtrace-limit=0 LANGUAGES CXX)

		# Warning flags
		check_compiler_flag(warn-pedantic MSVC /permissive- GCC -pedantic)
		check_compiler_flag(warn-all MSVC /W4 GCC -Wall)
		check_compiler_flag(warn-errors MSVC /WX GCC -Werror)
		check_compiler_flag(warn-extra GCC -Wextra Clang -Wextra)
		check_compiler_diagnostic(alignas-extra-padding MSVC 4324 LANGUAGES CXX)
		check_compiler_diagnostic(bit-int-extension)
		# (Wstringop-overflow) - [meta-bug] bogus/missing -Wstringop-overflow warnings
		#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88443
		# Bogus -Wstringop-overflow warning
		#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100395
		# [10 Regression] spurious -Wstringop-overflow writing to a trailing array plus offset
		#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95353
		if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
			check_compiler_diagnostic(stringop-overflow)
			check_compiler_diagnostic(stringop-overread)
			check_compiler_diagnostic(array-bounds)
		endif()
	endif()
endif()

foreach(include_dir lex parse global)
  include_directories("${PROJECT_SOURCE_DIR}/include/a_c_compiler/${include_dir}")
endforeach()

option(A_C_COMPILER_DOCUMENTATION "Enable build of documentation" OFF)
option(A_C_COMPILER_DOCUMENTATION_NO_SPHINX "Turn off Sphinx usage (useful for ReadTheDocs builds)" OFF)
option(A_C_COMPILER_EXAMPLES "Enable build of examples" OFF)
option(A_C_COMPILER_BENCHMARKS "Enable build of benchmarks" OFF)
option(A_C_COMPILER_TESTS "Enable build of tests" OFF)

add_subdirectory(lib)
if (A_C_COMPILER_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

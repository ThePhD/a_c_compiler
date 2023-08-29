# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================>

file(GLOB_RECURSE a_c_compiler.sources
	LIST_DIRECTORIES OFF
	CONFIGURE_DEPENDS
	*.cpp *.c)
add_executable(a_c_compiler ${a_c_compiler.sources})
add_executable(a_c_compiler::driver ALIAS a_c_compiler)
target_include_directories(a_c_compiler
	PRIVATE
	${A_C_COMPILER_INCLUDE_DIRECTORIES})
target_link_libraries(a_c_compiler
	PRIVATE
	a_c_compiler::fe
	a_c_compiler::global)

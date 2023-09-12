# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================>

file(GLOB_RECURSE a_c_compiler.sources
	LIST_DIRECTORIES OFF
	CONFIGURE_DEPENDS
	sources/**.cpp sources/**.c)
add_executable(a_c_compiler.driver ${a_c_compiler.sources})
add_executable(a_c_compiler::driver ALIAS a_c_compiler.driver)
target_include_directories(a_c_compiler.driver
	PRIVATE
	include)
target_link_libraries(a_c_compiler.driver
	PRIVATE
	a_c_compiler::fe
	a_c_compiler::options)

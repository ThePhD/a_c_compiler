# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================ #

file(GLOB_RECURSE file_check.sources
	LIST_DIRECTORIES NO
	CONFIGURE_DEPENDS
	source/**.c source/**.cpp)

add_executable(a_c_compiler.test.file_check ${file_check.sources})
add_executable(a_c_compiler::test::file_check ALIAS a_c_compiler.test.file_check)
target_compile_options(a_c_compiler.test.file_check
	PRIVATE
	${--utf8-literal-encoding}
	${--utf8-source-encoding}
	${--disable-permissive}
	${--warn-pedantic}
	${--warn-all}
	${--warn-extra}
	${--warn-errors}
	${--allow-alignas-extra-padding}
	${--allow-stringop-overflow} ${--allow-stringop-overread}
	${--allow-array-bounds}
)
target_link_libraries(a_c_compiler.test.file_check
	PRIVATE
	ctre::ctre
	ztd::idk
)
target_include_directories(a_c_compiler.test.file_check
	PRIVATE
	include
)

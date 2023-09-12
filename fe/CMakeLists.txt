# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================ #

file(GLOB_RECURSE a_c_compiler.fe.sources
     LIST_DIRECTORIES OFF
     CONFIGURE_DEPENDS
     sources/**.cpp sources/**.c)

add_library(a_c_compiler.fe ${a_c_compiler.fe.sources})
add_library(a_c_compiler::fe ALIAS a_c_compiler.fe)
target_include_directories(a_c_compiler.fe
	PUBLIC
	include
)
target_link_libraries(a_c_compiler.fe
	PUBLIC
	ztd::idk
	fmt::fmt
	a_c_compiler::options
)
target_compile_definitions(a_c_compiler.fe
	PRIVATE
	_CRT_SECURE_NO_WARNINGS=1
)

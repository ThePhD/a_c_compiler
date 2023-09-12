# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================ #


file(GLOB_RECURSE a_c_compiler.options.sources
     LIST_DIRECTORIES OFF
     CONFIGURE_DEPENDS
     sources/**.cpp sources/**.c)

add_library(a_c_compiler.options ${a_c_compiler.options.sources})
add_library(a_c_compiler::options ALIAS a_c_compiler.options)
target_include_directories(a_c_compiler.options
	PUBLIC
	include
)

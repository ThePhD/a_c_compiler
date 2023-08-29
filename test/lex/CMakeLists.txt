# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================ #

file(GLOB_RECURSE lex_test_sources
	LIST_DIRECTORIES OFF
	CONFIGURE_DEPENDS
	*.c)
foreach(test_source_file ${lex_test_sources})
  a_c_compiler_test_make_file_check_lex_test(lex ${test_source_file})
endforeach()

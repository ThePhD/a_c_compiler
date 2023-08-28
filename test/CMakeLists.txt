# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================ #

function (a_c_compiler_test_make_file_check_test prefix source_file)
	get_filename_component(source_name ${source_file} NAME_WE)
	add_test(
		NAME "a_c_compiler::test::${prefix}::${source_name}"
		COMMAND "$<TARGET_FILE:a_c_compiler>" -fdebug-lexer "${source_file}" | "$<TARGET_FILE:a_c_compiler::test::file_check>" "${source_file}"
	)
endfunction()

add_subdirectory(file_check)
add_subdirectory(lex)
add_subdirectory(parse)

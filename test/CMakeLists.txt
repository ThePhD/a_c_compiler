# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================ #

function (a_c_compiler_test_make_file_check_lex_test prefix source_file)
	get_filename_component(source_name ${source_file} NAME_WE)
	set(compiler_test_name a_c_compiler.test.lex_test.${prefix}.${source_name})
	set(check_test_name a_c_compiler.test.lex_test.${prefix}.${source_name}.file_check)
	set(check_test_input_file ${CMAKE_CURRENT_BINARY_DIR}/a_c_compiler.lex_test.${prefix}.${source_name}.output)
	
	add_test(NAME ${compiler_test_name}
		COMMAND a_c_compiler::driver
			--verbose
			-fdebug-lexer
			-fstop-after-phase lex
			--lex-output-file ${check_test_input_file}
			${source_file}
	)
	add_test(NAME ${check_test_name}
		COMMAND a_c_compiler::test::file_check
			${source_file}
			--input-file ${check_test_input_file}
	)
	set_tests_properties(${check_test_name}
		PROPERTIES
		DEPENDS ${compiler_test_name}
		REQUIRED_FILES ${check_test_input_file}
	)
endfunction()

add_subdirectory(file_check)
add_subdirectory(lex)
add_subdirectory(parse)

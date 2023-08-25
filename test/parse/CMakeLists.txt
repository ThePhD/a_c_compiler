file(GLOB lex_test_sources "*.c")
foreach(test_source ${lex_test_sources})
  get_filename_component(source_name ${test_source} NAME_WE)
  add_test(
    NAME "test.parse.${source_name}"
    COMMAND ${BASH_EXE} -c "$<TARGET_FILE:a_c_compiler> -fdebug-lexer ${test_source} 2>&1 | ${FILECHECK_EXE} ${test_source}"
    )
endforeach()

file(GLOB lex_test_sources "*.c")
foreach(test_source ${lex_test_sources})
  add_test(
    NAME "test.lex.${test_source}"
    COMMAND ${BASH_EXE} -c "$<TARGET_FILE:a_c_compiler> -fdebug-lexer ${test_source} | ${FILECHECK_EXE} ${test_source}"
    )
endforeach()

file(GLOB lex_test_sources "*.c")
set(ACC_TEST_FLAGS "-fdebug-lexer -fstop-after-phase lex")
foreach(test_source ${lex_test_sources})
  get_filename_component(source_name ${test_source} NAME_WE)
  add_test(
    NAME "test.lex.${source_name}"
    COMMAND ${BASH_EXE} -c "$<TARGET_FILE:a_c_compiler> ${ACC_TEST_FLAGS} ${test_source} | ${FILECHECK_EXE} ${test_source}"
    )
endforeach()

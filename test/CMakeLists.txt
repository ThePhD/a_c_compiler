add_test(
  NAME test-driver
  COMMAND $<TARGET_FILE:a_c_compiler> -v ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  )

include(${PROJECT_SOURCE_DIR}/cmake/FindFileCheck.cmake)
find_program(BASH_EXE "bash" REQUIRED)
add_subdirectory(lex)

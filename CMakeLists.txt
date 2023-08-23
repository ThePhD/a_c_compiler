# =============================================================================
#
# a_c_compiler
#
# ============================================================================>

cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21)

include(FetchContent)
FetchContent_Declare(ztd.cmake
	GIT_REPOSITORY https://github.com/soasis/cmake
	GIT_TAG main)
FetchContent_MakeAvailable(ztd.cmake)
set(CMAKE_PROJECT_INCLUDE ${ZTD_CMAKE_PROJECT_PRELUDE})

project(a_c_compiler
	VERSION 0.0.0
	DESCRIPTION "A C Compiler"
	LANGUAGES CXX)

include_directories("${PROJECT_SOURCE_DIR}/include/a_c_compiler/lex")
include_directories("${PROJECT_SOURCE_DIR}/include/a_c_compiler/parse")
add_subdirectory(lib)

# =============================================================================
#
# a_c_compiler
#
# ============================================================================>

FetchContent_Declare(ztd.idk
	GIT_REPOSITORY https://github.com/soasis/idk
	GIT_TAG main)
FetchContent_MakeAvailable(ztd.idk)

file(GLOB_RECURSE a_c_compiler.fe.sources
     LIST_DIRECTORIES FALSE
     CONFIGURE_DEPENDS
     lex/*.cpp parse/*.cpp)
add_library(a_c_compiler.fe ${a_c_compiler.fe.sources})
add_library(a_c_compiler::fe ALIAS a_c_compiler.fe)
target_link_libraries(a_c_compiler.fe
     PUBLIC
     ztd::idk)
target_compile_definitions(a_c_compiler.fe
     PRIVATE
     _CRT_SECURE_NO_WARNINGS=1
)
add_subdirectory(tools)

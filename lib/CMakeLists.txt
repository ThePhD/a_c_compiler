file(GLOB_RECURSE a_c_compiler.fe.sources
     LIST_DIRECTORIES FALSE
     CONFIGURE_DEPENDS
     lex/*.cpp parse/*.cpp)
add_library(a_c_compiler.fe ${a_c_compiler.fe.sources})
add_subdirectory(tools)

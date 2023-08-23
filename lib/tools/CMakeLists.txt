file(GLOB_RECURSE a_c_compiler.sources
     LIST_DIRECTORIES FALSE
     CONFIGURE_DEPENDS
     *.cpp)
add_executable(a_c_compiler ${a_c_compiler.sources})
target_include_directories(a_c_compiler PRIVATE ${A_C_COMPILER_INCLUDE_DIRECTORIES})

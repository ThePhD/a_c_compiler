# =============================================================================
# a_c_compiler
#
# © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
# All rights reserved.
# ============================================================================ #

file(GLOB_RECURSE a_c_compiler.fe.sources
     LIST_DIRECTORIES OFF
     CONFIGURE_DEPENDS
     lex/*.cpp parse/*.cpp)

file(GLOB_RECURSE a_c_compiler.global.sources
     LIST_DIRECTORIES OFF
     CONFIGURE_DEPENDS
     global/*.cpp)

add_library(a_c_compiler.global ${a_c_compiler.global.sources})
add_library(a_c_compiler.fe ${a_c_compiler.fe.sources})

foreach(library fe global)
  add_library(a_c_compiler::${library} ALIAS a_c_compiler.${library})
  target_link_libraries(a_c_compiler.${library}
       PUBLIC
       ztd::idk
	  fmt::fmt
  )
  target_compile_definitions(a_c_compiler.${library}
       PRIVATE
       _CRT_SECURE_NO_WARNINGS=1
  )
endforeach()

add_subdirectory(tools)

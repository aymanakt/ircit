# FPL static library -- compiled with strict ANSI C flags, separate from main code.

set(FPL_SOURCES
  src/compile.c
  src/frontend.c
  src/hash.c
  src/memory.c
  src/numexpr.c
  src/reference.c
  src/scan.c
  src/script.c
  src/sprintf.c
  src/sscanf.c
  src/statement.c
)

add_library(fpl STATIC ${FPL_SOURCES})

target_include_directories(fpl PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"   # for FPL.h, script.h, config.h etc.
  "${CMAKE_CURRENT_SOURCE_DIR}"       # for compile/pass2.h (resolved as ../compile/pass2.h)
)

# Strict ANSI C, matching the original FPL Makefile flags
set_target_properties(fpl PROPERTIES
  C_STANDARD          90
  C_STANDARD_REQUIRED OFF
  C_EXTENSIONS        OFF
)

target_compile_options(fpl PRIVATE
  -g
  -DUNIX
  -pedantic
  -O
  -funsigned-char
)

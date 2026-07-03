cmake_minimum_required(VERSION 3.10)

# ---- Compiler preference ---------------------------------------------------
# Prefer Clang when no compiler was explicitly specified (via -D or IDE).
# Falls back to cc (GCC on this system) when Clang isn't installed.
if(NOT CMAKE_C_COMPILER)
  find_program(_ircit_preferred_cc clang)
  if(_ircit_preferred_cc)
    set(CMAKE_C_COMPILER "${_ircit_preferred_cc}" CACHE STRING "C compiler" FORCE)
  endif()
  unset(_ircit_preferred_cc)
endif()

project(ircit VERSION 0.3.1 LANGUAGES C)

# ---- Compiler note ---------------------------------------------------------
# IRCIT uses GNU C extensions (statement expressions, __attribute__).
# Target C11 with GNU extensions — both GCC and Clang support these.
if(NOT (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang"))
  message(WARNING "Untested compiler ${CMAKE_C_COMPILER_ID}. "
                  "GNU C extensions (__attribute__, statement expressions) are required.")
endif()

# ---- Build type ------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---- C flags ---------------------------------------------------------------
# C11 with GNU extensions.  -fcommon is still needed for tentative definitions
# in headers (common symbol merging, removed in GCC 10+).
set(CMAKE_C_FLAGS_DEBUG   "-ggdb3 -O")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -fgnu89-inline -fcommon -D_GNU_SOURCE")

# ---- Ncurses dependency ----------------------------------------------------
# Vendored ncurses 5.2: auto-downloaded, extracted, and built under packages/.
option(FORCE_SYSTEM_NCURSES "Skip the vendored ncurses, use find_package(Curses)" OFF)

set(_NCURSES_VERSION  "5.2")
set(_NCURSES_TARBALL "ncurses-${_NCURSES_VERSION}.tar.gz")
set(_NCURSES_DIR     "${CMAKE_SOURCE_DIR}/packages/ncurses-${_NCURSES_VERSION}")
set(_NCURSES_LIB     "${_NCURSES_DIR}/lib/libncurses.a")

if(NOT FORCE_SYSTEM_NCURSES)

  # ---- Download the tarball if missing ---------------------------------
  set(_NCURSES_URL "https://ftp.gnu.org/gnu/ncurses/${_NCURSES_TARBALL}")
  set(_NCURSES_DST "${CMAKE_SOURCE_DIR}/packages/${_NCURSES_TARBALL}")

  if(NOT EXISTS "${_NCURSES_DST}")
    message(STATUS "Downloading ncurses ${_NCURSES_VERSION} from ${_NCURSES_URL} …")
    file(DOWNLOAD "${_NCURSES_URL}" "${_NCURSES_DST}"
         STATUS _dl_status SHOW_PROGRESS)
    list(GET _dl_status 0 _dl_code)
    if(NOT _dl_code EQUAL 0)
      list(GET _dl_status 1 _dl_msg)
      message(FATAL_ERROR "Failed to download ncurses: ${_dl_msg}")
    endif()
  endif()

  # ---- Extract and build if the library is missing --------------------
  if(NOT EXISTS "${_NCURSES_LIB}")
    message(STATUS "Building ncurses ${_NCURSES_VERSION} …")
    file(REMOVE_RECURSE "${_NCURSES_DIR}")

    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${_NCURSES_DST}"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/packages"
      RESULT_VARIABLE _tar_rc
    )
    if(NOT _tar_rc EQUAL 0)
      message(FATAL_ERROR "Failed to extract ${_NCURSES_TARBALL}")
    endif()

    # Replace 1998-era config.guess / config.sub with system versions
    # so configure can recognise modern x86_64 / aarch64 hosts.
    foreach(_cfg config.guess config.sub)
      if(EXISTS "/usr/share/misc/${_cfg}")
        configure_file("/usr/share/misc/${_cfg}" "${_NCURSES_DIR}/${_cfg}" COPYONLY)
      elseif(EXISTS "/usr/share/automake-1.16/${_cfg}")
        configure_file("/usr/share/automake-1.16/${_cfg}" "${_NCURSES_DIR}/${_cfg}" COPYONLY)
      endif()
    endforeach()

    # Patch ncurses 5.2 for modern GCC (MKlib_gen.sh uses obsolete grep syntax)
    set(_MKHELP "${_NCURSES_DIR}/ncurses/tty/MKlib_gen.sh")
    if(EXISTS "${_MKHELP}")
      file(READ "${_MKHELP}" _mkcontent)
      string(REPLACE "cat >$TMP <<" "cat >$TMP <<-" _mkcontent "${_mkcontent}")
      file(WRITE "${_MKHELP}" "${_mkcontent}")
    endif()


    execute_process(
      COMMAND env CC=gcc ${_NCURSES_DIR}/configure
              --prefix="${_NCURSES_DIR}"
              --without-shared
              --without-debug
              --without-ada
              --without-cxx-binding
              --without-progs
              --without-manpages
              --with-normal
      WORKING_DIRECTORY "${_NCURSES_DIR}"
      RESULT_VARIABLE _cfg_rc
    )
    if(NOT _cfg_rc EQUAL 0)
      message(FATAL_ERROR "ncurses configure failed")
    endif()

    # configure produces broken headers (empty or NCURSES_CONST=/*nothing*/).
    # Fix with a shell one-liner.
    set(_h "${_NCURSES_DIR}/include")
    execute_process(
      COMMAND bash -c
        "sed -i -e 's/@cf_cv_typeof_chtype@/long/g' -e 's/@cf_cv_1UL@/1UL/g' -e 's/@cf_cv_builtin_bool@/1/g' -e 's/@cf_cv_cc_bool_type@/0/g' -e 's/@cf_cv_type_of_bool@/unsigned char/g' -e 's/@cf_cv_shift_limit@/12/g' -e 's/@cf_cv_widec_shift@/0/g' -e 's/@NCURSES_CONST@/const/g' -e 's/@NCURSES_MAJOR@/5/g' -e 's/@NCURSES_MINOR@/2/g' -e 's/@NCURSES_PATCH@/20001021/g' ${_h}/curses.h.in > ${_h}/curses.h && ln -sf curses.h ${_h}/ncurses.h"
      WORKING_DIRECTORY "${_NCURSES_DIR}"
    )

    # Generate headers that the include/ Makefile would normally create
    if(EXISTS "${_h}/MKhashsize.sh")
      execute_process(COMMAND sh "${_h}/MKhashsize.sh" "${_h}/Caps"
        OUTPUT_FILE "${_h}/hashsize.h" WORKING_DIRECTORY "${_h}")
    endif()
    if(EXISTS "${_h}/MKncurses_def.sh")
      execute_process(COMMAND sh "${_h}/MKncurses_def.sh" "${_h}/ncurses_defs"
        OUTPUT_FILE "${_h}/ncurses_def.h" WORKING_DIRECTORY "${_h}")
    endif()
    if(EXISTS "${_h}/MKparametrized.sh")
      execute_process(COMMAND sh "${_h}/MKparametrized.sh" "${_h}/Caps"
        OUTPUT_FILE "${_h}/parametrized.h" WORKING_DIRECTORY "${_h}")
    endif()

    # Generate term.h (needed for back_color_erase etc.)
    if(EXISTS "${_h}/MKterm.h.awk")
      execute_process(
        COMMAND mawk -f "${_h}/MKterm.h.awk" "${_h}/Caps"
        OUTPUT_FILE "${_h}/term.h"
      )
      # Fix: MKterm.h.awk emits empty NCURSES_CONST
      file(READ "${_h}/term.h" _term)
      string(REPLACE "#define NCURSES_CONST /*nothing*/"
              "#define NCURSES_CONST const" _term "${_term}")
      file(WRITE "${_h}/term.h" "${_term}")
    endif()

    # Generate termcap.h
    if(EXISTS "${_h}/termcap.h.in")
      file(READ "${_h}/termcap.h.in" _tcap)
      string(REPLACE "@NCURSES_CONST@" "const" _tcap "${_tcap}")
      string(REPLACE "@cf_cv_typeof_chtype@" "long" _tcap "${_tcap}")
      file(WRITE "${_h}/termcap.h" "${_tcap}")
    endif()

    # Build ncurses — top-level make (which calls ncurses/ make)
    # We skip the broken include/ and man/ subdirs.
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E env CFLAGS="-std=gnu89 -DNCURSES_CONST=const"
              make -j1 -C ncurses ../lib/libncurses.a
      WORKING_DIRECTORY "${_NCURSES_DIR}"
      RESULT_VARIABLE _make_rc
    )
    if(NOT _make_rc EQUAL 0)
      message(FATAL_ERROR "ncurses build failed")
    endif()

    message(STATUS "ncurses ${_NCURSES_VERSION} built successfully")
  endif()

  set(NCURSES_INCLUDE_DIR "${_NCURSES_DIR}/include")
  set(NCURSES_LIBRARY     "${_NCURSES_LIB}")
  if(EXISTS "${_NCURSES_DIR}/lib/libtinfo.a")
    list(APPEND NCURSES_LIBRARY "${_NCURSES_DIR}/lib/libtinfo.a")
  endif()
  message(STATUS "Using vendored ncurses: ${_NCURSES_DIR}")

else()
  find_package(Curses REQUIRED)
  set(NCURSES_INCLUDE_DIR "${CURSES_INCLUDE_DIR}")
  set(NCURSES_LIBRARY     "${CURSES_LIBRARIES}")
  if(NOT CURSES_HAVE_NCURSES_H)
    message(WARNING "System ncurses not found; linking against curses may fail")
  endif()
endif()

# ---- Generated config.h ----------------------------------------------------
configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/config.h.in"
  "${CMAKE_BINARY_DIR}/include/config.h"
  @ONLY
)

# ---- Generated version.c ---------------------------------------------------
cmake_host_system_information(RESULT _ircit_hostname QUERY HOSTNAME)
set(_ircit_userhost "$ENV{USER}@${_ircit_hostname}")
string(TIMESTAMP _ircit_compiled "%a %b %d %Y at %H:%M:%S")
configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/version.c.in"
  "${CMAKE_BINARY_DIR}/src/version.c"
  @ONLY
)

# ---- FPL static library ----------------------------------------------------
add_subdirectory(src/fpl-14.11)

# ---- Main IRCIT executable --------------------------------------------------
set(IRCIT_SOURCES
  src/alias.c
  src/chanrec.c
  src/chanusers.c
  src/colors.c
  src/configfile.c
  src/ctcp.c
  src/dcc.c
  src/dialogbox.c
  src/dnscache.c
  src/exec.c
  src/flood.c
  src/fpl.c
  src/help.c
  src/history.c
  src/ignore.c
  src/input.c
  src/intrins.c
  src/ircop.c
  src/lastlog.c
  src/list.c
  src/lstatus.c
  src/main.c
  src/match.c
  src/menu.c
  src/misc.c
  src/modes.c
  src/mouse.c
  src/msgbox.c
  src/mvsprintf.c
  src/net.c
  src/notify.c
  src/output.c
  src/people.c
  src/prefs.c
  src/prefsfunc.c
  src/queue.c
  src/scroll.c
  src/serverio.c
  src/serverr.c
  src/servers.c
  src/socketio.c
  src/sockets.c
  src/soundsrv.c
  src/srvwindow.c
  src/stack.c
  src/str.c
  src/telnet.c
  src/telnetcmds.c
  src/terminal.c
  src/thread.c
  src/timer.c
  src/uDCC.c
  src/u_status.c
  src/updates.c
  src/useri.c
  ${CMAKE_BINARY_DIR}/src/version.c
  src/windows.c
)

add_executable(ircit ${IRCIT_SOURCES})

target_include_directories(ircit PRIVATE
  "${CMAKE_SOURCE_DIR}/src/include"
  "${CMAKE_BINARY_DIR}/include"          # generated config.h
  "${NCURSES_INCLUDE_DIR}"
)

target_compile_definitions(ircit PRIVATE
  HAVE_CONFIG_H
  "TERMINFO_DIR=\"${NCURSES_PREFIX}/share/terminfo\""
)

target_link_libraries(ircit PRIVATE
  fpl
  ${NCURSES_LIBRARY}
)

# ---- Install ---------------------------------------------------------------
install(
  CODE [[
    set(_dest "$ENV{HOME}/.ircit")
    if(NOT EXISTS "${_dest}")
      file(MAKE_DIRECTORY "${_dest}")
    endif()
    foreach(_f ircit.conf ircit.fpl)
      set(_src "${CMAKE_SOURCE_DIR}/etc/${_f}")
      set(_dst "${_dest}/${_f}")
      if(NOT EXISTS "${_dst}")
        file(INSTALL "${_src}" DESTINATION "${_dest}")
        message(STATUS "Installed ${_f} to ${_dest}")
      else()
        message(STATUS "Skipped ${_f} (already exists at ${_dst})")
      endif()
    endforeach()
  ]]
)

install(
  FILES "${CMAKE_SOURCE_DIR}/doc/ircit.1"
  DESTINATION share/man/man1
)

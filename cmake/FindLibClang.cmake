if (NOT CLANG_ROOT)
  set(CLANG_ROOT $ENV{CLANG_ROOT})
endif ()

# instead, ln -s llvm-config to the standard PATH
#set(LLVM_CONFIG "/opt/llvm-12/bin/llvm-config")

if (NOT LLVM_CONFIG)
  set(LLVM_CONFIG $ENV{LLVM_CONFIG})
  if (NOT LLVM_CONFIG)
    set(llvm_config_names llvm-config)
    foreach(minor RANGE 9 1)
      list(APPEND llvm_config_names "llvm-config3${minor}" "llvm-config-3.${minor}" "llvm-config-mp-3.${minor}")
    endforeach ()
    find_program(LLVM_CONFIG NAMES ${llvm_config_names})
  endif ()
endif ()

if (LLVM_CONFIG)
  message(STATUS "llvm-config found at: ${LLVM_CONFIG}")
else ()
  message(FATAL_ERROR "Could NOT find llvm-config executable.")
endif ()

if (NOT EXISTS ${CLANG_INCLUDEDIR})
  execute_process(COMMAND ${LLVM_CONFIG} --includedir OUTPUT_VARIABLE CLANG_INCLUDEDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (NOT EXISTS ${CLANG_INCLUDEDIR})
    message(FATAL_ERROR "Could NOT find clang includedir. You can fix this by setting CLANG_INCLUDEDIR in your shell or as a cmake variable.")
  endif ()
endif ()

if (NOT EXISTS ${CLANG_LIBDIR})
  execute_process(COMMAND ${LLVM_CONFIG} --libdir OUTPUT_VARIABLE CLANG_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (NOT EXISTS ${CLANG_LIBDIR})
    message(FATAL_ERROR "Could NOT find clang libdir. You can fix this by setting CLANG_LIBDIR in your shell or as a cmake variable.")
  endif ()
endif ()

if (NOT CLANG_LIBS)
  find_library(CLANG_LIB_HACK_CMAKECACHE_DOT_TEXT_BULLSHIT NAMES clang libclang ${CLANG_ROOT}/lib ${CLANG_LIBDIR} NO_DEFAULT_PATH)
  if (NOT EXISTS ${CLANG_CLANG_LIB_HACK_CMAKECACHE_DOT_TEXT_BULLSHIT})
    find_library(CLANG_LIBS NAMES clang libclang)
    if (NOT EXISTS ${CLANG_LIBS})
      if (MSVC)
        set (CLANG_LIBS "${CLANG_LIBDIR}/libclang.lib")
      else()
        set (CLANG_LIBS "-L${CLANG_LIBDIR}" "-lclang" "-Wl,-rpath,${CLANG_LIBDIR}")
      endif()
    endif ()
  else ()
    set(CLANG_LIBS "${CLANG_LIB_HACK_CMAKECACHE_DOT_TEXT_BULLSHIT}")
  endif ()
endif ()

execute_process(COMMAND ${LLVM_CONFIG} --version OUTPUT_VARIABLE CLANG_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
message("-- Using Clang ${CLANG_VERSION} from ${CLANG_LIBDIR} with LIBS ${CLANG_LIBS} and CXXFLAGS ${CLANG_CXXFLAGS}")



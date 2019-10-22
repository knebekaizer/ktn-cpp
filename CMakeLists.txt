cmake_minimum_required (VERSION 2.8.7)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -MMD" )

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # this does not work? cmdline only

project (ktn-cpp)
#enable_testing()

include_directories(${CMAKE_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++ -MMD")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(LLVM_CONFIG true)
	set(CLANG_INCLUDEDIR "C:/Program Files (x86)/LLVM/include")
	set(CLANG_LIBDIR "C:/Program Files (x86)/LLVM/lib")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(LibClang REQUIRED)

#set(CMAKE_MACOSX_RPATH 1)
#set(CMAKE_INSTALL_RPATH "/Volumes/vdi/work/src/llvm-project/clang-llvm-8.0.1-x86_64-apple-darwin/lib/")
#set(CMAKE_INSTALL_RPATH "/opt/llvm_9/lib/")

#set(CLANG_INCLUDEDIR "/Volumes/vdi/work/src/llvm-project/clang-llvm-8.0.1-x86_64-apple-darwin/include/")
#set(CLANG_INCLUDEDIR "/opt/llvm_9/include/")
#set(CLANG_LIBS "/Volumes/vdi/work/src/llvm-project/clang/_build/lib/")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -v  -L/Volumes/vdi/work/src/llvm-project/clang/_build/lib/ -lclang")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -v ")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CLANG_INCLUDEDIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#set(LLVM_ENABLE_IDE "ON")
#add_subdirectory(llvm-project/clang)

#add_library(libreflang STATIC lib/reflang.hpp lib/reflang.cpp sample/h1.h)

#add_executable(
#	reflang
#	src/cmdargs.cpp src/cmdargs.hpp
#	src/main.cpp
#	src/parser.class.cpp src/parser.class.hpp
#	src/parser.cpp src/parser.hpp
#	src/parser.enum.cpp src/parser.enum.hpp
#	src/parser.function.cpp src/parser.function.hpp
#	src/parser.util.cpp src/parser.util.hpp
#	src/types.cpp src/types.hpp src/generator.h src/generator.cpp
#)

add_executable(
	ktn-cpp
	src/main.cpp
	src/parser_tree.cpp
	src/utils.cpp
)

add_executable(
		files
#		src/main.cpp
		src/files.cpp
)

add_executable(
		parseTest
		src/parseTest.cpp
		src/utils.cpp
)

target_link_libraries(ktn-cpp ${CLANG_LIBS})
target_link_libraries(parseTest ${CLANG_LIBS})
#target_link_libraries(parseTest clang)

target_compile_definitions(ktn-cpp PUBLIC "-DUSE_RUNTIME_LOG_LEVEL -DDEF_LOG_LEVEL=6")
target_compile_definitions(files PUBLIC "-DUSE_RUNTIME_LOG_LEVEL -DDEF_LOG_LEVEL=6")


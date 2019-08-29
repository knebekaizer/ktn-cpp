cmake_minimum_required (VERSION 2.8.7)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror" )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project (ktn-cpp)
enable_testing()

include_directories(${CMAKE_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(LLVM_CONFIG true)
	set(CLANG_INCLUDEDIR "C:/Program Files (x86)/LLVM/include")
	set(CLANG_LIBDIR "C:/Program Files (x86)/LLVM/lib")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(LibClang REQUIRED)
include_directories(${CLANG_INCLUDEDIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


#add_library(libreflang STATIC lib/reflang.hpp lib/reflang.cpp sample/h1.h)

add_executable(
	reflang
	src/cmdargs.cpp src/cmdargs.hpp
	src/main.cpp
	src/parser.class.cpp src/parser.class.hpp
	src/parser.cpp src/parser.hpp
	src/parser.enum.cpp src/parser.enum.hpp
	src/parser.function.cpp src/parser.function.hpp
	src/parser.util.cpp src/parser.util.hpp
	src/types.cpp src/types.hpp src/generator.h src/generator.cpp
)

add_executable(
	ktn-cpp
	src/cmdargs.cpp src/cmdargs.hpp
	src/main.cpp
	src/parser_tree.cpp
	src/utils.cpp

#	src/parser.class.cpp src/parser.class.hpp
#	src/parser.cpp src/parser.hpp
#	src/parser.enum.cpp src/parser.enum.hpp
#	src/parser.function.cpp src/parser.function.hpp
#	src/parser.util.cpp src/parser.util.hpp
#	src/types.cpp src/types.hpp src/generator.h src/generator.cpp
)
target_link_libraries(ktn-cpp ${CLANG_LIBS})
target_link_libraries(reflang ${CLANG_LIBS})

target_compile_definitions(ktn-cpp PUBLIC "-DUSE_RUNTIME_LOG_LEVEL")


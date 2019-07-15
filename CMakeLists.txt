cmake_minimum_required (VERSION 2.8.7)
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

add_library(libreflang STATIC lib/reflang.hpp lib/reflang.cpp)

add_executable(
	reflang
	src/cmdargs.cpp src/cmdargs.hpp
	src/main.cpp
	src/parser.class.cpp src/parser.class.hpp
	src/parser.cpp src/parser.hpp
	src/parser.enum.cpp src/parser.enum.hpp
	src/parser.function.cpp src/parser.function.hpp
	src/parser.util.cpp src/parser.util.hpp
#	src/serializer.class.cpp src/serializer.class.hpp
#	src/serializer.cpp src/serializer.hpp
#	src/serializer.enum.cpp src/serializer.enum.hpp
#	src/serializer.function.cpp src/serializer.function.hpp
#	src/serializer.util.cpp src/serializer.util.hpp
	src/types.cpp src/types.hpp)
target_link_libraries(reflang ${CLANG_LIBS})

add_subdirectory(tests)

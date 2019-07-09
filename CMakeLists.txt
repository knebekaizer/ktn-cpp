cmake_minimum_required(VERSION 3.14)
project(try1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror" )

include_directories( . /usr/local/include )

# /Library/Developer/CommandLineTools/usr/include/c++/v1/
#add_library(clang SHARED IMPORTED)
link_directories(/usr/local/lib)

add_executable(try1 extents.cpp)

target_link_libraries(try1 -lclang)

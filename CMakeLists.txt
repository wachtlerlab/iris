cmake_minimum_required(VERSION 2.8.4)
project(spectre)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(lib)
file(GLOB_RECURSE lib_SOURCES lib/*.cc)
file(GLOB_RECURSE lib_INCLUDES lib/*.h)

add_library(spectre SHARED ${lib_INCLUDES} ${lib_SOURCES})

set(pr655_SOURCES tools/pr655.cc)
add_executable(pr655 ${pr655_SOURCES})

target_link_libraries(pr655 spectre)
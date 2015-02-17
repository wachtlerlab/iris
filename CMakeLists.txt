cmake_minimum_required(VERSION 2.8.4)
project(spectre)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

########################################
# Boost

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.49.0 REQUIRED program_options)

include_directories(${Boost_INCLUDE_DIR})
set(LINK_LIBS ${LINK_LIBS} ${Boost_LIBRARIES})

#######################################
#

include_directories(lib)
file(GLOB_RECURSE lib_SOURCES lib/*.cc)
file(GLOB_RECURSE lib_INCLUDES lib/*.h)

add_library(spectre SHARED ${lib_INCLUDES} ${lib_SOURCES})
target_link_libraries(spectre ${LINK_LIBS})

set(pr655_SOURCES tools/pr655.cc)
add_executable(pr655 ${pr655_SOURCES})

target_link_libraries(pr655 spectre)
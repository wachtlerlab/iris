cmake_minimum_required(VERSION 2.8.4)
project(iris)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

########################################
# Boost

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.49.0 REQUIRED program_options)

include_directories(${Boost_INCLUDE_DIR})
set(LINK_LIBS ${LINK_LIBS} ${Boost_LIBRARIES})

########################################
# OpenGL + Co
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(GLM REQUIRED)
pkg_search_module(FREETYPE REQUIRED freetype2)

include_directories(${GLM_INCLUDE_DIRS}
                    ${GLFW_INCLUDE_DIRS}
                    ${OpenGL_INCLUDE_DIRS}
                    ${FREETYPE_INCLUDE_DIRS})

link_directories(${GLFW_LIBRARY_DIRS}
                 ${OpenGL_LIBRARY_DIRS}
                 ${OPENCL_LIBRARY_DIRS})

set(LINK_LIBS ${LINK_LIBS} ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${FREETYPE_LIBRARIES})

#######################################
# lib

include_directories(lib)
file(GLOB_RECURSE lib_SOURCES lib/*.cc)
file(GLOB_RECURSE lib_INCLUDES lib/*.h)

add_library(iris SHARED ${lib_INCLUDES} ${lib_SOURCES})
target_link_libraries(iris ${LINK_LIBS})

#######################################
# tools

set(pr655_SOURCES tools/pr655.cc)
add_executable(pr655 ${pr655_SOURCES})
target_link_libraries(pr655 iris)

set(info_SOURCES tools/info.cc)
add_executable(iris-info ${info_SOURCES})
target_link_libraries(iris-info iris)

set(calib_SOURCES tools/calibrate.cc)
add_executable(iris-measure ${calib_SOURCES})
target_link_libraries(iris-measure iris)

########################################
# playground

if(EXISTS "${CMAKE_SOURCE_DIR}/play.cc")
  add_executable(iris-play EXCLUDE_FROM_ALL play.cc)
  target_link_libraries(iris-play iris)
endif()

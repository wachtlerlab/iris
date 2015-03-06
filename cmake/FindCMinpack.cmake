# - Try to find CMinpack
# Once done this will define
#  CMINPACK_FOUND - System has CMinpack
#  CMINPACK_INCLUDE_DIRS - The CMinpack include directories
#  CMINPACK_LIBRARIES - The libraries needed to use CMinpack

find_path(CMINPACK_INCLUDE_DIR cminpack.h
  HINTS /usr/local/include
        /usr/include
        $ENV{CMINPACK_ROOT}/include
        PATH_SUFFIXES cminpack-1)

find_library(CMINPACK_LIBRARY NAMES cminpack libcminpack
  HINTS ${CMINPACK_INCLUDE_DIR}/../lib
  HINTS ${CMINPACK_INCLUDE_DIRS}/../build
  /usr/local/lib
  /usr/lib)

set(CMINPACK_LIBRARIES ${CMINPACK_LIBRARY})
set(CMINPACK_INCLUDE_DIRS ${CMINPACK_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CMinpack DEFAULT_MSG
  CMINPACK_LIBRARY CMINPACK_INCLUDE_DIR)

mark_as_advanced(CMINPACK_INCLUDE_DIR CMINPACK_INCLUDE_DIR)
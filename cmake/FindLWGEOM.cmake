# - Find lwgeom
# Find the PostgreSQL includes and client library
# This module defines
#  LWGEOM_INCLUDE_DIR
#  LWGEOM_LIBRARIES
#
# TODO lots of refinement to be able to work on windows & mac at least
# Copyright (c) 2021, Vicky Vergara <vicky@georepublic.org>

find_library(LWGEOM_LIBRARIES
  NAMES lwgeom
  PATHS
     /lib /lib64 /usr/lib /usr/lib64
  )

find_path(LWGEOM_INCLUDE_DIRS
  NAMES liblwgeom.h
  PATHS
    /usr/include
  )


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LWGEOM
  FOUND_VAR LWGEOM_FOUND
  REQUIRED_VARS LWGEOM_INCLUDE_DIRS LWGEOM_LIBRARIES
  FAIL_MESSAGE "Could NOT find lwgeom")

message(STATUS "LWGEOM_INCLUDE_DIRS=${LWGEOM_INCLUDE_DIRS}")
message(STATUS "LWGEOM_LIBRARIES=${LWGEOM_LIBRARIES}")

if (LWGEOM_FOUND)
  mark_as_advanced(LWGEOM_INCLUDE_DIRS PROJ_LIBRARIES)
endif()

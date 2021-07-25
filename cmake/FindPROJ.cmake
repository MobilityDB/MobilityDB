# - Find proj
# Find the PostgreSQL includes and client library
# This module defines
#  PROJ_INCLUDE_DIR
#  PROJ_LIBRARIES
#
# TODO lots of refinement to be able to work on windows & mac at least
# Copyright (c) 2021, Vicky Vergara <vicky@georepublic.org>

find_library(PROJ_LIBRARIES
  NAMES proj
  PATHS
     /lib /lib64 /usr/lib /usr/lib64
  )

find_path(PROJ_INCLUDE_DIRS
  NAMES proj_api.h
  PATHS
    /usr/include
  )


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PROJ
  FOUND_VAR PROJ_FOUND
  REQUIRED_VARS PROJ_INCLUDE_DIRS PROJ_LIBRARIES
  FAIL_MESSAGE "Could NOT find proj")

if (PROJ_FOUND)
  mark_as_advanced(PROJ_INCLUDE_DIRS PROJ_LIBRARIES)
endif()

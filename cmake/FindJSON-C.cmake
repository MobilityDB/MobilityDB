# - Find json-c
# Find the PostgreSQL includes and client library
# This module defines
#  JSONC_INCLUDE_DIR
#  JSONC_LIBRARIES
#
# Copyright (c) 2021, Vicky Vergara <vicky@georepublic.org>

find_library(JSON-C_LIBRARIES
  NAMES json-c
  HINTS /lib /lib64 /usr/lib /usr/lib64
  )

find_path(JSON-C_INCLUDE_DIRS
  NAMES json.h
  HINTS /usr/include PATH_SUFFIXES json-c json
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSON-C
  FOUND_VAR JSON-C_FOUND
  REQUIRED_VARS JSON-C_INCLUDE_DIRS JSON-C_LIBRARIES
  FAIL_MESSAGE "Could NOT find json-c")

if(JSON-C_FOUND)
  mark_as_advanced(JSON-C_INCLUDE_DIRS JSON-C_LIBRARIES)
endif()

message(STATUS "JSON-C_INCLUDE_DIRS: ${JSON-C_INCLUDE_DIRS}")
message(STATUS "JSON-C_LIBRARIES: ${JSON-C_LIBRARIES}")

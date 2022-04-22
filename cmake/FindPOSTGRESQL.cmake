# - Find PostgreSQL
# Find the PostgreSQL includes and client library
# This module defines
#  POSTGRESQL_INCLUDE_DIR, where to find postgres.h
#  POSTGRESQL_BINARY_DIR, location of postgres binaries
#  POSTGRESQL_LIBRARIES, the libraries needed to use POSTGRESQL.
#  POSTGRESQL_FOUND, If false, do not try to use PostgreSQL.
#  POSTGRESQL_VERSION_STRING
#
# Copyright (c) 2021, Vicky Vergara <vicky@georepublic.org>
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET(POSTGRESQL_BIN "" CACHE STRING "non-standard path to the postgresql program executables")

if(NOT "${POSTGRESQL_BIN}" STREQUAL "")
  # Checking POSTGRESQL_PG_CONFIG
  find_program(POSTGRESQL_PG_CONFIG NAMES pg_config
    PATHS
    ${POSTGRESQL_BIN}
    NO_DEFAULT_PATH
    )
else()
  # Checking POSTGRESQL_PG_CONFIG
  # First we search for installation via source compilation
  # If not found, we search for normal installation (via package repositories)
  find_program(POSTGRESQL_PG_CONFIG NAMES pg_config
    PATHS
    /usr/local/pgsql/bin/
    NO_DEFAULT_PATH
    )
  if(NOT POSTGRESQL_PG_CONFIG)
    unset(POSTGRESQL_PG_CONFIG CACHE)
    find_program(POSTGRESQL_PG_CONFIG NAMES pg_config
      PATHS
      /usr/lib/postgresql/*/bin/
      )
  endif()
endif()

if(POSTGRESQL_PG_CONFIG)
  execute_process(
    COMMAND ${POSTGRESQL_PG_CONFIG} --bindir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE POSTGRESQL_BIN_DIR)
else()
  message(FATAL_ERROR "Could not find pg_config")
endif()

set(POSTGRESQL_PG_CONFIG "${POSTGRESQL_BIN_DIR}/pg_config")

execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --version
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE POSTGRESQL_VERSION_STRING)

string(SUBSTRING "${POSTGRESQL_VERSION_STRING}" 11 -1 POSTGRESQL_VERSION)

# for XbetaY XalphaY XrcY -> X.Y
string(REGEX REPLACE "^([0-9]+)[beta|alpha|rc|devel].*" "\\1.0" POSTGRESQL_VERSION ${POSTGRESQL_VERSION})

#for X.Y.Z -> XY  Y<10
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\1.\\2" POSTGRESQL_VERSION ${POSTGRESQL_VERSION})
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\1" POSTGRESQL_VERSION_MAJOR ${POSTGRESQL_VERSION})
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*" "\\2" POSTGRESQL_VERSION_MINOR ${POSTGRESQL_VERSION})

message(STATUS "POSTGRESQL_VERSION_MAJOR=${POSTGRESQL_VERSION_MAJOR}")
message(STATUS "POSTGRESQL_VERSION_MINOR=${POSTGRESQL_VERSION_MINOR}")
math(EXPR POSTGRESQL_VERSION_NUMBER "${POSTGRESQL_VERSION_MAJOR} * 10000 + ${POSTGRESQL_VERSION_MINOR} * 100")
# Set POSTGIS_PGSQL_VERSION which is needed in the PostGIS source code embedded in MobilityDB
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10")

execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --includedir-server
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE POSTGRESQL_INCLUDE_DIR)

#[[
#instead of path and our own guesses
find_path(POSTGRESQL_INCLUDE_DIR postgres.h
  HINTS
  ${TEMP_POSTGRESQL_INCLUDE_DIR}
  PATHS
  /usr/include/server
  /usr/include/pgsql/server
  /usr/local/include/pgsql/server
  /usr/include/postgresql/server
  /usr/include/postgresql/*/server
  /usr/local/include/postgresql/server
  /usr/local/include/postgresql/*/server
  $ENV{ProgramFiles}/PostgreSQL/*/include/server
  $ENV{SystemDrive}/PostgreSQL/*/include/server
  )
]]

execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --libdir
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE POSTGRESQL_LIBRARIES)
execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --sharedir
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE POSTGRESQL_SHARE_DIR)
execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --pkglibdir
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE POSTGRESQL_DYNLIB_DIR)

message(STATUS "POSTGRESQL_BIN_DIR: ${POSTGRESQL_BIN_DIR}")
message(STATUS "POSTGRESQL_INCLUDE_DIR: ${POSTGRESQL_INCLUDE_DIR}")
message(STATUS "POSTGRESQL_LIBRARIES: ${POSTGRESQL_LIBRARIES}")
message(STATUS "POSTGRESQL_SHARE_DIR: ${POSTGRESQL_SHARE_DIR}")
message(STATUS "POSTGRESQL_DYNLIB_DIR: ${POSTGRESQL_DYNLIB_DIR}")
message(STATUS "POSTGRESQL_VERSION: ${POSTGRESQL_VERSION}")
message(STATUS "POSTGRESQL_VERSION_NUMBER: ${POSTGRESQL_VERSION_NUMBER}")
message(STATUS "POSTGRESQL_VERSION_STRING: ${POSTGRESQL_VERSION_STRING}")
message(STATUS "POSTGIS_PGSQL_VERSION: ${POSTGIS_PGSQL_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(POSTGRESQL
  FOUND_VAR POSTGRESQL_FOUND
  REQUIRED_VARS POSTGRESQL_BIN_DIR POSTGRESQL_INCLUDE_DIR POSTGRESQL_LIBRARIES POSTGRESQL_SHARE_DIR POSTGRESQL_DYNLIB_DIR
  VERSION_VAR POSTGRESQL_VERSION)

if (POSTGRESQL_FOUND)
  mark_as_advanced(POSTGRESQL_BIN_DIR POSTGRESQL_INCLUDE_DIR POSTGRESQL_LIBRARIES POSTGRESQL_SHARE_DIR POSTGRESQL_DYNLIB_DIR)
endif()

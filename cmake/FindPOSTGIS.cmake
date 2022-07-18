# (c) 2015  pgRouting colaborators
#
# Finds the most recent postGIS for a particular postgreSQL
# We need this for the tests
#
# Usage:
# find_package(POSTGIS)
#
# The following variables are set if PostGIS is found:
# POSTGIS_FOUND             - Set to true when PostGIS is found.
# POSTGIS_LIBRARY           - if we ever need to link it
# POSTGIS_CONTROL           - if we ever need to see the contents
# POSTGIS_VERSION           - The full numbers
# POSTGIS_VERSION_STR       - The th PostGIS prefix

if(POSTGIS_FOUND)
  return()
endif()

if(NOT POSTGRESQL_FOUND)
  find_package(POSTGRESQL REQUIRED)
endif()

# Find PostGIS library

# If specific version of PostGIS requested, choose that one, otherwise get all versions
if(POSTGIS_REQUIRED_VERSION)
  message(STATUS "Selecting requested PostGIS version: Selecting postgis-${POSTGIS_REQUIRED_VERSION}")
  file(GLOB POSTGIS_LIBRARY "${POSTGRESQL_DYNLIB_DIR}/postgis-${POSTGIS_REQUIRED_VERSION}.*")
else()
  file(GLOB POSTGIS_LIBRARY "${POSTGRESQL_DYNLIB_DIR}/postgis-*.*")
endif()

if(POSTGIS_LIBRARY STREQUAL "")
  message(FATAL_ERROR "No PostGIS library have been found")
else()
  # If several versions of PostGIS found, choose the first one
  list(LENGTH POSTGIS_LIBRARY NO_POSTGIS_LIBRARIES)
  if(NO_POSTGIS_LIBRARIES GREATER 1)
    list(GET POSTGIS_LIBRARY 0 POSTGIS_LIBRARY)
    message(STATUS "Several PostGIS versions found: Selecting ${POSTGIS_LIBRARY}")
  endif()
endif()

message(STATUS "Looking for postgis.control file in ${POSTGRESQL_SHARE_DIR}/extension")
find_file(POSTGIS_CONTROL postgis.control
  PATHS "${POSTGRESQL_SHARE_DIR}/extension")

if(POSTGIS_CONTROL)
  file(READ ${POSTGIS_CONTROL} control_contents)
  string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" POSTGIS_VERSION ${control_contents})
  set(POSTGIS_VERSION_STR "PostGIS ${POSTGIS_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1" POSTGIS_VERSION_MAJOR ${POSTGIS_VERSION})
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\2" POSTGIS_VERSION_MINOR ${POSTGIS_VERSION})
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\3" POSTGIS_VERSION_MICRO ${POSTGIS_VERSION})
  math(EXPR POSTGIS_VERSION_NUMBER "${POSTGIS_VERSION_MAJOR} * 10000 + ${POSTGIS_VERSION_MINOR} * 100 + ${POSTGIS_VERSION_MICRO}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(POSTGIS
  FOUND_VAR POSTGIS_FOUND
  REQUIRED_VARS POSTGIS_LIBRARY POSTGIS_CONTROL
  VERSION_VAR POSTGIS_VERSION
  FAIL_MESSAGE "Could NOT find PostGIS")

if(POSTGIS_FOUND)
  mark_as_advanced(POSTGIS_LIBRARY POSTGIS_CONTROL POSTGIS_VERSION POSTGIS_VERSION_STR)
endif()

message(STATUS "POSTGIS_LIBRARY: ${POSTGIS_LIBRARY}")
message(STATUS "POSTGIS_CONTROL: ${POSTGIS_CONTROL}")
message(STATUS "POSTGIS_VERSION: ${POSTGIS_VERSION}")
message(STATUS "POSTGIS_VERSION_STR: ${POSTGIS_VERSION_STR}")

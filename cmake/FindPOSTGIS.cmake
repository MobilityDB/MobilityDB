# (c) 2015  pgRouting colaborators
#
# Finds the most recent postGIS for a particular postgreSQL
# We need this for the tests
#
# Usage:
# find_package(POSTGIS)
#
# Only finds version 2.5 needed for MobilityDB
#
# The following variables are set if PostGIS is found:
# POSTGIS_FOUND             - Set to true when PostGIS is found.
# POSTGIS_LIBRARY           - if we ever need to link it
# POSTGIS_CONTROL           - if we ever need to see the contents
# POSTGIS_VERSION           - The full numbers
# POSTGIS_VERSION_STR       - The th PostGIS prefix

if (POSTGIS_FOUND)
  return()
endif()

if (NOT POSTGRESQL_FOUND)
  find_package(POSTGRESQL REQUIRED)
endif()

# TODO Will worry about other versions of PostGIS when time arrives
find_library(POSTGIS_LIBRARY
  NAMES postgis-2.5.so
  PATHS "${POSTGRESQL_DYNLIB_DIR}")

find_file(POSTGIS_CONTROL  postgis.control
  PATHS "${POSTGRESQL_SHARE_DIR}/extension")

if (POSTGIS_CONTROL)
  file(READ ${POSTGIS_CONTROL} control_contents)
  string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" POSTGIS_VERSION ${control_contents})
  set(POSTGIS_VERSION_STR "PostGIS ${POSTGIS_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1" POSTGIS_VERSION_MAYOR ${POSTGIS_VERSION})
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\2" POSTGIS_VERSION_MINOR ${POSTGIS_VERSION})
  string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\3" POSTGIS_VERSION_MICRO ${POSTGIS_VERSION})
  math(EXPR POSTGIS_VERSION_NUMBER "${POSTGIS_VERSION_MAYOR} * 10000 + ${POSTGIS_VERSION_MINOR} * 100 + ${POSTGIS_VERSION_MICRO}")
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(POSTGIS
  FOUND_VAR POSTGIS_FOUND
  REQUIRED_VARS POSTGIS_LIBRARY POSTGIS_CONTROL
  VERSION_VAR POSTGIS_VERSION
  FAIL_MESSAGE "Could NOT find PostGIS")

if (POSTGIS_FOUND)
  mark_as_advanced(POSTGIS_LIBRARY POSTGIS_CONTROL POSTGIS_VERSION POSTGIS_VERSION_STR)
endif()

message(STATUS "POSTGIS_LIBRARY: ${POSTGIS_LIBRARY}")
message(STATUS "POSTGIS_CONTROL: ${POSTGIS_CONTROL}")
message(STATUS "POSTGIS_VERSION_STR: ${POSTGIS_VERSION_STR}")

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

file(READ ${POSTGIS_CONTROL} control_contents)
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" POSTGIS_VERSION ${control_contents})
message(STATUS "POSTGIS_VERSION ${POSTGIS_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(POSTGIS
  FOUND_VAR POSTGIS_FOUND
  REQUIRED_VARS POSTGIS_LIBRARY POSTGIS_CONTROL
  VERSION_VAR POSTGIS_VERSION
  FAIL_MESSAGE "Could NOT find PostGIS")

if (POSTGIS_FOUND)
  mark_as_advanced(POSTGIS_LIBRARY POSTGIS_CONTROL POSTGIS_VERSION)
endif()

message(STATUS POSTGIS_LIBRARY ${POSTGIS_LIBRARY})
message(STATUS POSTGIS_CONTROL ${POSTGIS_CONTROL})

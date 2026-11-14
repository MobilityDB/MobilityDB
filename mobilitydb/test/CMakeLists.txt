#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

# The suite runs with pg_regress, see scripts/regress.cmake. Each module
# registers its tests with mobilitydb_add_tests, and the schedule lists the
# setup scripts of every module, which create and load the tables the tests
# read, before the tests.

# The temporary instance of the tests lies in tmptest under the build
# directory, unless another directory is given at configure time
set(MOBILITYDB_TEST_TMPDIR "${CMAKE_BINARY_DIR}/tmptest" CACHE PATH
  "Directory of the temporary PostgreSQL instance of the tests")

# The instance is created in the C locale and, unless another encoding is
# given, in UTF-8. A database in another encoding takes other paths for text
# values, so a CI job runs the suite with SQL_ASCII as well.
set(MOBILITYDB_TEST_ENCODING "UTF8" CACHE STRING
  "Encoding of the temporary PostgreSQL instance of the tests")

# The directory pg_regress reads the tests from and writes their results to
set(MOBILITYDB_REGRESS_DIR "${CMAKE_CURRENT_BINARY_DIR}/regress")

# pg_regress is installed with the files for building extensions
execute_process(
  COMMAND ${POSTGRESQL_PG_CONFIG} --pgxs
  OUTPUT_VARIABLE _pgxs
  OUTPUT_STRIP_TRAILING_WHITESPACE)
get_filename_component(_pgxs_dir "${_pgxs}" DIRECTORY)
get_filename_component(PG_REGRESS
  "${_pgxs_dir}/../test/regress/pg_regress${CMAKE_EXECUTABLE_SUFFIX}" ABSOLUTE)

# Register the tests of the module of the current directory: its setup
# scripts, which run first in the order given, and every other file of its
# sql directory. A test whose name ends in _pg<N> or _pgis<N> runs only with
# PostgreSQL or PostGIS <N> or later.
function(mobilitydb_add_tests)
  cmake_parse_arguments(ARG "" "" "SETUP" ${ARGN})
  file(GLOB _files "${CMAKE_CURRENT_SOURCE_DIR}/sql/*.sql")
  list(SORT _files)
  foreach(_file ${_files})
    get_filename_component(_name ${_file} NAME_WE)
    if(_name IN_LIST ARG_SETUP)
      continue()
    endif()
    if(_name MATCHES "_pg([0-9]+)" AND
       POSTGRESQL_VERSION_MAJOR LESS CMAKE_MATCH_1)
      message(STATUS "Disabling test ${_name}")
      continue()
    endif()
    if(_name MATCHES "_pgis([0-9]+)" AND
       POSTGIS_VERSION_MAJOR LESS CMAKE_MATCH_1)
      message(STATUS "Disabling test ${_name}")
      continue()
    endif()
    set_property(GLOBAL APPEND PROPERTY MOBILITYDB_TESTS ${_name})
  endforeach()
  set_property(GLOBAL APPEND PROPERTY MOBILITYDB_TEST_SETUPS ${ARG_SETUP})
  set_property(GLOBAL APPEND PROPERTY MOBILITYDB_TEST_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

add_subdirectory(temporal)
add_subdirectory(geo)
if(CBUFFER)
  add_subdirectory(cbuffer)
endif()
if(H3)
  add_subdirectory(h3)
endif()
if(JSON)
  add_subdirectory(json)
endif()
if(NPOINT)
  add_subdirectory(npoint)
endif()
if(POINTCLOUD)
  add_subdirectory(pointcloud)
endif()
if(POSE)
  add_subdirectory(pose)
  add_subdirectory(posechain)
endif()
if(QUADBIN)
  add_subdirectory(quadbin)
endif()
if(RASTER)
  add_subdirectory(raster)
endif()
if(RGEO)
  add_subdirectory(rgeo)
endif()
if(S2CELL)
  add_subdirectory(s2cell)
endif()
# The test of the input and output of every type reads the tables of every
# module, so it is registered when every optional module is built
if(CBUFFER AND H3 AND JSON AND NPOINT AND POINTCLOUD AND POSE AND QUADBIN AND
    RASTER AND RGEO AND S2CELL)
  add_subdirectory(io)
endif()

get_property(MOBILITYDB_TEST_SETUPS GLOBAL PROPERTY MOBILITYDB_TEST_SETUPS)
get_property(_tests GLOBAL PROPERTY MOBILITYDB_TESTS)
get_property(MOBILITYDB_TEST_DIRS GLOBAL PROPERTY MOBILITYDB_TEST_DIRS)

set(_schedule "")
foreach(_test ${MOBILITYDB_TEST_SETUPS} ${_tests})
  string(APPEND _schedule "test: ${_test}\n")
endforeach()
file(WRITE ${MOBILITYDB_REGRESS_DIR}/schedule "${_schedule}")

# The settings of the temporary instance
file(WRITE ${MOBILITYDB_REGRESS_DIR}/regress.conf
  "shared_preload_libraries = '${POSTGIS_LIBRARY}'\n"
  "max_locks_per_transaction = 128\n"
  "timezone = 'America/Los_Angeles'\n"
  "datestyle = 'Postgres, MDY'\n"
  "log_error_verbosity = 'TERSE'\n"
  "parallel_tuple_cost = 100\n"
  "parallel_setup_cost = 100\n"
  "min_parallel_table_scan_size = 0\n"
  "min_parallel_index_scan_size = 0\n")

# The extensions of the test database, in the order of their dependencies:
# the ones the build requires and then mobilitydb
string(REPLACE ", " ";" MOBILITYDB_TEST_EXTENSIONS "${MOBILITYDB_REQUIRES}")
list(APPEND MOBILITYDB_TEST_EXTENSIONS mobilitydb)

configure_file(scripts/regress.cmake regress.cmake @ONLY)

add_test(
  NAME build
  COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR}
  )

add_test(
  NAME regress
  COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/regress.cmake"
  )
set_tests_properties(regress PROPERTIES DEPENDS build)

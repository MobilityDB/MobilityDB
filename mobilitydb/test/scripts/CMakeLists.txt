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

set(CTEST_PARALLEL_LEVEL 1)

find_package(Xz REQUIRED)

configure_file(test.cmake test.cmake @ONLY)

add_test(
  NAME build
  COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR}
  )

add_test(
  NAME test_setup
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND "${CMAKE_COMMAND}" -D TEST_OPER="test_setup" -P "${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake"
  )

add_test(
  NAME teardown
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND "${CMAKE_COMMAND}" -D TEST_OPER="teardown" -P "${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake"
  )

set_tests_properties(test_setup PROPERTIES
  DEPENDS build
  FIXTURES_SETUP DBSETUP
  RESOURCE_LOCK DBLOCK)

set_tests_properties(teardown PROPERTIES
  FIXTURES_CLEANUP DB;DBEXT;DBSETUP
  RESOURCE_LOCK DBLOCK)

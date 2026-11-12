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

# Unit tests of the cells of meos/src/h3
#
# These programs test internal functions of meos/src/h3, so CMake builds
# them against the internal headers; their names end in _unit.c, so that CI
# does not take them for tests of the public API.  Run them with
#   ctest -R 'h3_outline|h3_owner|h3_ring|h3_point|h3_walk|h3_cover'
# The first four read every cell up to the resolution they take, 2 here, 1
# for h3_point_unit; run them with 3 or more for a deeper check (about a
# minute at 3, at 2 for h3_point_unit). h3_walk_unit and h3_cover_unit read
# the rows of an exact oracle.

add_executable(h3_outline_unit h3_outline_unit.c)
target_link_libraries(h3_outline_unit ${MEOS_LIB_NAME} ${H3_LIBRARY} postgis
  pgtypes m)
add_test(NAME h3_outline_unit COMMAND h3_outline_unit)

add_executable(h3_owner_unit h3_owner_unit.c)
target_link_libraries(h3_owner_unit ${MEOS_LIB_NAME} ${H3_LIBRARY} postgis
  pgtypes m)
add_test(NAME h3_owner_unit COMMAND h3_owner_unit)

add_executable(h3_ring_unit h3_ring_unit.c)
target_include_directories(h3_ring_unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(h3_ring_unit ${MEOS_LIB_NAME} ${H3_LIBRARY} postgis
  pgtypes m)
add_test(NAME h3_ring_unit COMMAND h3_ring_unit)

add_executable(h3_point_unit h3_point_unit.c)
target_include_directories(h3_point_unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(h3_point_unit ${MEOS_LIB_NAME} ${H3_LIBRARY} postgis
  pgtypes m)
add_test(NAME h3_point_unit COMMAND h3_point_unit)

add_executable(h3_walk_unit h3_walk_unit.c)
target_include_directories(h3_walk_unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(h3_walk_unit ${MEOS_LIB_NAME} ${H3_LIBRARY} postgis
  pgtypes m)
add_test(NAME h3_walk_unit COMMAND h3_walk_unit)

add_executable(h3_cover_unit h3_cover_unit.c)
target_include_directories(h3_cover_unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(h3_cover_unit ${MEOS_LIB_NAME} ${H3_LIBRARY} postgis
  pgtypes m)
add_test(NAME h3_cover_unit COMMAND h3_cover_unit)

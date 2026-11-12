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

# Unit tests of the exact predicates of meos/include/geo/geo_funcs.h
#
# These programs test internal functions of the geometry engine, as the
# programs of ../rgeo do for the rigid geometries: they need the internal
# headers, so CMake builds them, and their names end in _unit.c so that CI
# does not take them for tests of the public API.
#
# Run them with
#   ctest -R _unit

add_executable(arc_cross_unit arc_cross_unit.c)
target_link_libraries(arc_cross_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME arc_cross_unit COMMAND arc_cross_unit)

add_executable(ray_cast_unit ray_cast_unit.c)
target_link_libraries(ray_cast_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME ray_cast_unit COMMAND ray_cast_unit)

add_executable(shift_signs_unit shift_signs_unit.c)
target_include_directories(shift_signs_unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(shift_signs_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME shift_signs_unit COMMAND shift_signs_unit)

add_executable(within_time_unit within_time_unit.c)
target_link_libraries(within_time_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME within_time_unit COMMAND within_time_unit)

add_executable(simple_traj_unit simple_traj_unit.c)
target_link_libraries(simple_traj_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME simple_traj_unit COMMAND simple_traj_unit)

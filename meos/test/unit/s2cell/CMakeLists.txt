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

# Unit tests of the cells of meos/src/s2cell
#
# These programs test internal functions of meos/src/s2cell, so CMake builds
# them against the internal headers; their names end in _unit.c, so that CI
# does not take them for tests of the public API.  Run them with
#   ctest -R 's2_owner|s2_corner'
# s2_owner_unit reads every cell up to the level it takes, 4 here;
# s2_corner_unit reads the rows of an exact judge.

add_executable(s2_owner_unit s2_owner_unit.c)
target_link_libraries(s2_owner_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME s2_owner_unit COMMAND s2_owner_unit)

add_executable(s2_corner_unit s2_corner_unit.c)
target_include_directories(s2_corner_unit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(s2_corner_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME s2_corner_unit COMMAND s2_corner_unit)

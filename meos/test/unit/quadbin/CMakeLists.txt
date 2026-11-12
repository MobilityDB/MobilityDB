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

# Unit tests of the cells of meos/src/quadbin
#
# These programs test internal functions of meos/src/quadbin, so CMake builds
# them against the internal headers; their names end in _unit.c, so that CI
# does not take them for tests of the public API.  Run them with
#   ctest -R 'quadbin_owner|quadbin_point|quadbin_border|quadbin_walk'
# The first reads every tile up to the zoom it takes, 8 here; the second the
# bounds of the tiles up to zoom 10, the third the split of a segment ending
# on those bounds, and the fourth the walks of quadbin_walk_rows.inc.

add_executable(quadbin_owner_unit quadbin_owner_unit.c)
target_link_libraries(quadbin_owner_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME quadbin_owner_unit COMMAND quadbin_owner_unit)

add_executable(quadbin_point_unit quadbin_point_unit.c)
target_link_libraries(quadbin_point_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME quadbin_point_unit COMMAND quadbin_point_unit)

add_executable(quadbin_border_unit quadbin_border_unit.c)
target_link_libraries(quadbin_border_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME quadbin_border_unit COMMAND quadbin_border_unit)

add_executable(quadbin_walk_unit quadbin_walk_unit.c)
target_link_libraries(quadbin_walk_unit ${MEOS_LIB_NAME} postgis pgtypes m)
add_test(NAME quadbin_walk_unit COMMAND quadbin_walk_unit)

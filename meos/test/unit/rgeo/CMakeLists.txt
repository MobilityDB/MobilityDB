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

# Unit tests of the temporal distance of rigid geometries
#
# These programs test internal functions of meos/src/rgeo.  The programs in
# meos/test are different: they test the installed public API, and CI builds
# each of them alone against the installed library.  These programs need the
# internal headers, so CMake builds them.  Their names end in _unit.c and not
# in _test.c, so that CI does not take them for tests of the public API.
#
# The files under test are compiled into each program with NDEBUG not
# defined, also in a release build, so that their assertions run.
#
# Run them with
#   ctest -R trgeo_dist
# The randomized programs take a number of trials.  The numbers below keep
# each program to about one second.  Run a program with a larger number for
# a deeper check.

# The root isolation builds with libm only, so that this remains possible
add_executable(trgeo_distsolve_unit trgeo_distsolve_unit.c
  ${CMAKE_SOURCE_DIR}/meos/src/rgeo/trgeo_distsolve.c)
target_compile_options(trgeo_distsolve_unit PRIVATE -UNDEBUG)
target_link_libraries(trgeo_distsolve_unit m)
add_test(NAME trgeo_distsolve_unit COMMAND trgeo_distsolve_unit 300)

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

#
# (c)2021 Esteban Zimanyi
#
# CMake module to find dbtoepub.
#
# Variables generated:
#
# DBTOEPUB_FOUND     true when DBTOEPUB_COMMAND has a valid entry
# DBTOEPUB_COMMAND   The command to run dbtoepub
# N.B. Currently dbtoepub DOES NOT provide version number
#

# Have a go at finding a dbtoepub executable (valid for POSIX systems, not for WIN systems)
find_program( DBTOEPUB_COMMAND dbtoepub )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(DBTOEPUB
  FOUND_VAR DBTOEPUB_FOUND
  REQUIRED_VARS DBTOEPUB_COMMAND )

if (DBTOEPUB_FOUND)
  mark_as_advanced(DBTOEPUB_COMMAND)
endif()
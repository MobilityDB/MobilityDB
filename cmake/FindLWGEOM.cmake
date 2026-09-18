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

# - Find lwgeom
# Find the PostgreSQL includes and client library
# This module defines
#  LWGEOM_INCLUDE_DIR
#  LWGEOM_LIBRARIES
#
# TODO lots of refinement to be able to work on windows & mac at least
# Copyright (c) 2021, Vicky Vergara <vicky@georepublic.org>

find_library(LWGEOM_LIBRARIES
  NAMES lwgeom
  PATHS
     /lib /lib64 /usr/lib /usr/lib64
  )

find_path(LWGEOM_INCLUDE_DIRS
  NAMES liblwgeom.h
  PATHS
    /usr/include
  )


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LWGEOM
  FOUND_VAR LWGEOM_FOUND
  REQUIRED_VARS LWGEOM_INCLUDE_DIRS LWGEOM_LIBRARIES
  FAIL_MESSAGE "Could NOT find lwgeom")

message(STATUS "LWGEOM_INCLUDE_DIRS=${LWGEOM_INCLUDE_DIRS}")
message(STATUS "LWGEOM_LIBRARIES=${LWGEOM_LIBRARIES}")

if (LWGEOM_FOUND)
  mark_as_advanced(LWGEOM_INCLUDE_DIRS PROJ_LIBRARIES)
endif()

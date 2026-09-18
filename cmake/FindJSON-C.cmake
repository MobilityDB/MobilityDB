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

# - Find json-c
# Find the PostgreSQL includes and client library
# This module defines
#  JSONC_INCLUDE_DIR
#  JSONC_LIBRARIES
#
# Copyright (c) 2021, Vicky Vergara <vicky@georepublic.org>

find_library(JSON-C_LIBRARIES
  NAMES json-c
  HINTS /lib /lib64 /usr/lib /usr/lib64
  )

find_path(JSON-C_INCLUDE_DIRS
  NAMES json.h
  HINTS /usr/include PATH_SUFFIXES json-c json
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSON-C
  FOUND_VAR JSON-C_FOUND
  REQUIRED_VARS JSON-C_INCLUDE_DIRS JSON-C_LIBRARIES
  FAIL_MESSAGE "Could NOT find json-c")

if(JSON-C_FOUND)
  mark_as_advanced(JSON-C_INCLUDE_DIRS JSON-C_LIBRARIES)
endif()

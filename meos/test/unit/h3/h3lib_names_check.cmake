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

# Fails when a global symbol of the object library h3core, the core library
# of H3 vendored under h3lib/ with h3_vertex.c, carries none of the prefixes
# `meos_` (the vendored names, see meos/src/h3/h3lib_names.h) and `h3_` (the
# functions of h3_vertex.c), since such a symbol has the name libh3 gives it
# and a static libmeos linked beside a static libh3 defines it twice. Fails
# too when a symbol carries `meos__`: a name H3 begins with an underscore
# takes the prefix `meos_h3_` in place of it, so no name carries two.
#
# Run as: cmake -DNM=<nm> -DOBJECTS="<o1>|<o2>|..." [-DLEADING_UNDERSCORE=ON]
#   -P h3lib_names_check.cmake

if(NOT NM OR NOT OBJECTS)
  message(FATAL_ERROR "NM and OBJECTS are required")
endif()
string(REPLACE "|" ";" _objects "${OBJECTS}")

set(_prefixed 0)
set(_own 0)
set(_bad "")
set(_double "")
foreach(_obj IN LISTS _objects)
  execute_process(COMMAND "${NM}" -g --defined-only "${_obj}"
    OUTPUT_VARIABLE _out RESULT_VARIABLE _rc ERROR_VARIABLE _err)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "${NM} failed on ${_obj}: ${_err}")
  endif()
  string(REPLACE "\n" ";" _lines "${_out}")
  foreach(_line IN LISTS _lines)
    # A defined global symbol reads: <address> <type> <name>
    if(NOT _line MATCHES "^[0-9A-Fa-f]+ [A-Za-z] ([^ ]+)$")
      continue()
    endif()
    set(_name "${CMAKE_MATCH_1}")
    if(LEADING_UNDERSCORE)
      string(REGEX REPLACE "^_" "" _name "${_name}")
    endif()
    if(_name MATCHES "^meos__")
      list(APPEND _double "${_name}")
    elseif(_name MATCHES "^meos_")
      math(EXPR _prefixed "${_prefixed} + 1")
    elseif(_name MATCHES "^h3_")
      math(EXPR _own "${_own} + 1")
    else()
      list(APPEND _bad "${_name}")
    endif()
  endforeach()
endforeach()

# The symbols must have been read at all: the vendored library defines its
# public and internal names and h3_vertex.c its own functions
if(_prefixed EQUAL 0 OR _own EQUAL 0)
  message(FATAL_ERROR "No symbol read from h3core (meos_: ${_prefixed}, "
    "h3_: ${_own}): the check reads nothing")
endif()
if(_double)
  list(REMOVE_DUPLICATES _double)
  list(LENGTH _double _ndouble)
  string(REPLACE ";" " " _double "${_double}")
  message(FATAL_ERROR "${_ndouble} global symbols of h3core carry meos__; "
    "map a name H3 begins with an underscore to meos_h3_ in "
    "meos/src/h3/h3lib_names.h: ${_double}")
endif()
if(_bad)
  list(REMOVE_DUPLICATES _bad)
  list(LENGTH _bad _nbad)
  string(REPLACE ";" " " _bad "${_bad}")
  message(FATAL_ERROR "${_nbad} global symbols of h3core keep a name of "
    "libh3; add them to meos/src/h3/h3lib_names.h: ${_bad}")
endif()
message(STATUS "h3core: ${_prefixed} symbols prefixed by meos_, ${_own} of "
  "h3_vertex.c")

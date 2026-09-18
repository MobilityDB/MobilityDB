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
# Based on:
# https://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen/
#

option(DOC_DEV
  "Set ON|OFF (default=OFF) to build the developer's documentation" OFF)

if(DOC_DEV)
  message(STATUS "Build the developer's documentation.")
  find_package(Doxygen ${DOXYGEN_MINIMUM_VERSION})

  if(DOXYGEN_FOUND)
    # Generate phantom C decls for every SQL function so Doxygen's
    # @sqlfn alias expansion autolinks into a symbol page listing the
    # SQL signatures and the C backing function. See
    # tools/codegen/sql_to_doxygen_stubs/sql_to_doxygen_stubs.py for the full rationale.
    set(SQL_STUB_DIR ${CMAKE_CURRENT_BINARY_DIR}/sql_stubs)
    set(SQL_STUB_FILE ${SQL_STUB_DIR}/sql_phantom_decls.c)
    file(GLOB_RECURSE SQL_SRC_FILES
      CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/mobilitydb/sql/*.in.sql")
    add_custom_command(
      OUTPUT ${SQL_STUB_FILE}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${SQL_STUB_DIR}
      COMMAND python3
        ${CMAKE_SOURCE_DIR}/tools/codegen/sql_to_doxygen_stubs/sql_to_doxygen_stubs.py
        --sql-dir ${CMAKE_SOURCE_DIR}/mobilitydb/sql
        --collision-dir
          ${CMAKE_SOURCE_DIR}/meos/include
          ${CMAKE_SOURCE_DIR}/meos/src
          ${CMAKE_SOURCE_DIR}/mobilitydb/pg_include
          ${CMAKE_SOURCE_DIR}/mobilitydb/src
        --out-dir ${SQL_STUB_DIR}
      DEPENDS
        ${CMAKE_SOURCE_DIR}/tools/codegen/sql_to_doxygen_stubs/sql_to_doxygen_stubs.py
        ${SQL_SRC_FILES}
      COMMENT "Generating SQL phantom declarations for Doxygen autolink"
      VERBATIM
    )
    add_custom_target(sql_stubs DEPENDS ${SQL_STUB_FILE})

    configure_file(${CMAKE_SOURCE_DIR}/doxygen/Doxyfile
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc_dev
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS sql_stubs
      COMMENT "Generating the developer's documentation with Doxygen" VERBATIM
    )
  endif()

endif()

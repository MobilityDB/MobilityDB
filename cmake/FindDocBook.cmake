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

# Copyright (c) 2021 Esteban Zimanyi
# Copyright (c) 2014 Thomas Heller
# Copyright (c) 2011 Bryce Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

find_path(DOCBOOK_DTD
  docbookx.dtd
  PATHS ${CMAKE_SYSTEM_PREFIX_PATH} ${DOCBOOK_ROOT} ENV DOCBOOK_ROOT
  PATH_SUFFIXES
    share/xml/docbook/schema/dtd/4.5
    docbook-dtd
    share/sgml/docbook/xml-dtd-4.5)

find_path(DOCBOOK_XSL
  NAMES html/html.xsl xhtml-1_1/html.xsl
  PATHS ${CMAKE_SYSTEM_PREFIX_PATH} ${DOCBOOK_ROOT} ENV DOCBOOK_ROOT
  PATH_SUFFIXES
      share/xml/docbook/stylesheet/docbook-xsl
      docbook-xsl
      share/sgml/docbook/xsl-stylesheets)

message(STATUS "DOCBOOK_DTD: ${DOCBOOK_DTD}")
message(STATUS "DOCBOOK_XSL: ${DOCBOOK_XSL}")

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(DOCBOOK
  FOUND_VAR DOCBOOK_FOUND
  REQUIRED_VARS DOCBOOK_DTD DOCBOOK_XSL )

if (POSTGRESQL_FOUND)
  mark_as_advanced(DOCBOOK_DTD DOCBOOK_XSL)
endif()

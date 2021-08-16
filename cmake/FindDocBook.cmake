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

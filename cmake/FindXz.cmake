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

# This module looks for xz. This module defines the following variables:
#
#  XZ_EXECUTABLE
#  XZCAT_EXECUTABLE
#  XZ_FOUND
#
# Copyright (c) 2021 Esteban Zimanyi
#
# Derived from
# https://github.com/WebKit/webkit/blob/main/Source/cmake/FindXz.cmake
#

include(FindCygwin)
# include(FindMsys.cmake) # Available in cmake version 3.21

find_program(XZ_EXECUTABLE
  xz
  ${CYGWIN_INSTALL_PATH}/bin
  # ${MSYS_INSTALL_PATH}/usr/bin # Available in cmake version 3.21
)
find_program(XZCAT_EXECUTABLE
  xzcat
  ${CYGWIN_INSTALL_PATH}/bin
  # ${MSYS_INSTALL_PATH}/usr/bin # Available in cmake version 3.21
)

# Handle the QUIETLY and REQUIRED arguments and set XZ_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xz DEFAULT_MSG XZ_EXECUTABLE XZCAT_EXECUTABLE)

mark_as_advanced(XZ_EXECUTABLE XZCAT_EXECUTABLE)

message(STATUS "XZ_EXECUTABLE: ${XZ_EXECUTABLE}")
message(STATUS "XZCAT_EXECUTABLE: ${XZCAT_EXECUTABLE}")

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

add_library(pdal_plugin_writer_tpcpatch SHARED
  TpcpatchWriter.cpp
)

set(POINTCLOUD_LIB_DIR ${CMAKE_SOURCE_DIR}/../../pointcloud-pg/lib)

target_include_directories(pdal_plugin_writer_tpcpatch
  PRIVATE
    ${PostgreSQL_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
    ${POINTCLOUD_LIB_DIR}
)

target_link_libraries(pdal_plugin_writer_tpcpatch
  PRIVATE
    pdalcpp
    PostgreSQL::PostgreSQL
    ${LIBXML2_LIBRARIES}
    ${POINTCLOUD_LIB_DIR}/libpc.a
    ${POINTCLOUD_LIB_DIR}/liblazperf.a
    z
    stdc++
)

set_target_properties(pdal_plugin_writer_tpcpatch PROPERTIES
  PREFIX ""
  OUTPUT_NAME "libpdal_plugin_writer_tpcpatch"
)

install(TARGETS pdal_plugin_writer_tpcpatch
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

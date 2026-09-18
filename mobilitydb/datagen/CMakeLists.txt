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

include(AssembleExtensionSql)

# Configure the control file
configure_file(mobilitydb_datagen.in.control ${CMAKE_BINARY_DIR}/mobilitydb_datagen.control)

# Define list of source files. Only the random_* generators belong to the
# extension; the create_test_tables_* files are per-domain test scaffolding.
set(DATAGEN_FILES
  temporal/random_temporal.sql
  geo/random_tpoint.sql
  geo/random_tgeo.sql
  )
if(CBUFFER)
  list(APPEND DATAGEN_FILES cbuffer/random_tcbuffer.sql)
endif()
if(H3)
  list(APPEND DATAGEN_FILES h3/random_th3index.sql)
endif()
# if(JSON)
  # list(APPEND DATAGEN_FILES json/random_tjson.sql)
# endif()
if(NPOINT)
  list(APPEND DATAGEN_FILES npoint/random_tnpoint.sql)
endif()
if(POINTCLOUD)
  list(APPEND DATAGEN_FILES pointcloud/random_tpcpoint.sql)
endif()
if(POSE)
  list(APPEND DATAGEN_FILES pose/random_tpose.sql)
  list(APPEND DATAGEN_FILES posechain/random_posechain.sql)
endif()
if(QUADBIN)
  list(APPEND DATAGEN_FILES quadbin/random_tquadbin.sql)
endif()
if(RASTER)
  list(APPEND DATAGEN_FILES raster/random_raquet.sql)
endif()
if(RGEO)
  list(APPEND DATAGEN_FILES rgeo/random_trgeo.sql)
endif()

list(TRANSFORM DATAGEN_FILES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

configure_file(header.in.sql ${CMAKE_CURRENT_BINARY_DIR}/header.sql @ONLY)

assemble_extension_sql(
  OUTPUT ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE}
  HEADER ${CMAKE_CURRENT_BINARY_DIR}/header.sql
  BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES ${DATAGEN_FILES})

add_sql_configure_depends(${DATAGEN_FILES})

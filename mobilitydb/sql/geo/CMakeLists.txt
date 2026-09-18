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

SET(LOCAL_FILES
  049_geo_funcs
  050_geoset
  051_stbox
  052_tpoint
  053_tpoint_inout
  054_tpoint_compops
  056_tpoint_spatialfuncs
  058_tpoint_tile
  060_tpoint_boxops
  061_tpoint_topops
  062_tpoint_posops
  064_tpoint_distance
  066_tpoint_similarity
  068_tpoint_aggfuncs
  070_tpoint_spatialrels
  071_tpoint_setset
  072_tpoint_tempspatialrels
  073_tpoint_gist
  074_tpoint_spgist
  076_tpoint_analytics
  078_tpoint_datagen
  )

# Files added for temporal geometries
# TODO These files should be added to the list above
list(APPEND LOCAL_FILES 
  052_tgeo
  053_tgeo_inout
  054_tgeo_compops
  056_tgeo_spatialfuncs
  058_tgeo_tile
  060_tgeo_boxops
  061_tgeo_topops
  062_tgeo_posops
  064_tgeo_distance
  068_tgeo_aggfuncs
  070_tgeo_spatialrels
  071_tgeo_setset
  072_tgeo_tempspatialrels
  073_tgeo_gist
  074_tgeo_spgist
  076_tgeo_analytics
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/mobilitydb/sql)
  list(APPEND PACKAGE_SQL_FILES ${CMAKE_BINARY_DIR}/mobilitydb/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)

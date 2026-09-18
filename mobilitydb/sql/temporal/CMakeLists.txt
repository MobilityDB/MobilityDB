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
  001_set
  002_set_ops
  003_span
  005_span_ops
  007_spanset
  009_spanset_ops
  011_span_indexes
  012_spanset_indexes
  013_set_indexes
  015_span_aggfuncs
  019_geo_constructors
  021_tbox
  022_temporal
  023_temporal_inout
  025_temporal_tile
  026_tnumber_mathfuncs
  028_tbool_boolops
  029_ttext_textfuncs
  030_temporal_compops
  032_temporal_boxops
  033_temporal_topops
  034_temporal_posops
  036_tnumber_distance
  038_temporal_similarity
  040_temporal_aggfuncs
  042_temporal_waggfuncs
  043_temporal_gist
  044_temporal_spgist
  046_temporal_analytics
  999_oid_cache
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/mobilitydb/sql)
  list(APPEND PACKAGE_SQL_FILES ${CMAKE_BINARY_DIR}/mobilitydb/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)

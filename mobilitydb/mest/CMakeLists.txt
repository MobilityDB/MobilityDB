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

# Configure the control file, its functions being those of the library of the
# extension, as mobilitydb/sql/CMakeLists.txt names it
set(MOBILITYDB_MODULE_PATHNAME "$libdir/lib${MOBILITYDB_LIB_NAME}")
configure_file(mobilitydb_mest.in.control ${CMAKE_BINARY_DIR}/mobilitydb_mest.control)

# The multi-entry operator classes the extension mobilitydb_mest holds, written
# by tools/codegen/inherited/generate.py: the functions every class takes, then
# a file per GiST and SP-GiST file of the extension holding the classes of its
# types
set(MEST_FILES
  temporal/000_mest.in.sql
  temporal/012_spanset_mgist.in.sql
  temporal/012_spanset_mspgist.in.sql
  temporal/013_set_mgist.in.sql
  temporal/013_set_mspgist.in.sql
  temporal/043_temporal_mgist.in.sql
  temporal/044_temporal_mspgist.in.sql
  geo/073_tpoint_mgist.in.sql
  geo/073_tgeo_mgist.in.sql
  geo/074_tpoint_mspgist.in.sql
  geo/074_tgeo_mspgist.in.sql
  )
# The classes of the families the extension builds, as mobilitydb/sql/CMakeLists.txt
# adds their directories
if(CBUFFER)
  list(APPEND MEST_FILES
    cbuffer/222_tcbuffer_mgist.in.sql
    cbuffer/223_tcbuffer_mspgist.in.sql)
endif()
if(H3)
  list(APPEND MEST_FILES
    h3/272_th3index_mgist.in.sql
    h3/273_th3index_mspgist.in.sql)
endif()
if(JSON)
  list(APPEND MEST_FILES
    json/466_tjsonb_mgist.in.sql
    json/467_tjsonb_mspgist.in.sql)
endif()
if(NPOINT)
  list(APPEND MEST_FILES
    npoint/322_tnpoint_mgist.in.sql
    npoint/323_tnpoint_mspgist.in.sql)
endif()
if(QUADBIN)
  list(APPEND MEST_FILES
    quadbin/372_tquadbin_mgist.in.sql
    quadbin/373_tquadbin_mspgist.in.sql)
endif()
if(S2CELL)
  list(APPEND MEST_FILES
    s2cell/622_ts2cell_mgist.in.sql
    s2cell/623_ts2cell_mspgist.in.sql)
endif()
if(POINTCLOUD)
  list(APPEND MEST_FILES
    pointcloud/439_tpc_mgist.in.sql
    pointcloud/440_tpc_mspgist.in.sql)
endif()
if(POSE)
  list(APPEND MEST_FILES
    pose/122_tpose_mgist.in.sql
    pose/123_tpose_mspgist.in.sql
    posechain/572_tposechain_mgist.in.sql
    posechain/573_tposechain_mspgist.in.sql)
endif()
if(RGEO)
  list(APPEND MEST_FILES
    rgeo/173_trgeo_mgist.in.sql
    rgeo/174_trgeo_mspgist.in.sql)
endif()

list(TRANSFORM MEST_FILES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

configure_file(header.in.sql ${CMAKE_CURRENT_BINARY_DIR}/header.sql @ONLY)

assemble_extension_sql(
  OUTPUT ${CMAKE_BINARY_DIR}/${MOBILITYDB_MEST_EXTENSION_FILE}
  HEADER ${CMAKE_CURRENT_BINARY_DIR}/header.sql
  BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES ${MEST_FILES})

add_sql_configure_depends(${MEST_FILES})

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

# geo_lwgeom_none.c answers the entry points of the geometry library's GEOS
# files that the rest of that library calls, the build leaving those files out
set(GEO_SOURCES
  geo_arrange.c
  geo_buffer.c
  geo_cluster.c
  geo_funcs.c
  geo_lwgeom_none.c
  geo_round.c
  postgis_funcs.c
  stbox.c
  stbox_index.c
  tgeo.c
  tgeo_aggfuncs.c
  tgeo_boxops.c
  tgeo_compops.c
  tgeo_distance.c
  tgeo_restrict.c
  tgeo_spatialfuncs.c
  tgeo_spatialrels.c
  tgeo_tile.c
  tpoint_datagen.c
  tpoint_geom_clip.c
  tpoint_spatialfuncs.c
  tspatial.c
  tspatial_parser.c
  tspatial_posops.c
  tspatial_spgist.c
  tspatial_tempspatialrels.c
  tspatial_srid.c
)

if(MEOS)
  list(APPEND GEO_SOURCES
  geoset_meos.c
  tgeo_meos.c
  tspatial_transform_meos.c
  # tspatial_rtree.c
  tspatial_topops_meos.c
)
endif()

add_library(geo OBJECT ${GEO_SOURCES})

# The GEOS entry points the vendored raster core calls directly are answered by
# geo_geos_none.c, an object library of its own so that its symbols are hidden
# from the dynamic table of the shared library, as pointcloud_libpc_stringbuffer
# is. They carry the names of the GEOS C API (GEOSUnaryUnion, GEOSisValid,
# initGEOS, ...), which PostGIS imports in the same backend, so an exported
# definition would be open to the ELF interposition that
# postgis/liblwgeom/CMakeLists.txt describes: PostGIS loaded after MobilityDB
# would bind its GEOS calls to these definitions.
if(RASTER)
  add_library(geo_geos_none OBJECT geo_geos_none.c)
  set_target_properties(geo_geos_none PROPERTIES
    C_VISIBILITY_PRESET hidden POSITION_INDEPENDENT_CODE ON)
endif()

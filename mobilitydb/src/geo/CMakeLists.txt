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

add_library(pg_geo OBJECT
  geo_funcs.c
  projection_gk.c
  spatialset.c
  stbox.c
  tgeo.c
  tgeo_aggfuncs.c
  tgeo_boxops.c
  tgeo_compops.c
  tgeo_distance.c
  tgeo_spatialfuncs.c
  tgeo_spatialrels.c
  tgeo_tile.c
  tspatial.c
  tpoint_datagen.c
  tspatial_analyze.c
  tspatial_gist.c
  tspatial_posops.c
  tspatial_selfuncs.c
  tspatial_spgist.c
  tspatial_tempspatialrels.c
  )

# pg_geo's tgeo.c calls pc_set_handlers() under `#if POINTCLOUD`.
# Give the target visibility into the pgpointcloud subtree headers.
if(POINTCLOUD)
  target_include_directories(pg_geo PRIVATE
    ${CMAKE_SOURCE_DIR}/pointcloud-pg/lib)
  # tgeo.c includes pc_api.h, which includes the generated pc_config.h. That
  # header is produced when the pointcloud module builds the vendored subtree
  # (autotools). Depend on the pointcloud target so pc_config.h exists before
  # tgeo.c compiles, instead of racing it under a parallel build.
  add_dependencies(pg_geo pointcloud)
endif()

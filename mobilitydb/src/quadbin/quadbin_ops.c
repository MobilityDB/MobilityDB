/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief PG V1 wrappers for the static `quadbin` cell operations.
 *
 * Each wrapper unpacks its arguments, delegates to the first-party
 * quadbin kernel declared in `meos_quadbin.h`, and returns the result.
 * The square subset shared with every DGGS family (resolution,
 * hierarchy, k-ring, point/boundary/bbox, area) plus the
 * quadbin-unique tile / quadkey conversions are wrapped here; the
 * hexagon-only H3 families (directed edges, vertices, pentagon /
 * base-cell / class-III inspection, local-IJ traversal, great-circle
 * metrics) have no quadbin analogue and are not present.
 *
 * The cell centroid and boundary geometries are emitted as planar
 * lon/lat (SRID 4326), matching the `quadbin_cellops` descriptor in
 * `meos/src/quadbin/tquadbin_ops.c`.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/array.h>
#include <catalog/pg_type_d.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_quadbin.h>
#include <pgtypes.h>
#include "geo/tgeo_spatialfuncs.h"
#include "temporal/set.h"
#include "quadbin/quadbin_meos.h"
/* MobilityDB */
#include "pg_geo/postgis.h"

#define PG_GETARG_QUADBIN(n) DatumGetQuadbin(PG_GETARG_DATUM(n))
#define PG_RETURN_QUADBIN(x) PG_RETURN_DATUM(QuadbinGetDatum(x))

/*****************************************************************************
 * Resolution
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_get_resolution(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_get_resolution);
/**
 * @ingroup mobilitydb_quadbin_accessor
 * @brief Return the resolution (zoom level) of a quadbin cell
 * @sqlfn quadbinGetResolution()
 */
Datum
Quadbin_get_resolution(PG_FUNCTION_ARGS)
{
  PG_RETURN_INT32((int32) quadbin_get_resolution(PG_GETARG_QUADBIN(0)));
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_cell_to_parent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_to_parent);
/**
 * @ingroup mobilitydb_quadbin_hierarchy
 * @brief Return the parent cell at the given coarser resolution
 * @sqlfn quadbinCellToParent()
 */
Datum
Quadbin_cell_to_parent(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  int32 resolution = PG_GETARG_INT32(1);
  PG_RETURN_QUADBIN(quadbin_cell_to_parent(cell, (uint32_t) resolution));
}

PGDLLEXPORT Datum Quadbin_cell_to_children(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_to_children);
/**
 * @ingroup mobilitydb_quadbin_hierarchy
 * @brief Return the children of a cell at the given finer resolution
 * as a quadbinset
 * @sqlfn quadbinCellToChildren()
 */
Datum
Quadbin_cell_to_children(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  int32 resolution = PG_GETARG_INT32(1);
  Set *result = quadbin_cell_to_children_set(cell, (uint32_t) resolution);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Quadbin_cell_sibling(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_sibling);
/**
 * @ingroup mobilitydb_quadbin_hierarchy
 * @brief Return the neighbouring cell at the same resolution in the
 * given direction (`up` / `down` / `left` / `right`)
 * @sqlfn quadbinCellSibling()
 */
Datum
Quadbin_cell_sibling(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  text *dir = PG_GETARG_TEXT_P(1);
  char *direction = text_to_cstring(dir);
  Quadbin result = quadbin_cell_sibling(cell, direction);
  pfree(direction);
  PG_FREE_IF_COPY(dir, 1);
  PG_RETURN_QUADBIN(result);
}

/*****************************************************************************
 * Grid traversal — k-ring
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_grid_disk(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_grid_disk);
/**
 * @ingroup mobilitydb_quadbin_set
 * @brief Return all cells within grid distance k of the origin as a
 * quadbinset
 * @sqlfn quadbinGridDisk()
 */
Datum
Quadbin_grid_disk(PG_FUNCTION_ARGS)
{
  Quadbin origin = PG_GETARG_QUADBIN(0);
  int k = PG_GETARG_INT32(1);
  Set *result = quadbin_grid_disk(origin, k);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Point <-> cell
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_point_to_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_point_to_cell);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the quadbin cell covering a lon/lat point (SRID 4326)
 * at the given resolution
 * @sqlfn geoToQuadbinCell()
 */
Datum
Quadbin_point_to_cell(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Quadbin result = geo_to_quadbin_cell(gs, resolution);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_QUADBIN(result);
}

PGDLLEXPORT Datum Quadbin_cell_to_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_to_point);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the cell centroid as a lon/lat point (SRID 4326)
 * @sqlfn quadbinCellToPoint()
 */
Datum
Quadbin_cell_to_point(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  PG_RETURN_GSERIALIZED_P(quadbin_cell_to_geompoint(cell));
}

/*****************************************************************************
 * Boundary / bounding box
 *
 * A quadbin cell is an axis-aligned square tile, so its boundary polygon
 * and its envelope geometry coincide; the geometry is constructed by the
 * MEOS export quadbin_cell_to_geom.
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_cell_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_to_boundary);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the cell boundary as a square polygon (SRID 4326)
 * @sqlfn quadbinCellToBoundary()
 */
Datum
Quadbin_cell_to_boundary(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  PG_RETURN_GSERIALIZED_P(quadbin_cell_to_geom(cell));
}

PGDLLEXPORT Datum Quadbin_cell_to_bounding_box(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_to_bounding_box);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the cell axis-aligned envelope as a polygon (SRID 4326)
 * @sqlfn quadbinCellToBoundingBox()
 */
Datum
Quadbin_cell_to_bounding_box(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  PG_RETURN_GSERIALIZED_P(quadbin_cell_to_geom(cell));
}

/*****************************************************************************
 * Area
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_cell_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_area);
/**
 * @ingroup mobilitydb_quadbin_accessor
 * @brief Return the cell area in square metres
 * @sqlfn quadbinCellArea()
 */
Datum
Quadbin_cell_area(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(quadbin_cell_area(PG_GETARG_QUADBIN(0)));
}

/*****************************************************************************
 * Tile / quadkey conversion (quadbin-unique, no H3 analogue)
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_tile_to_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_tile_to_cell);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Build a quadbin cell from slippy-map (x, y, z) tile coordinates
 * @sqlfn quadbinTileToCell()
 */
Datum
Quadbin_tile_to_cell(PG_FUNCTION_ARGS)
{
  int32 x = PG_GETARG_INT32(0);
  int32 y = PG_GETARG_INT32(1);
  int32 z = PG_GETARG_INT32(2);
  PG_RETURN_QUADBIN(quadbin_tile_to_cell((uint32_t) x, (uint32_t) y,
    (uint32_t) z));
}

PGDLLEXPORT Datum Quadbin_cell_to_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_to_tile);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Decompose a quadbin cell into its slippy-map tile coordinates,
 * returned as the integer array `{x, y, z}`
 * @sqlfn quadbinCellToTile()
 */
Datum
Quadbin_cell_to_tile(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  uint32_t x, y, z;
  quadbin_cell_to_tile(cell, &x, &y, &z);
  Datum elems[3];
  elems[0] = Int32GetDatum((int32) x);
  elems[1] = Int32GetDatum((int32) y);
  elems[2] = Int32GetDatum((int32) z);
  ArrayType *result = construct_array(elems, 3, INT4OID, sizeof(int32), true,
    TYPALIGN_INT);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Quadbin_cell_to_quadkey(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cell_to_quadkey);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the base-4 slippy-tile quadkey string of a quadbin cell
 * (the z0 world cell maps to the empty string)
 * @sqlfn quadbinCellToQuadkey()
 */
Datum
Quadbin_cell_to_quadkey(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  char *str = quadbin_cell_to_quadkey(cell);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

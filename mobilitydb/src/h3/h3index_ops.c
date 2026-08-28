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
 * @brief PG V1 wrappers for the static `h3index` cell operations.
 *
 * Each wrapper unpacks its arguments, delegates to the first-party H3
 * kernel declared in `meos_h3.h`, and returns the result. The file holds
 * the faces the `DggsCellOps` descriptor shares across every DGGS family
 * — resolution, hierarchy, cell centroid and boundary, area — spelled
 * with the bare slot names `quadbin` and `s2cell` already publish, so a
 * query reads the same whichever grid holds the cell.
 *
 * `isValidCell` is the sixth face and lives in `h3index.c` beside the
 * type plumbing, as `Quadbin_is_valid_cell` and `S2cell_is_valid_cell`
 * live beside theirs. The hexagon-only surface (directed edges, vertices,
 * pentagon / base-cell / class-III inspection, local-IJ traversal,
 * great-circle metrics) has no counterpart in a square or a spherical
 * quadrilateral grid and is wrapped in the `th3index_*` files.
 *
 * The cell centroid and boundary geometries are emitted as lon/lat
 * (SRID 4326), matching the `h3_cellops` descriptor in
 * `meos/src/h3/th3index_ops.c`.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include "h3/h3index.h"
/* MobilityDB */
#include "pg_geo/postgis.h"

/* DatumGetH3Index / H3IndexGetDatum live in h3index.h.
 * PG_GETARG_H3INDEX / PG_RETURN_H3INDEX are the fmgr-layer
 * conveniences defined locally here because fmgr.h is a
 * MobilityDB-side dependency. */
#define PG_GETARG_H3INDEX(n) DatumGetH3Index(PG_GETARG_DATUM(n))
#define PG_RETURN_H3INDEX(x) PG_RETURN_DATUM(H3IndexGetDatum(x))

/*****************************************************************************
 * Resolution
 *****************************************************************************/

PGDLLEXPORT Datum H3index_get_resolution(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_get_resolution);
/**
 * @ingroup mobilitydb_h3_base_inspection
 * @brief Return the resolution of an H3 cell
 * @sqlfn getResolution()
 */
Datum
H3index_get_resolution(PG_FUNCTION_ARGS)
{
  PG_RETURN_INT32((int32) h3index_get_resolution(PG_GETARG_H3INDEX(0)));
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

PGDLLEXPORT Datum H3index_cell_to_parent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_cell_to_parent);
/**
 * @ingroup mobilitydb_h3_base_hierarchy
 * @brief Return the parent cell at the given coarser resolution
 * @sqlfn cellToParent()
 */
Datum
H3index_cell_to_parent(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  int32 resolution = PG_GETARG_INT32(1);
  PG_RETURN_H3INDEX(h3index_cell_to_parent(cell, (uint32_t) resolution));
}

/*****************************************************************************
 * Lat/Lng
 *****************************************************************************/

PGDLLEXPORT Datum H3index_cell_to_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_cell_to_point);
/**
 * @ingroup mobilitydb_h3_base_latlng
 * @brief Return the centroid of an H3 cell
 * @sqlfn cellToPoint()
 */
Datum
H3index_cell_to_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *result = h3index_cell_to_point(PG_GETARG_H3INDEX(0));
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum H3index_cell_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_cell_to_boundary);
/**
 * @ingroup mobilitydb_h3_base_latlng
 * @brief Return the boundary of an H3 cell
 * @sqlfn cellToBoundary()
 */
Datum
H3index_cell_to_boundary(PG_FUNCTION_ARGS)
{
  GSERIALIZED *result = h3index_cell_to_boundary(PG_GETARG_H3INDEX(0));
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Metrics
 *****************************************************************************/

PGDLLEXPORT Datum H3index_cell_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_cell_area);
/**
 * @ingroup mobilitydb_h3_base_metrics
 * @brief Return the area in square metres of an H3 cell
 * @sqlfn cellArea()
 */
Datum
H3index_cell_area(PG_FUNCTION_ARGS)
{
  PG_RETURN_FLOAT8(h3index_cell_area(PG_GETARG_H3INDEX(0)));
}

/*****************************************************************************/

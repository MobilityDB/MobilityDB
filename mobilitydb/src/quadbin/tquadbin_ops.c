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
 * @brief PG V1 wrappers for the lifted temporal `tquadbin` cell operations.
 *
 * The shared DGGS operations (resolution, validity, parent, centroid
 * point, boundary, area) route through the generic `tcellindex_*`
 * entry points, which dispatch to the `quadbin_cellops` descriptor.
 * The quadbin-unique quadkey lift calls the typed
 * `tquadbin_cell_to_quadkey` directly because the quadkey string has
 * no DGGS analogue and is not part of the descriptor.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_quadbin.h>
#include "temporal/temporal.h"
#include "temporal/tcellindex.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Resolution + validity
 *****************************************************************************/

PGDLLEXPORT Datum Tquadbin_get_resolution(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tquadbin_get_resolution);
/**
 * @ingroup mobilitydb_quadbin_accessor
 * @brief Return the temporal resolution (zoom level) of a temporal
 * quadbin cell
 * @sqlfn tquadbinGetResolution()
 */
Datum
Tquadbin_get_resolution(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_get_resolution(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tquadbin_is_valid_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tquadbin_is_valid_cell);
/**
 * @ingroup mobilitydb_quadbin_accessor
 * @brief Return the temporal validity of a temporal quadbin cell
 * @sqlfn isValidCell()
 */
Datum
Tquadbin_is_valid_cell(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_is_valid_cell(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

PGDLLEXPORT Datum Tquadbin_cell_to_parent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tquadbin_cell_to_parent);
/**
 * @ingroup mobilitydb_quadbin_hierarchy
 * @brief Return the temporal parent cell at the given coarser resolution
 * @sqlfn tquadbinCellToParent()
 */
Datum
Tquadbin_cell_to_parent(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = tcellindex_cell_to_parent(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Centroid point / boundary (planar lon/lat, SRID 4326)
 *****************************************************************************/

PGDLLEXPORT Datum Tquadbin_cell_to_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tquadbin_cell_to_point);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the per-instant cell centroid as a temporal point
 * @sqlfn tquadbinCellToPoint()
 */
Datum
Tquadbin_cell_to_point(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_cell_to_point(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tquadbin_cell_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tquadbin_cell_to_boundary);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the per-instant square cell boundary as a temporal
 * geometry
 * @sqlfn tquadbinCellToBoundary()
 */
Datum
Tquadbin_cell_to_boundary(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_cell_to_boundary(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Area
 *****************************************************************************/

PGDLLEXPORT Datum Tquadbin_cell_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tquadbin_cell_area);
/**
 * @ingroup mobilitydb_quadbin_accessor
 * @brief Return the per-instant cell area (square metres) as a temporal
 * float
 * @sqlfn tquadbinCellArea()
 */
Datum
Tquadbin_cell_area(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_cell_area(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Quadkey conversion (quadbin-unique, no H3 analogue)
 *****************************************************************************/

PGDLLEXPORT Datum Tquadbin_cell_to_quadkey(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tquadbin_cell_to_quadkey);
/**
 * @ingroup mobilitydb_quadbin_conversion
 * @brief Return the per-instant base-4 slippy-tile quadkey string as a
 * temporal text
 * @sqlfn tquadbinCellToQuadkey()
 */
Datum
Tquadbin_cell_to_quadkey(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tquadbin_cell_to_quadkey(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

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
 * @brief PG V1 wrappers for the temporal `ts2cell` cell operations.
 *
 * Each wrapper delegates to the generic cell-index entry point in
 * `meos_cellindex.h`, which reads the `s2_cellops` descriptor
 * (`meos/src/s2cell/ts2cell_ops.c`) to reach the S2 kernel. The six
 * operations every DGGS shares are wrapped here under the names the
 * descriptor fixes; the token is S2's own and has no H3 or QUADBIN
 * analogue.
 *
 * A cell is a region of the WGS84 sphere, so the centre is a
 * `tgeogpoint` and the boundary a temporal geography, both in SRID
 * 4326, where the Web-Mercator `tquadbin` answers planar ones. The
 * family declares no SRID surface: its reference system is fixed by
 * the S2 specification rather than carried by a value.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_cellindex.h>
#include <meos_s2cell.h>
#include "temporal/temporal.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"

PGDLLEXPORT Datum Ts2cell_get_resolution(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ts2cell_get_resolution);
/**
 * @ingroup mobilitydb_s2cell_accessor
 * @brief Return the resolution (level) of each cell of a temporal S2 value
 * @sqlfn getResolution()
 */
Datum
Ts2cell_get_resolution(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_get_resolution(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ts2cell_is_valid_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ts2cell_is_valid_cell);
/**
 * @ingroup mobilitydb_s2cell_accessor
 * @brief Return true for each cell of a temporal S2 value that encodes a
 * valid cell
 * @sqlfn isValidCell()
 */
Datum
Ts2cell_is_valid_cell(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_is_valid_cell(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ts2cell_cell_to_parent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ts2cell_cell_to_parent);
/**
 * @ingroup mobilitydb_s2cell_hierarchy
 * @brief Return the ancestor at the given coarser level of each cell of a
 * temporal S2 value
 * @sqlfn cellToParent()
 */
Datum
Ts2cell_cell_to_parent(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 resolution = PG_GETARG_INT32(1);
  Temporal *result = tcellindex_cell_to_parent(temp, resolution);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ts2cell_cell_to_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ts2cell_cell_to_point);
/**
 * @ingroup mobilitydb_s2cell_conversion
 * @brief Return the centre of each cell of a temporal S2 value as a
 * temporal geodetic point
 * @sqlfn cellToPoint()
 */
Datum
Ts2cell_cell_to_point(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_cell_to_point(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ts2cell_cell_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ts2cell_cell_to_boundary);
/**
 * @ingroup mobilitydb_s2cell_conversion
 * @brief Return the boundary of each cell of a temporal S2 value as a
 * temporal geography
 * @sqlfn cellToBoundary()
 */
Datum
Ts2cell_cell_to_boundary(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_cell_to_boundary(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ts2cell_cell_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ts2cell_cell_area);
/**
 * @ingroup mobilitydb_s2cell_accessor
 * @brief Return the area in square metres of each cell of a temporal S2
 * value
 * @sqlfn cellArea()
 */
Datum
Ts2cell_cell_area(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcellindex_cell_area(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ts2cell_cell_to_token(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ts2cell_cell_to_token);
/**
 * @ingroup mobilitydb_s2cell_conversion
 * @brief Return the token of each cell of a temporal S2 value as a
 * temporal text
 * @sqlfn ts2CellToToken()
 */
Datum
Ts2cell_cell_to_token(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = ts2cell_cell_to_token(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

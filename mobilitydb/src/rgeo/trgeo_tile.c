/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Spatial and spatiotemporal grid tiling for temporal rigid geometries
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include "geo/stbox.h"
/* MobilityDB */
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

/**
 * @brief Compute the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatial or spatiotemporal grid
 */
static Datum
Trgeo_space_time_boxes_common(FunctionCallInfo fcinfo, bool spacetiles,
  bool timetiles)
{
  /* Get input parameters */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int i = 1;
  double xsize = 0, ysize = 0, zsize = 0;
  if (spacetiles)
  {
    xsize = PG_GETARG_FLOAT8(i++);
    ysize = PG_GETARG_FLOAT8(i++);
    zsize = PG_GETARG_FLOAT8(i++);
  }
  Interval *duration = timetiles ? PG_GETARG_INTERVAL_P(i++) : NULL;
  GSERIALIZED *sorigin = spacetiles ? PG_GETARG_GSERIALIZED_P(i++) : NULL;
  TimestampTz torigin = timetiles ? PG_GETARG_TIMESTAMPTZ(i++) : 0;
  bool bitmatrix = PG_GETARG_BOOL(i++);
  bool border_inc = PG_GETARG_BOOL(i++);

  /* Get the boxes */
  if (temporal_num_instants(temp) == 1)
    bitmatrix = false;
  int count;
  STBox *boxes = trgeometry_space_time_boxes(temp, xsize, ysize, zsize, duration,
    sorigin, torigin, bitmatrix, border_inc, &count);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Trgeometry_space_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_space_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatial grid
 * @sqlfn spaceBoxes()
 */
inline Datum
Trgeometry_space_boxes(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_boxes_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Trgeo_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_time_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to time bins
 * @sqlfn timeBoxes()
 */
inline Datum
Trgeo_time_boxes(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_boxes_common(fcinfo, false, true);
}

PGDLLEXPORT Datum Trgeometry_space_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_space_time_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatiotemporal grid
 * @sqlfn spaceTimeBoxes()
 */
inline Datum
Trgeometry_space_time_boxes(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_boxes_common(fcinfo, true, true);
}

/*****************************************************************************/

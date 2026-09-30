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
 * @brief Return the boxes a grid function answered as an array
 */
static Datum
Trgeo_boxes_array(STBox *boxes, int count)
{
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Trgeometry_space_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_space_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatial grid
 * @details The forms stating one or two spatial sizes leave out the ones that
 * follow, which MEOS reads as `xsize`, as #Tgeo_space_boxes reads them
 * @sqlfn spaceBoxes()
 */
Datum
Trgeometry_space_boxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double xsize = PG_GETARG_FLOAT8(1);
  double ysize = 0;
  double zsize = 0;
  int i = 2;
  if (PG_NARGS() > 5)
    ysize = PG_GETARG_FLOAT8(i++);
  if (PG_NARGS() > 6)
    zsize = PG_GETARG_FLOAT8(i++);
  GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
  bool bitmatrix = PG_GETARG_BOOL(i++);
  bool border_inc = PG_GETARG_BOOL(i++);
  int count;
  STBox *boxes = trgeometry_space_boxes(temp, xsize, ysize, zsize, sorigin,
    bitmatrix, border_inc, &count);
  PG_FREE_IF_COPY(temp, 0);
  return Trgeo_boxes_array(boxes, count);
}

PGDLLEXPORT Datum Trgeo_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_time_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to time bins
 * @sqlfn timeBoxes()
 */
Datum
Trgeo_time_boxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  bool bitmatrix = PG_GETARG_BOOL(3);
  bool border_inc = PG_GETARG_BOOL(4);
  int count;
  STBox *boxes = trgeometry_space_time_boxes(temp, 0.0, 0.0, 0.0, duration,
    NULL, torigin, bitmatrix, border_inc, &count);
  PG_FREE_IF_COPY(temp, 0);
  return Trgeo_boxes_array(boxes, count);
}

PGDLLEXPORT Datum Trgeometry_space_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_space_time_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatiotemporal grid
 * @details The forms stating one or two spatial sizes leave out the ones that
 * follow, which MEOS reads as `xsize`, as #Tgeo_space_time_boxes reads them
 * @sqlfn spaceTimeBoxes()
 */
Datum
Trgeometry_space_time_boxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double xsize = PG_GETARG_FLOAT8(1);
  double ysize = 0;
  double zsize = 0;
  int i = 2;
  if (PG_NARGS() > 7)
    ysize = PG_GETARG_FLOAT8(i++);
  if (PG_NARGS() > 8)
    zsize = PG_GETARG_FLOAT8(i++);
  Interval *duration = PG_GETARG_INTERVAL_P(i++);
  GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(i++);
  bool bitmatrix = PG_GETARG_BOOL(i++);
  bool border_inc = PG_GETARG_BOOL(i++);
  int count;
  STBox *boxes = trgeometry_space_time_boxes(temp, xsize, ysize, zsize,
    duration, sorigin, torigin, bitmatrix, border_inc, &count);
  PG_FREE_IF_COPY(temp, 0);
  return Trgeo_boxes_array(boxes, count);
}

/*****************************************************************************/

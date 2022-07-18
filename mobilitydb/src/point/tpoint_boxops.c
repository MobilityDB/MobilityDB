/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Bounding box operators for temporal points.
 *
 * These operators test the bounding boxes of temporal points, which are an
 * `STBOX`, where the *x*, *y*, and optional *z* coordinates are for the space
 * (value) dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: `overlaps`, `contains`, `contained`,
 * `same`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "point/tpoint_boxops.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "general/temporaltypes.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Boxes function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_stboxes);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return an array of spatiotemporal boxes from a temporal point
 * @sqlfunc stboxes()
 */
PGDLLEXPORT Datum
Tpoint_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  STBOX *boxes = tpoint_stboxes(temp, &count);
  PG_FREE_IF_COPY(temp, 0);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_geo_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = boxop_tpoint_geo(temp, gs, func, true);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Generic bounding box function for a temporal point and a geometry.
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_geo_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = boxop_tpoint_geo(temp, gs, func, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Generic bounding box function for a spatiotemporal box and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_stbox_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tpoint_stbox(temp, box, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for a temporal point and a spatiotemporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_stbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  bool result = boxop_tpoint_stbox(temp, box, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic topological function for two temporal points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_tpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tpoint_tpoint(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a geometry/geography and
 * a temporal point overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a temporal point and a
 * geometry/geography overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_bbox_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a geometry/geography contains
 * the one of a temporal point
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contains_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box contains the one of a
 * temporal point
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contains_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point contains the
 * one of a geometry/geography
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contains_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point contains a
 * spatiotemporal box
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contains_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal point contains
 * the one of the second temporal point
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contains_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a geometry/geography is
 * contained in the one of a temporal point
 * @sqlfunc contained_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contained_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box is contained in the spatiotemporal
 * box of a temporal point
 * @sqlfunc contained_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contained_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point is contained
 * in the one of a geometry/geography
 * @sqlfunc contained_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contained_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point is contained
 * in the spatiotemporal box
 * @sqlfunc contained_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contained_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal point is
 * contained in the one of the second temporal point
 * @sqlfunc contained_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contained_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Same_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a geometry/geography and
 * a temporal point are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a temporal point and
 * geometry/geography are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Same_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points are equal
 * in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a geometry/geography and
 * a temporal point are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a temporal point and a
 * geometry/geography are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/

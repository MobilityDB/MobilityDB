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
 * @brief Bounding box operators for temporal network points.
 *
 * These operators test the bounding boxes of temporal npoints, which are
 * STBOX boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "npoint/tnpoint_boxops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_util.h"
#include "pg_point/tpoint_boxops.h"
#include "pg_npoint/tnpoint.h"

/*****************************************************************************
 * Transform a temporal Npoint to a STBOX
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_to_stbox);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Return the bounding box of the network point value
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Npoint_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  STBOX *result = palloc0(sizeof(STBOX));
  npoint_set_stbox(np, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Nsegment_to_stbox);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Return the bounding box of the network segment value
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Nsegment_to_stbox(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  STBOX *result = palloc(sizeof(STBOX));
  nsegment_set_stbox(ns, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_timestamp_to_stbox);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Transform a network point and a timestamp to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Npoint_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBOX *result = palloc0(sizeof(STBOX));
  npoint_timestamp_set_stbox(np, t, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Npoint_period_to_stbox);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Transform a network point and a period to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Npoint_period_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  STBOX *result = palloc0(sizeof(STBOX));
  npoint_period_set_stbox(np, p, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_to_stbox);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Transform a temporal network point to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Tnpoint_to_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *result = tpoint_to_stbox(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic box function for a geometry and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_geo_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = boxop_tnpoint_geo(temp, gs, func, INVERT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Generic box function for a temporal network point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tnpoint_geo_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = boxop_tnpoint_geo(temp, gs, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Generic box function for an stbox and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] spatial True when the function considers the spatial dimension,
 * false when it considers the temporal dimension
 */
Datum
boxop_stbox_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *), bool spatial)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = boxop_tnpoint_stbox(temp, box, func, spatial, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Generic box function for a temporal network point and an stbox
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] spatial True when the function considers the spatial dimension,
 * false when it considers the temporal dimension
 */
Datum
boxop_tnpoint_stbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *), bool spatial)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  int result = boxop_tnpoint_stbox(temp, box, func, spatial, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Generic box function for a network point and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_npoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tnpoint_npoint(temp, np, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for a temporal network point and a network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tnpoint_npoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  bool result = boxop_tnpoint_npoint(temp, np, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for two temporal network points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tnpoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tnpoint_tnpoint(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the geometry and
 * the temporal network point overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of
 * the temporal network point overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the network point and the
 * temporal network point overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network point and
 * the geometry overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point and
 * the spatiotemporal box overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network point and
 * the network point overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network points
 * overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_bbox_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the geometry contains the one of
 * the temporal network point
 * @sqlfunc contains_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_bbox_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box contains the one of the temporal
 * network point
 * @sqlfunc contains_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox 
 * @brief Return true if the spatiotemporal box of the network point contains the one
 * of the temporal network point
 * @sqlfunc contains_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the one of the geometry
 * @sqlfunc contains_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the spatiotemporal box
 * @sqlfunc contains_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the one of the network point
 * @sqlfunc contains_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the first temporal network point
 * contain the one of the second temporal network point
 * @sqlfunc contains_bbox()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the geometry is contained by the
 * one of the temporal network point
 * @sqlfunc contained_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box is contained by the one of the
 * temporal network point
 * @sqlfunc contained_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the network point is contained by
 * the one of the temporal network point
 * @sqlfunc contained_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the geometry
 * @sqlfunc contained_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the spatiotemporal box
 * @sqlfunc contained_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the network point
 * @sqlfunc contained_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the first temporal network point
 * is contained by the one of the second temporal network point
 * @sqlfunc contained_bbox()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************
 * Same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Same_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the geometry and the temporal
 * network point are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal network point are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the network point and the
 * temporal network point are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Same_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network point and
 * the geometry are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point and
 * the spatiotemporal box are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network point and
 * the network point are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network points
 * are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
PGDLLEXPORT Datum
Same_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the geometry and the temporal
 * network point are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal network point are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the network point and the
 * temporal network point are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network point and
 * the geometry are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal box of the temporal network point
 * and the spatiotemporal box are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network point and
 * the network point are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox
 * @brief Return true if the spatiotemporal boxes of the temporal network points
 * are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/

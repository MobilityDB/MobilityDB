/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Bounding box operators for temporal network points.
 *
 * These operators test the bounding boxes of temporal npoints, which are
 * STBox boxes. The following operators are defined:
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
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "point/stbox.h"
#include "npoint/tnpoint.h"
/* MobilityDB */
#include "pg_point/tpoint_boxops.h"

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a network point converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Npoint_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  PG_RETURN_STBOX_P(npoint_to_stbox(np));
}

PGDLLEXPORT Datum Nsegment_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Nsegment_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a network segment converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Nsegment_to_stbox(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  PG_RETURN_STBOX_P(nsegment_to_stbox(ns));
}

/*****************************************************************************/

PGDLLEXPORT Datum Npointset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npointset_to_stbox);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a network point set converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Npointset_to_stbox(PG_FUNCTION_ARGS)
{
  Set *set = PG_GETARG_SET_P(0);
  STBox *result = spatialset_to_stbox(set);
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Npoint_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a network point and a timestamptz to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Npoint_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(npoint_timestamptz_to_stbox(np, t));
}

PGDLLEXPORT Datum Npoint_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a network point and a timestamptz span to a spatiotemporal
 * box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Npoint_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(npoint_tstzspan_to_stbox(np, s));
}

/*****************************************************************************/

PGDLLEXPORT Datum Tnpoint_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal network point converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Tnpoint_to_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *result = tpoint_to_stbox(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic box function for an stbox and a temporal network point
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
Boxop_stbox_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  STBox box1;
  temporal_set_bbox(temp, &box1);
  bool result = func(box, &box1);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for a temporal network point and an stbox
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
Boxop_tnpoint_stbox(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  STBox box1;
  temporal_set_bbox(temp, &box1);
  bool result = func(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for two temporal network points
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
Boxop_tnpoint_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  STBox box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_stbox_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of
 * a temporal network point overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tnpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal network point and
 * a spatiotemporal box overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal network points
 * overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_stbox_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box contains the one of a temporal
 * network point
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tnpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal network point
 * contains a spatiotemporal box
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of the first temporal network
 * point contains the one of the second temporal network point
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_stbox_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box is contained in the one of a
 * temporal network point
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tnpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal network point is
 * contained in a spatiotemporal box
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of the first temporal network
 * point is contained in the one of the second temporal network point
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************
 * Same
 *****************************************************************************/

PGDLLEXPORT Datum Same_stbox_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal network point are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tnpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal network point and
 * a spatiotemporal box are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal network points
 * are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PGDLLEXPORT Datum Adjacent_stbox_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal network point are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tnpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal network point
 * and a spatiotemporal box are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal network
 * points are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/

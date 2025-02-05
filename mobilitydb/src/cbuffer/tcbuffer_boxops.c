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
 * @brief Bounding box operators for temporal circular buffers.
 *
 * These operators test the bounding boxes of temporal circular buffers, which
 * are STBox boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "cbuffer/tcbuffer_boxops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "point/stbox.h"
#include "cbuffer/tcbuffer.h"
/* MobilityDB */
#include "pg_point/tpoint_boxops.h"

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a circular buffer converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Cbuffer_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_STBOX_P(cbuffer_to_stbox(cbuf));
}

/*****************************************************************************/

PGDLLEXPORT Datum Cbufferset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbufferset_to_stbox);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a circular buffer set converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Cbufferset_to_stbox(PG_FUNCTION_ARGS)
{
  Set *set = PG_GETARG_SET_P(0);
  STBox *result = spatialset_to_stbox(set);
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Cbuffer_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a circular buffer and a timestamptz to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Cbuffer_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(cbuffer_timestamptz_to_stbox(cbuf, t));
}

PGDLLEXPORT Datum Cbuffer_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a circular buffer and a timestamptz span to a spatiotemporal
 * box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Cbuffer_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(cbuffer_tstzspan_to_stbox(cbuf, s));
}

/*****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_to_stbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal circular buffer converted to a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Tcbuffer_to_stbox(PG_FUNCTION_ARGS)
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
 * @brief Generic box function for an stbox and a temporal circular buffer
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
Boxop_stbox_tcbuffer(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  STBox box1;
  tspatial_set_stbox(temp, &box1);
  bool result = func(box, &box1);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for a temporal circular buffer and an stbox
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
Boxop_tcbuffer_stbox(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  STBox box1;
  tspatial_set_stbox(temp, &box1);
  bool result = func(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for two temporal circular buffers
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
Boxop_tcbuffer_tcbuffer(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  STBox box1, box2;
  tspatial_set_stbox(temp1, &box1);
  tspatial_set_stbox(temp2, &box2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of
 * a temporal circular buffer overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal circular buffer and
 * a spatiotemporal box overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal circular buffers
 * overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box contains the one of a temporal
 * circular buffer
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal circular buffer
 * contains a spatiotemporal box
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of the first temporal network
 * point contains the one of the second temporal circular buffer
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box is contained in the one of a
 * temporal circular buffer
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal circular buffer is
 * contained in a spatiotemporal box
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of the first temporal network
 * point is contained in the one of the second temporal circular buffer
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************
 * Same
 *****************************************************************************/

PGDLLEXPORT Datum Same_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal circular buffer are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal circular buffer and
 * a spatiotemporal box are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal circular buffers
 * are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PGDLLEXPORT Datum Adjacent_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal circular buffer are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tpoint(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal circular buffer
 * and a spatiotemporal box are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_stbox(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal network
 * points are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tpoint_tpoint(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/

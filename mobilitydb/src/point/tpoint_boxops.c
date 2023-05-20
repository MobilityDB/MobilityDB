/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Bounding box operators for temporal points.
 *
 * These operators test the bounding boxes of temporal points, which are an
 * `STBox`, where the *x*, *y*, and optional *z* coordinates are for the space
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
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Boxes function
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_stboxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_stboxes);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return an array of spatiotemporal boxes from a temporal point
 * @sqlfunc stboxes()
 */
Datum
Tpoint_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  STBox *boxes = tpoint_stboxes(temp, &count);
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
 * @brief Generic bounding box function for a spatiotemporal box and a temporal point
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_stbox_tpoint_ext(FunctionCallInfo fcinfo,
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
 * @brief Generic bounding box function for a temporal point and a spatiotemporal box
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_stbox_ext(FunctionCallInfo fcinfo,
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
 * @brief Generic topological function for two temporal points
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
boxop_tpoint_tpoint_ext(FunctionCallInfo fcinfo,
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

PGDLLEXPORT Datum Overlaps_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overlaps_stbox_stbox);
}

PGDLLEXPORT Datum Overlaps_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points overlap
 * @sqlfunc overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box contains the one of a
 * temporal point
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
Datum
Contains_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point contains a
 * spatiotemporal box
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
Datum
Contains_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contains_stbox_stbox);
}

PGDLLEXPORT Datum Contains_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal point contains
 * the one of the second temporal point
 * @sqlfunc contains_bbox()
 * @sqlop @p <@
 */
Datum
Contains_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box is contained in the spatiotemporal
 * box of a temporal point
 * @sqlfunc contained_bbox()
 * @sqlop @p \@>
 */
Datum
Contained_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point is contained
 * in the spatiotemporal box
 * @sqlfunc contained_bbox()
 * @sqlop @p \@>
 */
Datum
Contained_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contained_stbox_stbox);
}

PGDLLEXPORT Datum Contained_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal point is
 * contained in the one of the second temporal point
 * @sqlfunc contained_bbox()
 * @sqlop @p \@>
 */
Datum
Contained_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PGDLLEXPORT Datum Same_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are equal in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &same_stbox_stbox);
}

PGDLLEXPORT Datum Same_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points are equal
 * in the common dimensions
 * @sqlfunc same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PGDLLEXPORT Datum Adjacent_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &adjacent_stbox_stbox);
}

PGDLLEXPORT Datum Adjacent_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points are adjacent
 * @sqlfunc adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/

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
 * @file tnpoint_boxops.c
 * Bounding box operators for temporal network points.
 *
 * These operators test the bounding boxes of temporal npoints, which are
 * STBOX boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "npoint/tnpoint_boxops.h"

#include <utils/timestamp.h>

#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "point/stbox.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Transform a temporal npoint to a STBOX
 *****************************************************************************/

/**
 * Set the spatiotemporal box from the network point value
 *
 * @param[out] box Spatiotemporal box
 * @param[in] np Network point
 */
bool
npoint_to_stbox_internal(STBOX *box, const npoint *np)
{
  Datum geom = npoint_as_geom_internal(DatumGetNpoint(np));
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  bool result = geo_to_stbox_internal(box, gs);
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * Set the spatiotemporal box from the network point value
 *
 * @param[out] box Spatiotemporal box
 * @param[in] inst Temporal network point
 */
void
tnpointinst_make_stbox(STBOX *box, const TInstant *inst)
{
  npoint_to_stbox_internal(box, DatumGetNpoint(tinstant_value(inst)));
  box->tmin = box->tmax = inst->t;
  MOBDB_FLAGS_SET_T(box->flags, true);
}

/**
 * Set the spatiotemporal box from the array of temporal network point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal network point values
 * @param[in] count Number of elements in the array
 */
void
tnpointinstarr_step_to_stbox(STBOX *box, const TInstant **instants, int count)
{
  tnpointinst_make_stbox(box, instants[0]);
  for (int i = 1; i < count; i++)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    tnpointinst_make_stbox(&box1, instants[i]);
    stbox_expand(box, &box1);
  }
}

/**
 * Set the spatiotemporal box from the array of temporal network point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 */
void
tnpointinstarr_linear_to_stbox(STBOX *box, const TInstant **instants, int count)
{
  npoint *np = DatumGetNpoint(tinstant_value(instants[0]));
  int64 rid = np->rid;
  double posmin = np->pos, posmax = np->pos;
  TimestampTz tmin = instants[0]->t, tmax = instants[0]->t;
  for (int i = 1; i < count; i++)
  {
    np = DatumGetNpoint(tinstant_value(instants[i]));
    posmin = Min(posmin, np->pos);
    posmax = Max(posmax, np->pos);
    tmin = Min(tmin, instants[i]->t);
    tmax = Max(tmax, instants[i]->t);
  }

  Datum line = route_geom(rid);
  Datum geom = (posmin == 0 && posmax == 1) ? line :
    call_function3(LWGEOM_line_substring, line, Float8GetDatum(posmin),
      Float8GetDatum(posmax));
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  geo_to_stbox_internal(box, gs);
  box->tmin = tmin;
  box->tmax = tmax;
  MOBDB_FLAGS_SET_T(box->flags, true);
  pfree(DatumGetPointer(line));
  if (posmin != 0 || posmax != 1)
    pfree(DatumGetPointer(geom));
}

/**
 * Set the spatiotemporal box from the array of temporal network point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] sequences Temporal network point values
 * @param[in] count Number of elements in the array
 */
void
tnpointseqarr_to_stbox(STBOX *box, const TSequence **sequences, int count)
{
  tsequence_bbox(box, sequences[0]);
  for (int i = 1; i < count; i++)
  {
    STBOX *box1 = tsequence_bbox_ptr(sequences[i]);
    stbox_expand(box, box1);
  }
}

PG_FUNCTION_INFO_V1(npoint_to_stbox);
/**
 * Returns the bounding box of the network point value
 */
PGDLLEXPORT Datum
npoint_to_stbox(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  STBOX *result = palloc0(sizeof(STBOX));
  npoint_to_stbox_internal(result, np);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the bounding box of the network segment value
 */
bool
nsegment_to_stbox_internal(STBOX *box, const nsegment *ns)
{
  Datum geom = nsegment_as_geom_internal(DatumGetNsegment(ns));
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  bool result = geo_to_stbox_internal(box, gs);
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  return result;
}

PG_FUNCTION_INFO_V1(nsegment_to_stbox);
/**
 * Returns the bounding box of the network segment value
 */
PGDLLEXPORT Datum
nsegment_to_stbox(PG_FUNCTION_ARGS)
{
  nsegment *ns = PG_GETARG_NSEGMENT(0);
  STBOX *result = palloc0(sizeof(STBOX));
  nsegment_to_stbox_internal(result, ns);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transform a network point and a timestamp to a spatiotemporal box
 */
static bool
npoint_timestamp_to_stbox_internal(STBOX *box, const npoint *np, TimestampTz t)
{
  npoint_to_stbox_internal(box, np);
  box->tmin = box->tmax = t;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return true;
}

PG_FUNCTION_INFO_V1(npoint_timestamp_to_stbox);
/**
 * Transform a network point and a timestamp to a spatiotemporal box
 */
PGDLLEXPORT Datum
npoint_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBOX *result = palloc0(sizeof(STBOX));
  npoint_timestamp_to_stbox_internal(result, np, t);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transform a network point and a period to a spatiotemporal box
 */
static bool
npoint_period_to_stbox_internal(STBOX *box, const npoint *np, const Period *p)
{
  npoint_to_stbox_internal(box, np);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return true;
}

PG_FUNCTION_INFO_V1(npoint_period_to_stbox);
/**
 * Transform a network point and a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
npoint_period_to_stbox(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Period *p = PG_GETARG_PERIOD(1);
  STBOX *result = palloc0(sizeof(STBOX));
  npoint_period_to_stbox_internal(result, np, p);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_to_stbox);
/**
 * Transform a temporal network point to a spatiotemporal box
 */
PGDLLEXPORT Datum
tnpoint_to_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *result = palloc0(sizeof(STBOX));
  temporal_bbox(result, temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * Generic box function for a static and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_npoint_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tnpoint_srid_internal(temp), npoint_srid_internal(np));
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box1, np);
  temporal_bbox(&box2, temp);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic box function for a temporal and a static network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tnpoint_npoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid(tnpoint_srid_internal(temp), npoint_srid_internal(np));
  STBOX box1, box2;
  memset(&box1, 0, sizeof(STBOX));
  memset(&box2, 0, sizeof(STBOX));
  temporal_bbox(&box1, temp);
  /* Returns an error if the geometry is not found, is null, or is empty */
  npoint_to_stbox_internal(&box2, np);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_geo_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry and
 * the temporal network point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_stbox_tnpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of
 * the temporal network point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_npoint_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the network point and the
 * temporal network point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_tnpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal network point and
 * the geometry overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal network point and
 * the spatiotemporal box overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnpoint_npoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal network point and
 * the network point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnpoint_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal network points
 * overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_geo_tnpoint);
/**
 * Returns true if the spatiotemporal box of the geometry contains the one of
 * the temporal network point
 */
PGDLLEXPORT Datum
contains_bbox_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_stbox_tnpoint);
/**
 * Returns true if the spatiotemporal box contains the one of the temporal
 * network point
 */
PGDLLEXPORT Datum
contains_bbox_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_npoint_tnpoint);
/**
 * Returns true if the spatiotemporal box of the network point contains the one
 * of the temporal network point
 */
PGDLLEXPORT Datum
contains_bbox_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint(fcinfo, &contains_stbox_stbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_tnpoint_geo);
/**
 * Returns true if the spatiotemporal box of the temporal network point
 * contain the one of the geometry
 */
PGDLLEXPORT Datum
contains_bbox_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal network point
 * contain the spatiotemporal box
 */
PGDLLEXPORT Datum
contains_bbox_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnpoint_npoint);
/**
 * Returns true if the spatiotemporal box of the temporal network point
 * contain the one of the network point
 */
PGDLLEXPORT Datum
contains_bbox_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnpoint_tnpoint);
/**
 * Returns true if the spatiotemporal box of the first temporal network point
 * contain the one of the second temporal network point
 */
PGDLLEXPORT Datum
contains_bbox_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_geo_tnpoint);
/**
 * Returns true if the spatiotemporal box of the geometry is contained by the
 * one of the temporal network point
 */
PGDLLEXPORT Datum
contained_bbox_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_stbox_tnpoint);
/**
 * Returns true if the spatiotemporal box is contained by the one of the
 * temporal network point
 */
PGDLLEXPORT Datum
contained_bbox_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_npoint_tnpoint);
/**
 * Returns true if the spatiotemporal box of the network point is contained by
 * the one of the temporal network point
 */
PGDLLEXPORT Datum
contained_bbox_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint(fcinfo, &contained_stbox_stbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_tnpoint_geo);
/**
 * Returns true if the spatiotemporal box of the temporal network point is
 * contained by the one of the geometry
 */
PGDLLEXPORT Datum
contained_bbox_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal network point is
 * contained by the spatiotemporal box
 */
PGDLLEXPORT Datum
contained_bbox_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnpoint_npoint);
/**
 * Returns true if the spatiotemporal box of the temporal network point is
 * contained by the one of the network point
 */
PGDLLEXPORT Datum
contained_bbox_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnpoint_tnpoint);
/**
 * Returns true if the spatiotemporal box of the first temporal network point
 * is contained by the one of the second temporal network point
 */
PGDLLEXPORT Datum
contained_bbox_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_geo_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry and the temporal
 * network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_stbox_tnpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the
 * temporal network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_npoint_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the network point and the
 * temporal network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint(fcinfo, &same_stbox_stbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_tnpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal network point and
 * the geometry are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tnpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal network point and
 * the spatiotemporal box are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tnpoint_npoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal network point and
 * the network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tnpoint_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal network points
 * are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &same_stbox_stbox_internal);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_geo_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry and the temporal
 * network point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_stbox_tnpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the
 * temporal network point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_npoint_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the network point and the
 * temporal network point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_tnpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal network point and
 * the geometry are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal network point
 * and the spatiotemporal box are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnpoint_npoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal network point and
 * the network point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnpoint_tnpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal network points
 * are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

/*****************************************************************************/

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
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "point/stbox.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Transform a temporal Npoint to a STBOX
 *****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Set the spatiotemporal box from the network point value.
 *
 * @param[out] box Spatiotemporal box
 * @param[in] np Network point
 */
bool
npoint_stbox(const Npoint *np, STBOX *box)
{
  Datum geom = npoint_geom(DatumGetNpointP(np));
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  bool result = geo_stbox(gs, box);
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
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
tnpointinst_make_stbox(const TInstant *inst, STBOX *box)
{
  npoint_stbox(DatumGetNpointP(tinstant_value(inst)), box);
  box->tmin = box->tmax = inst->t;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal network point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal network point values
 * @param[in] count Number of elements in the array
 */
void
tnpointinstarr_stbox(const TInstant **instants, int count, STBOX *box)
{
  tnpointinst_make_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBOX box1;
    tnpointinst_make_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal network point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 */
void
tnpointinstarr_linear_stbox(const TInstant **instants, int count,
  STBOX *box)
{
  Npoint *np = DatumGetNpointP(tinstant_value(instants[0]));
  int64 rid = np->rid;
  double posmin = np->pos, posmax = np->pos;
  TimestampTz tmin = instants[0]->t, tmax = instants[count - 1]->t;
  for (int i = 1; i < count; i++)
  {
    np = DatumGetNpointP(tinstant_value(instants[i]));
    posmin = Min(posmin, np->pos);
    posmax = Max(posmax, np->pos);
  }

  Datum line = route_geom(rid);
  Datum geom = (posmin == 0 && posmax == 1) ? line :
    call_function3(LWGEOM_line_substring, line, Float8GetDatum(posmin),
      Float8GetDatum(posmax));
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  geo_stbox(gs, box);
  box->tmin = tmin;
  box->tmax = tmax;
  MOBDB_FLAGS_SET_T(box->flags, true);
  pfree(DatumGetPointer(line));
  if (posmin != 0 || posmax != 1)
    pfree(DatumGetPointer(geom));
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal network point values.
 *
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[in] linear True when the interpolation is linear
 * @param[out] box Spatiotemporal box
 */
void
tnpointseq_make_stbox(const TInstant **instants, int count, bool linear,
  STBOX *box)
{
  if (linear)
    tnpointinstarr_linear_stbox(instants, count, box);
  else
    tnpointinstarr_stbox(instants, count, box);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return the bounding box of the network segment value
 */
bool
nsegment_stbox(STBOX *box, const Nsegment *ns)
{
  Datum geom = nsegment_geom(DatumGetNsegmentP(ns));
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  bool result = geo_stbox(gs, box);
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform a network point and a timestamp to a spatiotemporal box
 */
bool
npoint_timestamp_to_stbox(const Npoint *np, TimestampTz t, STBOX *box)
{
  npoint_stbox(np, box);
  box->tmin = box->tmax = t;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform a network point and a period to a spatiotemporal box
 */
bool
npoint_period_to_stbox(const Npoint *np, const Period *p, STBOX *box)
{
  npoint_stbox(np, box);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return true;
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_box
 * @brief Generic box function for a temporal network point and a geometry.
 *
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] func Function
 * @param[in] invert True when the geometry is the first argument of the
 * function
 */
int
boxop_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  if (gserialized_is_empty(gs))
    return -1;
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(gs));
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  geo_stbox(gs, &box2);
  temporal_bbox(temp, &box1);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return(result ? 1 : 0);
}

/**
 * @ingroup libmeos_temporal_box
 * @brief Generic box function for a temporal network point and an stbox.
 *
 * @param[in] temp Temporal network point
 * @param[in] box Bounding box
 * @param[in] func Function
 * @param[in] spatial True when the function considers the spatial dimension,
 * false when it considers the temporal dimension
 * @param[in] invert True when the bounding box is the first argument of the
 * function
 */
int
boxop_tnpoint_stbox(const Temporal *temp, const STBOX *box,
  bool (*func)(const STBOX *, const STBOX *), bool spatial, bool invert)
{
  if ((spatial && ! MOBDB_FLAGS_GET_X(box->flags)) ||
     (! spatial && ! MOBDB_FLAGS_GET_T(box->flags)))
    return -1;
  ensure_not_geodetic(box->flags);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  temporal_bbox(temp, &box1);
  bool result = invert ? func(box, &box1) : func(&box1, box);
  return result ? 1 : 0;
}

/**
 * @ingroup libmeos_temporal_box
 * @brief Generic box function for a temporal network point and a network point.
 *
 * @param[in] temp Temporal network point
 * @param[in] np network point
 * @param[in] func Function
 * @param[in] invert True when the network point is the first argument of the
 * function
 */
bool
boxop_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  STBOX box1, box2;
  /* Return an error if the geometry is not found, is null, or is empty */
  npoint_stbox(np, &box2);
  temporal_bbox(temp, &box1);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return result;
}

/**
 * @ingroup libmeos_temporal_box
 * @brief Generic box function for two temporal network points
 *
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] func Function
 */
bool
boxop_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const STBOX *, const STBOX *))
{
  ensure_same_srid(tnpoint_srid(temp1), tnpoint_srid(temp2));
  STBOX box1, box2;
  temporal_bbox(temp1, &box1);
  temporal_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Transform a temporal Npoint to a STBOX
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Npoint_to_stbox);
/**
 * Return the bounding box of the network point value
 */
PGDLLEXPORT Datum
Npoint_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  npoint_stbox(np, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Nsegment_to_stbox);
/**
 * Return the bounding box of the network segment value
 */
PGDLLEXPORT Datum
Nsegment_to_stbox(PG_FUNCTION_ARGS)
{
  Nsegment *ns = PG_GETARG_NSEGMENT_P(0);
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  nsegment_stbox(result, ns);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Npoint_timestamp_to_stbox);
/**
 * Transform a network point and a timestamp to a spatiotemporal box
 */
PGDLLEXPORT Datum
Npoint_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  npoint_timestamp_to_stbox(np, t, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Npoint_period_to_stbox);
/**
 * Transform a network point and a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
Npoint_period_to_stbox(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  npoint_period_to_stbox(np, p, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_to_stbox);
/**
 * Transform a temporal network point to a spatiotemporal box
 */
PGDLLEXPORT Datum
Tnpoint_to_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *result = tpoint_stbox(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * Generic box function for a geometry and a temporal network point
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
 * Generic box function for a temporal network point and a geometry
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
 * Generic box function for an stbox and a temporal network point
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
 * Generic box function for a temporal network point and an stbox
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
 * Generic box function for a network point and a temporal network point
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
 * Generic box function for a temporal network point and a network point
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
 * Generic box function for two temporal network points
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
 * Return true if the spatiotemporal boxes of the geometry and
 * the temporal network point overlap
 */
PGDLLEXPORT Datum
Overlaps_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box and the spatiotemporal box of
 * the temporal network point overlap
 */
PGDLLEXPORT Datum
Overlaps_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_npoint_tnpoint);
/**
 * Return true if the spatiotemporal boxes of the network point and the
 * temporal network point overlap
 */
PGDLLEXPORT Datum
Overlaps_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &overlaps_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_geo);
/**
 * Return true if the spatiotemporal boxes of the temporal network point and
 * the geometry overlap
 */
PGDLLEXPORT Datum
Overlaps_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal network point and
 * the spatiotemporal box overlap
 */
PGDLLEXPORT Datum
Overlaps_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_npoint);
/**
 * Return true if the spatiotemporal boxes of the temporal network point and
 * the network point overlap
 */
PGDLLEXPORT Datum
Overlaps_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &overlaps_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overlaps_tnpoint_tnpoint);
/**
 * Return true if the spatiotemporal boxes of the temporal network points
 * overlap
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
 * Return true if the spatiotemporal box of the geometry contains the one of
 * the temporal network point
 */
PGDLLEXPORT Datum
Contains_bbox_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box contains the one of the temporal
 * network point
 */
PGDLLEXPORT Datum
Contains_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_npoint_tnpoint);
/**
 * Return true if the spatiotemporal box of the network point contains the one
 * of the temporal network point
 */
PGDLLEXPORT Datum
Contains_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &contains_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_tnpoint_geo);
/**
 * Return true if the spatiotemporal box of the temporal network point
 * contain the one of the geometry
 */
PGDLLEXPORT Datum
Contains_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tnpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal network point
 * contain the spatiotemporal box
 */
PGDLLEXPORT Datum
Contains_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tnpoint_npoint);
/**
 * Return true if the spatiotemporal box of the temporal network point
 * contain the one of the network point
 */
PGDLLEXPORT Datum
Contains_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &contains_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contains_tnpoint_tnpoint);
/**
 * Return true if the spatiotemporal box of the first temporal network point
 * contain the one of the second temporal network point
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
 * Return true if the spatiotemporal box of the geometry is contained by the
 * one of the temporal network point
 */
PGDLLEXPORT Datum
Contained_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is contained by the one of the
 * temporal network point
 */
PGDLLEXPORT Datum
Contained_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_npoint_tnpoint);
/**
 * Return true if the spatiotemporal box of the network point is contained by
 * the one of the temporal network point
 */
PGDLLEXPORT Datum
Contained_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &contained_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_tnpoint_geo);
/**
 * Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the geometry
 */
PGDLLEXPORT Datum
Contained_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tnpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal network point is
 * contained by the spatiotemporal box
 */
PGDLLEXPORT Datum
Contained_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tnpoint_npoint);
/**
 * Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the network point
 */
PGDLLEXPORT Datum
Contained_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &contained_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Contained_tnpoint_tnpoint);
/**
 * Return true if the spatiotemporal box of the first temporal network point
 * is contained by the one of the second temporal network point
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
 * Return true if the spatiotemporal boxes of the geometry and the temporal
 * network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_npoint_tnpoint);
/**
 * Return true if the spatiotemporal boxes of the network point and the
 * temporal network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &same_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Same_tnpoint_geo);
/**
 * Return true if the spatiotemporal boxes of the temporal network point and
 * the geometry are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tnpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal network point and
 * the spatiotemporal box are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tnpoint_npoint);
/**
 * Return true if the spatiotemporal boxes of the temporal network point and
 * the network point are equal in the common dimensions
 */
PGDLLEXPORT Datum
Same_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &same_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Same_tnpoint_tnpoint);
/**
 * Return true if the spatiotemporal boxes of the temporal network points
 * are equal in the common dimensions
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
 * Return true if the spatiotemporal boxes of the geometry and the temporal
 * network point are adjacent
 */
PGDLLEXPORT Datum
Adjacent_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal network point are adjacent
 */
PGDLLEXPORT Datum
Adjacent_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_npoint_tnpoint);
/**
 * Return true if the spatiotemporal boxes of the network point and the
 * temporal network point are adjacent
 */
PGDLLEXPORT Datum
Adjacent_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_geo);
/**
 * Return true if the spatiotemporal boxes of the temporal network point and
 * the geometry are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_stbox);
/**
 * Return true if the spatiotemporal box of the temporal network point
 * and the spatiotemporal box are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_npoint);
/**
 * Return true if the spatiotemporal boxes of the temporal network point and
 * the network point are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &adjacent_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Adjacent_tnpoint_tnpoint);
/**
 * Return true if the spatiotemporal boxes of the temporal network points
 * are adjacent
 */
PGDLLEXPORT Datum
Adjacent_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &adjacent_stbox_stbox);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/

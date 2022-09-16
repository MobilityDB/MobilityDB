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
#include <meos_internal.h>
/* MobilityDB */
#include "general/temporal_util.h"
#include "point/pgis_call.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Transform a temporal Npoint to a STBOX
 *****************************************************************************/

/**
 * @brief Set the spatiotemporal box from the network point value.
 *
 * @param[in] np Network point
 * @param[out] box Spatiotemporal box
 */
bool
npoint_set_stbox(const Npoint *np, STBOX *box)
{
  GSERIALIZED *geom = npoint_geom(np);
  bool result = geo_set_stbox(geom, box);
  pfree(geom);
  return result;
}

/**
 * Set the spatiotemporal box from the network point value
 *
 * @param[in] inst Temporal network point
 * @param[out] box Spatiotemporal box
 */
void
tnpointinst_set_stbox(const TInstant *inst, STBOX *box)
{
  npoint_set_stbox(DatumGetNpointP(tinstant_value(inst)), box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, &box->period);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal network point values
 *
 * @param[in] instants Temporal network point values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_set_stbox(const TInstant **instants, int count, STBOX *box)
{
  tnpointinst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBOX box1;
    tnpointinst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal network point values
 *
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_linear_set_stbox(const TInstant **instants, int count,
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

  GSERIALIZED *line = route_geom(rid);
  GSERIALIZED *gs = (posmin == 0 && posmax == 1) ? line :
    gserialized_line_substring(line, posmin, posmax);
  geo_set_stbox(gs, box);
  span_set(TimestampTzGetDatum(tmin), TimestampTzGetDatum(tmax),
    true, true, T_TIMESTAMPTZ, &box->period);
  MOBDB_FLAGS_SET_T(box->flags, true);
  pfree(DatumGetPointer(line));
  if (posmin != 0 || posmax != 1)
    pfree(gs);
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal network point values.
 *
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[out] box Spatiotemporal box
 */
void
tnpointseq_set_stbox(const TInstant **instants, int count, interpType interp,
  STBOX *box)
{
  if (interp == LINEAR)
    tnpointinstarr_linear_set_stbox(instants, count, box);
  else
    tnpointinstarr_set_stbox(instants, count, box);
  return;
}

/**
 * @brief Return the bounding box of the network segment value
 */
bool
nsegment_set_stbox(const Nsegment *ns, STBOX *box)
{
  GSERIALIZED *geom = nsegment_geom(ns);
  bool result = geo_set_stbox(geom, box);
  pfree(geom);
  return result;
}

/**
 * @brief Transform a network point and a timestamp to a spatiotemporal box
 */
bool
npoint_timestamp_set_stbox(const Npoint *np, TimestampTz t, STBOX *box)
{
  npoint_set_stbox(np, box);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t),
    true, true, T_TIMESTAMPTZ, &box->period);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @brief Transform a network point and a period to a spatiotemporal box
 */
bool
npoint_period_set_stbox(const Npoint *np, const Period *p, STBOX *box)
{
  npoint_set_stbox(np, box);
  memcpy(&box->period, p, sizeof(Span));
  MOBDB_FLAGS_SET_T(box->flags, true);
  return true;
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic box function for a temporal network point and a geometry.
 *
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] func Function
 * @param[in] invert True if the geometry is the first argument of the
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
  geo_set_stbox(gs, &box2);
  temporal_set_bbox(temp, &box1);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return(result ? 1 : 0);
}

/**
 * @brief Generic box function for a temporal network point and an stbox.
 *
 * @param[in] temp Temporal network point
 * @param[in] box Bounding box
 * @param[in] func Function
 * @param[in] spatial True if the function considers the spatial dimension,
 * false when it considers the temporal dimension
 * @param[in] invert True if the bounding box is the first argument of the
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
  temporal_set_bbox(temp, &box1);
  bool result = invert ? func(box, &box1) : func(&box1, box);
  return result ? 1 : 0;
}

/**
 * @brief Generic box function for a temporal network point and a network point.
 *
 * @param[in] temp Temporal network point
 * @param[in] np network point
 * @param[in] func Function
 * @param[in] invert True if the network point is the first argument of the
 * function
 */
bool
boxop_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  STBOX box1, box2;
  /* Return an error if the geometry is not found, is null, or is empty */
  npoint_set_stbox(np, &box2);
  temporal_set_bbox(temp, &box1);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return result;
}

/**
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
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  return result;
}

/*****************************************************************************/

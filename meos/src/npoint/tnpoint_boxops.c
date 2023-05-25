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
#include "general/type_util.h"
#include "point/pgis_call.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Transform a temporal Npoint to a STBox
 *****************************************************************************/

/**
 * @brief Set the spatiotemporal box from the network point value.
 * @param[in] np Network point
 * @param[out] box Spatiotemporal box
 */
bool
npoint_set_stbox(const Npoint *np, STBox *box)
{
  GSERIALIZED *geom = npoint_geom(np);
  bool result = geo_set_stbox(geom, box);
  pfree(geom);
  return result;
}

/**
 * @brief Set the spatiotemporal box from an array of network point values
 * @param[in] values Temporal network point values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
npointarr_set_stbox(const Datum *values, int count, STBox *box)
{
  npoint_set_stbox(DatumGetNpointP(values[0]), box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    npoint_set_stbox(DatumGetNpointP(values[i]), &box1);
    stbox_expand(&box1, box);
  }
  return;
}
/**
 * @brief Set the spatiotemporal box from the network point value
 * @param[in] inst Temporal network point
 * @param[out] box Spatiotemporal box
 */
void
tnpointinst_set_stbox(const TInstant *inst, STBox *box)
{
  npoint_set_stbox(DatumGetNpointP(tinstant_value(inst)), box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal network point
 * values
 * @param[in] instants Temporal network point values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_step_set_stbox(const TInstant **instants, int count, STBox *box)
{
  tnpointinst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    tnpointinst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal network point
 * values
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_linear_set_stbox(const TInstant **instants, int count,
  STBox *box)
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
  MEOS_FLAGS_SET_T(box->flags, true);
  pfree(line);
  if (posmin != 0 || posmax != 1)
    pfree(gs);
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal network point
 * values.
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_set_stbox(const TInstant **instants, int count,
  interpType interp, STBox *box)
{
  if (interp == LINEAR)
    tnpointinstarr_linear_set_stbox(instants, count, box);
  else
    tnpointinstarr_step_set_stbox(instants, count, box);
  return;
}

/**
 * @brief Expand the temporal box of a temporal network point sequence with an
 * instant
 * @param[in] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tnpointseq_expand_stbox(const TSequence *seq, const TInstant *inst)
{
  /* Compute the bounding box of the end point of the sequence and the instant */
  STBox box;
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR)
  {
    const TInstant *last = TSEQUENCE_INST_N(seq, seq->count - 1);
    Npoint *np1 = DatumGetNpointP(tinstant_value(last));
    Npoint *np2 = DatumGetNpointP(tinstant_value(inst));
    int64 rid = np1->rid;
    double posmin = Min(np1->pos, np2->pos);
    double posmax = Min(np1->pos, np2->pos);
    GSERIALIZED *line = route_geom(rid);
    GSERIALIZED *gs = (posmin == 0 && posmax == 1) ? line :
      gserialized_line_substring(line, posmin, posmax);
    geo_set_stbox(gs, &box);
    span_set(TimestampTzGetDatum(last->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, &box.period);
    MEOS_FLAGS_SET_T(box.flags, true);
    pfree(line);
    if (posmin != 0 || posmax != 1)
      pfree(gs);
  }
  else
  {
    tnpointinst_set_stbox(inst, &box);
  }
  /* Expand the bounding box of the sequence with the last edge */
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/**
 * @brief Return the bounding box of the network segment value
 */
bool
nsegment_set_stbox(const Nsegment *ns, STBox *box)
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
npoint_timestamp_set_stbox(const Npoint *np, TimestampTz t, STBox *box)
{
  npoint_set_stbox(np, box);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @brief Transform a network point and a period to a spatiotemporal box
 */
bool
npoint_period_set_stbox(const Npoint *np, const Span *p, STBox *box)
{
  npoint_set_stbox(np, box);
  memcpy(&box->period, p, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/*****************************************************************************/

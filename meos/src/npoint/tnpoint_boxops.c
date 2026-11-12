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
 * @brief Bounding box operators for temporal network points
 */

#include "npoint/tnpoint_boxops.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "npoint/tnpoint.h"

/*****************************************************************************
 * Transform a temporal network point to a STBox
 *****************************************************************************/

/**
 * @brief Return in the last argument the spatiotemporal box of a temporal
 * network point instant
 * @param[in] inst Temporal network point
 * @param[out] box Spatiotemporal box
 */
void
tnpointinst_set_stbox(const TInstant *inst, STBox *box)
{
  npoint_set_stbox(DatumGetNpointP(tinstant_value_p(inst)), box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from
 * an array of temporal network point instants
 * @details The spatial box is the one of the network points of the instants
 * as #npointarr_set_stbox computes it for a set, reading a route once for
 * each run of points on it, and the period goes from the first instant to
 * the last
 * @param[in] instants Temporal network point values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_step_set_stbox(TInstant **instants, int count, STBox *box)
{
  Datum *values = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    values[i] = tinstant_value_p(instants[i]);
  npointarr_set_stbox(values, count, box);
  pfree(values);
  span_set(TimestampTzGetDatum(instants[0]->t),
    TimestampTzGetDatum(instants[count - 1]->t), true, true, T_TIMESTAMPTZ,
    T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Expand the spatial extent of a box with a point
 */
static void
stbox_expand_point2d(const POINT2D *p, STBox *box)
{
  box->xmin = Min(box->xmin, p->x); box->xmax = Max(box->xmax, p->x);
  box->ymin = Min(box->ymin, p->y); box->ymax = Max(box->ymax, p->y);
  return;
}

/**
 * @brief Return in the last argument the spatial box of the stretch of a
 * route between two positions
 * @details The box is the one of the points a temporal network point
 * travelling the stretch passes as #tnpointseq_tgeompointseq_cont states
 * them: the points at the two positions, which #route_point_at locates, and
 * the vertices of the route strictly between them, whose positions are the
 * ones #route_vertex_positions gives. The route is a planar line, as
 * #ensure_valid_route tests it when the network is read, and its points are
 * read in place, from the line #route_line keeps in the ways cache, up to
 * the first vertex past the stretch
 * @param[in] rid Route identifier
 * @param[in] pos1,pos2 Positions on the route
 * @param[out] box Spatiotemporal box
 * @return False when the route is not found
 */
static bool
route_stretch_set_stbox(int64 rid, double pos1, double pos2, STBox *box)
{
  const LWLINE *line = route_line(rid);
  if (! line)
    return false;
  int32_t srid = line->srid;
  /* The routes of the table ways are planar geometries */
  assert(! FLAGS_GET_GEODETIC(line->flags));
  assert(! FLAGS_GET_Z(line->flags));
  const POINTARRAY *pa = line->points;
  double length = ptarray_length_2d(pa);
  double posmin = Min(pos1, pos2), posmax = Max(pos1, pos2);

  POINT4D p;
  route_point_at(pa, length, posmin, &p);
  stbox_set(true, false, false, srid, p.x, p.x, p.y, p.y, 0.0, 0.0, NULL,
    box);
  route_point_at(pa, length, posmax, &p);
  stbox_expand_point2d((const POINT2D *) &p, box);

  /* The position of a vertex is its distance along the route over the length
   * of the route, as #route_vertex_positions computes it. The positions do
   * not decrease along the route, so the walk stops at the first vertex at or
   * past the stretch */
  if (pa->npoints > 2 && length > 0.0)
  {
    double cum = 0.0;
    const POINT2D *p1 = getPoint2d_cp(pa, 0);
    for (uint32_t k = 1; k < pa->npoints - 1; k++)
    {
      const POINT2D *p2 = getPoint2d_cp(pa, k);
      cum += hypot(p2->x - p1->x, p2->y - p1->y);
      double pos = cum / length;
      if (pos >= posmax)
        break;
      if (pos > posmin)
        stbox_expand_point2d(p2, box);
      p1 = p2;
    }
  }
  return true;
}

/**
 * @brief Return in the last argument q spatiotemporal box constructed from
 * an array of temporal network point instants
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_linear_set_stbox(TInstant **instants, int count, STBox *box)
{
  Npoint *np = DatumGetNpointP(tinstant_value_p(instants[0]));
  int64 rid = np->rid;
  double posmin, posmax;
  posmin = posmax = np->pos;
  TimestampTz tmin = instants[0]->t, tmax = instants[count - 1]->t;
  for (int i = 1; i < count; i++)
  {
    np = DatumGetNpointP(tinstant_value_p(instants[i]));
    posmin = Min(posmin, np->pos);
    posmax = Max(posmax, np->pos);
  }

  if (! route_stretch_set_stbox(rid, posmin, posmax, box))
  {
    memset(box, 0, sizeof(STBox));
    return;
  }
  span_set(TimestampTzGetDatum(tmin), TimestampTzGetDatum(tmax),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Return in the last argument a spatiotemporal box constructed from
 * an array of temporal network point instants
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_set_stbox(TInstant **instants, int count, interpType interp,
  STBox *box)
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
  if (MEOS_FLAGS_GET_INTERP(seq->flags) != LINEAR)
    tnpointinst_set_stbox(inst, &box);
  else
  {
    const TInstant *last = TSEQUENCE_INST_N(seq, seq->count - 1);
    const Npoint *np1 = DatumGetNpointP(tinstant_value_p(last));
    const Npoint *np2 = DatumGetNpointP(tinstant_value_p(inst));
    if (! route_stretch_set_stbox(np1->rid, np1->pos, np2->pos, &box))
      return;
    span_set(TimestampTzGetDatum(last->t), TimestampTzGetDatum(inst->t),
      true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box.period);
    MEOS_FLAGS_SET_T(box.flags, true);
  }
  /* Expand the bounding box of the sequence with the last edge */
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}


/*****************************************************************************/

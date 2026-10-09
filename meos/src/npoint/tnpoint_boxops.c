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
 * @param[in] instants Temporal network point values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tnpointinstarr_step_set_stbox(TInstant **instants, int count, STBox *box)
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
 * @brief Expand the spatial extent of a box with a point
 */
static void
stbox_expand_point4d(const POINT4D *p, bool hasz, STBox *box)
{
  box->xmin = Min(box->xmin, p->x); box->xmax = Max(box->xmax, p->x);
  box->ymin = Min(box->ymin, p->y); box->ymax = Max(box->ymax, p->y);
  if (hasz)
  {
    box->zmin = Min(box->zmin, p->z); box->zmax = Max(box->zmax, p->z);
  }
  return;
}

/**
 * @brief Return in the last argument the spatial box of the stretch of a
 * route between two positions
 * @details The box is the one of the points a temporal network point
 * travelling the stretch passes as #tnpointseq_tgeompointseq_cont states
 * them: the points at the two positions, located as #npoint_to_geompoint
 * locates a position, and the vertices of the route strictly between them,
 * whose positions #route_vertex_positions gives. The points are read as
 * #npointarr_set_stbox reads them
 * @param[in] rid Route identifier
 * @param[in] pos1,pos2 Positions on the route
 * @param[out] box Spatiotemporal box
 * @return False when the route is not found
 */
static bool
route_stretch_set_stbox(int64 rid, double pos1, double pos2, STBox *box)
{
  const GSERIALIZED *gsline = route_geom(rid);
  if (! gsline)
    return false;
  int32_t srid = gserialized_get_srid(gsline);
  bool hasz = (bool) FLAGS_GET_Z(gsline->gflags);
  /* The routes of the table ways are geometries */
  assert(! FLAGS_GET_GEODETIC(gsline->gflags));
  LWLINE *line = (LWLINE *) lwgeom_from_gserialized(gsline);
  double posmin = Min(pos1, pos2), posmax = Max(pos1, pos2);

  POINT4D p;
  POINTARRAY *opa = lwline_interpolate_points(line, posmin, 0);
  getPoint4d_p(opa, 0, &p);
  ptarray_free(opa);
  stbox_set(true, hasz, false, srid, p.x, p.x, p.y, p.y,
    hasz ? p.z : 0.0, hasz ? p.z : 0.0, NULL, box);
  opa = lwline_interpolate_points(line, posmax, 0);
  getPoint4d_p(opa, 0, &p);
  ptarray_free(opa);
  stbox_expand_point4d(&p, hasz, box);

  int count;
  double *positions = route_vertex_positions(line->points, &count);
  for (int k = 0; k < count; k++)
  {
    if (positions[k] <= posmin || positions[k] >= posmax)
      continue;
    getPoint4d_p(line->points, k + 1, &p);
    stbox_expand_point4d(&p, hasz, box);
  }
  if (positions)
    pfree(positions);
  lwline_free(line);
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

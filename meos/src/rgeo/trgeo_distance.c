/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Distance functions for temporal rigid geometries
 */

#include "rgeo/trgeo_distance.h"

/* C */
#include <assert.h>
#include <c.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <stdio.h>
#include <utils/timestamp.h>
#include <utils/float.h>
/* PostGIS */
#include <liblwgeom.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "temporal/temporal_aggfuncs.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"
#include "rgeo/trgeo_utils.h"
#include "rgeo/trgeo_vclip.h"

/*****************************************************************************
 * cfp array utility functions
 *****************************************************************************/

/**
 * @brief Return a closest-feature pair for two geometries under their poses at
 * an instant
 */
static cfp_elem
cfp_make(LWGEOM *geom_1, LWGEOM *geom_2, Pose *pose_1, Pose *pose_2,
  uint32_t cf_1, uint32_t cf_2, TimestampTz t, bool store)
{
  cfp_elem cfp;
  cfp.geom_1 = geom_1;
  cfp.geom_2 = geom_2;
  cfp.pose_1 = pose_1;
  cfp.pose_2 = pose_2;
  cfp.cf_1 = cf_1;
  cfp.cf_2 = cf_2;
  cfp.t = t;
  cfp.store = store;
  cfp.free_pose_1 = MEOS_CFP_FREE_NO;
  cfp.free_pose_2 = MEOS_CFP_FREE_NO;
  return cfp;
}

/**
 * @brief Return a closest-feature pair whose two features are the first of each
 * geometry
 */
static inline cfp_elem
cfp_make_zero(LWGEOM *geom_1, LWGEOM *geom_2, Pose *pose_1, Pose *pose_2,
  TimestampTz t, bool store)
{
  return cfp_make(geom_1, geom_2, pose_1, pose_2, 0, 0, t, store);
}

/**
 * @brief Initialize an array of closest-feature pairs with a starting capacity
 */
static void
init_cfp_array(cfp_array *cfpa, size_t n)
{
  cfpa->arr = palloc0(sizeof(cfp_elem) * n);
  cfpa->count = 0;
  cfpa->size = n;
}

/**
 * @brief Free an array of closest-feature pairs and the poses it owns
 */
static void
free_cfp_array(cfp_array *cfpa)
{
  for (uint32_t i = 0; i < cfpa->count; ++i)
  {
    if (cfpa->arr[i].free_pose_1)
      pfree(cfpa->arr[i].pose_1);
    if (cfpa->arr[i].free_pose_2)
      pfree(cfpa->arr[i].pose_2);
  }
  pfree(cfpa->arr);
}

/**
 * @brief Append a closest-feature pair to its array, growing the array as needed
 */
static void
append_cfp_elem(cfp_array *cfpa, cfp_elem cfp)
{
  if (cfpa->count == cfpa->size)
  {
    cfpa->size *= 2;
    cfp_elem *new_arr = repalloc(cfpa->arr, sizeof(cfp_elem) * cfpa->size);
    if (new_arr == NULL)
    {
      /* See doc-comment on meos_error in meos/include/meos.h: handler is
       * not guaranteed to abort. Restore the size field and bail before
       * the OOB write at cfpa->arr[cfpa->count++] below -- repalloc
       * failure leaves cfpa->arr pointing at the OLD (now too-small)
       * buffer relative to the bumped cfpa->size. */
      cfpa->size /= 2;
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Not enough memory");
      return;
    }
    cfpa->arr = new_arr;
  }
  cfpa->arr[cfpa->count++] = cfp;
}

/**
 * @brief Return a distance point, that is a distance and the time it holds at
 */
static tdist_elem
tdist_make(double dist, TimestampTz t)
{
  tdist_elem td;
  td.dist = dist;
  td.t = t;
  return td;
}

/**
 * @brief Initialize an array of distance points with a starting capacity
 */
static void
init_tdist_array(tdist_array *tda, size_t n)
{
  tda->arr = palloc0(sizeof(tdist_elem) * n);
  tda->count = 0;
  tda->size = n;
}

/**
 * @brief Free the distance points held by an array
 */
static inline void
free_tdist_array(tdist_array *tda)
{
  pfree(tda->arr);
}

/**
 * @brief Append a distance point to its array, growing the array as needed
 */
static void
append_tdist_elem(tdist_array *tda, tdist_elem td)
{
  if (tda->count == tda->size)
  {
    tda->size *= 2;
    tdist_elem *new_arr = repalloc(tda->arr, sizeof(tdist_elem) * tda->size);
    if (new_arr == NULL)
    {
      /* See doc-comment on meos_error in meos/include/meos.h: handler is
       * not guaranteed to abort. Restore the size field and bail before
       * the OOB write at tda->arr[tda->count++] below (same pattern as
       * append_cfp_elem above). */
      tda->size /= 2;
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Not enough memory");
      return;
    }
    tda->arr = new_arr;
  }
  tda->arr[tda->count++] = td;
}

/**
 * @brief Comparator ordering distance elements by their timestamp
 */
static int
tdist_elem_cmp(const void *a, const void *b)
{
  TimestampTz ta = ((const tdist_elem *) a)->t;
  TimestampTz tb = ((const tdist_elem *) b)->t;
  return (ta < tb) ? -1 : ((ta > tb) ? 1 : 0);
}

/**
 * @brief Sort the distance elements by time and drop duplicate timestamps,
 * keeping the smallest distance so that a coincident kink and extremum collapse
 * to the true value
 */
static void
tdist_array_sort(tdist_array *tda)
{
  if (tda->count < 2)
    return;
  qsort(tda->arr, tda->count, sizeof(tdist_elem), &tdist_elem_cmp);
  uint32_t w = 0;
  for (uint32_t r = 1; r < tda->count; ++r)
  {
    if (tda->arr[r].t == tda->arr[w].t)
    {
      if (tda->arr[r].dist < tda->arr[w].dist)
        tda->arr[w].dist = tda->arr[r].dist;
    }
    else
      tda->arr[++w] = tda->arr[r];
  }
  tda->count = w + 1;
}

/*****************************************************************************
 * V-clip
 *****************************************************************************/

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Return the temporal distance between a temporal rigid geometry instant
 * and a geometry
 */
TInstant *
dist2d_trgeoinst_geo(const TInstant *inst, const GSERIALIZED *gs)
{
  /* The reference geometry is the body at the origin; the distance is measured
   * from the body placed by the pose of the instant */
  GSERIALIZED *body = pose_apply_geo(DatumGetPoseP(tinstant_value_p(inst)),
    trgeoinst_geom_p(inst));
  double dist = geom_distance2d(body, gs);
  pfree(body);
  return tinstant_make(Float8GetDatum(dist), T_TFLOAT, inst->t);
}

/**
 * @brief Interpolate the position and the rotation of a pose segment at a ratio
 */
static void
pose_interpolate_2d(Pose *pose1, Pose *pose2, double ratio, double *x,
  double *y, double *theta)
{
  assert(0 <= ratio && ratio <= 1);
  *x = pose1->data[0] * (1 - ratio) + pose2->data[0] * ratio;
  *y = pose1->data[1] * (1 - ratio) + pose2->data[1] * ratio;
  double theta_delta = pose2->data[2] - pose1->data[2];
  /* If fabs(theta_delta) == M_PI: Always turn counter-clockwise */
  if (fabs(theta_delta) < MEOS_EPSILON)
    *theta = pose1->data[2];
  else if (theta_delta > 0 && fabs(theta_delta) <= M_PI)
    *theta = pose1->data[2] + theta_delta*ratio;
  else if (theta_delta > 0 && fabs(theta_delta) > M_PI)
    *theta = pose2->data[2] + (2*M_PI - theta_delta)*(1 - ratio);
  else if (theta_delta < 0 && fabs(theta_delta) < M_PI)
    *theta = pose1->data[2] + theta_delta*ratio;
  else /* (theta_delta < 0 && fabs(theta_delta) >= M_PI) */
    *theta = pose1->data[2] + (2*M_PI + theta_delta)*ratio;
  if (*theta > M_PI)
    *theta = *theta - 2*M_PI;
}


/**
 * @brief Interpolate at @p ratio the pose of the first rigid geometry
 * expressed in the moving frame of the second one
 * @details A rigid transformation preserves distances, so evaluating the
 * first polygon at this relative pose against the second polygon taken raw
 * (static in its own frame) reproduces the exact distance between the two
 * moving polygons. This lets the moving-vs-static machinery serve the
 * moving-vs-moving case unchanged: only the pose fed to the first polygon
 * becomes relative. When @p pose2_s is @p NULL the second geometry is static
 * and this reduces to @ref pose_interpolate_2d on the first pose.
 */
static void
rel_pose_interpolate_2d(Pose *pose1_s, Pose *pose1_e, Pose *pose2_s,
  Pose *pose2_e, double ratio, double *x, double *y, double *theta)
{
  if (! pose2_s)
  {
    pose_interpolate_2d(pose1_s, pose1_e, ratio, x, y, theta);
    return;
  }
  double x1, y1, th1, x2, y2, th2;
  pose_interpolate_2d(pose1_s, pose1_e, ratio, &x1, &y1, &th1);
  pose_interpolate_2d(pose2_s, pose2_e, ratio, &x2, &y2, &th2);
  /* Relative pose = pose2(ratio)^-1 o pose1(ratio):
   * rotation  = th1 - th2
   * translation = R(-th2) . (T1 - T2) */
  double co = cos(th2), si = sin(th2);
  double ex = x1 - x2, ey = y1 - y2;
  *x = ex * co + ey * si;
  *y = - ex * si + ey * co;
  *theta = th1 - th2;
  if (*theta > M_PI)
    *theta -= 2 * M_PI;
  else if (*theta <= - M_PI)
    *theta += 2 * M_PI;
}

/**
 * @brief Return the pose of the first rigid geometry in the moving frame of
 * the second one at @p ratio (see @ref rel_pose_interpolate_2d); the caller
 * owns the result. When @p pose2_s is @p NULL this is
 * @ref posesegm_interpolate on the first pose.
 */
static Pose *
rel_posesegm_interpolate(Pose *pose1_s, Pose *pose1_e, Pose *pose2_s,
  Pose *pose2_e, double ratio)
{
  if (! pose2_s)
    return posesegm_interpolate(pose1_s, pose1_e, ratio);
  double x, y, theta;
  rel_pose_interpolate_2d(pose1_s, pose1_e, pose2_s, pose2_e, ratio, &x, &y,
    &theta);
  return pose_make_2d(x, y, theta, MEOS_FLAGS_GET_GEODETIC(pose1_s->flags),
    pose_srid(pose1_s));
}

/**
 * @brief Return, at a ratio of a segment, the function whose root is a transition
 * of the closest feature between a fixed point and a rotating polygon edge
 */
static double
f_tpoint_poly(POINT4D p, POINT4D q, POINT4D r, Pose *poly_pose_s,
  Pose *poly_pose_e, double ratio, bool solution_kind)
{
  double dx, dy, dtheta;
  double co, si, qx, qy, rx, ry;
  pose_interpolate_2d(poly_pose_s, poly_pose_e, ratio, &dx, &dy, &dtheta);
  co = cos(dtheta);
  si = sin(dtheta);
  qx = q.x * co - q.y * si + dx;
  qy = q.x * si + q.y * co + dy;
  rx = r.x * co - r.y * si + dx;
  ry = r.x * si + r.y * co + dy;
  if (solution_kind) /* MEOS_SOLVE_0 */
    return (p.x - qx) * (rx - qx) + (p.y - qy) * (ry - qy);
  else /* MEOS_SOLVE_1 */
    return (p.x - rx) * (rx - qx) + (p.y - ry) * (ry - qy);
}

/**
 * @brief Return @p t if it is a closest-feature transition of the segment,
 * return 2 otherwise
 * @details A transition counts only when it advances strictly past the
 * previous one and lies strictly inside the segment. Both the closed-form and
 * the iterative branch of every solver answer through this function so that
 * they agree on what a transition is: the iterative branch leaves its root at
 * the value it was initialized with when the bracket is already narrower than
 * @p MEOS_EPSILON, which happens once the walk comes within an epsilon of the
 * segment end, and such a value reenters the walk as a transition at the
 * segment start.
 * @param[in] t Candidate ratio
 * @param[in] prev_result Ratio of the previous transition of the segment
 */
static inline double
transition_ratio(double t, double prev_result)
{
  return (t > prev_result + MEOS_EPSILON && t < 1 - MEOS_EPSILON) ? t : 2;
}

/**
 * @brief Return the ratio at which the closest feature between a fixed point and a
 * rotating polygon transitions across an end of an edge
 */
static double
solve_s_tpoly_point(LWPOLY *poly, LWPOINT *point, Pose *poly_pose_s,
  Pose *poly_pose_e, uint32_t poly_v, double prev_result, bool solution_kind)
{
  uint32_t n = poly->rings[0]->npoints - 1;
  POINT4D p, q, r;
  lwpoint_getPoint4d_p(point, &p);
  getPoint4d_p(poly->rings[0], poly_v, &q);
  getPoint4d_p(poly->rings[0], uint_mod_add(poly_v, 1, n), &r);

/*  if (solution_kind)
    printf("s(t) = 0; p = (%lf, %lf), q = (%lf, %lf), r = (%lf, %lf), \npose_1 = (%lf, %lf, %lf), pose_2 = (%lf, %lf, %lf)\n",
      p.x, p.y, q.x, q.y, r.x, r.y,
      poly_pose_s->data[0], poly_pose_s->data[1], poly_pose_s->data[2],
      poly_pose_e->data[0], poly_pose_e->data[1], poly_pose_e->data[2]);
  else
    printf("s(t) = 1; p = (%lf, %lf), q = (%lf, %lf), r = (%lf, %lf), \npose_1 = (%lf, %lf, %lf), pose_2 = (%lf, %lf, %lf)\n",
      p.x, p.y, q.x, q.y, r.x, r.y,
      poly_pose_s->data[0], poly_pose_s->data[1], poly_pose_s->data[2],
      poly_pose_e->data[0], poly_pose_e->data[1], poly_pose_e->data[2]);
  fflush(stdout);*/

  if (fabs(poly_pose_s->data[2] - poly_pose_e->data[2]) < MEOS_EPSILON)
  {
    apply_pose_point4d(&q, poly_pose_s);
    apply_pose_point4d(&r, poly_pose_s);
    double result;
    double discr = ((poly_pose_e->data[0] - poly_pose_s->data[0]) * (r.x - q.x)
      + (poly_pose_e->data[1] - poly_pose_s->data[1]) * (r.y - q.y));
    if (solution_kind) /* MEOS_SOLVE_0 */
      result = ((p.x - q.x) * (r.x - q.x) + (p.y - q.y) * (r.y - q.y)) / discr;
    else /* MEOS_SOLVE_1 */
      result = ((p.x - r.x) * (r.x - q.x) + (p.y - r.y) * (r.y - q.y)) / discr;
    return transition_ratio(result, prev_result);
  }

  double tl, tr, t0 = 0; /* Make compiler quiet */
  double vl, vr, v0;
  double ts = prev_result, te = 1;
  vl = f_tpoint_poly(p, q, r, poly_pose_s, poly_pose_e,
    ts, solution_kind);
  v0 = f_tpoint_poly(p, q, r, poly_pose_s, poly_pose_e,
    (ts + te) / 2, solution_kind);
  vr = f_tpoint_poly(p, q, r, poly_pose_s, poly_pose_e,
    te, solution_kind);
  if (fabs(vl) > MEOS_EPSILON && vl * v0 < 0)
  {
    tl = ts;
    tr = (ts + te) / 2;
    vr = v0;
  }
  else if (v0 * vr < 0)
  {
    tl = (ts + te) / 2;
    tr = te;
    vl = v0;
  }
  else
    return 2;

  uint8_t i = 0;
  while(fabs(tr - tl) >= MEOS_EPSILON && i < 100)
  {
    ++i;
    t0 = (tl * vr - tr * vl) / (vr - vl);
    v0 = f_tpoint_poly(p, q, r, poly_pose_s, poly_pose_e,
      t0, solution_kind);
    if (fabs(v0) < MEOS_EPSILON)
      break;
    if (vl * v0 <= 0)
      tr = t0, vr = v0;
    else
      tl = t0, vl = v0;
  }
  return transition_ratio(t0, prev_result);
}

/**
 * @brief Return the sentinel reporting no transition, the crossing of an edge line
 * contributing none for a point target
 */
static double
solve_angle_0_tpoly_point(LWPOLY *poly UNUSED,
  LWPOINT *point UNUSED,
  Pose *poly_pose_s UNUSED,
  Pose *poly_pose_e UNUSED,
  uint32_t poly_v UNUSED,
  double r_prev UNUSED)
{
  return 2;
}

/* Forward declaration: defined with the sequence-set distance helpers below */
static int trgeoseq_segment_index(const TSequence *seq, TimestampTz t);

/**
 * @brief Append the time at which a translating polygon reaches a point it
 * meets, where that time carries no point of its own
 * @details The twin of #compute_contact_tpoly_poly for a point target: the
 * oracle answers the separation while the point stands outside the polygon
 * and zero once it is inside, so the same kink stands between the two and the
 * same fit through two readings taken while they stand apart gives its time
 * @param[in] geom_1,geom_2 Reference polygon and target point
 * @param[in] pose_s,pose_e Poses at the ends of the temporal segment
 * @param[in] t_lo,t_hi Ends of the temporal segment
 * @param[in] ta,tb Ends of the interval, @p ta standing apart
 * @param[in] da Separation at @p ta
 * @param[in,out] tda Distance points to append to
 */
static void
compute_contact_tpoly_point(const LWGEOM *geom_1, const LWGEOM *geom_2,
  const Pose *pose_s, const Pose *pose_e, TimestampTz t_lo, TimestampTz t_hi,
  TimestampTz ta, TimestampTz tb, double da, tdist_array *tda)
{
  if (t_hi <= t_lo || tb <= ta || da <= 0.0)
    return;
  if (fabs(pose_e->data[2] - pose_s->data[2]) > MEOS_GEOM_TOLERANCE)
    return;

  double span = (double) (t_hi - t_lo);
  double fa = (double) (ta - t_lo) / span;
  double fb = (double) (tb - t_lo) / span;

  double fm = 0.0, dm = 0.0;
  for (double frac = 0.5; frac > 1e-9; frac *= 0.5)
  {
    double f = fa + (fb - fa) * frac;
    Pose *pm = posesegm_interpolate(pose_s, pose_e, f);
    uint32_t cf = 0;
    double d;
    v_clip_tpoly_point((LWPOLY *) geom_1, (LWPOINT *) geom_2, pm, &cf, &d);
    pfree(pm);
    if (d > 0.0)
    {
      fm = f; dm = d;
      break;
    }
  }
  if (dm <= 0.0 || dm >= da)
    return;

  double froot = fa + (fm - fa) * da / (da - dm);
  TimestampTz tc = t_lo + (TimestampTz) llround(froot * span);
  if (tc <= ta || tc >= tb)
    return;
  tdist_elem td = tdist_make(0.0, tc);
  append_tdist_elem(tda, td);
  return;
}

static void
compute_dist_tpoly_point(cfp_elem *cfp, tdist_array *tda)
{
  /* Take the distance from the v-clip oracle on the pose stored in the feature
   * pair: it re-establishes the true closest feature (robust to any
   * mis-tracking in the walk) and returns zero when the point is inside */
  uint32_t cf = 0;
  double dist;
  v_clip_tpoly_point((LWPOLY *) cfp->geom_1, (LWPOINT *) cfp->geom_2,
    cfp->pose_1, &cf, &dist);
  tdist_elem td = tdist_make(dist, cfp->t);
  append_tdist_elem(tda, td);
}

/**
 * @brief Append the interior turning points (local extrema) of the distance
 * between the moving rigid geometry and the point realized by the fixed
 * closest-feature pair of @p cfp_s over the temporal segment @p [t_lo,t_hi]
 * @details The distance of a fixed feature pair is smooth in the ratio, so its
 * extrema are the roots of its derivative: the derivative is bracketed on a
 * subdivision and each root refined to machine precision, and the exact
 * distance there (from the v-clip oracle on the interpolated pose) is emitted
 * as a turning point of the tfloat.
 */
static void
compute_turnpoints_tpoly_point(const cfp_elem *cfp_s, const cfp_elem *cfp_e,
  const Pose *pose_s, const Pose *pose_e, TimestampTz t_lo, TimestampTz t_hi,
  tdist_array *tda)
{
  const LWPOLY *poly = (const LWPOLY *) cfp_s->geom_1;
  const LWPOINT *point = (const LWPOINT *) cfp_s->geom_2;
  double span = (double) (t_hi - t_lo);
  if (span <= 0)
    return;
  double ga = (double) (cfp_s->t - t_lo) / span;
  double gb = (double) (cfp_e->t - t_lo) / span;
  if (gb - ga < MEOS_EPSILON)
    return;

  /* Distance of the closest feature pair at segment ratio g */
  #define TP_DIST(g) __extension__ ({ \
    Pose *_pp = posesegm_interpolate(pose_s, pose_e, (g)); \
    uint32_t _c = 0; double _d; \
    v_clip_tpoly_point(poly, point, _pp, &_c, &_d); \
    pfree(_pp); _d; })

  /* Locate the interior turning points (local minima and maxima) of the
   * distance over [ga, gb]. The distance is smooth within a fixed feature
   * pair, but it drops to a flat zero over any sub-interval where the bodies
   * overlap, so a plain derivative-sign-change bracket misses that minimum: a
   * run of zero-derivative samples inside the plateau separates the falling
   * and rising samples. Track the slope sign (falling / flat / rising) on a
   * uniform subdivision and emit a turning point at every falling->rising and
   * falling->flat and flat->rising transition (a minimum, including a plateau
   * boundary) and every rising->falling and rising->flat and flat->falling
   * transition (a maximum), refining each by golden section on the value. */
  int M = 64;
  double step = (gb - ga) / M;
  double d0 = TP_DIST(ga), d1 = TP_DIST(ga + step);
  double gprev = ga + step, dprev = d1;
  int sprev = (d1 > d0 + MEOS_EPSILON) ? 1 : (d1 < d0 - MEOS_EPSILON ? -1 : 0);
  for (int k = 2; k <= M; ++k)
  {
    double gcur = ga + step * k, dcur = TP_DIST(gcur);
    int scur = (dcur > dprev + MEOS_EPSILON) ? 1 :
      (dcur < dprev - MEOS_EPSILON ? -1 : 0);
    bool is_min = (sprev < 0 && scur >= 0) || (sprev == 0 && scur > 0);
    bool is_max = (sprev > 0 && scur <= 0) || (sprev == 0 && scur < 0);
    if (is_min || is_max)
    {
      /* Golden-section refine the extremum in [gprev, gcur] */
      const double gr = 0.6180339887498949;
      double lo = gprev, hi = gcur;
      double gc = hi - gr * (hi - lo), ge = lo + gr * (hi - lo);
      double fc = TP_DIST(gc), fe = TP_DIST(ge);
      for (int it = 0; it < 60 && hi - lo > 1e-15; ++it)
      {
        bool pick_left = is_min ? (fc < fe) : (fc > fe);
        if (pick_left)
        { hi = ge; ge = gc; fe = fc; gc = hi - gr * (hi - lo); fc = TP_DIST(gc); }
        else
        { lo = gc; gc = ge; fc = fe; ge = lo + gr * (hi - lo); fe = TP_DIST(ge); }
      }
      double gstar = 0.5 * (lo + hi);
      tdist_elem td = tdist_make(TP_DIST(gstar),
        t_lo + (TimestampTz) (span * gstar));
      append_tdist_elem(tda, td);
    }
    gprev = gcur;
    dprev = dcur;
    sprev = scur;
  }
  #undef TP_DIST
}

/**
 * @brief Find the next change in closest feature
 */
static int
vertex_vertex_tpoly_point(LWPOLY *poly, Pose *pose_start, Pose *pose_end,
  LWPOINT *point, uint32_t *poly_feature, int *direction, double *ratio)
{
  uint32_t n = poly->rings[0]->npoints - 1;
  uint32_t i = *poly_feature / 2;
  /* Detect next change in closest feature */
  double ratio_1 = 2, ratio_2 = 2;
  if (*direction == MEOS_RIGHT || *direction == MEOS_ANY)
    ratio_1 = solve_s_tpoly_point(poly, point, pose_start, pose_end,
      i, *ratio, MEOS_SOLVE_0);
  if (*direction == MEOS_LEFT || *direction == MEOS_ANY)
    ratio_2 = solve_s_tpoly_point(poly, point, pose_start, pose_end,
      uint_mod_sub(i, 1, n), *ratio, MEOS_SOLVE_1);

  /* No change in closest feature */
  if (ratio_1 == 2 && ratio_2 == 2)
    return MEOS_DISJOINT;
  /* Intersection through vertex */
  else if (fabs(ratio_1 - ratio_2) < MEOS_EPSILON)
    return MEOS_INTERSECT;
  /* Go to next closest feature */
  else if (ratio_1 < ratio_2)
  {
    *direction = MEOS_RIGHT;
    *poly_feature = uint_mod_add(*poly_feature, 1, 2 * n);
    *ratio = ratio_1;
    return MEOS_CONTINUE;
  }
  /* Go to previous closest feature */
  else if (ratio_2 < ratio_1)
  {
    *direction = MEOS_LEFT;
    *poly_feature = uint_mod_sub(*poly_feature, 1, 2 * n);
    *ratio = ratio_2;
    return MEOS_CONTINUE;
  }
  /* Cannot happen */
  assert(false);
  return MEOS_DISJOINT;
}

/**
 * @brief Find the next change in closest feature
 */
static int
edge_vertex_tpoly_point(LWPOLY *poly, Pose *pose_start, Pose *pose_end,
  LWPOINT *point, uint32_t *poly_feature, int *direction, double *ratio)
{
  uint32_t n = poly->rings[0]->npoints - 1;
  uint32_t i = *poly_feature / 2;
  /* Detect next change in closest feature */
  double ratio_1 = 2, ratio_2 = 2;
  if (*direction == MEOS_RIGHT || *direction == MEOS_ANY)
    ratio_1 = solve_s_tpoly_point(poly, point, pose_start, pose_end,
      i, *ratio, MEOS_SOLVE_1);
  if (*direction == MEOS_LEFT || *direction == MEOS_ANY)
    ratio_2 = solve_s_tpoly_point(poly, point, pose_start, pose_end,
      i, *ratio, MEOS_SOLVE_0);
  /* Detect intersection with the edge */
  double ratio_inter = solve_angle_0_tpoly_point(poly, point, pose_start,
    pose_end, i, *ratio);

  /* Intersection through edge */
  if (ratio_inter < ratio_1 && ratio_inter < ratio_2)
    return MEOS_INTERSECT;
  /* No change in closest feature */
  else if (ratio_1 == 2 && ratio_2 == 2)
    return MEOS_DISJOINT;
  /* Go to next closest feature */
  else if (ratio_1 < ratio_2)
  {
    *direction = MEOS_RIGHT;
    *poly_feature = uint_mod_add(*poly_feature, 1, 2 * n);
    *ratio = ratio_1;
    return MEOS_CONTINUE;
  }
  /* Go to previous closest feature */
  else if (ratio_2 < ratio_1)
  {
    *direction = MEOS_LEFT;
    *poly_feature = uint_mod_sub(*poly_feature, 1, 2 * n);
    *ratio = ratio_2;
    return MEOS_CONTINUE;
  }
  /* Cannot happen */
  assert(false);
  return MEOS_DISJOINT;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry sequence
 * and a point
 */
TSequence *
dist2d_trgeoseq_point(const TSequence *seq, const GSERIALIZED *gs,
  const GSERIALIZED *ref_gs)
{
  /* TODO: Add check and code for stepwise seq */

  /* TODO: check that polygon is convex */
  LWPOLY *poly = lwgeom_as_lwpoly(lwgeom_from_gserialized(ref_gs));
  LWPOINT *point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));

  const TInstant *inst1, *inst2;
  Pose *pose1, *pose2;

  inst1 = TSEQUENCE_INST_N(seq, 0);
  pose1 = DatumGetPoseP(tinstant_value_p(inst1));

  /* Compute the initial closest features */
  cfp_array cfpa;
  init_cfp_array(&cfpa, seq->count);
  cfp_elem cfp = cfp_make_zero((LWGEOM *)poly, (LWGEOM *)point,
    pose1, NULL, inst1->t, MEOS_CFP_STORE);
  v_clip_tpoly_point(poly, point, pose1, &cfp.cf_1, NULL);
  append_cfp_elem(&cfpa, cfp);
  for (int i = 0; i < seq->count - 1; ++i)
  {
    /* TODO: optimise using simple checks, such as:
     * 1) cfp(0) == cfp(0.5) == cfp(1) -> no change in cf
     */
    inst1 = TSEQUENCE_INST_N(seq, i);
    inst2 = TSEQUENCE_INST_N(seq, i + 1);
    pose1 = DatumGetPoseP(tinstant_value_p(inst1));
    pose2 = DatumGetPoseP(tinstant_value_p(inst2));
    double ratio = 0.0;
    int loop = 0, state, direction = MEOS_ANY;
    /* Compute the evolution of closest features for this segment */
    do
    {
      if (cfp.cf_1 % 2 == 0) /* poly_feature is a vertex */
        state = vertex_vertex_tpoly_point(poly, pose1, pose2, point,
          &cfp.cf_1, &direction, &ratio);
      else /* poly_feature is an edge */
        state = edge_vertex_tpoly_point(poly, pose1, pose2, point,
          &cfp.cf_1, &direction, &ratio);

      if (state == MEOS_CONTINUE)
      {
        cfp.t = inst1->t + (inst2->t - inst1->t) * ratio;
        cfp.pose_1 = posesegm_interpolate(pose1, pose2, ratio);
        cfp.free_pose_1 = MEOS_CFP_FREE;
        cfp.store = MEOS_CFP_STORE_NO;
        append_cfp_elem(&cfpa, cfp);
      }

      if (loop++ == MEOS_MAX_ITERS) break;

    } while (state == MEOS_CONTINUE);

    if (loop > MEOS_MAX_ITERS)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Temporal distance: Cycle detected, current feature: %d", cfp.cf_1);
      return NULL;
    }

    cfp.pose_1 = pose2;
    cfp.free_pose_1 = MEOS_CFP_FREE_NO;
    cfp.t = inst2->t;
    cfp.store = MEOS_CFP_STORE;
    cfp_elem next_cfp = cfp;
    v_clip_tpoly_point(poly, point, pose2, &next_cfp.cf_1, NULL);
    append_cfp_elem(&cfpa, next_cfp);
    cfp = next_cfp;
  }

  /* Compute the piecewise-linear distance from the array of closest features:
   * a point at every closest-feature time (segment ends and transition kinks)
   * plus the interior extrema of each fixed feature pair */
  tdist_array tda;
  init_tdist_array(&tda, cfpa.count);
  for (uint32_t i = 0; i < cfpa.count - 1; ++i)
  {
    compute_dist_tpoly_point(&cfpa.arr[i], &tda);
    int s = trgeoseq_segment_index(seq,
      cfpa.arr[i].t + (cfpa.arr[i + 1].t - cfpa.arr[i].t) / 2);
    const TInstant *si = TSEQUENCE_INST_N(seq, s);
    const TInstant *ei = TSEQUENCE_INST_N(seq, s + 1);
    compute_turnpoints_tpoly_point(&cfpa.arr[i], &cfpa.arr[i + 1],
      DatumGetPoseP(tinstant_value_p(si)), DatumGetPoseP(tinstant_value_p(ei)),
      si->t, ei->t, &tda);
  }
  compute_dist_tpoly_point(&cfpa.arr[cfpa.count-1], &tda);

  /* Order the distance points by time before building the result */
  tdist_array_sort(&tda);

  /* Where two neighbouring points stand on either side of the floor the
   * oracle imposes, the join between them is a kink, and the time the
   * separation reaches zero is a point of the answer */
  uint32_t nbefore = tda.count;
  for (uint32_t i = 0; i + 1 < nbefore; ++i)
  {
    double d1 = tda.arr[i].dist, d2 = tda.arr[i + 1].dist;
    if ((d1 > 0.0) == (d2 > 0.0))
      continue;
    int sgi = trgeoseq_segment_index(seq,
      tda.arr[i].t + (tda.arr[i + 1].t - tda.arr[i].t) / 2);
    const TInstant *ssi = TSEQUENCE_INST_N(seq, sgi);
    const TInstant *sei = TSEQUENCE_INST_N(seq, sgi + 1);
    if (d1 > 0.0)
      compute_contact_tpoly_point((LWGEOM *) poly, (LWGEOM *) point,
        DatumGetPoseP(tinstant_value_p(ssi)),
        DatumGetPoseP(tinstant_value_p(sei)), ssi->t, sei->t,
        tda.arr[i].t, tda.arr[i + 1].t, d1, &tda);
    else
      compute_contact_tpoly_point((LWGEOM *) poly, (LWGEOM *) point,
        DatumGetPoseP(tinstant_value_p(sei)),
        DatumGetPoseP(tinstant_value_p(ssi)), sei->t, ssi->t,
        tda.arr[i + 1].t, tda.arr[i].t, d2, &tda);
  }
  if (tda.count != nbefore)
    tdist_array_sort(&tda);

  /* Create the result tfloat */
  TInstant **instants = palloc(sizeof(TInstant *) * tda.count);
  for (uint32_t i = 0; i < tda.count; ++i)
    instants[i] = tinstant_make(Float8GetDatum(tda.arr[i].dist), T_TFLOAT,
      tda.arr[i].t);
  TSequence *result = tsequence_make_free(instants, tda.count,
    seq->period.lower_inc, seq->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);

  lwpoly_free(poly);
  lwpoint_free(point);
  free_cfp_array(&cfpa);
  free_tdist_array(&tda);
  return result;
}

/**
 * @brief Return, at a ratio of a segment, the function whose root is a transition
 * of the closest feature between a fixed polygon and a rotating one
 */
static double
f_tpoly_poly(POINT4D p, POINT4D q, POINT4D r, Pose *poly_pose_s,
  Pose *poly_pose_e, Pose *poly2_pose_s, Pose *poly2_pose_e, double ratio,
  bool solution_kind)
{
  double dx, dy, dtheta;
  double co, si, qx, qy, rx, ry;
  rel_pose_interpolate_2d(poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    ratio, &dx, &dy, &dtheta);
  co = cos(dtheta);
  si = sin(dtheta);
  qx = q.x * co - q.y * si + dx;
  qy = q.x * si + q.y * co + dy;
  rx = r.x * co - r.y * si + dx;
  ry = r.x * si + r.y * co + dy;
  if (solution_kind) /* MEOS_SOLVE_0 */
    return (p.x - qx) * (rx - qx) + (p.y - qy) * (ry - qy);
  else /* MEOS_SOLVE_1 */
    return (p.x - rx) * (rx - qx) + (p.y - ry) * (ry - qy);
}

/**
 * @brief Return the ratio at which the closest feature between a fixed polygon and
 * a rotating one transitions across an end of an edge of the rotating one
 */
static double
solve_s_tpoly_poly(LWPOLY *poly1, Pose *poly_pose_s, Pose *poly_pose_e,
  Pose *poly2_pose_s, Pose *poly2_pose_e, LWPOLY *poly2, uint32_t poly1_v,
  uint32_t poly2_v, double prev_result, bool solution_kind)
{
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  POINT4D p, q, r;
  getPoint4d_p(poly2->rings[0], poly2_v, &p);
  getPoint4d_p(poly1->rings[0], poly1_v, &q);
  getPoint4d_p(poly1->rings[0], uint_mod_add(poly1_v, 1, n1), &r);

  /* The closed-form shortcut assumes the second polygon is static; with both
   * polygons moving the relative motion is not a linear pose segment, so fall
   * through to the general root find. */
  if (! poly2_pose_s &&
      fabs(poly_pose_s->data[2] - poly_pose_e->data[2]) < MEOS_EPSILON)
  {
    apply_pose_point4d(&q, poly_pose_s);
    apply_pose_point4d(&r, poly_pose_s);
    double result;
    double discr = (poly_pose_e->data[0] - poly_pose_s->data[0]) * (r.x - q.x)
                 + (poly_pose_e->data[1] - poly_pose_s->data[1]) * (r.y - q.y);
    if (fabs(discr) < MEOS_EPSILON)
      return 2;
    if (solution_kind) /* MEOS_SOLVE_0 */
      result = ((p.x - q.x) * (r.x - q.x) + (p.y - q.y) * (r.y - q.y)) / discr;
    else /* MEOS_SOLVE_1 */
      result = ((p.x - r.x) * (r.x - q.x) + (p.y - r.y) * (r.y - q.y)) / discr;
    return transition_ratio(result, prev_result);
  }

  double tl, tr, t0 = 0; /* Make compiler quiet */
  double vl, vr, v0;
  double ts = prev_result, te = 1;
  vl = f_tpoly_poly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    ts, solution_kind);
  v0 = f_tpoly_poly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    (ts + te) / 2, solution_kind);
  vr = f_tpoly_poly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    te, solution_kind);
  if (fabs(vl) > MEOS_EPSILON && vl * v0 < 0)
  {
    tl = ts;
    tr = (ts + te) / 2;
    vr = v0;
  }
  else if (v0 * vr < 0)
  {
    tl = (ts + te) / 2;
    tr = te;
    vl = v0;
  }
  else
    return 2;

  uint8_t i = 0;
  while(fabs(tr - tl) >= MEOS_EPSILON && i < 100)
  {
    ++i;
    t0 = (tl * vr - tr * vl) / (vr - vl);
    v0 = f_tpoly_poly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
      t0, solution_kind);
    if (fabs(v0) < MEOS_EPSILON)
      break;
    if (vl * v0 <= 0)
      tr = t0, vr = v0;
    else
      tl = t0, vl = v0;
  }
  return transition_ratio(t0, prev_result);
}

/**
 * @brief Return, at a ratio of a segment, the function whose root is a transition
 * of the closest feature between a rotating polygon and a fixed one
 */
static double
f_poly_tpoly(POINT4D p, POINT4D q, POINT4D r, Pose *poly_pose_s,
  Pose *poly_pose_e, Pose *poly2_pose_s, Pose *poly2_pose_e, double ratio,
  bool solution_kind)
{
  double dx, dy, dtheta;
  double co, si, px, py;
  rel_pose_interpolate_2d(poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    ratio, &dx, &dy, &dtheta);
  co = cos(dtheta);
  si = sin(dtheta);
  px = p.x * co - p.y * si + dx;
  py = p.x * si + p.y * co + dy;
  if (solution_kind) /* MEOS_SOLVE_0 */
    return (px - q.x) * (r.x - q.x) + (py - q.y) * (r.y - q.y);
  else /* MEOS_SOLVE_1 */
    return (px - r.x) * (r.x - q.x) + (py - r.y) * (r.y - q.y);
}

/**
 * @brief Return the ratio at which the closest feature between a rotating polygon
 * and a fixed one transitions across an end of an edge of the fixed one
 */
static double
solve_s_poly_tpoly(LWPOLY *poly1, LWPOLY *poly2, Pose *poly_pose_s,
  Pose *poly_pose_e, Pose *poly2_pose_s, Pose *poly2_pose_e, uint32_t poly1_v,
  uint32_t poly2_v, double prev_result, bool solution_kind)
{
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  POINT4D p, q, r;
  getPoint4d_p(poly2->rings[0], poly2_v, &p);
  getPoint4d_p(poly1->rings[0], poly1_v, &q);
  getPoint4d_p(poly1->rings[0], uint_mod_add(poly1_v, 1, n1), &r);

  /* The closed-form shortcut assumes the second polygon is static; with both
   * polygons moving the relative motion is not a linear pose segment, so fall
   * through to the general root find. */
  if (! poly2_pose_s &&
      fabs(poly_pose_s->data[2] - poly_pose_e->data[2]) < MEOS_EPSILON)
  {
    apply_pose_point4d(&p, poly_pose_s);
    double result;
    double discr = - (poly_pose_e->data[0] - poly_pose_s->data[0]) * (r.x - q.x)
                   - (poly_pose_e->data[1] - poly_pose_s->data[1]) * (r.y - q.y);
    if (fabs(discr) < MEOS_EPSILON)
      return 2;
    if (solution_kind) /* MEOS_SOLVE_0 */
      result = ((p.x - q.x) * (r.x - q.x) + (p.y - q.y) * (r.y - q.y)) / discr;
    else /* MEOS_SOLVE_1 */
      result = ((p.x - r.x) * (r.x - q.x) + (p.y - r.y) * (r.y - q.y)) / discr;
    return transition_ratio(result, prev_result);
  }

  double tl, tr, t0 = 0; /* Make compiler quiet */
  double vl, vr, v0;
  double ts = prev_result, te = 1;
  vl = f_poly_tpoly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    ts, solution_kind);
  v0 = f_poly_tpoly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    (ts + te) / 2, solution_kind);
  vr = f_poly_tpoly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    te, solution_kind);
  if (fabs(vl) > MEOS_EPSILON && vl * v0 < 0)
  {
    tl = ts;
    tr = (ts + te) / 2;
    vr = v0;
  }
  else if (v0 * vr < 0)
  {
    tl = (ts + te) / 2;
    tr = te;
    vl = v0;
  }
  else
    return 2;

  uint8_t i = 0;
  while(fabs(tr - tl) >= MEOS_EPSILON && i < 100)
  {
    ++i;
    t0 = (tl * vr - tr * vl) / (vr - vl);
    v0 = f_poly_tpoly(p, q, r, poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
      t0, solution_kind);
    if (fabs(v0) < MEOS_EPSILON)
      break;
    if (vl * v0 <= 0)
      tr = t0, vr = v0;
    else
      tl = t0, vl = v0;
  }
  return transition_ratio(t0, prev_result);
}

/**
 * @brief Return true if the ring of @p poly describes a segment, that is, two
 * vertices whose two edges traverse the same points in opposite directions
 * @details Each vertex of such a ring has ONE adjacent edge, reached either way
 * round, so the two roots that a vertex-exit test computes for it are the same
 * event and their coincidence carries no meaning.
 */
static inline bool
ring_is_segment(const LWPOLY *poly)
{
  return poly->rings[0]->npoints == 3;
}

/**
 * @brief Find the next change in closest feature
 */
static int
vertex_vertex_tpoly_poly(LWPOLY *poly1, Pose *pose_start, Pose *pose_end,
  Pose *pose2_start, Pose *pose2_end,
  LWPOLY *poly2, uint32_t *poly1_feature, uint32_t *poly2_feature,
  int *dir1, int *dir2, double *ratio)
{
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  uint32_t i1 = *poly1_feature / 2;
  uint32_t n2 = poly2->rings[0]->npoints - 1;
  uint32_t i2 = *poly2_feature / 2;
  double ratio_1 = 2, ratio_2 = 2, ratio_3 = 2, ratio_4 = 2;
  /* Detect if vertex of poly2 exits vertex of poly1 -> change poly1_feature */
  if (*dir1 == MEOS_RIGHT || *dir1 == MEOS_ANY)
    ratio_1 = solve_s_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
      i1, i2, *ratio, MEOS_SOLVE_0);
  if (*dir1 == MEOS_LEFT || *dir1 == MEOS_ANY)
    ratio_2 = solve_s_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
      uint_mod_sub(i1, 1, n1), i2, *ratio, MEOS_SOLVE_1);
  /* Detect if vertex of poly1 exits vertex of poly2 -> change poly2_feature */
  if (*dir2 == MEOS_RIGHT || *dir2 == MEOS_ANY)
    ratio_3 = solve_s_poly_tpoly(poly2, poly1, pose_start, pose_end, pose2_start, pose2_end,
      i2, i1, *ratio, MEOS_SOLVE_0);
  if (*dir2 == MEOS_LEFT || *dir2 == MEOS_ANY)
    ratio_4 = solve_s_poly_tpoly(poly2, poly1, pose_start, pose_end, pose2_start, pose2_end,
      uint_mod_sub(i2, 1, n2), i1, *ratio, MEOS_SOLVE_1);

  // printf("%lf, %lf, %lf, %f\n", ratio_1, ratio_2, ratio_3, ratio_4);
  // fflush(stdout);

  /* No change in closest feature */
  if (ratio_1 == 2 && ratio_2 == 2 && ratio_3 == 2 && ratio_4 == 2)
    return MEOS_DISJOINT;
  /* Intersection through vertex */
  else if (((ratio_1 != 2 || ratio_2 != 2) && fabs(ratio_1 - ratio_2) < MEOS_EPSILON)
        || (! ring_is_segment(poly2) && (ratio_3 != 2 || ratio_4 != 2) &&
            fabs(ratio_3 - ratio_4) < MEOS_EPSILON))
    return MEOS_INTERSECT;
  /* Go to next closest feature */
  else if (ratio_1 <= ratio_2 && ratio_1 <= ratio_3 && ratio_1 <= ratio_4)
  {
    *dir1 = MEOS_RIGHT;
    *poly1_feature = uint_mod_add(*poly1_feature, 1, 2 * n1);
    *ratio = ratio_1;
    return MEOS_CONTINUE;
  }
  /* Go to previous closest feature */
  else if (ratio_2 <= ratio_3 && ratio_2 <= ratio_4)
  {
    *dir1 = MEOS_LEFT;
    *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2 * n1);
    *ratio = ratio_2;
    return MEOS_CONTINUE;
  }
  else if (ratio_3 <= ratio_4)
  {
    *dir2 = MEOS_RIGHT;
    *poly2_feature = uint_mod_add(*poly2_feature, 1, 2 * n2);
    *ratio = ratio_3;
    return MEOS_CONTINUE;
  }
  /* Go to previous closest feature */
  else
  {
    *dir2 = MEOS_LEFT;
    *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2 * n2);
    *ratio = ratio_4;
    return MEOS_CONTINUE;
  }
}

/**
 * @brief Return, at a ratio of a segment, the function whose root is the ratio at
 * which two edges of the polygons become parallel
 */
static double
f_parallel_edges_tpoly_poly(LWPOLY *poly1, Pose *poly_pose_s, Pose *poly_pose_e,
  Pose *poly2_pose_s, Pose *poly2_pose_e, LWPOLY *poly2, uint32_t poly1_v,
  uint32_t poly2_v, double ratio)
{
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  uint32_t n2 = poly2->rings[0]->npoints - 1;
  POINT4D ps, pe, qs, qe;
  getPoint4d_p(poly1->rings[0], poly1_v, &qs);
  getPoint4d_p(poly1->rings[0], uint_mod_add(poly1_v, 1, n1), &qe);
  getPoint4d_p(poly2->rings[0], poly2_v, &ps);
  getPoint4d_p(poly2->rings[0], uint_mod_add(poly2_v, 1, n2), &pe);
  double dx, dy, dtheta;
  double co, si, qsx, qsy, qex, qey;
  rel_pose_interpolate_2d(poly_pose_s, poly_pose_e, poly2_pose_s, poly2_pose_e,
    ratio, &dx, &dy, &dtheta);
  co = cos(dtheta);
  si = sin(dtheta);
  qsx = qs.x * co - qs.y * si + dx;
  qsy = qs.x * si + qs.y * co + dy;
  qex = qe.x * co - qe.y * si + dx;
  qey = qe.x * si + qe.y * co + dy;
  return (pe.x - ps.x) * (qey - qsy) - (pe.y - ps.y) * (qex - qsx);
}

/**
 * @brief Return the ratio at which an edge of each polygon becomes parallel to the
 * other
 */
static double
solve_parallel_edges_tpoly_poly(LWPOLY *poly1, Pose *poly_pose_s,
  Pose *poly_pose_e, Pose *poly2_pose_s, Pose *poly2_pose_e, LWPOLY *poly2,
  uint32_t poly1_v, uint32_t poly2_v, double prev_result)
{
  /* No rotation during movement
   * Edges do not rotate, so no need to solve this. With both polygons moving
   * the relative rotation is resolved by the general root find below. */
  if (! poly2_pose_s &&
      fabs(poly_pose_s->data[2] - poly_pose_e->data[2]) < MEOS_EPSILON)
    return 2;

  double tl, tr, t0 = 0; /* Make compiler quiet */
  double vl, vr, v0;
  double ts = prev_result, te = 1;
  vl = f_parallel_edges_tpoly_poly(poly1, poly_pose_s, poly_pose_e, poly2_pose_s,
    poly2_pose_e, poly2, poly1_v, poly2_v, ts);
  v0 = f_parallel_edges_tpoly_poly(poly1, poly_pose_s, poly_pose_e, poly2_pose_s,
    poly2_pose_e, poly2, poly1_v, poly2_v, (ts + te) / 2);
  vr = f_parallel_edges_tpoly_poly(poly1, poly_pose_s, poly_pose_e, poly2_pose_s,
    poly2_pose_e, poly2, poly1_v, poly2_v, te);
  // printf("%lf, %lf, %lf\n", vl, v0, vr);
  // fflush(stdout);
  if (fabs(vl) > MEOS_EPSILON && vl * v0 < 0)
  {
    tl = ts;
    tr = (ts + te) / 2;
    vr = v0;
  }
  else if (v0 * vr < 0)
  {
    tl = (ts + te) / 2;
    tr = te;
    vl = v0;
  }
  else
    return 2;

  uint8_t i = 0;
  while(fabs(tr - tl) >= MEOS_EPSILON && i < 100)
  {
    ++i;
    t0 = (tl * vr - tr * vl) / (vr - vl);
    v0 = f_parallel_edges_tpoly_poly(poly1, poly_pose_s, poly_pose_e,
      poly2_pose_s, poly2_pose_e, poly2, poly1_v, poly2_v, t0);
    if (fabs(v0) < MEOS_EPSILON)
      break;
    if (vl * v0 <= 0)
      tr = t0, vr = v0;
    else
      tl = t0, vl = v0;
  }
  return transition_ratio(t0, prev_result);
}

/**
 * @brief Return the sentinel reporting no transition, the crossing of an edge line
 * contributing none between two polygons
 */
static inline double
solve_angle_0_poly_tpoly(LWPOLY *poly1 UNUSED,
  LWPOLY *poly2 UNUSED,
  Pose *poly_pose_s UNUSED,
  Pose *poly_pose_e UNUSED,
  uint32_t poly1_v UNUSED,
  uint32_t poly2_v UNUSED,
  double ratio UNUSED)
{
  return 2;
}

/**
 * @brief Find the next change in closest feature
 */
static int
vertex_edge_tpoly_poly(LWPOLY *poly1, Pose *pose_start, Pose *pose_end,
  Pose *pose2_start, Pose *pose2_end,
  LWPOLY *poly2, uint32_t *poly1_feature, uint32_t *poly2_feature,
  int *dir1 UNUSED, int *dir2, double *ratio)
{
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  uint32_t i1 = *poly1_feature / 2;
  uint32_t n2 = poly2->rings[0]->npoints - 1;
  uint32_t i2 = *poly2_feature / 2;
  double ratio_1 = 2, ratio_2 = 2, ratio_3 = 2, ratio_4 = 2, ratio_inter;
  /* Detect if vertex of poly1 exits edge of poly2 -> change poly2_feature */
  if (*dir2 == MEOS_RIGHT || *dir2 == MEOS_ANY)
    ratio_1 = solve_s_poly_tpoly(poly2, poly1, pose_start, pose_end, pose2_start, pose2_end,
      i2, i1, *ratio, MEOS_SOLVE_1);
  if (*dir2 == MEOS_LEFT || *dir2 == MEOS_ANY)
    ratio_2 = solve_s_poly_tpoly(poly2, poly1, pose_start, pose_end, pose2_start, pose2_end,
      i2, i1, *ratio, MEOS_SOLVE_0);
  /* Detect parallel edges -> 2 changes in closest features at once */
  ratio_3 = solve_parallel_edges_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
    i1, i2, *ratio);
  ratio_4 = solve_parallel_edges_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
    uint_mod_sub(i1, 1, n1), i2, *ratio);
  /* Detect intersection with the edge */
  ratio_inter = solve_angle_0_poly_tpoly(poly2, poly1, pose_start, pose_end,
    i2, i1, *ratio);

  // printf("%lf, %lf, %lf, %f\n", ratio_1, ratio_2, ratio_3, ratio_4);
  // fflush(stdout);

  /* Intersection through edge */
  if (ratio_inter < ratio_1 && ratio_inter < ratio_2)
    return MEOS_INTERSECT;
  /* No change in closest feature */
  else if (ratio_1 == 2 && ratio_2 == 2 && ratio_3 == 2 && ratio_4 == 2)
    return MEOS_DISJOINT;
  /* Go to next closest feature of poly2 */
  else if (ratio_1 <= ratio_2 && ratio_1 <= ratio_3 && ratio_1 <= ratio_4)
  {
    *dir2 = MEOS_RIGHT;
    *poly2_feature = uint_mod_add(*poly2_feature, 1, 2 * n2);
    *ratio = ratio_1;
    return MEOS_CONTINUE;
  }
  /* Go to previous closest feature of poly2 */
  else if (ratio_2 <= ratio_3 && ratio_2 <= ratio_4)
  {
    *dir2 = MEOS_LEFT;
    *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2 * n2);
    *ratio = ratio_2;
    return MEOS_CONTINUE;
  }
  /* Next edge of poly1 is parallel with edge of poly2 */
  else if (ratio_3 <= ratio_4)
  {
    /* Determine how to update closest feature */
    uint32_t n1 = poly1->rings[0]->npoints - 1;
    uint32_t n2 = poly2->rings[0]->npoints - 1;
    POINT4D ps, pe, qs, qe;
    getPoint4d_p(poly1->rings[0], i1, &qs);
    getPoint4d_p(poly1->rings[0], uint_mod_add(i1, 1, n1), &qe);
    getPoint4d_p(poly2->rings[0], i2, &ps);
    getPoint4d_p(poly2->rings[0], uint_mod_add(i2, 1, n2), &pe);
    Pose *pose = rel_posesegm_interpolate(pose_start, pose_end, pose2_start,
      pose2_end, ratio_3);
    apply_pose_point4d(&qs, pose);
    apply_pose_point4d(&qe, pose);
    pfree(pose);
    /* TODO: check if we assume that ccw1 == ccw2 here or not */
    double s1 = compute_s(qe, ps, pe);
    double s2 = compute_s(ps, qs, qe);
    // printf("C: %lf, %lf\n", s1, s2);
    // fflush(stdout);
    if (0 < s1 && s1 < 1)
    {
      /* Next features:
       * - next vertex of poly1
       * - current edge of poly2*/
      // *dir1 = MEOS_RIGHT;
      *poly1_feature = uint_mod_add(*poly1_feature, 2, 2 * n1);
    }
    else if (0 < s2 && s2 < 1)
    {
      /* Next features:
       * - next edge of poly1
       * - previous vertex of poly2*/
      // *dir1 = MEOS_RIGHT;
      *poly1_feature = uint_mod_add(*poly1_feature, 1, 2 * n1);
      // *dir2 = MEOS_LEFT;
      *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2 * n2);
    }
    else
    {
      /* Endpoints of the edges are aligned
       * Next features:
       * - next vertex of poly1
       * - previous vertex of poly2*/
      // *dir1 = MEOS_RIGHT;
      *poly1_feature = uint_mod_add(*poly1_feature, 2, 2 * n1);
      // *dir2 = MEOS_LEFT;
      *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2 * n2);
    }
    *ratio = ratio_3;
    return MEOS_CONTINUE;
  }
  /* Next edge of poly1 is parallel with edge of poly2 */
  else
  {
    /* Determine how to update closest feature */
    uint32_t n1 = poly1->rings[0]->npoints - 1;
    uint32_t n2 = poly2->rings[0]->npoints - 1;
    POINT4D ps, pe, qs, qe;
    getPoint4d_p(poly1->rings[0], uint_mod_sub(i1, 1, n1), &qs);
    getPoint4d_p(poly1->rings[0], i1, &qe);
    getPoint4d_p(poly2->rings[0], i2, &ps);
    getPoint4d_p(poly2->rings[0], uint_mod_add(i2, 1, n2), &pe);
    Pose *pose = rel_posesegm_interpolate(pose_start, pose_end, pose2_start,
      pose2_end, ratio_4);
    apply_pose_point4d(&qs, pose);
    apply_pose_point4d(&qe, pose);
    pfree(pose);
    /* TODO: check if we assume that ccw1 == ccw2 here or not */
    double s1 = compute_s(qs, ps, pe);
    double s2 = compute_s(pe, qs, qe);
    // printf("D: %lf, %lf\n", s1, s2);
    // fflush(stdout);
    if (0 < s1 && s1 < 1)
    {
      /* Next features:
       * - previous vertex of poly1
       * - current edge of poly2*/
      // *dir1 = MEOS_LEFT;
      *poly1_feature = uint_mod_sub(*poly1_feature, 2, 2 * n1);
    }
    else if (0 < s2 && s2 < 1)
    {
      /* Next features:
       * - previous edge of poly1
       * - next vertex of poly2*/
      // *dir1 = MEOS_LEFT;
      *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2 * n1);
      // *dir2 = MEOS_RIGHT;
      *poly2_feature = uint_mod_add(*poly2_feature, 1, 2 * n2);
    }
    else
    {
      /* Endpoints of the edges are aligned
       * Next features:
       * - previous vertex of poly1
       * - next vertex of poly2*/
      // *dir1 = MEOS_LEFT;
      *poly1_feature = uint_mod_sub(*poly1_feature, 2, 2 * n1);
      // *dir2 = MEOS_RIGHT;
      *poly2_feature = uint_mod_add(*poly2_feature, 1, 2 * n2);
    }
    *ratio = ratio_4;
    return MEOS_CONTINUE;
  }
  /* Cannot happen */
  assert(false);
  return MEOS_DISJOINT;
}

/**
 * @brief Find the next change in closest feature
 */
static int
edge_vertex_tpoly_poly(LWPOLY *poly1, Pose *pose_start, Pose *pose_end,
  Pose *pose2_start, Pose *pose2_end,
  LWPOLY *poly2, uint32_t *poly1_feature, uint32_t *poly2_feature,
  int *dir1, int *dir2 UNUSED, double *ratio)
{
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  uint32_t i1 = *poly1_feature / 2;
  uint32_t n2 = poly2->rings[0]->npoints - 1;
  uint32_t i2 = *poly2_feature / 2;
  double ratio_1 = 2, ratio_2 = 2, ratio_3 = 2, ratio_4 = 2, ratio_inter;
  /* Detect if vertex of poly2 exits edge of poly1 -> change poly1_feature */
  if (*dir1 == MEOS_RIGHT || *dir1 == MEOS_ANY)
    ratio_1 = solve_s_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
      i1, i2, *ratio, MEOS_SOLVE_1);
  if (*dir1 == MEOS_LEFT || *dir1 == MEOS_ANY)
    ratio_2 = solve_s_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
      i1, i2, *ratio, MEOS_SOLVE_0);
  /* Detect parallel edges -> 2 changes in closest features at once */
  ratio_3 = solve_parallel_edges_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
    i1, i2, *ratio);
  ratio_4 = solve_parallel_edges_tpoly_poly(poly1, pose_start, pose_end, pose2_start, pose2_end, poly2,
    i1, uint_mod_sub(i2, 1, n2), *ratio);
  /* Detect intersection with the edge */
  ratio_inter = solve_angle_0_poly_tpoly(poly2, poly1, pose_start, pose_end,
    i2, i1, *ratio);

  // printf("%lf, %lf, %lf, %f\n", ratio_1, ratio_2, ratio_3, ratio_4);
  // fflush(stdout);

  /* Intersection through edge */
  if (ratio_inter < ratio_1 && ratio_inter < ratio_2)
    return MEOS_INTERSECT;
  /* No change in closest feature */
  else if (ratio_1 == 2 && ratio_2 == 2 && ratio_3 == 2 && ratio_4 == 2)
    return MEOS_DISJOINT;
  /* Go to next closest feature of poly1 */
  else if (ratio_1 <= ratio_2 && ratio_1 <= ratio_3 && ratio_1 <= ratio_4)
  {
    *dir1 = MEOS_RIGHT;
    *poly1_feature = uint_mod_add(*poly1_feature, 1, 2 * n1);
    *ratio = ratio_1;
    return MEOS_CONTINUE;
  }
  /* Go to previous closest feature of poly1 */
  else if (ratio_2 <= ratio_3 && ratio_2 <= ratio_4)
  {
    *dir1 = MEOS_LEFT;
    *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2 * n1);
    *ratio = ratio_2;
    return MEOS_CONTINUE;
  }
  /* Next edge of poly2 is parallel with edge of poly1 */
  else if (ratio_3 <= ratio_4)
  {
    /* Determine how to update closest feature */
    uint32_t n1 = poly1->rings[0]->npoints - 1;
    uint32_t n2 = poly2->rings[0]->npoints - 1;
    POINT4D ps, pe, qs, qe;
    getPoint4d_p(poly1->rings[0], i1, &qs);
    getPoint4d_p(poly1->rings[0], uint_mod_add(i1, 1, n1), &qe);
    getPoint4d_p(poly2->rings[0], i2, &ps);
    getPoint4d_p(poly2->rings[0], uint_mod_add(i2, 1, n2), &pe);
    Pose *pose = rel_posesegm_interpolate(pose_start, pose_end, pose2_start,
      pose2_end, ratio_3);
    apply_pose_point4d(&qs, pose);
    apply_pose_point4d(&qe, pose);
    pfree(pose);
    /* TODO: check if we assume that ccw1 == ccw2 here or not */
    double s1 = compute_s(pe, qs, qe);
    double s2 = compute_s(qs, ps, pe);
    // printf("A: %lf, %lf\n", s1, s2);
    // fflush(stdout);
    if (0 < s1 && s1 < 1)
    {
      /* Next features:
       * - next vertex of poly2
       * - current edge of poly1 */
      // *dir2 = MEOS_RIGHT;
      *poly2_feature = uint_mod_add(*poly2_feature, 2, 2 * n2);
    }
    else if (0 < s2 && s2 < 1)
    {
      /* Next features:
       * - next edge of poly2
       * - previous vertex of poly1 */
      // *dir2 = MEOS_RIGHT;
      *poly2_feature = uint_mod_add(*poly2_feature, 1, 2 * n2);
      // *dir1 = MEOS_LEFT;
      *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2 * n1);
    }
    else
    {
      /* Endpoints of the edges are aligned
       * Next features:
       * - next vertex of poly2
       * - previous vertex of poly1 */
      // *dir2 = MEOS_RIGHT;
      *poly2_feature = uint_mod_add(*poly2_feature, 2, 2 * n2);
      // *dir1 = MEOS_LEFT;
      *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2 * n1);
    }
    *ratio = ratio_3;
    return MEOS_CONTINUE;
  }
  /* Next edge of poly2 is parallel with edge of poly1 */
  else
  {
    /* Determine how to update closest feature */
    uint32_t n1 = poly1->rings[0]->npoints - 1;
    uint32_t n2 = poly2->rings[0]->npoints - 1;
    POINT4D ps, pe, qs, qe;
    getPoint4d_p(poly1->rings[0], i1, &qs);
    getPoint4d_p(poly1->rings[0], uint_mod_add(i1, 1, n1), &qe);
    getPoint4d_p(poly2->rings[0], uint_mod_sub(i2, 1, n2), &ps);
    getPoint4d_p(poly2->rings[0], i2, &pe);
    Pose *pose = rel_posesegm_interpolate(pose_start, pose_end, pose2_start,
      pose2_end, ratio_4);
    apply_pose_point4d(&qs, pose);
    apply_pose_point4d(&qe, pose);
    pfree(pose);
    /* TODO: check if we assume that ccw1 == ccw2 here or not */
    double s1 = compute_s(ps, qs, qe);
    double s2 = compute_s(qe, ps, pe);
    // printf("B: %lf, %lf\n", s1, s2);
    // fflush(stdout);
    if (0 < s1 && s1 < 1)
    {
      /* Next features:
       * - previous vertex of poly2
       * - current edge of poly1 */
      // *dir2 = MEOS_LEFT;
      *poly2_feature = uint_mod_sub(*poly2_feature, 2, 2 * n2);
    }
    else if (0 < s2 && s2 < 1)
    {
      /* Next features:
       * - previous edge of poly2
       * - next vertex of poly1 */
      // *dir2 = MEOS_LEFT;
      *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2 * n2);
      // *dir1 = MEOS_RIGHT;
      *poly1_feature = uint_mod_add(*poly1_feature, 1, 2 * n1);
    }
    else
    {
      /* Endpoints of the edges are aligned
       * Next features:
       * - previous vertex of poly2
       * - next vertex of poly1 */
      // *dir2 = MEOS_LEFT;
      *poly2_feature = uint_mod_sub(*poly2_feature, 2, 2 * n2);
      // *dir1 = MEOS_RIGHT;
      *poly1_feature = uint_mod_add(*poly1_feature, 1, 2 * n1);
    }
    *ratio = ratio_4;
    return MEOS_CONTINUE;
  }
  /* Cannot happen */
  assert(false);
  return MEOS_DISJOINT;
}

/**
 * @brief Append the distance the v-clip oracle answers for a closest-feature pair
 */
static void
compute_dist_tpoly_poly(cfp_elem *cfp, tdist_array *tda)
{
  /* Take the distance from the v-clip oracle on the pose stored in the feature
   * pair: it re-establishes the true closest feature (robust to any
   * mis-tracking in the walk) and returns zero when the polygons overlap */
  uint32_t cf_1 = 0, cf_2 = 0;
  double dist;
  v_clip_tpoly_tpoly((LWPOLY *) cfp->geom_1, (LWPOLY *) cfp->geom_2,
    cfp->pose_1, NULL, &cf_1, &cf_2, &dist);
  tdist_elem td = tdist_make(dist, cfp->t);
  append_tdist_elem(tda, td);
}

/**
 * @brief Append the time at which a translating polygon reaches a polygon it
 * meets, where that time carries no point of its own
 * @details The v-clip oracle answers the separation of two polygons while they
 * stand apart and zero for every pose at which they overlap, so the distance
 * of a fixed closest-feature pair falls to a floor and stays there. The join
 * is a kink, and a kink carrying no point is drawn as a straight line across
 * it, which leaves zero early on the way in and reaches it late on the way
 * out.
 *
 * Along a segment on which the body only translates, the separation of a fixed
 * feature pair is linear in the ratio, so two readings taken while the
 * polygons STAND APART give the line whose root is the contact. No reading
 * taken after they meet enters the fit, which is what keeps the answer off the
 * floor the oracle imposes: halving a bracket on that floor instead converges
 * on the boundary of the oracle's answer, which stands before the contact.
 *
 * A contact that a change of closest features already carries is answered by
 * the walk exactly, and the root then falls on that point rather than inside
 * the interval, so nothing is added. A body that only grazes reaches the floor
 * at a single instant, and its root falls on that instant for the same reason.
 * @param[in] geom_1,geom_2 Reference polygon and target polygon
 * @param[in] pose_s,pose_e Poses at the ends of the temporal segment
 * @param[in] t_lo,t_hi Ends of the temporal segment
 * @param[in] ta,tb Ends of the interval, @p ta standing apart
 * @param[in] da Separation at @p ta
 * @param[in,out] tda Distance points to append to
 */
static void
compute_contact_tpoly_poly(const LWGEOM *geom_1, const LWGEOM *geom_2,
  const Pose *pose_s, const Pose *pose_e, TimestampTz t_lo, TimestampTz t_hi,
  TimestampTz ta, TimestampTz tb, double da, tdist_array *tda)
{
  if (t_hi <= t_lo || tb <= ta || da <= 0.0)
    return;
  /* A rotating segment moves each vertex along an arc, so its separation is
   * not linear in the ratio and two readings do not give its root */
  if (fabs(pose_e->data[2] - pose_s->data[2]) > MEOS_GEOM_TOLERANCE)
    return;

  double span = (double) (t_hi - t_lo);
  double fa = (double) (ta - t_lo) / span;
  double fb = (double) (tb - t_lo) / span;

  /* The second reading of the fit, taken as far from the first as it can be
   * while the polygons still stand apart */
  double fm = 0.0, dm = 0.0;
  for (double frac = 0.5; frac > 1e-9; frac *= 0.5)
  {
    double f = fa + (fb - fa) * frac;
    Pose *pm = posesegm_interpolate(pose_s, pose_e, f);
    uint32_t c1 = 0, c2 = 0;
    double d;
    v_clip_tpoly_tpoly((LWPOLY *) geom_1, (LWPOLY *) geom_2, pm, NULL, &c1,
      &c2, &d);
    pfree(pm);
    if (d > 0.0)
    {
      fm = f; dm = d;
      break;
    }
  }
  if (dm <= 0.0 || dm >= da)
    return;

  /* The root of the line through the two readings that stand apart */
  double froot = fa + (fm - fa) * da / (da - dm);
  TimestampTz tc = t_lo + (TimestampTz) llround(froot * span);
  /* A contact the walk already carries needs no point of its own */
  if (tc <= ta || tc >= tb)
    return;
  tdist_elem td = tdist_make(0.0, tc);
  append_tdist_elem(tda, td);
  return;
}

/**
 * @brief Append the interior turning points (local extrema) of the distance
 * realized by the fixed closest-feature pair of @p cfp_s while the rigid
 * geometry moves over the temporal segment @p [t_lo,t_hi]
 * @details The distance of a fixed feature pair is smooth in the ratio, so its
 * extrema are the roots of its derivative: the derivative is bracketed on a
 * subdivision and each root refined to machine precision, and the exact
 * distance there (from the v-clip oracle on the interpolated pose) is emitted
 * as a turning point of the tfloat.
 */
static void
compute_turnpoints_tpoly_poly(const cfp_elem *cfp_s, const cfp_elem *cfp_e,
  Pose *pose_s, Pose *pose_e, Pose *pose2_s, Pose *pose2_e, TimestampTz t_lo,
  TimestampTz t_hi, tdist_array *tda)
{
  const LWPOLY *poly1 = (const LWPOLY *) cfp_s->geom_1;
  const LWPOLY *poly2 = (const LWPOLY *) cfp_s->geom_2;
  double span = (double) (t_hi - t_lo);
  if (span <= 0)
    return;
  double ga = (double) (cfp_s->t - t_lo) / span;
  double gb = (double) (cfp_e->t - t_lo) / span;
  if (gb - ga < MEOS_EPSILON)
    return;

  /* Distance of the closest feature pair at segment ratio g, with the first
   * polygon in the moving frame of the second one */
  #define TP_DIST(g) __extension__ ({ \
    Pose *_pp = rel_posesegm_interpolate(pose_s, pose_e, pose2_s, pose2_e, (g)); \
    uint32_t _c1 = 0, _c2 = 0; double _d; \
    v_clip_tpoly_tpoly(poly1, poly2, _pp, NULL, &_c1, &_c2, &_d); \
    pfree(_pp); _d; })

  /* Locate the interior turning points (local minima and maxima) of the
   * distance over [ga, gb]. The distance is smooth within a fixed feature
   * pair, but it drops to a flat zero over any sub-interval where the bodies
   * overlap, so a plain derivative-sign-change bracket misses that minimum: a
   * run of zero-derivative samples inside the plateau separates the falling
   * and rising samples. Track the slope sign (falling / flat / rising) on a
   * uniform subdivision and emit a turning point at every falling->rising and
   * falling->flat and flat->rising transition (a minimum, including a plateau
   * boundary) and every rising->falling and rising->flat and flat->falling
   * transition (a maximum), refining each by golden section on the value. */
  int M = 64;
  double step = (gb - ga) / M;
  double d0 = TP_DIST(ga), d1 = TP_DIST(ga + step);
  double gprev = ga + step, dprev = d1;
  int sprev = (d1 > d0 + MEOS_EPSILON) ? 1 : (d1 < d0 - MEOS_EPSILON ? -1 : 0);
  for (int k = 2; k <= M; ++k)
  {
    double gcur = ga + step * k, dcur = TP_DIST(gcur);
    int scur = (dcur > dprev + MEOS_EPSILON) ? 1 :
      (dcur < dprev - MEOS_EPSILON ? -1 : 0);
    bool is_min = (sprev < 0 && scur >= 0) || (sprev == 0 && scur > 0);
    bool is_max = (sprev > 0 && scur <= 0) || (sprev == 0 && scur < 0);
    if (is_min || is_max)
    {
      /* Golden-section refine the extremum in [gprev, gcur] */
      const double gr = 0.6180339887498949;
      double lo = gprev, hi = gcur;
      double gc = hi - gr * (hi - lo), ge = lo + gr * (hi - lo);
      double fc = TP_DIST(gc), fe = TP_DIST(ge);
      for (int it = 0; it < 60 && hi - lo > 1e-15; ++it)
      {
        bool pick_left = is_min ? (fc < fe) : (fc > fe);
        if (pick_left)
        { hi = ge; ge = gc; fe = fc; gc = hi - gr * (hi - lo); fc = TP_DIST(gc); }
        else
        { lo = gc; gc = ge; fc = fe; ge = lo + gr * (hi - lo); fe = TP_DIST(ge); }
      }
      double gstar = 0.5 * (lo + hi);
      tdist_elem td = tdist_make(TP_DIST(gstar),
        t_lo + (TimestampTz) (span * gstar));
      append_tdist_elem(tda, td);
    }
    gprev = gcur;
    dprev = dcur;
    sprev = scur;
  }
  #undef TP_DIST
}

/**
 * @brief Return the index of the segment of @p seq that contains @p t (the
 * largest @p i with @p INST_N(i)->t <= t, capped at count - 2)
 */
static int
trgeoseq_segment_index(const TSequence *seq, TimestampTz t)
{
  int lo = 0, hi = seq->count - 1;
  while (lo < hi)
  {
    int mid = (lo + hi + 1) / 2;
    if (TSEQUENCE_INST_N(seq, mid)->t <= t)
      lo = mid;
    else
      hi = mid - 1;
  }
  return (lo > seq->count - 2) ? seq->count - 2 : lo;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry sequence
 * and a polygon
 */
TSequence *
dist2d_trgeoseq_poly(const TSequence *seq, const GSERIALIZED *gs,
  const GSERIALIZED *ref_gs)
{
  /* TODO: Add check and code for stepwise seq */

  /* TODO: check that both polygons are convex */
  LWPOLY *poly1 = lwgeom_as_lwpoly(lwgeom_from_gserialized(ref_gs));
  LWPOLY *poly2 = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs));

  const TInstant *inst1, *inst2;
  Pose *pose1, *pose2;

  inst1 = TSEQUENCE_INST_N(seq, 0);
  pose1 = DatumGetPoseP(tinstant_value_p(inst1));

  /* Compute the initial closest features */
  cfp_array cfpa;
  init_cfp_array(&cfpa, seq->count);
  cfp_elem cfp = cfp_make_zero((LWGEOM *)poly1, (LWGEOM *)poly2,
    pose1, NULL, inst1->t, MEOS_CFP_STORE);
  v_clip_tpoly_tpoly(poly1, poly2, pose1, NULL, &cfp.cf_1, &cfp.cf_2, NULL);
  append_cfp_elem(&cfpa, cfp);
  for (int i = 0; i < seq->count - 1; ++i)
  {
    // printf("Segment %d\n", i);
    // fflush(stdout);
    /* TODO: optimise using simple checks, such as:
     * 1) cfp(0) == cfp(0.5) == cfp(1) -> no change in cf
     */
    inst1 = TSEQUENCE_INST_N(seq, i);
    inst2 = TSEQUENCE_INST_N(seq, i + 1);
    pose1 = DatumGetPoseP(tinstant_value_p(inst1));
    pose2 = DatumGetPoseP(tinstant_value_p(inst2));
    double ratio = 0.0;
    int loop = 0, state, dir1 = MEOS_ANY, dir2 = MEOS_ANY;
    /* Compute the evolution of closest features for this segment */
    do
    {
      // printf("Features before %d, %d\n", cfp.cf_1, cfp.cf_2);
      // printf("Dirs before = (%d, %d)\n", dir1, dir2);
      // fflush(stdout);

      if (cfp.cf_1 % 2 == 0 && cfp.cf_2 % 2 == 0) /* vertex <-> vertex */
        state = vertex_vertex_tpoly_poly(poly1, pose1, pose2, NULL, NULL, poly2,
          &cfp.cf_1, &cfp.cf_2, &dir1, &dir2, &ratio);
      else if (cfp.cf_1 % 2 == 0) /* vertex <-> edge */
        state = vertex_edge_tpoly_poly(poly1, pose1, pose2, NULL, NULL, poly2,
          &cfp.cf_1, &cfp.cf_2, &dir1, &dir2, &ratio);
      else if (cfp.cf_2 % 2 == 0) /* edge <-> vertex */
        state = edge_vertex_tpoly_poly(poly1, pose1, pose2, NULL, NULL, poly2,
          &cfp.cf_1, &cfp.cf_2, &dir1, &dir2, &ratio);
      else /* edge <-> edge */
      {
        /* Two edges are the closest feature pair, e.g. the parallel facing
         * edges of two convex polygons. No further feature transition is
         * tracked from this state, so the walk ends for the segment. The
         * distance at the segment ends and every interior turning point are
         * taken from the v-clip oracle, so the result stays exact. */
        state = MEOS_DISJOINT;
      }

      // printf("Features after %d, %d\n", cfp.cf_1, cfp.cf_2);
      // printf("Dirs after = (%d, %d)\n", dir1, dir2);
      // fflush(stdout);

      if (state == MEOS_CONTINUE)
      {
        cfp.t = inst1->t + (inst2->t - inst1->t) * ratio;
        cfp.pose_1 = posesegm_interpolate(pose1, pose2, ratio);
        cfp.free_pose_1 = MEOS_CFP_FREE;
        cfp.store = MEOS_CFP_STORE_NO;
        append_cfp_elem(&cfpa, cfp);
      }

      // cfp_elem test_cfp = cfp;
      // v_clip_tpoly_tpoly(poly1, poly2, cfp.pose_1, NULL, &test_cfp.cf_1, &test_cfp.cf_2, NULL);
      // if (test_cfp.cf_1 != cfp.cf_1 || test_cfp.cf_2 != cfp.cf_2)
      // {
      //   printf("Problem, test cfp changed from (%d, %d) to (%d, %d) during temporal segment\n",
      //     cfp.cf_1, cfp.cf_2, test_cfp.cf_1, test_cfp.cf_2);
      //   fflush(stdout);
      // }

      if (loop++ == MEOS_MAX_ITERS) break;

    } while (state == MEOS_CONTINUE);

    if (loop > MEOS_MAX_ITERS)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Temporal distance: Cycle detected, current features: (%d, %d)",
        cfp.cf_1, cfp.cf_2);
      return NULL;
    }

    cfp.pose_1 = pose2;
    cfp.free_pose_1 = MEOS_CFP_FREE_NO;
    cfp.t = inst2->t;
    cfp.store = MEOS_CFP_STORE;
    cfp_elem next_cfp = cfp;
    v_clip_tpoly_tpoly(poly1, poly2, pose2, NULL, &next_cfp.cf_1,
      &next_cfp.cf_2, NULL);
    append_cfp_elem(&cfpa, next_cfp);
    cfp = next_cfp;
  }

  /* Compute the piecewise-linear distance from the array of closest features:
   * a point at every closest-feature time (segment ends and transition kinks)
   * plus the interior extrema of each fixed feature pair */
  tdist_array tda;
  init_tdist_array(&tda, cfpa.count);
  for (uint32_t i = 0; i < cfpa.count - 1; ++i)
  {
    compute_dist_tpoly_poly(&cfpa.arr[i], &tda);
    int s = trgeoseq_segment_index(seq,
      cfpa.arr[i].t + (cfpa.arr[i + 1].t - cfpa.arr[i].t) / 2);
    const TInstant *si = TSEQUENCE_INST_N(seq, s);
    const TInstant *ei = TSEQUENCE_INST_N(seq, s + 1);
    compute_turnpoints_tpoly_poly(&cfpa.arr[i], &cfpa.arr[i + 1],
      DatumGetPoseP(tinstant_value_p(si)), DatumGetPoseP(tinstant_value_p(ei)),
      NULL, NULL, si->t, ei->t, &tda);
  }
  compute_dist_tpoly_poly(&cfpa.arr[cfpa.count-1], &tda);

  /* Order the distance points by time before building the result */
  tdist_array_sort(&tda);

  /* Where two neighbouring points stand on either side of the floor the
   * oracle imposes, the join between them is a kink, and the time the
   * separation reaches zero is a point of the answer */
  uint32_t nbefore = tda.count;
  for (uint32_t i = 0; i + 1 < nbefore; ++i)
  {
    double d1 = tda.arr[i].dist, d2 = tda.arr[i + 1].dist;
    if ((d1 > 0.0) == (d2 > 0.0))
      continue;
    int sgi = trgeoseq_segment_index(seq,
      tda.arr[i].t + (tda.arr[i + 1].t - tda.arr[i].t) / 2);
    const TInstant *ssi = TSEQUENCE_INST_N(seq, sgi);
    const TInstant *sei = TSEQUENCE_INST_N(seq, sgi + 1);
    /* The reading that stands apart opens the interval on the way in and
     * closes it on the way out, so the fit runs from it either way */
    if (d1 > 0.0)
      compute_contact_tpoly_poly((LWGEOM *) poly1, (LWGEOM *) poly2,
        DatumGetPoseP(tinstant_value_p(ssi)),
        DatumGetPoseP(tinstant_value_p(sei)), ssi->t, sei->t,
        tda.arr[i].t, tda.arr[i + 1].t, d1, &tda);
    else
      compute_contact_tpoly_poly((LWGEOM *) poly1, (LWGEOM *) poly2,
        DatumGetPoseP(tinstant_value_p(sei)),
        DatumGetPoseP(tinstant_value_p(ssi)), sei->t, ssi->t,
        tda.arr[i + 1].t, tda.arr[i].t, d2, &tda);
  }
  if (tda.count != nbefore)
    tdist_array_sort(&tda);

  /* Create the result tfloat */
  TInstant **instants = palloc(sizeof(TInstant *) * tda.count);
  for (uint32_t i = 0; i < tda.count; ++i)
    instants[i] = tinstant_make(Float8GetDatum(tda.arr[i].dist), T_TFLOAT,
      tda.arr[i].t);
  TSequence *result = tsequence_make_free(instants, tda.count,
    seq->period.lower_inc, seq->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);

  lwpoly_free(poly1); lwpoly_free(poly2);
  free_cfp_array(&cfpa); free_tdist_array(&tda);
  return result;
}

/**
 * @brief Return 1 if two temporal float segments cross during the period
 * defined by the output timestamps, return 0 otherwise
 * @param[in] start1,end1 Values defining the first segment
 * @param[in] start2,end2 Values defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @note This function is passed to the lifting infrastructure when computing
 * the pointwise minimum of two distances, whose kink is where the two cross
 * @post As there is a single turning point, `t2` is set to `t1`
 */
static int
tfloatsegm_min_turnpt(Datum start1, Datum end1, Datum start2, Datum end2,
  Datum param UNUSED, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2)
{
  return tnumbersegm_intersection(start1, end1, start2, end2, T_FLOAT8, lower,
    upper, t1, t2);
}

/**
 * @brief Return the pointwise minimum of two temporal float sequences
 * @details The minimum of two piecewise-linear functions is piecewise linear
 * with an additional kink wherever the two cross, so the lifted minimum is
 * given the crossing as its turning point
 * @pre The two sequences have the same period
 */
static TSequence *
tfloatseq_min_tfloatseq(const TSequence *seq1, const TSequence *seq2)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_min_float8;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = lfinfo.argtype[1] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(seq1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_temp = lfinfo.reslinear ? &tfloatsegm_min_turnpt : NULL;
  Temporal *result = tfunc_tcontseq_tcontseq(seq1, seq2, &lfinfo);
  /* The two sequences share their period and the minimum is continuous, so
   * the lifted function yields a single sequence */
  assert(result->subtype == TSEQUENCE);
  return (TSequence *) result;
}

/**
 * @brief Return @p dist carrying the distance at each of its instants
 * @details A fold of per-component distances answers the minimum of two
 * piecewise-linear curves. Where the two cross, neither component carries a
 * turning point of its own, so both values come from interpolation; each
 * interpolation lies above its own convex curve, and the minimum of the two
 * lies above the distance. Every instant of a folded sequence therefore takes
 * the distance from the body placed at that instant to the whole geometry,
 * which is the value a turning point stands for.
 * @param[in] dist Folded distance sequence, consumed
 * @param[in] seq Temporal rigid geometry the distance is computed from
 * @param[in] gs Geometry the distance is computed to
 * @param[in] ref_gs Reference geometry of @p seq
 */
static TSequence *
dist2d_seq_exact_values(TSequence *dist, const TSequence *seq,
  const GSERIALIZED *gs, const GSERIALIZED *ref_gs)
{
  TInstant **instants = palloc(sizeof(TInstant *) * dist->count);
  for (int i = 0; i < dist->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(dist, i);
    Datum pose;
    /* The folded sequence spans the period of the rigid geometry, so the pose
     * is there; where a bound makes it absent the folded value stands */
    if (! tsequence_value_at_timestamptz(seq, inst->t, false, &pose))
    {
      instants[i] = tinstant_copy(inst);
      continue;
    }
    GSERIALIZED *body = pose_apply_geo(DatumGetPoseP(pose), ref_gs);
    instants[i] = tinstant_make(Float8GetDatum(geom_distance2d(body, gs)),
      T_TFLOAT, inst->t);
    pfree(body); pfree(DatumGetPointer(pose));
  }
  TSequence *result = tsequence_make_free(instants, dist->count,
    dist->period.lower_inc, dist->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(dist->flags), NORMALIZE);
  pfree(dist);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry
 * sequence and a multi-component geometry
 * @details The distance to a multi-component geometry is the pointwise minimum
 * of the distances to its components
 */
static TSequence *
dist2d_trgeoseq_multi(const TSequence *seq, const GSERIALIZED *gs,
  const GSERIALIZED *ref_gs, TSequence *(*distfunc)(const TSequence *,
  const GSERIALIZED *, const GSERIALIZED *))
{
  TSequence *result = NULL;
  int count = geo_num_geos(gs), nfolds = 0;
  for (int i = 1; i <= count; i++)
  {
    GSERIALIZED *comp = geo_geo_n(gs, i);
    /* An empty component contributes no point to be close to */
    if (gserialized_is_empty(comp))
    {
      pfree(comp);
      continue;
    }
    TSequence *dist = distfunc(seq, comp, ref_gs);
    pfree(comp);
    if (! dist)
    {
      if (result)
        pfree(result);
      return NULL;
    }
    if (! result)
      result = dist;
    else
    {
      TSequence *min = tfloatseq_min_tfloatseq(result, dist);
      pfree(result); pfree(dist);
      result = min;
      nfolds++;
    }
  }
  return nfolds ? dist2d_seq_exact_values(result, seq, gs, ref_gs) : result;
}

static GSERIALIZED *
line_segment_ring(const LWLINE *line, uint32_t i, int32_t srid)
{
  POINT4D a, b;
  getPoint4d_p(line->points, i, &a);
  getPoint4d_p(line->points, i + 1, &b);
  POINTARRAY **rings = lwalloc(sizeof(POINTARRAY *));
  rings[0] = ptarray_construct_empty(0, 0, 3);
  ptarray_append_point(rings[0], &a, LW_TRUE);
  ptarray_append_point(rings[0], &b, LW_TRUE);
  ptarray_append_point(rings[0], &a, LW_TRUE);
  LWPOLY *poly = lwpoly_construct(srid, NULL, 1, rings);
  GSERIALIZED *result = geo_serialize((LWGEOM *) poly);
  lwpoly_free(poly);
  return result;
}

static TSequence *
dist2d_trgeoseq_line(const TSequence *seq, const GSERIALIZED *gs,
  const GSERIALIZED *ref_gs)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  const LWLINE *line = lwgeom_as_lwline(geom);
  int32_t srid = gserialized_get_srid(gs);
  TSequence *result = NULL;
  int nfolds = 0;
  for (uint32_t i = 0; i + 1 < line->points->npoints; i++)
  {
    GSERIALIZED *ring = line_segment_ring(line, i, srid);
    TSequence *dist = dist2d_trgeoseq_poly(seq, ring, ref_gs);
    pfree(ring);
    if (! dist)
    {
      if (result) pfree(result);
      lwgeom_free(geom);
      return NULL;
    }
    if (! result) result = dist;
    else
    {
      TSequence *min = tfloatseq_min_tfloatseq(result, dist);
      pfree(result); pfree(dist);
      result = min;
      nfolds++;
    }
  }
  lwgeom_free(geom);
  return nfolds ? dist2d_seq_exact_values(result, seq, gs, ref_gs) : result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry sequence
 * and a geometry
 */
TSequence *
dist2d_trgeoseq_geo(const TSequence *seq, const GSERIALIZED *gs,
  const GSERIALIZED *ref_gs)
{
  uint32_t gs_type = gserialized_get_type(gs);
  TSequence *result = NULL;
  switch (gs_type)
  {
    case POINTTYPE:
      result = dist2d_trgeoseq_point(seq, gs, ref_gs);
      break;
    case POLYGONTYPE:
      result = dist2d_trgeoseq_poly(seq, gs, ref_gs);
      break;
    case MULTIPOINTTYPE:
      result = dist2d_trgeoseq_multi(seq, gs, ref_gs, &dist2d_trgeoseq_point);
      break;
    case MULTIPOLYGONTYPE:
      result = dist2d_trgeoseq_multi(seq, gs, ref_gs, &dist2d_trgeoseq_poly);
      break;
    case LINETYPE:
      result = dist2d_trgeoseq_line(seq, gs, ref_gs);
      break;
    case MULTILINETYPE:
      result = dist2d_trgeoseq_multi(seq, gs, ref_gs, &dist2d_trgeoseq_line);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "Unsupported geometry type: %s", lwtype_name(gs_type));
      break;
  }
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry sequence
 * set and a geometry
 */
TSequenceSet *
dist2d_trgeoseqset_geo(const TSequenceSet *ss, const GSERIALIZED *gs,
  const GSERIALIZED *ref_gs)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = dist2d_trgeoseq_geo(TSEQUENCESET_SEQ_N(ss, i), gs, ref_gs);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return the temporal distance between two synchronized temporal rigid
 * geometry instants
 */
static TInstant *
dist2d_trgeoinst_trgeoinst(const TInstant *inst1, const TInstant *inst2,
  const GSERIALIZED *ref_gs1, const GSERIALIZED *ref_gs2)
{
  GSERIALIZED *gs1 = pose_apply_geo(DatumGetPoseP(tinstant_value_p(inst1)),
    ref_gs1);
  GSERIALIZED *gs2 = pose_apply_geo(DatumGetPoseP(tinstant_value_p(inst2)),
    ref_gs2);
  double dist = geom_distance2d(gs1, gs2);
  pfree(gs1); pfree(gs2);
  return tinstant_make(Float8GetDatum(dist), T_TFLOAT, inst1->t);
}

/**
 * @brief Return the temporal distance between two synchronized temporal rigid
 * geometry sequences
 * @details The two reference polygons both move, so the closest-feature-pair
 * walk of @ref dist2d_trgeoseq_poly is reused by expressing the first polygon
 * in the moving frame of the second one (see @ref rel_pose_interpolate_2d):
 * @ref compute_dist_tpoly_poly and the transition solvers evaluate the first
 * polygon at that relative pose against the second polygon taken static, which
 * reproduces the exact distance between the two moving polygons.
 * @pre The two sequences are synchronized (same timestamps and count)
 */
TSequence *
dist2d_trgeoseq_trgeoseq(const TSequence *seq1, const TSequence *seq2,
  const GSERIALIZED *ref_gs1, const GSERIALIZED *ref_gs2)
{
  /* TODO: check that both polygons are convex */
  LWPOLY *poly1 = lwgeom_as_lwpoly(lwgeom_from_gserialized(ref_gs1));
  LWPOLY *poly2 = lwgeom_as_lwpoly(lwgeom_from_gserialized(ref_gs2));

  const TInstant *inst1, *jnst1;
  Pose *p1s, *p1e, *p2s, *p2e;

  inst1 = TSEQUENCE_INST_N(seq1, 0);
  jnst1 = TSEQUENCE_INST_N(seq2, 0);
  p1s = DatumGetPoseP(tinstant_value_p(inst1));
  p2s = DatumGetPoseP(tinstant_value_p(jnst1));

  /* Seed the closest features in the world frame; feature indices are frame
   * invariant, so the relative-frame walk below is consistent with them */
  cfp_array cfpa;
  init_cfp_array(&cfpa, seq1->count);
  cfp_elem cfp = cfp_make_zero((LWGEOM *) poly1, (LWGEOM *) poly2,
    rel_posesegm_interpolate(p1s, p1s, p2s, p2s, 0.0), NULL, inst1->t,
    MEOS_CFP_STORE);
  cfp.free_pose_1 = MEOS_CFP_FREE;
  v_clip_tpoly_tpoly(poly1, poly2, p1s, p2s, &cfp.cf_1, &cfp.cf_2, NULL);
  append_cfp_elem(&cfpa, cfp);
  for (int i = 0; i < seq1->count - 1; ++i)
  {
    inst1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq1, i + 1);
    jnst1 = TSEQUENCE_INST_N(seq2, i);
    const TInstant *jnst2 = TSEQUENCE_INST_N(seq2, i + 1);
    p1s = DatumGetPoseP(tinstant_value_p(inst1));
    p1e = DatumGetPoseP(tinstant_value_p(inst2));
    p2s = DatumGetPoseP(tinstant_value_p(jnst1));
    p2e = DatumGetPoseP(tinstant_value_p(jnst2));
    double ratio = 0.0;
    int loop = 0, state, dir1 = MEOS_ANY, dir2 = MEOS_ANY;
    do
    {
      if (cfp.cf_1 % 2 == 0 && cfp.cf_2 % 2 == 0)
        state = vertex_vertex_tpoly_poly(poly1, p1s, p1e, p2s, p2e, poly2,
          &cfp.cf_1, &cfp.cf_2, &dir1, &dir2, &ratio);
      else if (cfp.cf_1 % 2 == 0)
        state = vertex_edge_tpoly_poly(poly1, p1s, p1e, p2s, p2e, poly2,
          &cfp.cf_1, &cfp.cf_2, &dir1, &dir2, &ratio);
      else if (cfp.cf_2 % 2 == 0)
        state = edge_vertex_tpoly_poly(poly1, p1s, p1e, p2s, p2e, poly2,
          &cfp.cf_1, &cfp.cf_2, &dir1, &dir2, &ratio);
      else /* edge <-> edge */
      {
        /* Two edges are the closest feature pair, e.g. the parallel facing
         * edges of two convex polygons. No further feature transition is
         * tracked from this state, so the walk ends for the segment. The
         * distance at the segment ends and every interior turning point are
         * taken from the v-clip oracle, so the result stays exact. */
        state = MEOS_DISJOINT;
      }

      if (state == MEOS_CONTINUE)
      {
        cfp.t = inst1->t + (inst2->t - inst1->t) * ratio;
        cfp.pose_1 = rel_posesegm_interpolate(p1s, p1e, p2s, p2e, ratio);
        cfp.free_pose_1 = MEOS_CFP_FREE;
        cfp.store = MEOS_CFP_STORE_NO;
        append_cfp_elem(&cfpa, cfp);
      }

      if (loop++ == MEOS_MAX_ITERS) break;
    } while (state == MEOS_CONTINUE);

    if (loop > MEOS_MAX_ITERS)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Temporal distance: Cycle detected, current features: (%d, %d)",
        cfp.cf_1, cfp.cf_2);
      return NULL;
    }

    cfp.pose_1 = rel_posesegm_interpolate(p1s, p1e, p2s, p2e, 1.0);
    cfp.free_pose_1 = MEOS_CFP_FREE;
    cfp.t = inst2->t;
    cfp.store = MEOS_CFP_STORE;
    cfp_elem next_cfp = cfp;
    v_clip_tpoly_tpoly(poly1, poly2, p1e, p2e, &next_cfp.cf_1, &next_cfp.cf_2,
      NULL);
    append_cfp_elem(&cfpa, next_cfp);
    cfp = next_cfp;
  }

  /* Piecewise-linear distance: a point at every closest-feature time plus the
   * interior extrema of each fixed feature pair, evaluated with the actual
   * segment poses of both bodies in the relative frame */
  tdist_array tda;
  init_tdist_array(&tda, cfpa.count);
  for (uint32_t i = 0; i < cfpa.count - 1; ++i)
  {
    compute_dist_tpoly_poly(&cfpa.arr[i], &tda);
    int s = trgeoseq_segment_index(seq1,
      cfpa.arr[i].t + (cfpa.arr[i + 1].t - cfpa.arr[i].t) / 2);
    const TInstant *s1 = TSEQUENCE_INST_N(seq1, s);
    const TInstant *e1 = TSEQUENCE_INST_N(seq1, s + 1);
    const TInstant *s2 = TSEQUENCE_INST_N(seq2, s);
    const TInstant *e2 = TSEQUENCE_INST_N(seq2, s + 1);
    compute_turnpoints_tpoly_poly(&cfpa.arr[i], &cfpa.arr[i + 1],
      DatumGetPoseP(tinstant_value_p(s1)), DatumGetPoseP(tinstant_value_p(e1)),
      DatumGetPoseP(tinstant_value_p(s2)), DatumGetPoseP(tinstant_value_p(e2)),
      s1->t, e1->t, &tda);
  }
  compute_dist_tpoly_poly(&cfpa.arr[cfpa.count - 1], &tda);
  tdist_array_sort(&tda);

  TInstant **instants = palloc(sizeof(TInstant *) * tda.count);
  for (uint32_t i = 0; i < tda.count; ++i)
    instants[i] = tinstant_make(Float8GetDatum(tda.arr[i].dist), T_TFLOAT,
      tda.arr[i].t);
  TSequence *result = tsequence_make_free(instants, tda.count,
    seq1->period.lower_inc, seq1->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq1->flags), NORMALIZE);

  lwpoly_free(poly1); lwpoly_free(poly2);
  free_cfp_array(&cfpa); free_tdist_array(&tda);
  return result;
}

/**
 * @brief Return the temporal distance between two synchronized temporal rigid
 * geometry sequence sets
 * @pre The two sequence sets are synchronized (same sequence structure)
 */
static TSequenceSet *
dist2d_trgeoseqset_trgeoseqset(const TSequenceSet *ss1, const TSequenceSet *ss2,
  const GSERIALIZED *ref_gs1, const GSERIALIZED *ref_gs2)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->count);
  for (int i = 0; i < ss1->count; i++)
    sequences[i] = dist2d_trgeoseq_trgeoseq(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i), ref_gs1, ref_gs2);
  return tsequenceset_make_free(sequences, ss1->count, NORMALIZE);
}

/**
 * @brief Ensure that the closest-feature walk can read a reference geometry
 * @details The sequence kernels of this file read the body as an @p LWPOLY --
 * the closest-feature walk is written on a convex polygon -- and answer nothing
 * for a reference geometry of another type, which the constructor nevertheless
 * accepts: a polyhedral surface is a legal reference geometry.  Reporting it
 * here is what keeps such a pair from reaching a cast that answers NULL and a
 * walk that reads it.
 * @note The INSTANT paths need no such reference: they place the body with
 * #pose_apply_geo and measure it whole, so they answer for any body and are
 * left unguarded.
 */
static bool
ensure_trgeo_ref_walkable(const GSERIALIZED *ref_gs)
{
  uint32_t type = gserialized_get_type(ref_gs);
  if (type == POLYGONTYPE)
    return true;
  meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
    "The temporal distance of a rigid geometry with a reference geometry of "
    "type %s is not supported", lwtype_name(type));
  return false;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between a temporal rigid geometry and a
 * geometry/geography point
 * @param[in] temp Temporal
 * @param[in] gs Geometry
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_geo() #Tdistance_geo_trgeometry()
 */
Temporal *
tdistance_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  if (MEOS_FLAGS_GET_Z(temp->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Distance computation in 3D is not currently supported");
    return NULL;
  }

  /* The reference geometry of the rigid body is invariant in time; read it
   * from the temporal value because a sequence set stores it once and not in
   * each of its composing sequences */
  const GSERIALIZED *ref_gs = trgeo_geom_p(temp);

  if (temp->subtype != TINSTANT && ! ensure_trgeo_ref_walkable(ref_gs))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) dist2d_trgeoinst_geo((const TInstant *) temp, gs);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) dist2d_trgeoseq_geo((const TSequence *) temp, gs,
      ref_gs);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) dist2d_trgeoseqset_geo((const TSequenceSet *) temp,
      gs, ref_gs);
  return result;
}

/**
 * @brief Return a temporal rigid geometry instant whose pose is the pose of
 * @p pinst translated by minus the point value of @p qinst, keeping @p ref_gs
 * as its reference geometry
 * @details A temporal point does not rotate, so subtracting its position from
 * the body pose places the point at the origin while the body pose stays a
 * linear segment: the trgeometry-vs-tgeompoint distance then reduces to the
 * trgeometry-vs-point distance against the origin.
 */
static TInstant *
trgeoinst_translate_by_tpoint(const TInstant *pinst, const TInstant *qinst,
  const GSERIALIZED *ref_gs)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(pinst));
  POINT4D pt;
  datum_point4d(tinstant_value_p(qinst), &pt);
  Pose *tpose = pose_make_2d(pose->data[0] - pt.x, pose->data[1] - pt.y,
    pose->data[2], MEOS_FLAGS_GET_GEODETIC(pose->flags), pose_srid(pose));
  TInstant *result = trgeometryinst_make(ref_gs, tpose, pinst->t);
  pfree(tpose);
  return result;
}

/**
 * @brief Return the temporal rigid geometry @p pseq with each pose translated
 * by minus the corresponding point value of @p qseq
 * @pre The two sequences are synchronized
 */
static TSequence *
trgeoseq_translate_by_tpoint(const TSequence *pseq, const TSequence *qseq,
  const GSERIALIZED *ref_gs)
{
  TInstant **instants = palloc(sizeof(TInstant *) * pseq->count);
  for (int i = 0; i < pseq->count; i++)
    instants[i] = trgeoinst_translate_by_tpoint(TSEQUENCE_INST_N(pseq, i),
      TSEQUENCE_INST_N(qseq, i), ref_gs);
  return trgeoseq_make_free(ref_gs, instants, pseq->count,
    pseq->period.lower_inc, pseq->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(pseq->flags), NORMALIZE);
}

/**
 * @brief Return the temporal rigid geometry @p pss with each pose translated
 * by minus the corresponding point value of @p qss
 * @pre The two sequence sets are synchronized
 */
static TSequenceSet *
trgeoseqset_translate_by_tpoint(const TSequenceSet *pss,
  const TSequenceSet *qss, const GSERIALIZED *ref_gs)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * pss->count);
  for (int i = 0; i < pss->count; i++)
    sequences[i] = trgeoseq_translate_by_tpoint(TSEQUENCESET_SEQ_N(pss, i),
      TSEQUENCESET_SEQ_N(qss, i), ref_gs);
  return trgeoseqset_make_free(ref_gs, sequences, pss->count, NORMALIZE);
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between a temporal rigid geometry and a
 * temporal geometry point
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_tpoint() #Tdistance_tpoint_trgeometry()
 */
Temporal *
tdistance_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return NULL;

  if (MEOS_FLAGS_GET_Z(temp1->flags) || MEOS_FLAGS_GET_Z(temp2->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Distance computation in 3D is not currently supported");
    return NULL;
  }

  /* The reference geometry of the rigid body is invariant in time; read it
   * from the original because synchronization keeps only the temporal pose */
  const GSERIALIZED *ref_gs = trgeo_geom_p(temp1);

  /* Synchronize the two temporal values without adding crossings */
  Temporal *sync1, *sync2;
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  /* Translate the body pose by minus the point so the point sits at the origin,
   * then reuse the trgeometry-vs-point distance against the origin */
  GSERIALIZED *origin = geopoint_make(0.0, 0.0, 0.0, false, false,
    tspatial_srid(temp1));
  Temporal *result;
  assert(temptype_subtype(sync1->subtype));
  if (sync1->subtype == TINSTANT)
  {
    TInstant *tinst = trgeoinst_translate_by_tpoint((const TInstant *) sync1,
      (const TInstant *) sync2, ref_gs);
    GSERIALIZED *posed = pose_apply_geo(DatumGetPoseP(tinstant_value_p(tinst)),
      ref_gs);
    double dist = geom_distance2d(posed, origin);
    result = (Temporal *) tinstant_make(Float8GetDatum(dist), T_TFLOAT,
      tinst->t);
    pfree(posed); pfree(tinst);
  }
  else if (sync1->subtype == TSEQUENCE)
  {
    TSequence *tseq = trgeoseq_translate_by_tpoint((const TSequence *) sync1,
      (const TSequence *) sync2, ref_gs);
    result = tdistance_trgeometry_geo((const Temporal *) tseq, origin);
    pfree(tseq);
  }
  else /* sync1->subtype == TSEQUENCESET */
  {
    TSequenceSet *tss = trgeoseqset_translate_by_tpoint(
      (const TSequenceSet *) sync1, (const TSequenceSet *) sync2, ref_gs);
    result = tdistance_trgeometry_geo((const Temporal *) tss, origin);
    pfree(tss);
  }
  pfree(sync1); pfree(sync2); pfree(origin);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between two temporal rigid geometries
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_trgeometry()
 */
Temporal *
tdistance_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;

  if (MEOS_FLAGS_GET_Z(temp1->flags) || MEOS_FLAGS_GET_Z(temp2->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Distance computation in 3D is not currently supported");
    return NULL;
  }

  /* The reference geometry of each rigid body is invariant in time; read it
   * from the originals because synchronization keeps only the temporal pose */
  const GSERIALIZED *ref_gs1 = trgeo_geom_p(temp1);
  const GSERIALIZED *ref_gs2 = trgeo_geom_p(temp2);

  /* Synchronize the temporal rigid geometries without adding crossings */
  Temporal *sync1, *sync2;
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  if (sync1->subtype != TINSTANT &&
      (! ensure_trgeo_ref_walkable(ref_gs1) ||
       ! ensure_trgeo_ref_walkable(ref_gs2)))
  {
    pfree(sync1); pfree(sync2);
    return NULL;
  }

  Temporal *result;
  assert(temptype_subtype(sync1->subtype));
  if (sync1->subtype == TINSTANT)
    result = (Temporal *) dist2d_trgeoinst_trgeoinst((const TInstant *) sync1,
      (const TInstant *) sync2, ref_gs1, ref_gs2);
  else if (sync1->subtype == TSEQUENCE)
    result = (Temporal *) dist2d_trgeoseq_trgeoseq((const TSequence *) sync1,
      (const TSequence *) sync2, ref_gs1, ref_gs2);
  else /* sync1->subtype == TSEQUENCESET */
    result = (Temporal *) dist2d_trgeoseqset_trgeoseqset(
      (const TSequenceSet *) sync1, (const TSequenceSet *) sync2, ref_gs1,
      ref_gs2);
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between a temporal rigid geometry
 * and a geometry
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_geo() #NAI_geo_trgeometry()
 */
TInstant *
nai_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  TInstant *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_copy((TInstant *) temp);
  else
  {
    Temporal *dist = tdistance_trgeometry_geo(temp, gs);
    if (dist != NULL)
    {
      const TInstant *min = temporal_min_inst_p(dist);
      /* The closest point may be at an exclusive bound. */
      Datum value;
      temporal_value_at_timestamptz(temp, min->t, false, &value);
      result = trgeometryinst_make(trgeo_geom_p(temp), DatumGetPoseP(value),
        min->t);
      pfree(dist); pfree(DatumGetPointer(value));
    }
  }
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between a temporal rigid
 * geometry and a temporal point
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_tpoint() #NAI_tpoint_trgeometry()
 */
TInstant *
nai_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_tpoint(temp1, temp2);
  if (dist != NULL)
  {
    /* temporal_min_instant returns a copy that must be freed */
    TInstant *min = temporal_min_instant(dist);
    /* The closest point may be at an exclusive bound */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeo_geom_p(temp1), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(min); pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between two temporal rigid
 * geometries
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_trgeometry()
 */
TInstant *
nai_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_trgeometry(temp1, temp2);
  if (dist != NULL)
  {
    /* temporal_min_instant returns a copy that must be freed */
    TInstant *min = temporal_min_instant(dist);
    /* The closest point may be at an exclusive bound. */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeo_geom_p(temp1), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(min); pfree(DatumGetPointer(value));
  }
  return result;
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a geometry
 * @csqlfn #NAD_trgeometry_geo() #NAD_geo_trgeometry()
 */
double
nad_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return DBL_MAX;

  Temporal *dist = tdistance_trgeometry_geo(temp, gs);
  if (dist == NULL)
    return DBL_MAX;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a spatiotemporal box
 * @csqlfn #NAD_trgeometry_stbox() #NAD_stbox_trgeometry()
 */
double
nad_trgeometry_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_stbox(temp, box))
    return DBL_MAX;

  /* Project the temporal value to the timespan of the box */
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  Span p, inter;
  Temporal *temp1 = (Temporal *) temp;
  if (hast)
  {
    temporal_set_tstzspan(temp, &p);
    if (! inter_span_span(&p, &box->period, &inter))
      return DBL_MAX;
    /* The generic temporal restriction drops the reference geometry, so the
     * rigid geometry restriction is the one that answers a trgeometry */
    temp1 = trgeometry_restrict_tstzspan(temp, &inter, REST_AT);
    /* The two spans meet while no value lies inside the intersection, which a
     * discrete or a step value can do */
    if (! temp1)
      return DBL_MAX;
  }
  /* Convert the stbox to a geometry */
  GSERIALIZED *geo = stbox_geo(box);
  /* Compute the result */
  Temporal *dist = tdistance_trgeometry_geo(temp1, geo);
  if (dist == NULL)
  {
    pfree(geo);
    if (hast)
      pfree(temp1);
    return DBL_MAX;
  }

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist); pfree(geo);
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box and
 * a temporal rigid geometry
 */
double
nad_stbox_trgeometry(const STBox *box, const Temporal *temp)
{
  return nad_trgeometry_stbox(temp, box);
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a temporal point
 * @csqlfn #NAD_trgeometry_tpoint() #NAD_tpoint_trgeometry()
 */
double
nad_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return DBL_MAX;

  Temporal *dist = tdistance_trgeometry_tpoint(temp1, temp2);
  if (dist == NULL)
    return DBL_MAX;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between two temporal rigid
 * geometries
 * @csqlfn #NAD_trgeometry_trgeometry()
 */
double
nad_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return DBL_MAX;

  Temporal *dist = tdistance_trgeometry_trgeometry(temp1, temp2);
  if (dist == NULL)
    return DBL_MAX;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal rigid geometry and a geometry
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_geo() #Shortestline_geo_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Temporal *dist = tdistance_trgeometry_geo(temp, gs);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value;
  trgeo_value_at_timestamptz(temp, inst->t, false, &value);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value), gs);
  pfree(DatumGetPointer(value)); pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal rigid geometry and a temporal geometry point
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_tpoint() #Shortestline_tpoint_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return NULL;

  Temporal *dist = tdistance_trgeometry_tpoint(temp1, temp2);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeo_value_at_timestamptz(temp1, inst->t, false, &value1);
  temporal_value_at_timestamptz(temp2, inst->t, false, &value2);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value1),
    DatumGetGserializedP(value2));
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2)); pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal rigid geometries
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;

  Temporal *dist = tdistance_trgeometry_trgeometry(temp1, temp2);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeo_value_at_timestamptz(temp1, inst->t, false, &value1);
  trgeo_value_at_timestamptz(temp2, inst->t, false, &value2);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value1),
    DatumGetGserializedP(value2));
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2)); pfree(dist);
  return result;
}

/*****************************************************************************/

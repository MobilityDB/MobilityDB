/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Distance functions for temporal geos
 */

#include "geo/tgeo_distance.h"

/* C */
#include <assert.h>
/* PostGIS */
#include <lwgeodetic_tree.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "geo/postgis_funcs.h"
#include "geo/geography_funcs.h"
#include "geo/tgeo_spatialfuncs.h"

/* Functions not exported by PostGIS */
extern double circ_tree_distance_tree_internal(const CIRC_NODE* n1,
  const CIRC_NODE* n2, double threshold, double* min_dist, double* max_dist,
  GEOGRAPHIC_POINT* closest1, GEOGRAPHIC_POINT* closest2);

/*****************************************************************************
 * Compute the distance between two temporal geo instants
 *****************************************************************************/

/**
 * @brief Return the distance between two temporal instants
 * @param[in] inst1,inst2 Temporal instants
 * @param[in] func Distance function
 */
inline double
tgeoinst_distance(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func)
{
  return DatumGetFloat8(func(tinstant_value_p(inst1), tinstant_value_p(inst2)));
}

/*****************************************************************************/

/**
 * @brief Return the distance between two geometries
 * @details When the first geometry is a segment it also computes a value
 * between 0 and 1 that represents the location in the segment of the closest
 * point to the second geometry, as a fraction of total segment length.
 * @note Function inspired by PostGIS function lw_dist2d_distancepoint
 * from measures.c
 */
static double
lw_distance_fraction(const LWGEOM *geom1, const LWGEOM *geom2, int mode,
  double *fraction)
{
  double result;
  if (FLAGS_GET_GEODETIC(geom1->flags))
  {
    double min_dist = FLT_MAX;
    double max_dist = FLT_MAX;
    GEOGRAPHIC_POINT closest1, closest2;
    GEOGRAPHIC_EDGE e;
    CIRC_NODE *circ_tree1 = lwgeom_calculate_circ_tree(geom1);
    CIRC_NODE *circ_tree2 = lwgeom_calculate_circ_tree(geom2);
    circ_tree_distance_tree_internal(circ_tree1, circ_tree2, FP_TOLERANCE,
      &min_dist, &max_dist, &closest1, &closest2);
    result = sphere_distance(&closest1, &closest2);
    if (fraction)
    {
      assert(geom1->type == LINETYPE);
      LWLINE *lwline = lwgeom_as_lwline(geom1);
      /* Initialize edge */
      POINT4D a, b;
      GEOGRAPHIC_POINT proj;
      getPoint4d_p(lwline->points, 0, &a);
      getPoint4d_p(lwline->points, 1, &b);
      geographic_point_init(a.x, a.y, &(e.start));
      geographic_point_init(b.x, b.y, &(e.end));
      /* Get the spherical distance between point and edge */
      edge_distance_to_point(&e, &closest1, &proj);
      /* Compute distance from beginning of the segment to closest point */
      double seglength = sphere_distance(&(e.start), &(e.end));
      double length = sphere_distance(&(e.start), &proj);
      *fraction = length / seglength;
    }
  }
  else
  {
    if (FLAGS_GET_Z(geom1->flags))
    {
      DISTPTS3D dl;
      dl.mode = mode;
      dl.distance= FLT_MAX;
      dl.tolerance = 0;
      lw_dist3d_recursive(geom1, geom2, &dl);
      result = dl.distance;
      if (fraction)
      {
        assert(geom1->type == LINETYPE);
        LWLINE *lwline = lwgeom_as_lwline(geom1);
        POINT3DZ a, b, closest;
        getPoint3dz_p(lwline->points, 0, &a);
        getPoint3dz_p(lwline->points, 1, &b);
        *fraction = closest_point3dz_on_segment_ratio(&dl.p1, &a, &b, &closest);
      }
    }
    else
    {
      DISTPTS dl;
      dl.mode = mode;
      dl.distance= FLT_MAX;
      dl.tolerance = 0;
      lw_dist2d_recursive(geom1, geom2, &dl);
      result = dl.distance;
      if (fraction)
      {
        assert(geom1->type == LINETYPE);
        LWLINE *lwline = lwgeom_as_lwline(geom1);
        POINT2D a, b, closest;
        getPoint2d_p(lwline->points, 0, &a);
        getPoint2d_p(lwline->points, 1, &b);
        *fraction = closest_point2d_on_segment_ratio(&dl.p1, &a, &b, &closest);
      }
    }
  }
  return result;
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Return the distance and timestamp at which a temporal geo segment
 * and a point are at the minimum distance
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start,end Instants defining the first segment
 * @param[in] point Point
 * @param[in] basetype Type of the point
 * @param[out] value Minimum distance at turning point
 * @param[out] t Timestamp at turning point
 * @pre The segment is not constant.
 * @note The parameter basetype is not needed for temporal geos
 */
static bool
tpoint_geo_min_dist_at_timestamptz(const TInstant *start, const TInstant *end,
  Datum point, meosType basetype __attribute__((unused)), Datum *value,
  TimestampTz *t)
{
  long double duration = (long double) (end->t - start->t);
  Datum value1 = tinstant_value_p(start);
  Datum value2 = tinstant_value_p(end);
  double dist;
  double fraction = pointsegm_locate(value1, value2, point, &dist);
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return false;
  *value = Float8GetDatum(dist);
  *t = start->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * @brief Compute d/dx (Euclidean distance) = 0 for two 2D segments
 */
static bool
point2d_min_dist(const POINT2D *p1, const POINT2D *p2, const POINT2D *p3,
  const POINT2D *p4, double *fraction)
{
  long double denum, dx1, dy1, dx2, dy2, f1, f2, f3, f4;

  dx1 = p2->x - p1->x;
  dy1 = p2->y - p1->y;
  dx2 = p4->x - p3->x;
  dy2 = p4->y - p3->y;

  f1 = p3->x * (dx1 - dx2);
  f2 = p1->x * (dx2 - dx1);
  f3 = p3->y * (dy1 - dy2);
  f4 = p1->y * (dy2 - dy1);

  denum = dx1 * (dx1 - 2 * dx2) + dy1 * (dy1 - 2 * dy2) + dy2 * dy2 +
    dx2 * dx2;
  if (denum == 0)
    return false;

  *fraction = (f1 + f2 + f3 + f4) / denum;
  if (*fraction <= MEOS_EPSILON || *fraction >= (1.0 - MEOS_EPSILON))
    return false;

  return true;
}

/**
 * @brief Compute d/dx (Euclidean distance) = 0 for two 3D segments
 */
static bool
point3d_min_dist(const POINT3DZ *p1, const POINT3DZ *p2, const POINT3DZ *p3,
  const POINT3DZ *p4, double *fraction)
{
  long double denum, dx1, dy1, dz1, dx2, dy2, dz2, f1, f2, f3, f4, f5, f6;

  dx1 = p2->x - p1->x;
  dy1 = p2->y - p1->y;
  dz1 = p2->z - p1->z;
  dx2 = p4->x - p3->x;
  dy2 = p4->y - p3->y;
  dz2 = p4->z - p3->z;

  f1 = p3->x * (dx1 - dx2);
  f2 = p1->x * (dx2 - dx1);
  f3 = p3->y * (dy1 - dy2);
  f4 = p1->y * (dy2 - dy1);
  f5 = p3->z * (dz1 - dz2);
  f6 = p1->z * (dz2 - dz1);

  denum = dx1 * (dx1 - 2 * dx2) + dy1 * (dy1 - 2 * dy2) +
    dz1 * (dz1 - 2 * dz2) + dx2 * dx2 + dy2 * dy2 + dz2 * dz2;
  if (denum == 0)
    return false;

  *fraction = (f1 + f2 + f3 + f4 + f5 + f6) / denum;
  if (*fraction <= MEOS_EPSILON || *fraction >= (1.0 - MEOS_EPSILON))
    return false;

  return true;
}

/**
 * @brief Return the distance and timestamp at which two temporal geometry
 * point segments are at the minimum distance
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] mindist Mininum distance
 * @param[out] t Timestamp
 * @note The PostGIS functions @p lw_dist2d_seg_seg and @p lw_dist3d_seg_seg
 * cannot be used since they do not take time into consideration and would
 * return, e.g., that the minimum distance between the two following segments
 * `[Point(2 2)@t1, Point(1 1)@t2]` and `[Point(3 1)@t1, Point(1 1)@t2]`
 * is at `Point(2 2)@t2` instead of `Point(1.5 1.5)@(t1 + (t2 - t1)/2)`.
 * @pre The segments are not both constants.
 */
bool
tgeompoint_min_dist_at_timestamptz(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *mindist, TimestampTz *t)
{
  double fraction;
  bool hasz = MEOS_FLAGS_GET_Z(start1->flags);
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(tinstant_value_p(start1));
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(tinstant_value_p(end1));
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(tinstant_value_p(start2));
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(tinstant_value_p(end2));
    bool found = point3d_min_dist(p1, p2, p3, p4, &fraction);
    if (!found)
      return false;
  }
  else /* 2D */
  {
    const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value_p(start1));
    const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value_p(end1));
    const POINT2D *p3 = DATUM_POINT2D_P(tinstant_value_p(start2));
    const POINT2D *p4 = DATUM_POINT2D_P(tinstant_value_p(end2));
    bool found = point2d_min_dist(p1, p2, p3, p4, &fraction);
    if (!found)
      return false;
  }

  double duration = end1->t - start1->t;
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* We know that this function is called only for linear segments */
  Datum value1 = tsegment_value_at_timestamptz(start1, end1, LINEAR, *t);
  Datum value2 = tsegment_value_at_timestamptz(start2, end2, LINEAR, *t);
  *mindist = hasz ? datum_geom_distance3d(value1, value2) :
    datum_geom_distance2d(value1, value2);
  return true;
}

/**
 * @brief Return the single timestamp at which the two temporal geography
 * point segments are at the minimum distance
 * @details These are the turning points when computing the temporal distance
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] mindist Minimum distance
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
bool
tgeogpoint_min_dist_at_timestamptz(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *mindist, TimestampTz *t)
{
  const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value_p(start1));
  const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value_p(end1));
  const POINT2D *q1 = DATUM_POINT2D_P(tinstant_value_p(start2));
  const POINT2D *q2 = DATUM_POINT2D_P(tinstant_value_p(end2));
  GEOGRAPHIC_EDGE e1, e2;
  GEOGRAPHIC_POINT close1, close2;
  POINT3D A1, A2, B1, B2;
  geographic_point_init(p1->x, p1->y, &(e1.start));
  geographic_point_init(p2->x, p2->y, &(e1.end));
  geographic_point_init(q1->x, q1->y, &(e2.start));
  geographic_point_init(q2->x, q2->y, &(e2.end));
  geog2cart(&(e1.start), &A1);
  geog2cart(&(e1.end), &A2);
  geog2cart(&(e2.start), &B1);
  geog2cart(&(e2.end), &B2);
  double fraction;
  // TODO: The next computation should be done on geodetic coordinates
  // The value found by the linear approximation below could be the starting
  // point for an iterative method such as gradient descent or Newton's method
  bool found = point3d_min_dist((const POINT3DZ *) &A1, (const POINT3DZ *) &A2,
    (const POINT3DZ *) &B1, (const POINT3DZ *) &B2, &fraction);
  if (! found)
    return false;

  if (mindist)
  {
    /* Calculate distance and direction for the edges */
    double dist1 = sphere_distance(&(e1.start), &(e1.end));
    double dir1 = sphere_direction(&(e1.start), &(e1.end), dist1);
    double dist2 = sphere_distance(&(e2.start), &(e2.end));
    double dir2 = sphere_direction(&(e2.start), &(e2.end), dist2);
    /* Compute minimum distance */
    int res = sphere_project(&(e1.start), dist1 * fraction, dir1, &close1);
    if (res == LW_FAILURE)
      return false;
    res = sphere_project(&(e2.start), dist2 * fraction, dir2, &close2);
    if (res == LW_FAILURE)
      return false;
    double dist = sphere_distance(&close1, &close2);
    /* Ensure robustness */
    if (fabs(dist) < FP_TOLERANCE)
      dist = 0.0;
    *mindist = Float8GetDatum(dist);
  }

  /* Compute the timestamp of intersection */
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return false;
  double duration = (double) (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * @brief Return the value and timestamp at which the two temporal geo
 * segments are at the minimum distance.
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] value Value
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
bool
tpoint_min_dist_at_timestamptz(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  if (MEOS_FLAGS_GET_GEODETIC(start1->flags))
    return tgeogpoint_min_dist_at_timestamptz(start1, end1, start2, end2,
      value, t);
  else
    return tgeompoint_min_dist_at_timestamptz(start1, end1, start2, end2,
      value, t);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_dist
 * @brief Return the temporal distance between a temporal geo and a
 * geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #Distance_tgeo_geo()
 */
Temporal *
distance_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      (tpoint_type(temp->temptype) && ! ensure_point_type(gs)) ||
      gserialized_is_empty(gs))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) distance_fn(temp->flags);
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = lfinfo.reslinear ?
    &tpoint_geo_min_dist_at_timestamptz : NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}

/**
 * @ingroup meos_geo_dist
 * @brief Return the temporal distance between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Distance_tgeo_tgeo()
 */
Temporal *
distance_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, NULL); VALIDATE_TGEO(temp2, NULL);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) pt_distance_fn(temp1->flags);
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = lfinfo.reslinear ? &tpoint_min_dist_at_timestamptz : NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @brief Return the new current nearest approach instant between a temporal
 * point sequence with step interpolation and a geometry/geography
 * (iterator function)
 * @param[in] seq Temporal geo
 * @param[in] geo Geometry/geography
 * @param[in] mindist Current minimum distance, it is set at DBL_MAX at the
 * begining but contains the minimum distance found in the previous
 * sequences of a temporal sequence set
 * @param[out] result Instant with the minimum distance
 * @return Minimum distance
 */
static double
nai_tgeoseq_discstep_geo_iter(const TSequence *seq, const LWGEOM *geo,
  double mindist, const TInstant **result)
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
    LWGEOM *point = lwgeom_from_gserialized(gs);
    double dist = lw_distance_fraction(point, geo, DIST_MIN, NULL);
    if (dist < mindist)
    {
      mindist = dist;
      *result = inst;
    }
    lwgeom_free(point);
  }
  return mindist;
}

/**
 * @brief Return the nearest approach instant between a temporal sequence
 * point with step interpolation and a geometry/geography
 * @param[in] seq Temporal geo
 * @param[in] geo Geometry/geography
 */
static TInstant *
nai_tgeoseq_discstep_geo(const TSequence *seq, const LWGEOM *geo)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  nai_tgeoseq_discstep_geo_iter(seq, geo, DBL_MAX, &inst);
  return tinstant_copy(inst);
}

/**
 * @brief Return the nearest approach instant between a temporal sequence set
 * point with step interpolation and a geometry/geography
 * @param[in] ss Temporal geo
 * @param[in] geo Geometry/geography
 */
static TInstant *
nai_tgeoseqset_step_geo(const TSequenceSet *ss, const LWGEOM *geo)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  double mindist = DBL_MAX;
  for (int i = 0; i < ss->count; i++)
    mindist = nai_tgeoseq_discstep_geo_iter(TSEQUENCESET_SEQ_N(ss, i), geo,
      mindist, &inst);
  assert(inst);
  return tinstant_copy(inst);
}

/*****************************************************************************/

/**
 * @brief Return the distance and the timestamp of the nearest approach instant
 * between a temporal point sequence with linear interpolation and a
 * geometry/geography
 * @param[in] inst1,inst2 Temporal segment
 * @param[in] geo Geometry/geography
 * @param[out] t Timestamp
 */
static double
nai_tpointsegm_linear_geo1(const TInstant *inst1, const TInstant *inst2,
  const LWGEOM *geo, TimestampTz *t)
{
  Datum value1 = tinstant_value_p(inst1);
  Datum value2 = tinstant_value_p(inst2);
  double dist;
  double fraction;

  /* Constant segment */
  if (datum_point_eq(value1, value2))
  {
    GSERIALIZED *gs = DatumGetGserializedP(value1);
    LWGEOM *point = lwgeom_from_gserialized(gs);
    dist = lw_distance_fraction(point, geo, DIST_MIN, NULL);
    lwgeom_free(point);
    *t = inst1->t;
    return dist;
  }

  /* The trajectory is a line */
  LWGEOM *line = (LWGEOM *) lwline_make(value1, value2);
  dist = lw_distance_fraction(line, geo, DIST_MIN, &fraction);
  lwgeom_free(line);

  if (fabsl(fraction) < MEOS_EPSILON)
    *t = inst1->t;
  else if (fabsl(fraction - 1.0) < MEOS_EPSILON)
    *t = inst2->t;
  else
  {
    double duration = (double) (inst2->t - inst1->t);
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return dist;
}

/**
 * @brief Return the distance and the timestamp of the nearest approach instant
 * between a temporal point sequence with linear interpolation and a
 * geometry/geography (iterator function)
 * @param[in] seq Temporal geo
 * @param[in] geo Geometry/geography
 * @param[in] mindist Minimum distance found so far, or DBL_MAX at the beginning
 * @param[out] t Timestamp
 */
static double
nai_tpointseq_linear_geo_iter(const TSequence *seq, const LWGEOM *geo,
  double mindist, TimestampTz *t)
{
  double dist;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);

  if (seq->count == 1)
  {
    /* Instantaneous sequence */
    Datum value1 = tinstant_value_p(inst1);
    GSERIALIZED *gs = DatumGetGserializedP(value1);
    LWGEOM *point = lwgeom_from_gserialized(gs);
    dist = lw_distance_fraction(point, geo, DIST_MIN, NULL);
    if (dist < mindist)
    {
      mindist = dist;
      *t = inst1->t;
    }
    lwgeom_free(point);
  }
  else
  {
    /* General case */
    TimestampTz t1;
    for (int i = 0; i < seq->count - 1; i++)
    {
      const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
      dist = nai_tpointsegm_linear_geo1(inst1, inst2, geo, &t1);
      if (dist < mindist)
      {
        mindist = dist;
        *t = t1;
      }
      if (mindist == 0.0)
        break;
      inst1 = inst2;
    }
  }
  return mindist;
}

/**
 * @brief Return the nearest approach instant between a temporal sequence
 * point with linear interpolation and a geometry (iterator function)
 */
static TInstant *
nai_tpointseq_linear_geo(const TSequence *seq, const LWGEOM *geo)
{
  TimestampTz t;
  nai_tpointseq_linear_geo_iter(seq, geo, DBL_MAX, &t);
  /* The closest point may be at an exclusive bound */
  Datum value;
  tsequence_value_at_timestamptz(seq, t, false, &value);
  return tinstant_make_free(value, seq->temptype, t);
}

/**
 * @brief Return the nearest approach instant between a temporal sequence set
 * point with linear interpolation and a geometry
 */
static TInstant *
nai_tpointseqset_linear_geo(const TSequenceSet *ss, const LWGEOM *geo)
{
  TimestampTz t = 0; /* make compiler quiet */
  double mindist = DBL_MAX;
  for (int i = 0; i < ss->count; i++)
  {
    TimestampTz t1;
    double dist = nai_tpointseq_linear_geo_iter(TSEQUENCESET_SEQ_N(ss, i), geo,
      mindist, &t1);
    if (dist < mindist)
    {
      mindist = dist;
      t = t1;
    }
    if (mindist == 0.0)
      break;
  }
  /* The closest point may be at an exclusive bound. */
  Datum value;
  tsequenceset_value_at_timestamptz(ss, t, false, &value);
  return tinstant_make_free(value, ss->temptype, t);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_dist
 * @brief Return the nearest approach instant between a temporal geo and
 * a geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #NAI_tgeo_geo()
 */
TInstant *
nai_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return NULL;

  LWGEOM *geo = lwgeom_from_gserialized(gs);
  TInstant *result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = tinstant_copy((TInstant *) temp);
      break;
    case TSEQUENCE:
      result = MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        nai_tpointseq_linear_geo((TSequence *) temp, geo) :
        nai_tgeoseq_discstep_geo((TSequence *) temp, geo);
      break;
    default: /* TSEQUENCESET */
      result = MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        nai_tpointseqset_linear_geo((TSequenceSet *) temp, geo) :
        nai_tgeoseqset_step_geo((TSequenceSet *) temp, geo);
  }
  lwgeom_free(geo);
  return result;
}

/**
 * @ingroup meos_geo_dist
 * @brief Return the nearest approach instant between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #NAI_tgeo_tgeo()
 */
TInstant *
nai_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, NULL); VALIDATE_TGEO(temp2, NULL);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return NULL;

  /* Compute the temporal distance, it may be NULL if the points do not
   * intersect on time */
  Temporal *dist = distance_tgeo_tgeo(temp1, temp2);
  if (dist == NULL)
    return NULL;

  const TInstant *min = temporal_min_instant(dist);
  pfree(dist);
  /* The closest point may be at an exclusive bound => 3rd argument = false */
  Datum value;
  temporal_value_at_timestamptz(temp1, min->t, false, &value);
  return tinstant_make_free(value, temp1->temptype, min->t);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_geo_dist
 * @brief Return the nearest approach distance between a temporal geo
 * and a geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #NAD_tgeo_geo()
 */
double
nad_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, -1.0); VALIDATE_NOT_NULL(gs, -1.0);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return -1.0;

  datum_func2 func = distance_fn(temp->flags);
  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp) : tgeo_traversed_area(temp);
  double result = DatumGetFloat8(
    func(PointerGetDatum(traj), PointerGetDatum(gs)));
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_geo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry/geography
 * @param[in] box Spatiotemporal box/geography
 * @param[in] gs Geometry
 * @csqlfn #NAD_stbox_geo()
 */
double
nad_stbox_geo(const STBox *box, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, -1.0); VALIDATE_NOT_NULL(gs, -1.0);
  if (! ensure_valid_stbox_geo(box, gs) ||
      ! ensure_same_spatial_dimensionality_stbox_geo(box, gs))
    return -1.0;

  datum_func2 func = distance_fn(box->flags);
  Datum geo = PointerGetDatum(stbox_geo(box));
  double result = DatumGetFloat8(func(geo, PointerGetDatum(gs)));
  pfree(DatumGetPointer(geo));
  return result;
}

/**
 * @ingroup meos_geo_dist
 * @brief Return the nearest approach distance between two spatiotemporal
 * boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @return On error return -1.0
 * @csqlfn #NAD_stbox_stbox ()
 */
double
nad_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, -1.0); VALIDATE_NOT_NULL(box2, -1.0);
  if (! ensure_valid_spatial_stbox_stbox(box1, box2) ||
      ! ensure_same_spatial_dimensionality(box1->flags, box2->flags))
    return -1.0;

  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
      return DBL_MAX;

  /* If the boxes intersect in the value dimension return 0 */
  if (box1->xmin <= box2->xmax && box2->xmin <= box1->xmax)
    return 0.0;

  /* Select the distance function to be applied */
  datum_func2 func = distance_fn(box1->flags);
  /* Convert the boxes to geometries */
  Datum geo1 = PointerGetDatum(stbox_geo(box1));
  Datum geo2 = PointerGetDatum(stbox_geo(box2));
  /* Compute the result */
  double result = DatumGetFloat8(func(geo1, geo2));
  pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2));
  return result;
}

/**
 * @ingroup meos_geo_dist
 * @brief Return the nearest approach distance between a temporal geo
 * and a spatiotemporal box
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @return On error return -1.0
 * @csqlfn #NAD_tgeo_stbox()
 */
double
nad_tgeo_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, -1.0);  VALIDATE_NOT_NULL(box, -1.0);
  if (! ensure_valid_tgeo_stbox(temp, box) ||
      ! ensure_same_spatial_dimensionality(temp->flags, box->flags))
    return -1.0;

  /* Project the temporal geo to the timespan of the box */
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  Span p, inter;
  if (hast)
  {
    temporal_set_tstzspan(temp, &p);
    if (! inter_span_span(&p, &box->period, &inter))
      return DBL_MAX;
  }

  /* Select the distance function to be applied */
  datum_func2 func = distance_fn(box->flags);
  /* Convert the stbox to a geometry */
  Datum geo = PointerGetDatum(stbox_geo(box));
  Temporal *temp1 = hast ?
    temporal_restrict_tstzspan(temp, &inter, REST_AT) :
    (Temporal *) temp;
  /* Compute the result */
  Datum traj = tpoint_type(temp->temptype) ? 
    PointerGetDatum(tpoint_trajectory(temp)) :
    PointerGetDatum(tgeo_traversed_area(temp));
  double result = DatumGetFloat8(func(traj, geo));

  pfree(DatumGetPointer(traj));
  pfree(DatumGetPointer(geo));
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_geo_dist
 * @brief Return the nearest approach distance between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #NAD_tgeo_tgeo()
 */
double
nad_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, -1.0); VALIDATE_TGEO(temp2, -1.0);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return -1.0;

  Temporal *dist = distance_tgeo_tgeo(temp1, temp2);
  if (dist == NULL)
    return -1.0;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @brief Return the point in first input geography that is closest to the
 * second input geography in 2D
 */
GSERIALIZED *
geography_shortestline_internal(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  bool use_spheroid)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  /* Return NULL on empty arguments. */
  if ( gserialized_is_empty(gs1) || gserialized_is_empty(gs2) )
    return NULL;

  /* Initialize spheroid */
  SPHEROID s;
  spheroid_init_from_srid(gserialized_get_srid(gs1), &s);
  
  /* Set to sphere if requested */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  LWGEOM *geo1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geo2 = lwgeom_from_gserialized(gs2);
  LWGEOM *line = geography_tree_shortestline(geo1, geo2, FP_TOLERANCE, &s);
  GSERIALIZED *result = geo_serialize(line);
  lwgeom_free(line); lwgeom_free(geo1); lwgeom_free(geo2);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal geo and a geometry/geography
 * @param[in] temp Temporal value
 * @param[in] gs Geometry/geography
 * @csqlfn #Shortestline_tgeo_geo()
 */
GSERIALIZED *
shortestline_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return NULL;
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(temp->flags);
  if (geodetic && ! ensure_has_not_Z_geo(gs))
    return NULL;

  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp) : tgeo_traversed_area(temp);
  GSERIALIZED *result;
  if (geodetic)
    result = geography_shortestline_internal(traj, gs, true);
  else
  {
    result = MEOS_FLAGS_GET_Z(temp->flags) ?
      geom_shortestline3d(traj, gs) : geom_shortestline2d(traj, gs);
  }
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_geo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal geos
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Shortestline_tgeo_tgeo()
 */
GSERIALIZED *
shortestline_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, NULL); VALIDATE_TGEO(temp2, NULL);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return NULL;

  Temporal *dist = distance_tgeo_tgeo(temp1, temp2);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_instant(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  temporal_value_at_timestamptz(temp1, inst->t, false, &value1);
  temporal_value_at_timestamptz(temp2, inst->t, false, &value2);
  LWGEOM *line = (LWGEOM *) lwline_make(value1, value2);
  GSERIALIZED *result = geo_serialize(line);
  lwgeom_free(line);
  return result;
}

/*****************************************************************************/

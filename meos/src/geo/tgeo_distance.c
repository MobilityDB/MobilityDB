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
 * @brief Temporal distance functions for temporal geos
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
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"

/* Functions not exported by PostGIS */
extern double circ_tree_distance_tree_internal(const CIRC_NODE* n1,
  const CIRC_NODE* n2, double threshold, double* min_dist, double* max_dist,
  GEOGRAPHIC_POINT* closest1, GEOGRAPHIC_POINT* closest2);

/*****************************************************************************
 * Compute the distance between two geometries
 *****************************************************************************/

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
 * @brief Return 1 or 2 if a temporal point segment and a point are at a 
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start,end Values defining the first segment
 * @param[in] point Point to locate
 * @param[in] lower,upper Minimum distance at turning point
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segment is not constant
 * @post As there is a single turning point, `t2` is set to `t1`
 */
static int
tpoint_geo_distance_turnpt(Datum start, Datum end, Datum point, 
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  long double duration = (long double) (upper - lower);
  double dist;
  double fraction = (double) pointsegm_locate(start, end, point, &dist);
  if (fraction < 0.0)
    return 0;
  *t1 = *t2 = lower + (TimestampTz) (duration * fraction);
  return 1;
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
bool
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
 * @brief Return 1 or 2 if two temporal geometry point segments are at a
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start1,end1 Values defining the first segment
 * @param[in] start2,end2 Values defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @note The PostGIS functions @p lw_dist2d_seg_seg and @p lw_dist3d_seg_seg
 * cannot be used since they do not take time into consideration and would
 * return, e.g., that the minimum distance between the two following segments
 * `[Point(2 2)@t1, Point(1 1)@t2]` and `[Point(3 1)@t1, Point(1 1)@t2]`
 * is at `Point(2 2)@t2` instead of `Point(1.5 1.5)@(t1 + (t2 - t1)/2)`.
 * @pre The segments are not both constants.
 * @post As there is a single turning point, `t2` is set to `t`
 */
int
tgeompointsegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  double fraction;
  bool hasz = FLAGS_GET_Z(DatumGetGserializedP(start1)->gflags);
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(start1);
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(end1);
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(start2);
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(end2);
    bool found = point3d_min_dist(p1, p2, p3, p4, &fraction);
    if (!found)
      return 0;
  }
  else /* 2D */
  {
    const POINT2D *p1 = DATUM_POINT2D_P(start1);
    const POINT2D *p2 = DATUM_POINT2D_P(end1);
    const POINT2D *p3 = DATUM_POINT2D_P(start2);
    const POINT2D *p4 = DATUM_POINT2D_P(end2);
    bool found = point2d_min_dist(p1, p2, p3, p4, &fraction);
    if (! found)
      return 0;
  }

  double duration = upper - lower;
  *t1 = *t2 = lower + (TimestampTz) (duration * fraction);
  return 1;
}

/**
 * @brief Return 1 or 2 if two temporal geography point segments are at a
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance
 * @param[in] start1,end1 Values defining the first segment
 * @param[in] start2,end2 Values defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not both constants
 * @post As there is a single turning point, `t2` is  set to `t1`
 */
int
tgeogpointsegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  const POINT2D *p1 = DATUM_POINT2D_P(start1);
  const POINT2D *p2 = DATUM_POINT2D_P(end1);
  const POINT2D *q1 = DATUM_POINT2D_P(start2);
  const POINT2D *q2 = DATUM_POINT2D_P(end2);
  GEOGRAPHIC_EDGE e1, e2;
  POINT3D A1, A2, B1, B2;
  geographic_point_init(p1->x, p1->y, &(e1.start));
  geographic_point_init(p2->x, p2->y, &(e1.end));
  geographic_point_init(q1->x, q1->y, &(e2.start));
  geographic_point_init(q2->x, q2->y, &(e2.end));
  geog2cart(&(e1.start), &A1);
  geog2cart(&(e1.end), &A2);
  geog2cart(&(e2.start), &B1);
  geog2cart(&(e2.end), &B2);
  // TODO: The next computation should be done on geodetic coordinates
  // The value found by the linear approximation below could be the starting
  // point for an iterative method such as gradient descent or Newton's method
  double fraction;
  if (! point3d_min_dist((const POINT3DZ *) &A1, (const POINT3DZ *) &A2,
      (const POINT3DZ *) &B1, (const POINT3DZ *) &B2, &fraction))
    return 0;

  /* Compute the timestamp of intersection */
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return 0;
  double duration = (double) (upper - lower);
  *t1 = *t2 = lower + (TimestampTz) (duration * fraction);
  return 1;
}

/**
 * @brief Return 1 or 2 if two temporal point segments are at a minimum
 * distance during the period defined by the output timestamps, return 0
 * otherwise
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not both constants.
 */
int
tpointsegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum param, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  if (FLAGS_GET_GEODETIC(DatumGetGserializedP(start1)->gflags))
    return tgeogpointsegm_distance_turnpt(start1, end1, start2, end2,
      param, lower, upper, t1, t2);
  else
    return tgeompointsegm_distance_turnpt(start1, end1, start2, end2,
      param, lower, upper, t1, t2);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_distance
 * @brief Return the temporal distance between a temporal geo and a
 * geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #Tdistance_tgeo_geo()
 */
Temporal *
tdistance_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
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
  lfinfo.func = (varfunc) geo_distance_fn(temp->flags);
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_base = lfinfo.reslinear ? &tpoint_geo_distance_turnpt : NULL;
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the temporal distance between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Tdistance_tgeo_tgeo()
 */
Temporal *
tdistance_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, NULL); VALIDATE_TGEO(temp2, NULL);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) geo_distance_fn(temp1->flags);
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_temp = lfinfo.reslinear ? &tpointsegm_distance_turnpt : NULL;
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
 * @ingroup meos_geo_distance
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
 * @ingroup meos_geo_distance
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
  Temporal *dist = tdistance_tgeo_tgeo(temp1, temp2);
  if (dist == NULL)
    return NULL;

  const TInstant *min = temporal_min_inst_p(dist);
  TimestampTz t = min->t;
  pfree(dist);
  /* The closest point may be at an exclusive bound => 3rd argument = false */
  Datum value;
  temporal_value_at_timestamptz(temp1, t, false, &value);
  return tinstant_make_free(value, temp1->temptype, t);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between a temporal geo and a
 * geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #NAD_tgeo_geo()
 * @return On error return infinity
 */
double
nad_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, DBL_MAX); VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return DBL_MAX;

  datum_func2 func = geo_distance_fn(temp->flags);
  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp, UNARY_UNION_NO) :
    tgeo_traversed_area(temp, UNARY_UNION_NO);
  double result = DatumGetFloat8(
    func(PointerGetDatum(traj), PointerGetDatum(gs)));
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry/geography
 * @param[in] box Spatiotemporal box/geography
 * @param[in] gs Geometry
 * @csqlfn #NAD_stbox_geo()
 * @return On error return infinity
 */
double
nad_stbox_geo(const STBox *box, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, DBL_MAX); VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_valid_stbox_geo(box, gs) ||
      ! ensure_same_spatial_dimensionality_stbox_geo(box, gs))
    return DBL_MAX;

  datum_func2 func = geo_distance_fn(box->flags);
  Datum geo = PointerGetDatum(stbox_geo(box));
  double result = DatumGetFloat8(func(geo, PointerGetDatum(gs)));
  pfree(DatumGetPointer(geo));
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between two spatiotemporal
 * boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @return On error or if the time frames do not intersect return infinity
 * @csqlfn #NAD_stbox_stbox ()
 */
double
nad_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, DBL_MAX); VALIDATE_NOT_NULL(box2, DBL_MAX);
  if (! ensure_valid_spatial_stbox_stbox(box1, box2) ||
      ! ensure_same_spatial_dimensionality(box1->flags, box2->flags))
    return DBL_MAX;

  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
      return DBL_MAX;

  /* If the boxes intersect in the value dimension return 0 */
  if (box1->xmin <= box2->xmax && box2->xmin <= box1->xmax)
    return 0.0;

  /* Select the distance function to be applied */
  datum_func2 func = geo_distance_fn(box1->flags);
  /* Convert the boxes to geometries */
  Datum geo1 = PointerGetDatum(stbox_geo(box1));
  Datum geo2 = PointerGetDatum(stbox_geo(box2));
  /* Compute the result */
  double result = DatumGetFloat8(func(geo1, geo2));
  pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2));
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between a temporal geo
 * and a spatiotemporal box
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @return On error or if the time frames do not intersect return infinity
 * @csqlfn #NAD_tgeo_stbox()
 */
double
nad_tgeo_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, DBL_MAX);  VALIDATE_NOT_NULL(box, DBL_MAX);
  if (! ensure_valid_tgeo_stbox(temp, box) ||
      ! ensure_same_spatial_dimensionality(temp->flags, box->flags))
    return DBL_MAX;

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
  datum_func2 func = geo_distance_fn(box->flags);
  /* Convert the stbox to a geometry */
  Datum geo = PointerGetDatum(stbox_geo(box));
  Temporal *temp1 = hast ?
    temporal_restrict_tstzspan(temp, &inter, REST_AT) :
    (Temporal *) temp;
  /* Compute the result */
  Datum traj = tpoint_type(temp->temptype) ? 
    PointerGetDatum(tpoint_trajectory(temp, UNARY_UNION_NO)) :
    PointerGetDatum(tgeo_traversed_area(temp, UNARY_UNION_NO));
  double result = DatumGetFloat8(func(traj, geo));

  pfree(DatumGetPointer(traj));
  pfree(DatumGetPointer(geo));
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #NAD_tgeo_tgeo()
 * @return On error or if the time frames do not intersect return infinity
 */
double
nad_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, DBL_MAX); VALIDATE_TGEO(temp2, DBL_MAX);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return DBL_MAX;

  Temporal *dist = tdistance_tgeo_tgeo(temp1, temp2);
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
 * @ingroup meos_geo_distance
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
    tpoint_trajectory(temp, UNARY_UNION_NO) :
    tgeo_traversed_area(temp, UNARY_UNION_NO);
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
 * @ingroup meos_geo_distance
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

  Temporal *dist = tdistance_tgeo_tgeo(temp1, temp2);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  temporal_value_at_timestamptz(temp1, inst->t, false, &value1);
  temporal_value_at_timestamptz(temp2, inst->t, false, &value2);
  LWGEOM *line = (LWGEOM *) lwline_make(value1, value2);
  GSERIALIZED *result = geo_serialize(line);
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2));
  pfree(dist); lwgeom_free(line);
  return result;
}

/*****************************************************************************
 * Set-set spatial minimum distance
 *****************************************************************************/

/* Spatial-only distance between two STBoxes; ignores time entirely. */
static double
stbox_spatial_distance(const STBox *box1, const STBox *box2)
{
  /* Spatial extents overlap → exact minimum is 0 (some pair of points
   * inside the joined extent has zero distance) */
  if (box1->xmin <= box2->xmax && box2->xmin <= box1->xmax &&
      box1->ymin <= box2->ymax && box2->ymin <= box1->ymax)
    return 0.0;

  /* Spatial extents disjoint → exact bbox-to-bbox euclidean distance.
   * Drop the time component of each box before serialising to geometry. */
  datum_func2 func = geo_distance_fn(box1->flags);
  STBox b1 = *box1, b2 = *box2;
  MEOS_FLAGS_SET_T(b1.flags, false);
  MEOS_FLAGS_SET_T(b2.flags, false);
  Datum g1 = PointerGetDatum(stbox_geo(&b1));
  Datum g2 = PointerGetDatum(stbox_geo(&b2));
  double result = DatumGetFloat8(func(g1, g2));
  pfree(DatumGetPointer(g1));
  pfree(DatumGetPointer(g2));
  return result;
}

/*****************************************************************************
 * Threshold-aware plane-sweep spatial-min kernel (Option B'')
 *
 * Walks two tgeompoint TSequence instant arrays directly via
 * `lw_dist2d_seg_seg`, no `tpoint_trajectory()` materialisation, no
 * `lw_dist2d_recursive` wrapper.  T2's segment bboxes are computed once
 * and sorted by minx; for each T1 segment we binary-search the candidate
 * window in T2 and walk it with the running threshold tightening as we
 * go.  Same answer as `geom_distance2d(trajectory(seq1), trajectory(seq2))`
 * for the 2D, planar, tgeompoint, linear-interpolation case.
 *****************************************************************************/

typedef struct
{
  int idx;
  float minx;
  float maxx;
  float miny;
  float maxy;
} SegBox;

static int
segbox_cmp_minx(const void *a, const void *b)
{
  float da = ((const SegBox *) a)->minx;
  float db = ((const SegBox *) b)->minx;
  return (da < db) ? -1 : (da > db ? 1 : 0);
}

static double
mindist_tpointseq_tpointseq_threshold(const TSequence *seq1,
  const TSequence *seq2, double threshold)
{
  DISTPTS dl;
  lw_dist2d_distpts_init(&dl, DIST_MIN);
  dl.distance = threshold;

  /* Both single-instant */
  if (seq1->count == 1 && seq2->count == 1)
  {
    const POINT2D *p = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, 0)));
    const POINT2D *q = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, 0)));
    lw_dist2d_pt_pt(p, q, &dl);
    return dl.distance;
  }
  if (seq1->count == 1)
  {
    const POINT2D *p = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, 0)));
    for (int j = 0; j < seq2->count - 1; j++)
    {
      const POINT2D *q1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
      const POINT2D *q2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1)));
      lw_dist2d_pt_seg(p, q1, q2, &dl);
      if (dl.distance == 0.0) return 0.0;
    }
    return dl.distance;
  }
  if (seq2->count == 1)
  {
    const POINT2D *q = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, 0)));
    for (int i = 0; i < seq1->count - 1; i++)
    {
      const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i)));
      const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i + 1)));
      lw_dist2d_pt_seg(q, p1, p2, &dl);
      if (dl.distance == 0.0) return 0.0;
    }
    return dl.distance;
  }
  /* Both segmented: plane-sweep. */
  int n2 = seq2->count - 1;
  SegBox *boxes2 = palloc(n2 * sizeof(SegBox));
  for (int j = 0; j < n2; j++)
  {
    const POINT2D *q1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
    const POINT2D *q2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1)));
    boxes2[j].idx = j;
    boxes2[j].minx = (float) fmin(q1->x, q2->x);
    boxes2[j].maxx = (float) fmax(q1->x, q2->x);
    boxes2[j].miny = (float) fmin(q1->y, q2->y);
    boxes2[j].maxy = (float) fmax(q1->y, q2->y);
  }
  qsort(boxes2, n2, sizeof(SegBox), segbox_cmp_minx);

  for (int i = 0; i < seq1->count - 1; i++)
  {
    const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i)));
    const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i + 1)));
    double s1_minx = fmin(p1->x, p2->x), s1_maxx = fmax(p1->x, p2->x);
    double s1_miny = fmin(p1->y, p2->y), s1_maxy = fmax(p1->y, p2->y);
    double thresh = dl.distance;
    double hi_x = s1_maxx + thresh;
    double lo_x = s1_minx - thresh;
    /* Binary-search the first j with minx > hi_x (upper bound).  Walk
     * forward from j=0 to that bound; skip those whose maxx < lo_x. */
    int hi_idx;
    {
      int lo = 0, hi = n2;
      while (lo < hi)
      {
        int mid = (lo + hi) / 2;
        if (boxes2[mid].minx > hi_x) hi = mid;
        else lo = mid + 1;
      }
      hi_idx = lo;
    }
    for (int k = 0; k < hi_idx; k++)
    {
      if (boxes2[k].maxx < lo_x) continue;
      /* Y-axis prefilter */
      if (boxes2[k].maxy < s1_miny - dl.distance) continue;
      if (boxes2[k].miny > s1_maxy + dl.distance) continue;
      double dx = fmax(0.0, fmax(s1_minx, (double) boxes2[k].minx)
                          - fmin(s1_maxx, (double) boxes2[k].maxx));
      double dy = fmax(0.0, fmax(s1_miny, (double) boxes2[k].miny)
                          - fmin(s1_maxy, (double) boxes2[k].maxy));
      if (dx >= dl.distance || dy >= dl.distance) continue;
      int j = boxes2[k].idx;
      const POINT2D *q1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
      const POINT2D *q2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1)));
      lw_dist2d_seg_seg(p1, p2, q1, q2, &dl);
      if (dl.distance == 0.0) { pfree(boxes2); return 0.0; }
    }
  }
  pfree(boxes2);
  return dl.distance;
}

/* Dispatch across subtypes, threading the running threshold. */
static double
mindist_tpoint_tpoint_threshold(const Temporal *temp1, const Temporal *temp2,
  double threshold)
{
  if (temp1->subtype == TSEQUENCESET)
  {
    const TSequenceSet *ss1 = (const TSequenceSet *) temp1;
    for (int i = 0; i < ss1->count; i++)
    {
      double d = mindist_tpoint_tpoint_threshold(
        (const Temporal *) TSEQUENCESET_SEQ_N(ss1, i), temp2, threshold);
      if (d < threshold) threshold = d;
      if (threshold == 0.0) return 0.0;
    }
    return threshold;
  }
  if (temp2->subtype == TSEQUENCESET)
  {
    const TSequenceSet *ss2 = (const TSequenceSet *) temp2;
    for (int j = 0; j < ss2->count; j++)
    {
      double d = mindist_tpointseq_tpointseq_threshold(
        (const TSequence *) temp1, TSEQUENCESET_SEQ_N(ss2, j), threshold);
      if (d < threshold) threshold = d;
      if (threshold == 0.0) return 0.0;
    }
    return threshold;
  }
  if (temp1->subtype == TSEQUENCE && temp2->subtype == TSEQUENCE)
    return mindist_tpointseq_tpointseq_threshold(
      (const TSequence *) temp1, (const TSequence *) temp2, threshold);
  /* TInstant: caller falls back to lwgeom path. */
  return DBL_MAX;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the exact minimum spatial distance between two temporal
 * geos, ignoring time
 * @details Computes
 * `ST_Distance(trajectory(temp1), trajectory(temp2))` (the spatial-min
 * over all pairs of points on the two trajectories) using the
 * threshold-aware plane-sweep kernel directly on the `TInstant` arrays.
 * Distinct from `nearestApproachDistance(tgeompoint, tgeompoint)` which
 * is time-synchronous.
 * @param[in] temp1,temp2 Temporal geos (must share SRID, dimensionality,
 *   and geodetic flag)
 * @return Minimum spatial distance; falls back to `geom_distance2d` for
 *   subtype combinations the inline kernel does not handle (e.g.
 *   `TInstant`).
 * @csqlfn #Mindistance_tgeo_tgeo()
 */
double
mindistance_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  double threshold)
{
  VALIDATE_NOT_NULL(temp1, DBL_MAX);
  VALIDATE_NOT_NULL(temp2, DBL_MAX);
  VALIDATE_TGEO(temp1, DBL_MAX);
  VALIDATE_TGEO(temp2, DBL_MAX);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return DBL_MAX;
  /* Outer STBox prune: every pair of points on the two trajectories is
   * contained in the respective STBoxes, so the minimum spatial distance
   * is bounded below by the spatial distance between the STBoxes. If
   * that lower bound already meets or exceeds the running threshold the
   * caller cannot improve, so short-circuit without entering the
   * segment kernel. Critical for cross-join aggregations where most
   * pairs are far apart relative to the running min. */
  if (! MEOS_FLAGS_GET_GEODETIC(temp1->flags))
  {
    const STBox *bbox1 = (const STBox *) temporal_bbox_ptr(temp1);
    const STBox *bbox2 = (const STBox *) temporal_bbox_ptr(temp2);
    double bbox_dist = stbox_spatial_distance(bbox1, bbox2);
    if (bbox_dist >= threshold)
      return threshold;
  }
  bool inline_eligible =
    ! MEOS_FLAGS_GET_Z(temp1->flags) &&
    ! MEOS_FLAGS_GET_GEODETIC(temp1->flags) &&
    MEOS_FLAGS_LINEAR_INTERP(temp1->flags) &&
    tpoint_type(temp1->temptype) &&
    tpoint_type(temp2->temptype) &&
    temp1->subtype != TINSTANT && temp2->subtype != TINSTANT;
  if (inline_eligible)
    return mindist_tpoint_tpoint_threshold(temp1, temp2, threshold);
  /* Fallback */
  GSERIALIZED *traj1 = tpoint_type(temp1->temptype) ?
    tpoint_trajectory(temp1, UNARY_UNION_NO) :
    tgeo_traversed_area(temp1, UNARY_UNION_NO);
  GSERIALIZED *traj2 = tpoint_type(temp2->temptype) ?
    tpoint_trajectory(temp2, UNARY_UNION_NO) :
    tgeo_traversed_area(temp2, UNARY_UNION_NO);
  double d = MEOS_FLAGS_GET_Z(temp1->flags) ?
    geom_distance3d(traj1, traj2) :
    geom_distance2d(traj1, traj2);
  pfree(traj1); pfree(traj2);
  return (d < threshold) ? d : threshold;
}

/* qsort comparator: pair record ordered by bbox-distance ascending. */
typedef struct
{
  int i;
  int j;
  double bd;
} TgeoarrPair;

static int
tgeoarr_pair_cmp(const void *a, const void *b)
{
  double da = ((const TgeoarrPair *) a)->bd;
  double db = ((const TgeoarrPair *) b)->bd;
  if (da < db) return -1;
  if (da > db) return 1;
  return 0;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the exact minimum spatial distance between two arrays of
 * temporal geos
 * @details Computes the same value as
 * `ST_Distance(ST_Collect(trajectory(arr1[*])), ST_Collect(trajectory(arr2[*])))`
 * but uses each input's STBox as a sound lower-bound prefilter: trip pairs
 * are processed in ascending bbox-distance order, and the iteration short
 * circuits once the running minimum is provably smaller than every
 * remaining pair's lower bound.  The per-pair distance still goes through
 * `geom_distance2d` (liblwgeom segment-pair sweep, no GEOS call), so the
 * result is bit-equivalent to the materialised aggregate form.
 * @param[in] arr1,arr2 Arrays of temporal geos (each element must be
 *   non-NULL and share SRID / dimensionality with the rest)
 * @param[in] count1,count2 Array lengths (must be > 0)
 * @return Minimum spatial distance; DBL_MAX on validation failure or on
 *   any empty input array.
 * @csqlfn #Tgeoarr_tgeoarr_mindist()
 */
double
tgeoarr_tgeoarr_mindist(const Temporal **arr1, int count1,
  const Temporal **arr2, int count2)
{
  VALIDATE_NOT_NULL(arr1, DBL_MAX);
  VALIDATE_NOT_NULL(arr2, DBL_MAX);
  if (count1 <= 0 || count2 <= 0)
    return DBL_MAX;
  for (int i = 0; i < count1; i++)
    VALIDATE_TGEO(arr1[i], DBL_MAX);
  for (int j = 0; j < count2; j++)
    VALIDATE_TGEO(arr2[j], DBL_MAX);

  /* Soundness gate: all inputs share SRID, dimensionality, geodetic flag */
  int32 srid = tspatial_srid(arr1[0]);
  int16 flags = arr1[0]->flags;
  for (int i = 1; i < count1; i++)
    if (! ensure_same_srid(tspatial_srid(arr1[i]), srid) ||
        ! ensure_same_dimensionality(arr1[i]->flags, flags) ||
        ! ensure_same_geodetic(arr1[i]->flags, flags))
      return DBL_MAX;
  for (int j = 0; j < count2; j++)
    if (! ensure_same_srid(tspatial_srid(arr2[j]), srid) ||
        ! ensure_same_dimensionality(arr2[j]->flags, flags) ||
        ! ensure_same_geodetic(arr2[j]->flags, flags))
      return DBL_MAX;

  /* Pre-compute STBoxes for every input.  Each tspatial_to_stbox is a
   * cheap aggregate over the temporal value, amortised across all pairs
   * involving that input. */
  STBox **bb1 = palloc(count1 * sizeof(STBox *));
  STBox **bb2 = palloc(count2 * sizeof(STBox *));
  for (int i = 0; i < count1; i++) bb1[i] = tspatial_to_stbox(arr1[i]);
  for (int j = 0; j < count2; j++) bb2[j] = tspatial_to_stbox(arr2[j]);

  /* Materialise all candidate pairs with their bbox-distance lower bound */
  int npairs = count1 * count2;
  TgeoarrPair *pairs = palloc(npairs * sizeof(TgeoarrPair));
  int k = 0;
  for (int i = 0; i < count1; i++)
    for (int j = 0; j < count2; j++)
    {
      pairs[k].i = i;
      pairs[k].j = j;
      pairs[k].bd = stbox_spatial_distance(bb1[i], bb2[j]);
      k++;
    }
  qsort(pairs, npairs, sizeof(TgeoarrPair), tgeoarr_pair_cmp);

  /* Trajectory cache: each input's trajectory is materialised at most
   * once and reused across every pair it participates in. */
  GSERIALIZED **traj1 = palloc0(count1 * sizeof(GSERIALIZED *));
  GSERIALIZED **traj2 = palloc0(count2 * sizeof(GSERIALIZED *));

  /* Eligibility for the inline tgeompoint kernel: 2D, planar, all inputs
   * are tgeompoint with linear interpolation, and at least one of seq /
   * seqset on each side.  TInstant inputs fall through to the lwgeom
   * path. */
  bool inline_eligible =
    ! MEOS_FLAGS_GET_Z(flags) &&
    ! MEOS_FLAGS_GET_GEODETIC(flags) &&
    MEOS_FLAGS_LINEAR_INTERP(flags);
  if (inline_eligible)
  {
    for (int i = 0; i < count1; i++)
      if (! tpoint_type(arr1[i]->temptype) ||
          arr1[i]->subtype == TINSTANT)
      { inline_eligible = false; break; }
  }
  if (inline_eligible)
  {
    for (int j = 0; j < count2; j++)
      if (! tpoint_type(arr2[j]->temptype) ||
          arr2[j]->subtype == TINSTANT)
      { inline_eligible = false; break; }
  }

  double running_min = DBL_MAX;
  for (k = 0; k < npairs; k++)
  {
    /* All remaining pairs have bbox-distance >= pairs[k].bd, which is
     * a sound lower bound on the trajectory pair's actual distance.
     * Once it crosses running_min, no remaining pair can improve it. */
    if (pairs[k].bd >= running_min)
      break;
    int i = pairs[k].i, j = pairs[k].j;
    double d;
    if (inline_eligible)
    {
      d = mindist_tpoint_tpoint_threshold(arr1[i], arr2[j], running_min);
    }
    else
    {
      if (traj1[i] == NULL)
        traj1[i] = tpoint_type(arr1[i]->temptype) ?
          tpoint_trajectory(arr1[i], UNARY_UNION_NO) :
          tgeo_traversed_area(arr1[i], UNARY_UNION_NO);
      if (traj2[j] == NULL)
        traj2[j] = tpoint_type(arr2[j]->temptype) ?
          tpoint_trajectory(arr2[j], UNARY_UNION_NO) :
          tgeo_traversed_area(arr2[j], UNARY_UNION_NO);
      d = MEOS_FLAGS_GET_Z(flags) ?
        geom_distance3d(traj1[i], traj2[j]) :
        geom_distance2d(traj1[i], traj2[j]);
    }
    if (d < running_min)
      running_min = d;
    if (running_min == 0.0)
      break;
  }

  /* Cleanup */
  for (int i = 0; i < count1; i++)
  {
    pfree(bb1[i]);
    if (traj1[i] != NULL) pfree(traj1[i]);
  }
  for (int j = 0; j < count2; j++)
  {
    pfree(bb2[j]);
    if (traj2[j] != NULL) pfree(traj2[j]);
  }
  pfree(bb1); pfree(bb2); pfree(traj1); pfree(traj2); pfree(pairs);
  return running_min;
}

/*****************************************************************************/

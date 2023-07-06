/***********************************************************************
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
 * @brief Distance functions for temporal points.
 */

#include "point/tpoint_distance.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */

/* PostGIS */
#include <lwgeodetic_tree.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/temporaltypes.h"
#include "general/type_util.h"
#include "point/pgis_call.h"
#include "point/geography_funcs.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Compute the distance between two instants depending on their type
 *****************************************************************************/

/**
 * @brief Compute the distance between two temporal instants.
 * @param[in] inst1,inst2 Temporal instants
 */
double
tnumberinst_distance(const TInstant *inst1, const TInstant *inst2)
{
  double result = fabs(tnumberinst_double(inst1) - tnumberinst_double(inst2));
  return result;
}

/**
 * @brief Compute the distance between two temporal instants.
 * @param[in] inst1,inst2 Temporal instants
 * @param[in] func Distance function
 */
double
tpointinst_distance(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  double result = DatumGetFloat8(func(value1, value2));
  return result;
}

/**
 * @brief Compute the distance between two temporal instants.
 * @param[in] inst1,inst2 Temporal instants
 * @param[in] func Distance function
 */
double
tinstant_distance(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func)
{
  assert (tnumber_type(inst1->temptype) || tgeo_type(inst1->temptype));
  if (tnumber_type(inst1->temptype))
    return tnumberinst_distance(inst1, inst2);
  else /* tgeo_type(inst1->temptype) */
    return tpointinst_distance(inst1, inst2, func);
}

/*****************************************************************************/

/**
 * @brief Return the distance between the two geometries. When the first
 * geometry is a segment it also computes a value between 0 and 1 that
 * represents the location in the segment of the closest point to the second
 * geometry, as a fraction of total segment length.
 *
 * @note Function inspired by PostGIS function lw_dist2d_distancepoint
 * from measures.c
 */
static double
lw_distance_fraction(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
  double *fraction)
{
  double result;
  if (FLAGS_GET_GEODETIC(lw1->flags))
  {
    double min_dist = FLT_MAX;
    double max_dist = FLT_MAX;
    GEOGRAPHIC_POINT closest1, closest2;
    GEOGRAPHIC_EDGE e;
    CIRC_NODE *circ_tree1 = lwgeom_calculate_circ_tree(lw1);
    CIRC_NODE *circ_tree2 = lwgeom_calculate_circ_tree(lw2);
    circ_tree_distance_tree_internal(circ_tree1, circ_tree2, FP_TOLERANCE,
      &min_dist, &max_dist, &closest1, &closest2);
    result = sphere_distance(&closest1, &closest2);
    if (fraction != NULL)
    {
      assert(lw1->type == LINETYPE);
      LWLINE *lwline = lwgeom_as_lwline(lw1);
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
      long double seglength = sphere_distance(&(e.start), &(e.end));
      long double length = sphere_distance(&(e.start), &proj);
      *fraction = length / seglength;
    }
  }
  else
  {
    if (FLAGS_GET_Z(lw1->flags))
    {
      DISTPTS3D dl;
      dl.mode = mode;
      dl.distance= FLT_MAX;
      dl.tolerance = 0;
      lw_dist3d_recursive(lw1, lw2, &dl);
      result = dl.distance;
      if (fraction != NULL)
      {
        assert(lw1->type == LINETYPE);
        LWLINE *lwline = lwgeom_as_lwline(lw1);
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
      lw_dist2d_recursive(lw1, lw2, &dl);
      result = dl.distance;
      if (fraction != NULL)
      {
        assert(lw1->type == LINETYPE);
        LWLINE *lwline = lwgeom_as_lwline(lw1);
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
 * @brief Return the value and timestamp at which the a temporal point segment
 * and a point are at the minimum distance. These are the turning points when
 * computing the temporal distance.
 *
 * @param[in] start,end Instants defining the first segment
 * @param[in] point Base point
 * @param[in] basetype Base type
 * @param[out] value Projected value at turning point
 * @param[out] t Timestamp at turning point
 * @pre The segment is not constant.
 * @note The parameter basetype is not needed for temporal points
 */
static bool
tpoint_geo_min_dist_at_timestamp(const TInstant *start, const TInstant *end,
  Datum point, meosType basetype __attribute__((unused)), Datum *value,
  TimestampTz *t)
{
  long double duration = (long double) (end->t - start->t);
  Datum value1 = tinstant_value(start);
  Datum value2 = tinstant_value(end);
  double dist;
  double fraction = geosegm_locate_point(value1, value2, point, &dist);
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
  return true;
}

/**
 * @brief Return the value and timestamp at which the two temporal geometric
 * point segments are at the minimum distance. These are the turning points
 * when computing the temporal distance.
 *
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] value Value
 * @param[out] t Timestamp
 * @note The PostGIS functions `lw_dist2d_seg_seg` and `lw_dist3d_seg_seg`
 * cannot be used since they do not take time into consideration and would
 * return, e.g., that the minimum distance between the two following segments
 * `[Point(2 2)@t1, Point(1 1)@t2]` and `[Point(3 1)@t1, Point(1 1)@t2]`
 * is at `Point(2 2)@t2` instead of `Point(1.5 1.5)@(t1 + (t2 - t1)/2)`.
 * @pre The segments are not both constants.
 * @note
 */
static bool
tgeompoint_min_dist_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  double fraction;
  bool hasz = MEOS_FLAGS_GET_Z(start1->flags);
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(tinstant_value(start1));
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(tinstant_value(end1));
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(tinstant_value(start2));
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(tinstant_value(end2));
    bool found = point3d_min_dist(p1, p2, p3, p4, &fraction);
    if (!found)
      return false;
  }
  else /* 2D */
  {
    const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value(start1));
    const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value(end1));
    const POINT2D *p3 = DATUM_POINT2D_P(tinstant_value(start2));
    const POINT2D *p4 = DATUM_POINT2D_P(tinstant_value(end2));
    bool found = point2d_min_dist(p1, p2, p3, p4, &fraction);
    if (!found)
      return false;
  }
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return false;

  double duration = end1->t - start1->t;
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* We know that this function is called only for linear segments */
  Datum value1 = tsegment_value_at_timestamp(start1, end1, true, *t);
  Datum value2 = tsegment_value_at_timestamp(start2, end2, true, *t);
  *value = hasz ?
    geom_distance3d(value1, value2) : geom_distance2d(value1, value2);
  return true;
}

/**
 * @brief Return the single timestamp at which the two temporal geographic
 * point segments are at the minimum distance. These are the turning points
 * when computing the temporal distance.
 *
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] mindist Minimum distance
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
static bool
tgeogpoint_min_dist_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *mindist, TimestampTz *t)
{
  const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value(start1));
  const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value(end1));
  const POINT2D *p3 = DATUM_POINT2D_P(tinstant_value(start2));
  const POINT2D *p4 = DATUM_POINT2D_P(tinstant_value(end2));
  GEOGRAPHIC_EDGE e1, e2;
  GEOGRAPHIC_POINT close1, close2;
  POINT3D A1, A2, B1, B2;
  geographic_point_init(p1->x, p1->y, &(e1.start));
  geographic_point_init(p2->x, p2->y, &(e1.end));
  geographic_point_init(p3->x, p3->y, &(e2.start));
  geographic_point_init(p4->x, p4->y, &(e2.end));
  geog2cart(&(e1.start), &A1);
  geog2cart(&(e1.end), &A2);
  geog2cart(&(e2.start), &B1);
  geog2cart(&(e2.end), &B2);
  double fraction;
  if (edge_intersects(&A1, &A2, &B1, &B2))
  {
    /* We know that the distance is 0 */
    if (mindist)
      *mindist = Float8GetDatum(0.0);
    bool found = point3d_min_dist((const POINT3DZ *) &A1, (const POINT3DZ *) &A2,
      (const POINT3DZ *) &B1, (const POINT3DZ *) &B2, &fraction);
    if (! found)
      return false;
  }
  else
  {
    /* Compute closest points en each segment */
    edge_distance_to_edge(&e1, &e2, &close1, &close2);
    if (geographic_point_equals(&e1.start, &close1) ||
      geographic_point_equals(&e1.end, &close1))
      return false;
    /* Compute distance between closest points */
    if (mindist)
      *mindist = Float8GetDatum(WGS84_RADIUS * sphere_distance(&close1,
        &close2));
    /* Compute distance from beginning of the segment to one closest point */
    long double seglength = sphere_distance(&(e1.start), &(e1.end));
    long double length = sphere_distance(&(e1.start), &close1);
    fraction = (double) (length / seglength);
  }

  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return false;
  double duration = (double) (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * @brief Return the value and timestamp at which the two temporal point
 * segments are at the minimum distance.
 *
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] value Value
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
static bool
tpoint_min_dist_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  if (MEOS_FLAGS_GET_GEODETIC(start1->flags))
    /* The distance output parameter is not used here */
    return tgeogpoint_min_dist_at_timestamp(start1, end1, start2, end2, value, t);
  else
    return tgeompoint_min_dist_at_timestamp(start1, end1, start2, end2, value, t);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the temporal distance between a temporal point and a
 * geometry/geography point
 * @sqlop @p <->
 */
Temporal *
distance_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  if (gserialized_is_empty(geo))
    return NULL;
  ensure_point_type(geo);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(geo));
  ensure_same_dimensionality_tpoint_gs(temp, geo);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) distance_fn(temp->flags);
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_GET_LINEAR(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = lfinfo.reslinear ?
    &tpoint_geo_min_dist_at_timestamp : NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal_base(temp, PointerGetDatum(geo), &lfinfo);
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the temporal distance between two temporal points.
 * @sqlop @p <->
 */
Temporal *
distance_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) pt_distance_fn(temp1->flags);
  lfinfo.numparam = 0;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_GET_LINEAR(temp1->flags) ||
    MEOS_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = lfinfo.reslinear ? &tpoint_min_dist_at_timestamp : NULL;
  Temporal *result = tfunc_temporal_temporal(temp1, temp2, &lfinfo);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @brief Return the new current nearest approach instant between the temporal
 * sequence point with step interpolation and a geometry/geography
 * (iterator function)
 * @param[in] seq Temporal point
 * @param[in] geo Geometry/geography
 * @param[in] mindist Current minimum distance, it is set at DBL_MAX at the
 * begining but contains the minimum distance found in the previous
 * sequences of a temporal sequence set
 * @param[out] result Instant with the minimum distance
 * @result Minimum distance
 */
static double
NAI_tpointseq_discstep_geo_iter(const TSequence *seq, const LWGEOM *geo,
  double mindist, const TInstant **result)
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    Datum value = tinstant_value(inst);
    GSERIALIZED *gs = DatumGetGserializedP(value);
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
 * @brief Return the nearest approach instant between the temporal sequence
 * point with step interpolation and a geometry/geography
 * @param[in] seq Temporal point
 * @param[in] geo Geometry/geography
 */
static TInstant *
NAI_tpointseq_discstep_geo(const TSequence *seq, const LWGEOM *geo)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  NAI_tpointseq_discstep_geo_iter(seq, geo, DBL_MAX, &inst);
  return tinstant_copy(inst);
}

/**
 * @brief Return the nearest approach instant between the temporal sequence set
 * point with step interpolation and a geometry/geography
 * @param[in] ss Temporal point
 * @param[in] geo Geometry/geography
 */
static TInstant *
NAI_tpointseqset_step_geo(const TSequenceSet *ss, const LWGEOM *geo)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  double mindist = DBL_MAX;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    mindist = NAI_tpointseq_discstep_geo_iter(seq, geo, mindist, &inst);
  }
  assert(inst != NULL);
  return tinstant_copy(inst);
}

/*****************************************************************************/

/**
 * @brief Return the distance and the timestamp of the nearest approach instant
 * between the temporal sequence point with linear interpolation and the
 * geometry/geography
 * @param[in] inst1,inst2 Temporal segment
 * @param[in] geo Geometry/geography
 * @param[out] t Timestamp
 * @result Distance
 */
static double
NAI_tpointsegm_linear_geo1(const TInstant *inst1, const TInstant *inst2,
  const LWGEOM *geo, TimestampTz *t)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
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
 * between the temporal sequence point with linear interpolation and the
 * geometry/geography (iterator function)
 * @param[in] seq Temporal point
 * @param[in] geo Geometry/geography
 * @param[in] mindist Minimum distance found so far, or DBL_MAX at the beginning
 * @param[out] t Timestamp
 */
static double
NAI_tpointseq_linear_geo_iter(const TSequence *seq, const LWGEOM *geo,
  double mindist, TimestampTz *t)
{
  double dist;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);

  if (seq->count == 1)
  {
    /* Instantaneous sequence */
    Datum value1 = tinstant_value(inst1);
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
      dist = NAI_tpointsegm_linear_geo1(inst1, inst2, geo, &t1);
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
 * @brief Return the nearest approach instant between the temporal sequence
 * point with linear interpolation and a geometry (iterator function)
 */
static TInstant *
NAI_tpointseq_linear_geo(const TSequence *seq, const LWGEOM *geo)
{
  TimestampTz t;
  NAI_tpointseq_linear_geo_iter(seq, geo, DBL_MAX, &t);
  /* The closest point may be at an exclusive bound */
  Datum value;
  tsequence_value_at_timestamp(seq, t, false, &value);
  TInstant *result = tinstant_make(value, seq->temptype, t);
  pfree(DatumGetPointer(value));
  return result;
}

/**
 * @brief Return the nearest approach instant between the temporal sequence set
 * point with linear interpolation and a geometry
 */
static TInstant *
NAI_tpointseqset_linear_geo(const TSequenceSet *ss, const LWGEOM *geo)
{
  TimestampTz t = 0; /* make compiler quiet */
  double mindist = DBL_MAX;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    TimestampTz t1;
    double dist = NAI_tpointseq_linear_geo_iter(seq, geo, mindist, &t1);
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
  tsequenceset_value_at_timestamp(ss, t, false, &value);
  TInstant *result = tinstant_make(value, ss->temptype, t);
  pfree(DatumGetPointer(value));
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach instant between a temporal point and
 * a geometry.
 * @sqlfunc nearestApproachInstant()
 */
TInstant *
nai_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);

  LWGEOM *geo = lwgeom_from_gserialized(gs);
  TInstant *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = MEOS_FLAGS_GET_LINEAR(temp->flags) ?
      NAI_tpointseq_linear_geo((TSequence *) temp, geo) :
      NAI_tpointseq_discstep_geo((TSequence *) temp, geo);
  else /* temp->subtype == TSEQUENCESET */
    result = MEOS_FLAGS_GET_LINEAR(temp->flags) ?
      NAI_tpointseqset_linear_geo((TSequenceSet *) temp, geo) :
      NAI_tpointseqset_step_geo((TSequenceSet *) temp, geo);
  lwgeom_free(geo);
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach instant between the temporal points.
 * @sqlfunc nearestApproachInstant()
 */
TInstant *
nai_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  TInstant *result = NULL;
  Temporal *dist = distance_tpoint_tpoint(temp1, temp2);
  if (dist != NULL)
  {
    const TInstant *min = temporal_min_instant(dist);
    /* The closest point may be at an exclusive bound => 3rd argument = false */
    Datum value;
    temporal_value_at_timestamp(temp1, min->t, false, &value);
    result = tinstant_make(value, temp1->temptype, min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance between a temporal point
 * and a geometry
 * @sqlop @p |=|
 */
double
nad_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return -1;
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  datum_func2 func = distance_fn(temp->flags);
  Datum traj = PointerGetDatum(tpoint_trajectory(temp));
  double result = DatumGetFloat8(func(traj, PointerGetDatum(gs)));
  pfree(DatumGetPointer(traj));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry
 * @sqlop @p |=|
 */
double
nad_stbox_geo(const STBox *box, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return -1;
  ensure_same_srid_stbox_gs(box, gs);
  ensure_same_spatial_dimensionality_stbox_gs(box, gs);
  datum_func2 func = distance_fn(box->flags);
  Datum geo = PointerGetDatum(stbox_to_geo(box));
  double result = DatumGetFloat8(func(geo, PointerGetDatum(gs)));
  pfree(DatumGetPointer(geo));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance between the spatio-temporal
 * boxes.
 * @sqlop @p |=|
 */
double
nad_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Test the validity of the arguments */
  ensure_has_X_stbox(box1); ensure_has_X_stbox(box2);
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_spatial_dimensionality(box1->flags, box2->flags);
  ensure_same_srid(stbox_srid(box1), stbox_srid(box2));

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
  Datum geo1 = PointerGetDatum(stbox_to_geo(box1));
  Datum geo2 = PointerGetDatum(stbox_to_geo(box2));
  /* Compute the result */
  double result = DatumGetFloat8(func(geo1, geo2));
  pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance between a temporal point
 * and a spatio-temporal box
 * @sqlop @p |=|
 */
double
nad_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  /* Test the validity of the arguments */
  ensure_has_X_stbox(box);
  ensure_same_geodetic(temp->flags, box->flags);
  ensure_same_spatial_dimensionality_temp_box(temp->flags, box->flags);
  ensure_same_srid(tpoint_srid(temp), stbox_srid(box));
  /* Project the temporal point to the timespan of the box */
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  Span p, inter;
  if (hast)
  {
    temporal_set_period(temp, &p);
    if (! inter_span_span(&p, &box->period, &inter))
      return DBL_MAX;
  }

  /* Select the distance function to be applied */
  datum_func2 func = distance_fn(box->flags);
  /* Convert the stbox to a geometry */
  Datum geo = PointerGetDatum(stbox_to_geo(box));
  Temporal *temp1 = hast ?
    temporal_restrict_period(temp, &inter, REST_AT) :
    (Temporal *) temp;
  /* Compute the result */
  Datum traj = PointerGetDatum(tpoint_trajectory(temp1));
  double result = DatumGetFloat8(func(traj, geo));

  pfree(DatumGetPointer(traj));
  pfree(DatumGetPointer(geo));
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance between the temporal points
 * @sqlop @p |=|
 */
double
nad_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  Temporal *dist = distance_tpoint_tpoint(temp1, temp2);
  if (dist == NULL)
    return -1;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal point and a geometry.
 * @sqlfunc shortestLine()
 */
bool
shortestline_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  GSERIALIZED **result)
{
  if (gserialized_is_empty(gs))
    return false;
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(temp->flags);
  if (geodetic)
    ensure_has_not_Z_gs(gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  GSERIALIZED *traj = tpoint_trajectory(temp);
  if (geodetic)
    /* Notice that geography_shortestline_internal is a MobilityDB function */
    *result = geography_shortestline_internal(traj, gs, true);
  else
  {
    *result = MEOS_FLAGS_GET_Z(temp->flags) ?
      gserialized_shortestline3d(traj, gs) :
      gserialized_shortestline2d(traj, gs);
  }
  pfree(traj);
  return true;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the line connecting the nearest approach point between the
 * temporal points
 * @sqlfunc shortestLine()
 */
bool
shortestline_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  GSERIALIZED **result)
{
  ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  Temporal *dist = distance_tpoint_tpoint(temp1, temp2);
  if (dist == NULL)
    return false;
  const TInstant *inst = temporal_min_instant(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  temporal_value_at_timestamp(temp1, inst->t, false, &value1);
  temporal_value_at_timestamp(temp2, inst->t, false, &value2);
  LWGEOM *line = (LWGEOM *) lwline_make(value1, value2);
  *result = geo_serialize(line);
  lwgeom_free(line);
  return true;
}

/*****************************************************************************/

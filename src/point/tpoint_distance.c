/***********************************************************************
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
 * @file tpoint_distance.c
 * Distance functions for temporal points.
 */

#include "point/tpoint_distance.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif

#if POSTGIS_VERSION_NUMBER >= 30000
#include <lwgeodetic_tree.h>
#include <measures.h>
#include <measures3d.h>
#endif

#include "general/period.h"
#include "general/timeops.h"
#include "general/temporaltypes.h"
#include "point/postgis.h"
#include "point/geography_funcs.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialrels.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************/

/**
 * Compute the projected point and the distance between the closest point
 * (2D version).
 *
 * @note Function inspired by PostGIS function lw_dist2d_distancepoint
 * from measures.c
 */
static double
lw_dist2d_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
  long double *fraction)
{
  DISTPTS thedl;
  thedl.mode = mode;
  thedl.distance= FLT_MAX;
  thedl.tolerance = 0;
  lw_dist2d_recursive(lw1, lw2, &thedl);
  LWLINE *lwline = lwgeom_as_lwline(lw1);
  POINT2D a, b, closest;
  getPoint2d_p(lwline->points, 0, &a);
  getPoint2d_p(lwline->points, 1, &b);
  *fraction = closest_point2d_on_segment_ratio(&thedl.p1, &a, &b, &closest);
  return thedl.distance;
}

/**
 * Compute the projected point and the distance between the closest point
 * (3D version).
 *
 * @note Function inspired by PostGIS function lw_dist3d_distancepoint
 * from measures3d.c
 */
static double
lw_dist3d_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
  long double *fraction)
{
  assert(FLAGS_GET_Z(lw1->flags) && FLAGS_GET_Z(lw2->flags));
  DISTPTS3D thedl;
  thedl.mode = mode;
  thedl.distance= FLT_MAX;
  thedl.tolerance = 0;
  lw_dist3d_recursive(lw1, lw2, &thedl);
  LWLINE *lwline = lwgeom_as_lwline(lw1);
  POINT3DZ a, b, closest;
  getPoint3dz_p(lwline->points, 0, &a);
  getPoint3dz_p(lwline->points, 1, &b);
  *fraction = closest_point3dz_on_segment_ratio(&thedl.p1, &a, &b, &closest);
  return thedl.distance;
}

/**
 * Compute the projected point and the distance between the closest point
 * (geodetic version).
 */
double
lw_dist_sphere_point_dist(const LWGEOM *lw1, const LWGEOM *lw2, int mode,
  long double *fraction)
{
  double min_dist = FLT_MAX;
  double max_dist = FLT_MAX;
  GEOGRAPHIC_POINT closest1, closest2, proj;
  GEOGRAPHIC_EDGE e;
  POINT4D a, b;

  CIRC_NODE *circ_tree1 = lwgeom_calculate_circ_tree(lw1);
  CIRC_NODE *circ_tree2 = lwgeom_calculate_circ_tree(lw2);
  circ_tree_distance_tree_internal(circ_tree1, circ_tree2, FP_TOLERANCE,
    &min_dist, &max_dist, &closest1, &closest2);
  double result = sphere_distance(&closest1, &closest2);

  /* Initialize edge */
  LWLINE *lwline = lwgeom_as_lwline(lw1);
  getPoint4d_p(lwline->points, 0, &a);
  getPoint4d_p(lwline->points, 1, &b);
  geographic_point_init(a.x, a.y, &(e.start));
  geographic_point_init(b.x, b.y, &(e.end));

  /* Get the spherical distance between point and edge */
  edge_distance_to_point(&e, &closest1, &proj);

  /* Compute distance from beginning of the segment to closest point */
  long double seglength = sphere_distance(&(e.start), &(e.end));
  long double length = sphere_distance(&(e.start), &closest1);
  *fraction = length / seglength;

  return result;
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * Returns the value and timestamp at which the a temporal point segment and
 * a point are at the minimum distance. These are the turning points when
 * computing the temporal distance.
 *
 * @param[in] start,end Instants defining the first segment
 * @param[in] point Base point
 * @param[in] basetypid Oid of the base point
 * @param[out] value Projected value at turning point
 * @param[out] t Timestamp at turning point
 * @pre The segment is not constant.
 * @note The parameter basetypid is not needed for temporal points
 */
static bool
tpoint_geo_min_dist_at_timestamp(const TInstant *start, const TInstant *end,
  Datum point, Oid basetypid, Datum *value, TimestampTz *t)
{
  long double duration = (long double) (end->t - start->t);
  Datum value1 = tinstant_value(start);
  Datum value2 = tinstant_value(end);
  double dist;
  long double fraction = geosegm_locate_point(value1, value2, point, &dist);
  if (fraction <= MOBDB_EPSILON || fraction >= (1.0 - MOBDB_EPSILON))
    return false;
  *value = Float8GetDatum(dist);
  *t = start->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * Returns the value and timestamp at which the two temporal geometric point
 * segments are at the minimum distance. These are the turning points
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
  long double denum, fraction;
  long double dx1, dy1, dx2, dy2, f1, f2, f3, f4;
  long double duration = (long double) (end1->t - start1->t);

  bool hasz = MOBDB_FLAGS_GET_Z(start1->flags);
  if (hasz) /* 3D */
  {
    long double dz1, dz2, f5, f6;
    const POINT3DZ *p1 = datum_get_point3dz_p(tinstant_value(start1));
    const POINT3DZ *p2 = datum_get_point3dz_p(tinstant_value(end1));
    const POINT3DZ *p3 = datum_get_point3dz_p(tinstant_value(start2));
    const POINT3DZ *p4 = datum_get_point3dz_p(tinstant_value(end2));
    /* The following basically computes d/dx (Euclidean distance) = 0->
       To reduce problems related to floating point arithmetic, t1 and t2
       are shifted, respectively, to 0 and 1 before computing d/dx */
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

    denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) +
      dx2*dx2 + dy2*dy2 + dz2*dz2;
    if (denum == 0)
      return false;

    fraction = (f1 + f2 + f3 + f4 + f5 + f6) / denum;
  }
  else /* 2D */
  {
    const POINT2D *p1 = datum_get_point2d_p(tinstant_value(start1));
    const POINT2D *p2 = datum_get_point2d_p(tinstant_value(end1));
    const POINT2D *p3 = datum_get_point2d_p(tinstant_value(start2));
    const POINT2D *p4 = datum_get_point2d_p(tinstant_value(end2));
    /* The following basically computes d/dx (Euclidean distance) = 0.
       To reduce problems related to floating point arithmetic, t1 and t2
       are shifted, respectively, to 0 and 1 before computing d/dx */
    dx1 = p2->x - p1->x;
    dy1 = p2->y - p1->y;
    dx2 = p4->x - p3->x;
    dy2 = p4->y - p3->y;

    f1 = p3->x * (dx1 - dx2);
    f2 = p1->x * (dx2 - dx1);
    f3 = p3->y * (dy1 - dy2);
    f4 = p1->y * (dy2 - dy1);

    denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dy2*dy2 + dx2*dx2;
    /* If the segments are parallel */
    if (denum == 0)
      return false;

    fraction = (f1 + f2 + f3 + f4) / denum;
  }
  if (fraction <= MOBDB_EPSILON || fraction >= (1.0 - MOBDB_EPSILON))
    return false;
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* We know that this function is called only for linear segments */
  Datum value1 = tsegment_value_at_timestamp(start1, end1, LINEAR, *t);
  Datum value2 = tsegment_value_at_timestamp(start2, end2, LINEAR, *t);
  *value = hasz ?
    geom_distance3d(value1, value2) : geom_distance2d(value1, value2);
  return true;
}

/**
 * Returns the single timestamp at which the two temporal geographic point
 * segments are at the minimum distance. These are the turning points
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
  const POINT2D *p1 = datum_get_point2d_p(tinstant_value(start1));
  const POINT2D *p2 = datum_get_point2d_p(tinstant_value(end1));
  const POINT2D *p3 = datum_get_point2d_p(tinstant_value(start2));
  const POINT2D *p4 = datum_get_point2d_p(tinstant_value(end2));
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
      *mindist = 0.0;
    /* In this case we must take the temporality into account */
    long double dx1, dy1, dz1, dx2, dy2, dz2, f1, f2, f3, f4, f5, f6, denum;
    dx1 = A2.x - A1.x;
    dy1 = A2.y - A1.y;
    dz1 = A2.z - A1.z;
    dx2 = B2.x - B1.x;
    dy2 = B2.y - B1.y;
    dz2 = B2.z - B1.z;

    f1 = B1.x * (dx1 - dx2);
    f2 = A1.x * (dx2 - dx1);
    f3 = B1.y * (dy1 - dy2);
    f4 = A1.y * (dy2 - dy1);
    f5 = B1.z * (dz1 - dz2);
    f6 = A1.z * (dz2 - dz1);

    denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) +
      dx2*dx2 + dy2*dy2 + dz2*dz2;
    if (denum == 0)
      return false;

    fraction = (double) ((f1 + f2 + f3 + f4 + f5 + f6) / denum);
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
      *mindist = WGS84_RADIUS * sphere_distance(&close1, &close2);
    /* Compute distance from beginning of the segment to one closest point */
    long double seglength = sphere_distance(&(e1.start), &(e1.end));
    long double length = sphere_distance(&(e1.start), &close1);
    fraction = (double) (length / seglength);
  }

  if (fraction <= MOBDB_EPSILON || fraction >= (1.0 - MOBDB_EPSILON))
    return false;
  long double duration = (long double) (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * Returns the value and timestamp at which the two temporal point segments
 * are at the minimum distance (dispatch function).
 *
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[in] linear1,linear2 State whether the interpolation is linear
 * @param[out] value Value
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
bool
tpoint_min_dist_at_timestamp(const TInstant *start1, const TInstant *end1,
  bool linear1, const TInstant *start2, const TInstant *end2, bool linear2,
  Datum *value, TimestampTz *t)
{
  if (MOBDB_FLAGS_GET_GEODETIC(start1->flags))
    /* The distance output parameter is not used here */
    return tgeogpoint_min_dist_at_timestamp(start1, end1, start2, end2, value, t);
  else
    return tgeompoint_min_dist_at_timestamp(start1, end1, start2, end2, value, t);
}

/*****************************************************************************/

/**
 * Returns the temporal distance between the temporal point and the
 * geometry/geography point (distpatch function)
 */
Temporal *
distance_tpoint_geo_internal(const Temporal *temp, Datum geo)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) get_distance_fn(temp->flags);
  lfinfo.numparam = 0;
  lfinfo.argtypid[0] = lfinfo.argtypid[1] = temp->basetypid;
  lfinfo.restypid = FLOAT8OID;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = lfinfo.reslinear ?
    &tpoint_geo_min_dist_at_timestamp : NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal_base(temp, geo, &lfinfo);
  return result;
}

PG_FUNCTION_INFO_V1(distance_geo_tpoint);
/**
 * Returns the temporal distance between the geometry/geography point
 * and the temporal point
 */
PGDLLEXPORT Datum
distance_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_geo_internal(temp, PointerGetDatum(gs));
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(distance_tpoint_geo);
/**
 * Returns the temporal distance between the temporal point and the
 * geometry/geography point
 */
PGDLLEXPORT Datum
distance_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_geo_internal(temp, PointerGetDatum(gs));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the temporal distance between the two temporal points
 * (dispatch function)
 */
Temporal *
distance_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) get_pt_distance_fn(temp1->flags);
  lfinfo.numparam = 0;
  lfinfo.restypid = FLOAT8OID;
  lfinfo.reslinear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = lfinfo.reslinear ? &tpoint_min_dist_at_timestamp : NULL;
  Temporal *result = tfunc_temporal_temporal(temp1, temp2, &lfinfo);
  return result;
}

PG_FUNCTION_INFO_V1(distance_tpoint_tpoint);
/**
 * Returns the temporal distance between the two temporal points
 */
PGDLLEXPORT Datum
distance_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp1), tpoint_srid_internal(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_tpoint_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

/**
 * Returns the nearest approach instant between the temporal instant set point
 * and the geometry/geography
 *
 * @param[in] ti Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TInstant *
NAI_tpointinstset_geo(const TInstantSet *ti, Datum geo, datum_func2 func)
{
  double mindist = DBL_MAX;
  int number = 0; /* keep compiler quiet */
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    Datum value = tinstant_value(inst);
    double dist = DatumGetFloat8(func(value, geo));
    if (dist < mindist)
    {
      mindist = dist;
      number = i;
    }
  }
  return tinstant_copy(tinstantset_inst_n(ti, number));
}

/*****************************************************************************/

/* NAI between temporal sequence point with step interpolation and a
 * geometry/geography */

/**
 * Returns the new current nearest approach instant between the temporal
 * sequence point with stepwise interpolation and the geometry/geography
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] mindist Current minimum distance, it is set at DBL_MAX at the
 * begining but contains the minimum distance found in the previous
 * sequences of a temporal sequence set
 * @param[in] func Distance function
 * @param[out] mininst Instant with the minimum distance
 */
static double
NAI_tpointseq_step_geo1(const TSequence *seq, Datum geo, double mindist,
  datum_func2 func, const TInstant **mininst)
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    double dist = DatumGetFloat8(func(tinstant_value(inst), geo));
    if (dist < mindist)
    {
      mindist = dist;
      *mininst = inst;
    }
  }
  return mindist;
}

/**
 * Returns the nearest approach instant between the temporal sequence
 * point with stepwise interpolation and the geometry/geography
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TInstant *
NAI_tpointseq_step_geo(const TSequence *seq, Datum geo, datum_func2 func)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  NAI_tpointseq_step_geo1(seq, geo, DBL_MAX, func, &inst);
  return tinstant_copy(inst);
}

/**
 * Returns the nearest approach instant between the temporal sequence set
 * point with stepwise interpolation and the geometry/geography
 *
 * @param[in] ts Temporal point
 * @param[in] geo Geometry
 * @param[in] func Distance function
 */
static TInstant *
NAI_tpointseqset_step_geo(const TSequenceSet *ts, Datum geo, datum_func2 func)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  double mindist = DBL_MAX;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    mindist = NAI_tpointseq_step_geo1(seq, geo, mindist, func, &inst);
  }
  assert(inst != NULL);
  return tinstant_copy(inst);
}

/*****************************************************************************/

/**
 * Returns the nearest approach instant between the segment of a temporal
 * sequence point with linear interpolation and the geometry
 *
 * @param[in] inst1,inst2 Temporal segment
 * @param[in] lwgeom Geometry
 * @param[out] closest Closest point
 * @param[out] t Timestamp
 * @param[out] tofree True when the resulting instant should be freed
 */
static double
NAI_tpointseq_linear_geo1(const TInstant *inst1, const TInstant *inst2,
  LWGEOM *lwgeom, Datum *closest, TimestampTz *t, bool *tofree)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  *tofree = false;
  double dist;
  long double fraction;

  /* Constant segment */
  if (datum_point_eq(value1, value2))
  {
    *closest = value1;
    *t = inst1->t;
    return 0.0;
  }

  /* The trajectory is a line */
  LWLINE *lwline = geopoint_lwline(value1, value2);
  if (MOBDB_FLAGS_GET_GEODETIC(inst1->flags))
    dist = lw_dist_sphere_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction);
  else
    dist = MOBDB_FLAGS_GET_Z(inst1->flags) ?
      lw_dist3d_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction) :
      lw_dist2d_point_dist((LWGEOM *) lwline, lwgeom, DIST_MIN, &fraction);
  lwline_free(lwline);

  if (fabsl(fraction) < MOBDB_EPSILON)
  {
    *closest = value1;
    *t = inst1->t;
    return 0.0;
  }
  if (fabsl(fraction - 1.0) < MOBDB_EPSILON)
  {
    *closest = value2;
    *t = inst2->t;
    return 0.0;
  }

  double duration = (inst2->t - inst1->t);
  *t = inst1->t + (TimestampTz) (duration * fraction);
  *tofree = true;
  /* We are sure that it is linear interpolation */
  *closest =  tsegment_value_at_timestamp(inst1, inst2, true, *t);
  return dist;
}

/**
 * Returns the nearest approach instant between the temporal sequence
 * point with linear interpolation and the geometry
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] mindist Minimum distance found so far, or DBL_MAX at the beginning
 * @param[in] func Distance function
 * @param[out] closest Closest point
 * @param[out] t Timestamp
 * @param[out] tofree True when the resulting instant should be freed
 */
static double
NAI_tpointseq_linear_geo2(const TSequence *seq, Datum geo, double mindist,
  datum_func2 func, Datum *closest, TimestampTz *t, bool *tofree)
{
  const TInstant *inst1;
  double dist;
  Datum point;
  TimestampTz t1;
  bool tofree1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    point = tinstant_value(inst1);
    dist =  DatumGetFloat8(func(point, geo));
    if (dist < mindist)
    {
      mindist = dist;
      *closest = point;
      *t = inst1->t;
      *tofree = false;
    }
    return mindist;
  }

  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geo);
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  inst1 = tsequence_inst_n(seq, 0);
  *tofree = false;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    dist = NAI_tpointseq_linear_geo1(inst1, inst2, lwgeom, &point, &t1, &tofree1);
    if (dist < mindist)
    {
      if (*tofree)
        pfree(DatumGetPointer(*closest));
      mindist = dist;
      *closest = point;
      *t = t1;
      *tofree = tofree1;
    }
    if (mindist == 0.0)
      break;
    inst1 = inst2;
  }
  return mindist;
}

/**
 * Returns the nearest approach instant between the temporal sequence
 * point with linear interpolation and the geometry
 */
static TInstant *
NAI_tpointseq_linear_geo(const TSequence *seq, Datum geo, datum_func2 func)
{
  Datum closest;
  TimestampTz t;
  bool tofree;
  NAI_tpointseq_linear_geo2(seq, geo, DBL_MAX, func, &closest, &t, &tofree);
  TInstant *result = tinstant_make(closest, t, seq->basetypid);
  if (tofree)
    pfree(DatumGetPointer(closest));
  return result;
}

/**
 * Returns the nearest approach instant between the temporal sequence set
 * point with linear interpolation and the geometry
 */
static TInstant *
NAI_tpointseqset_linear_geo(const TSequenceSet *ts, Datum geo, datum_func2 func)
{
  Datum point, closest = PointerGetDatum(NULL); /* make compiler quiet */
  TimestampTz t = 0, t1; /* make compiler quiet */
  bool tofree = false, tofree1;
  double mindist = DBL_MAX;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    double dist = NAI_tpointseq_linear_geo2(seq, geo, mindist, func,
      &point, &t1, &tofree1);
    if (dist < mindist)
    {
      if (tofree)
        pfree(DatumGetPointer(closest));
      mindist = dist;
      closest = point;
      t = t1;
      tofree = tofree1;
    }
    if (mindist == 0.0)
      break;
  }
  TInstant *result = tinstant_make(closest, t, ts->basetypid);
  if (tofree)
    pfree(DatumGetPointer(closest));
  return result;
}

/*****************************************************************************/

/**
 * Returns the nearest approach instant between the temporal point and
 * the geometry (dispatch function)
 */
TInstant *
NAI_tpoint_geo_internal(FunctionCallInfo fcinfo, const Temporal *temp,
  GSERIALIZED *gs)
{
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  datum_func2 func = get_distance_fn(temp->flags);
  TInstant *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_copy((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = NAI_tpointinstset_geo((TInstantSet *) temp, PointerGetDatum(gs), func);
  else if (temp->subtype == SEQUENCE)
    result = MOBDB_FLAGS_GET_LINEAR(temp->flags) ?
      NAI_tpointseq_linear_geo((TSequence *) temp, PointerGetDatum(gs), func) :
      NAI_tpointseq_step_geo((TSequence *) temp, PointerGetDatum(gs), func);
  else /* temp->subtype == SEQUENCESET */
    result = MOBDB_FLAGS_GET_LINEAR(temp->flags) ?
      NAI_tpointseqset_linear_geo((TSequenceSet *) temp, PointerGetDatum(gs), func) :
      NAI_tpointseqset_step_geo((TSequenceSet *) temp, PointerGetDatum(gs), func);
  return result;
}

PG_FUNCTION_INFO_V1(NAI_geo_tpoint);
/**
 * Returns the nearest approach instant between the geometry and
 * the temporal point
 */
PGDLLEXPORT Datum
NAI_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  TInstant *result = NAI_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_geo);
/**
 * Returns the nearest approach instant between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
NAI_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TInstant *result = NAI_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_tpoint);
/**
 * Returns the nearest approach instant between the temporal points
 */
PGDLLEXPORT Datum
NAI_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp1), tpoint_srid_internal(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  TInstant *result = NULL;
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
  if (dist != NULL)
  {
    const TInstant *min = temporal_min_instant(dist);
    result = (TInstant *) temporal_restrict_timestamp_internal(temp1,
      min->t, REST_AT);
    pfree(dist);
    if (result == NULL)
    {
      if (temp1->subtype == SEQUENCE)
        result = tinstant_copy(tsequence_inst_at_timestamp_excl(
          (TSequence *) temp1, min->t));
      else /* temp->subtype == SEQUENCESET */
        result = tinstant_copy(tsequenceset_inst_at_timestamp_excl(
          (TSequenceSet *) temp1, min->t));
    }
  }
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/**
 * Returns the nearest approach distance between the temporal point and the
 * geometry (internal function)
 */
Datum
NAD_tpoint_geo_internal(FunctionCallInfo fcinfo, Temporal *temp,
  GSERIALIZED *gs)
{
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  datum_func2 func = get_distance_fn(temp->flags);
  Datum traj = tpoint_trajectory_internal(temp);
  Datum result = func(traj, PointerGetDatum(gs));
#ifdef STORE_TRAJ
  tpoint_trajectory_free(temp, traj);
#else
  pfree(DatumGetPointer(traj));
#endif
  return result;
}

PG_FUNCTION_INFO_V1(NAD_geo_tpoint);
/**
 * Returns the nearest approach distance between the geometry and
 * the temporal point
 */
PGDLLEXPORT Datum
NAD_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum result = NAD_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_geo);
/**
 * Returns the nearest approach distance between the temporal point
 * and the geometry
 */
PGDLLEXPORT Datum
NAD_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = NAD_tpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

/**
 * Returns the nearest approach distance between the spatiotemporal box and
 * the geometry (internal function)
 */
Datum
NAD_stbox_geo_internal(FunctionCallInfo fcinfo, STBOX *box,
  GSERIALIZED *gs)
{
  ensure_same_srid_stbox_gs(box, gs);
  ensure_same_spatial_dimensionality_stbox_gs(box, gs);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  datum_func2 func = get_distance_fn(box->flags);
  GBOX gbox;
  BOX3D box3d;
  Datum geo;
  if (hasz || geodetic)
  {
    stbox_box3d(box, &box3d);
    geo = call_function1(BOX3D_to_LWGEOM, PointerGetDatum(&box3d));
  }
  else
  {
    stbox_gbox(box, &gbox);
    geo = call_function1(BOX2D_to_LWGEOM, PointerGetDatum(&gbox));
  }
  Datum result = func(geo, PointerGetDatum(gs));
  pfree(DatumGetPointer(geo));
  return result;
}

PG_FUNCTION_INFO_V1(NAD_geo_stbox);
/**
 * Returns the nearest approach distance between the geometry and
 * the spatiotemporal box
 */
PGDLLEXPORT Datum
NAD_geo_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  STBOX *box = PG_GETARG_STBOX_P(1);
  Datum result = NAD_stbox_geo_internal(fcinfo, box, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_stbox_geo);
/**
 * Returns the nearest approach distance between the spatiotemporal box
 * and the geometry
 */
PGDLLEXPORT Datum
NAD_stbox_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum result = NAD_stbox_geo_internal(fcinfo, box, gs);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

/**
 * Returns the nearest approach distance between the spatio-temporal boxes
 * (internal function)
 */
double
NAD_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  /* Test the validity of the arguments */
  ensure_has_X_stbox(box1); ensure_has_X_stbox(box2);
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_spatial_dimensionality(box1->flags, box2->flags);
  ensure_same_srid_stbox(box1, box2);
  /* Project the boxes to their common timespan */
  bool hast = MOBDB_FLAGS_GET_T(box1->flags) &&
    MOBDB_FLAGS_GET_T(box2->flags);
  Period p1, p2;
  Period *inter = NULL;
  if (hast)
  {
    period_set(box1->tmin, box1->tmax, true, true, &p1);
    period_set(box2->tmin, box2->tmax, true, true, &p2);
    inter = intersection_period_period_internal(&p1, &p2);
    if (!inter)
      return DBL_MAX;
    pfree(inter);
  }

  /* Select the distance function to be applied */
  bool hasz = MOBDB_FLAGS_GET_Z(box1->flags);
  datum_func2 func = get_distance_fn(box1->flags);
  /* Convert the boxes to geometries */
  GBOX gbox1, gbox2;
  stbox_gbox(box1, &gbox1);
  Datum geo1 = hasz ? call_function1(BOX3D_to_LWGEOM, PointerGetDatum(&gbox1)) :
    call_function1(BOX2D_to_LWGEOM, PointerGetDatum(&gbox1));
  Datum geo11 = call_function2(LWGEOM_set_srid, geo1,
    Int32GetDatum(box1->srid));
  stbox_gbox(box2, &gbox2);
  Datum geo2 = hasz ? call_function1(BOX3D_to_LWGEOM, PointerGetDatum(&gbox2)) :
    call_function1(BOX2D_to_LWGEOM, PointerGetDatum(&gbox2));
  Datum geo21 = call_function2(LWGEOM_set_srid, geo2,
    Int32GetDatum(box2->srid));
  /* Compute the result */
  double result = DatumGetFloat8(func(geo11, geo21));

  pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo11));
  pfree(DatumGetPointer(geo2)); pfree(DatumGetPointer(geo21));
  return result;
}

PG_FUNCTION_INFO_V1(NAD_stbox_stbox);
/**
 * Returns the nearest approach distance between the spatio-temporal boxes
 */
PGDLLEXPORT Datum
NAD_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = NAD_stbox_stbox_internal(box1, box2);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/**
 * Returns the nearest approach distance between the temporal point and the
 * spatio-temporal box (internal function)
 */
double
NAD_tpoint_stbox_internal(const Temporal *temp, STBOX *box)
{
  /* Test the validity of the arguments */
  ensure_has_X_stbox(box);
  ensure_same_geodetic(temp->flags, box->flags);
  ensure_same_spatial_dimensionality(temp->flags, box->flags);
  ensure_same_srid_tpoint_stbox(temp, box);
  /* Project the temporal point to the timespan of the box */
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  Period p1, p2;
  Period *inter;
  if (hast)
  {
    temporal_period(temp, &p1);
    period_set(box->tmin, box->tmax, true, true, &p2);
    inter = intersection_period_period_internal(&p1, &p2);
    if (!inter)
      return DBL_MAX;
  }

  /* Select the distance function to be applied */
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  datum_func2 func = get_distance_fn(box->flags);
  /* Convert the stbox to a geometry */
  GBOX gbox;
  stbox_gbox(box, &gbox);
  Datum geo = hasz ? call_function1(BOX3D_to_LWGEOM, PointerGetDatum(&gbox)) :
    call_function1(BOX2D_to_LWGEOM, PointerGetDatum(&gbox));
  Datum geo1 = call_function2(LWGEOM_set_srid, geo,
    Int32GetDatum(box->srid));
  Temporal *temp1 = hast ?
    temporal_restrict_period_internal(temp, inter, REST_AT) :
    (Temporal *) temp;
  /* Compute the result */
  Datum traj = tpoint_trajectory_internal(temp1);
  double result = DatumGetFloat8(func(traj, geo1));

#ifdef STORE_TRAJ
  tpoint_trajectory_free(temp1, traj);
#else
  pfree(DatumGetPointer(traj));
#endif
  pfree(DatumGetPointer(geo)); pfree(DatumGetPointer(geo1));
  if (hast)
  {
    pfree(inter); pfree(temp1);
  }
  return result;
}

PG_FUNCTION_INFO_V1(NAD_stbox_tpoint);
/**
 * Returns the nearest approach distance between the spatio-temporal box and the
 * temporal point
 */
PGDLLEXPORT Datum
NAD_stbox_tpoint(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = NAD_tpoint_stbox_internal(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_stbox);
/**
 * Returns the nearest approach distance between the temporal point and the
 * spatio-temporal box
 */
PGDLLEXPORT Datum
NAD_tpoint_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = NAD_tpoint_stbox_internal(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_tpoint);
/**
 * Returns the nearest approach distance between the temporal points
 */
PGDLLEXPORT Datum
NAD_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp1), tpoint_srid_internal(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
  if (dist == NULL)
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  Datum result = temporal_min_value_internal(dist);
  pfree(dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * Returns the line connecting the nearest approach point between the
 * temporal instant point and the geometry (internal function)
 */
Datum
shortestline_tpoint_geo_internal(Temporal *temp, GSERIALIZED *gs)
{
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(temp->flags);
  if (geodetic)
    ensure_has_not_Z_gs(gs);
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  Datum traj = tpoint_trajectory_internal(temp);
  Datum result;
  if (geodetic)
    result = call_function2(geography_shortestline, traj, PointerGetDatum(gs));
  else
    result = MOBDB_FLAGS_GET_Z(temp->flags) ?
      call_function2(LWGEOM_shortestline3d, traj, PointerGetDatum(gs)) :
      call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
#ifdef STORE_TRAJ
  tpoint_trajectory_free(temp, traj);
#else
  pfree(DatumGetPointer(traj));
#endif
  return result;
}

PG_FUNCTION_INFO_V1(shortestline_geo_tpoint);
/**
 * Returns the line connecting the nearest approach point between the
 * geometry and the temporal instant point
 */
PGDLLEXPORT Datum
shortestline_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum result = shortestline_tpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(shortestline_tpoint_geo);
/**
 * Returns the line connecting the nearest approach point between the
 * temporal instant point and the geometry/geography
 */
PGDLLEXPORT Datum
shortestline_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = shortestline_tpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

/**
 * Returns the line connecting the nearest approach point between the
 * temporal points
 */
bool
shortestline_tpoint_tpoint_internal(const Temporal *temp1,
  const Temporal *temp2, Datum *line)
{
  Temporal *dist = distance_tpoint_tpoint_internal(temp1, temp2);
  if (dist == NULL)
    return false;
  const TInstant *inst = temporal_min_instant(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  bool found1 = temporal_value_at_timestamp_inc(temp1, inst->t, &value1);
  bool found2 = temporal_value_at_timestamp_inc(temp2, inst->t, &value2);
  assert (found1 && found2);
  *line = geopoint_line(value1, value2);
  return true;
}

PG_FUNCTION_INFO_V1(shortestline_tpoint_tpoint);
/**
 * Returns the line connecting the nearest approach point between the
 * temporal points
 */
PGDLLEXPORT Datum
shortestline_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp1), tpoint_srid_internal(temp2));
  ensure_same_dimensionality(temp1->flags, temp2->flags);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum result;
  bool found = shortestline_tpoint_tpoint_internal(temp1, temp2, &result);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (!found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

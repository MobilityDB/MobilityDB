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
 * @brief Spatial restriction functions for temporal points
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/span.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tcellindex.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/geo_funcs.h"
#include "geo/clip_clipper2.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_spatialrels.h"

/*****************************************************************************/

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal point restricted to a point
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tpoint_at_value(const Temporal *temp, GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINT(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  return temporal_restrict_value(temp, PointerGetDatum(gs), REST_AT);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to a geometry/geography
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tgeo_at_value(const Temporal *temp, GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  return temporal_restrict_value(temp, PointerGetDatum(gs), REST_AT);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal point restricted to the complement of a point
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tpoint_minus_value(const Temporal *temp, GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINT(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  return temporal_restrict_value(temp, PointerGetDatum(gs), REST_MINUS);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to the complement of a geo
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tgeo_minus_value(const Temporal *temp, GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  return temporal_restrict_value(temp, PointerGetDatum(gs), REST_MINUS);
}

/**
 * @ingroup meos_geo_accessor
 * @brief Return the value of a temporal geo at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] result Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tgeo_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  GSERIALIZED **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, false); VALIDATE_NOT_NULL(result, false);

  Datum res;
  bool found = temporal_value_at_timestamptz(temp, t, strict, &res);
  *result = DatumGetGserializedP(res);
  return found;
}

/*****************************************************************************
 * Force a temporal point to be 2D
 *****************************************************************************/

/**
 * @brief Force a point to be in 2D
 */
static Datum
point_force2d(Datum point, Datum srid)
{
  const POINT2D *p = DATUM_POINT2D_P(point);
  GSERIALIZED *result = geopoint_make(p->x, p->y, 0.0, false, false,
    DatumGetInt32(srid));
  return PointerGetDatum(result);
}

/**
 * @brief Force a temporal point to be in 2D
 * @param[in] temp Temporal point
 * @pre The temporal point has Z dimension
 */
static Temporal *
tpoint_force2d(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOINT(temp, NULL);
  if (! ensure_tpoint_type(temp->temptype) ||
      ! ensure_has_Z(temp->temptype, temp->flags))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &point_force2d;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(tspatial_srid(temp));
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = T_TGEOMPOINT;
  /* Dropping the Z dimension is affine: linear input -> linear output */
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/

/**
 * @brief Return the timestamp at which a segment of a geodetic temporal point
 * is equal to a 2D point, using planar (lon/lat-as-Cartesian) arithmetic
 * @details GEOS intersection is computed in 2D planar space; the resulting
 * point lies on the planar straight line but deviates from the geodetic
 * great-circle arc by up to a few microradians, which exceeds MEOS_EPSILON
 * when using spherical distance. This function uses 2D Cartesian arithmetic
 * matching GEOS's computation, so the residual distance is essentially zero.
 * @param[in] inst1,inst2 Temporal values
 * @param[in] gs Intersection point (GSERIALIZED, may be geodetic or planar)
 * @param[out] t Timestamp
 * @return Return true if the point is found within the segment
 */
static bool
tpointsegm_timestamp_at_value_2d_iter(const TInstant *inst1,
  const TInstant *inst2, const GSERIALIZED *gs, TimestampTz *t)
{
  const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value_p(inst1));
  const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value_p(inst2));
  const POINT2D *p  = GSERIALIZED_POINT2D_P(gs);
  if (MEOS_FP_EQ(p->x, p1->x) && MEOS_FP_EQ(p->y, p1->y))
  {
    *t = inst1->t;
    return true;
  }
  if (MEOS_FP_EQ(p->x, p2->x) && MEOS_FP_EQ(p->y, p2->y))
  {
    *t = inst2->t;
    return true;
  }
  POINT2D proj;
  double fraction = (double) closest_point2d_on_segment_ratio(p, p1, p2, &proj);
  double dist = distance2d_pt_pt(p, &proj);
  if (fraction < 0.0 || fraction > 1.0 || fabs(dist) >= MEOS_EPSILON)
    return false;
  double duration = (double) (inst2->t - inst1->t);
  *t = inst1->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * @brief Return the timestamp at which a geodetic temporal point sequence
 * is equal to a point, using planar (lon/lat-as-Cartesian) arithmetic
 * @param[in] seq Temporal point sequence
 * @param[in] gs Intersection point (serialized, may be geodetic or planar)
 * @param[out] t Timestamp
 */
static bool
tpointseq_timestamp_at_value_2d(const TSequence *seq, const GSERIALIZED *gs,
  TimestampTz *t)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    if (tpointsegm_timestamp_at_value_2d_iter(inst1, inst2, gs, t))
      return true;
    inst1 = inst2;
  }
  meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
    "The value has not been found due to roundoff errors");
  return false;
}

/**
 * @brief Return the timestamp at which a segment of a temporal point takes a
 * base value (iterator function)
 * @details To take into account roundoff errors, the function considers that
 * two values are equal even if their coordinates may differ by `MEOS_EPSILON`.
 * @param[in] inst1,inst2 Temporal values
 * @param[in] value Base value
 * @param[out] t Timestamp
 * @return Return true if the point is found in the temporal point segment
 * @pre The segment is not constant and has linear interpolation
 * @note The resulting timestamptz may be at an exclusive bound
 */
static bool
tpointsegm_timestamp_at_value1_iter(const TInstant *inst1,
  const TInstant *inst2, Datum value, TimestampTz *t)
{
  Datum value1 = tinstant_value_p(inst1);
  Datum value2 = tinstant_value_p(inst2);
  /* Is the lower bound the answer? */
  bool result = true;
  if (datum_point_eq(value1, value))
    *t = inst1->t;
  /* Is the upper bound the answer? */
  else if (datum_point_eq(value2, value))
    *t = inst2->t;
  else
  {
    double dist;
    double fraction = (double) pointsegm_locate(value1, value2, value, &dist);
    if (fraction < 0 || fabs(dist) >= MEOS_EPSILON)
      result = false;
    else
    {
      double duration = (double) (inst2->t - inst1->t);
      *t = inst1->t + (TimestampTz) (duration * fraction);
    }
  }
  return result;
}

/**
 * @brief Return the timestamp at which a temporal point sequence is equal to a
 * point
 * @details This function is called by the #tpointseq_interperiods function
 * while computing atGeometry to find the timestamp at which an intersection
 * point found by PostGIS is located.
 * @param[in] seq Temporal point sequence
 * @param[in] value Base value
 * @param[out] t Timestamp
 * @return Return true if the point is found in the temporal point
 * @pre The point is known to belong to the temporal sequence (taking into
 * account roundoff errors), the temporal sequence has linear interpolation,
 * and is simple
 * @note The resulting timestamp may be at an exclusive bound
 */
static bool
tpointseq_timestamp_at_value(const TSequence *seq, Datum value,
  TimestampTz *t)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    /* We are sure that the segment is not constant since the
     * sequence is simple */
    if (tpointsegm_timestamp_at_value1_iter(inst1, inst2, value, t))
      return true;
    inst1 = inst2;
  }
  /* We should never arrive here */
  meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
    "The value has not been found due to roundoff errors");
  return false;
}

/*****************************************************************************
 * Restriction functions for a spatiotemporal box
 *****************************************************************************/

/*
 * Region codes for the Cohen-Sutherland algorithm for 3D line clipping
 */
const int INSIDE  = 0;  /* 000000 */
const int LEFT    = 1;  /* 000001 */
const int RIGHT   = 2;  /* 000010 */
const int BOTTOM  = 4;  /* 000100 */
const int TOP     = 8;  /* 001000 */
const int FRONT   = 16; /* 010000 */
const int BACK    = 32; /* 100000 */

/*
 * Border codes for the excluding the top border of a spatiotemporal box
 */
const int XMAX    = 1;  /* 001 */
const int YMAX    = 2;  /* 010 */
const int ZMAX    = 4;  /* 100 */

/**
 * @brief Compute region code for a point(x, y, z)
 */
static int
computeRegionCode(double x, double y, double z, bool hasz, const STBox *box)
{
  /* Initialized as being inside */
  int code = INSIDE;
  if (x < box->xmin)      /* to the left of the box */
    code |= LEFT;
  else if (x > box->xmax) /* to the right of the box */
    code |= RIGHT;
  if (y < box->ymin)      /* below the box */
    code |= BOTTOM;
  else if (y > box->ymax) /* above the box */
    code |= TOP;
  if (hasz)
  {
    if (z < box->zmin)      /* in front of the box */
      code |= FRONT;
    else if (z > box->zmax) /* behind the the box */
      code |= BACK;
  }
  return code;
}

/**
 * @brief Compute max border code for a point(x, y, z)
 */
static int
computeMaxBorderCode(double x, double y, double z, bool hasz, const STBox *box)
{
  /* Initialized as being inside */
  int code = INSIDE;
  /* Check if we are on a max border
   * Note: After clipping, we don't need to apply fabs() */
  if (box->xmax - x < MEOS_EPSILON) /* on xmax border */
    code |= XMAX;
  if (box->ymax - y < MEOS_EPSILON) /* on ymax border */
    code |= YMAX;
  if (hasz && box->zmax - z < MEOS_EPSILON) /* on zmax border */
    code |= ZMAX;
  return code;
}

/**
 * @brief Clip the paramaters t0 and t1 for the Liang-Barsky clipping algorithm
 */
bool
clipt(double p, double q, double *t0, double *t1)
{
  double r;
  if (p < 0)
  {
    /* entering visible region, so compute t0 */
    if (q < 0)
    {
      /* t0 will be nonnegative, so continue */
      r = q / p;
      if (r > *t1)
        /* t0 will exceed t1, so reject */
        return false;
      if (r > *t0)
        *t0 = r; /* t0 is max of r's */
    }
  }
  else if (p > 0)
  {
    /* exiting visible region, so compute t1 */
    if (q < p)
    {
      /* t1 will be <= 1, so continue */
      r = q / p;
      if (r < *t0)
        /* t1 will be <= t0, so reject */
        return false;
      if (r < *t1)
        *t1 = r; /* t1 is min of r's */
    }
  }
  else /* p == 0 */
  {
    if (q < 0)
      /* line parallel and outside */
      return false;
  }
  return true;
}

/**
 * @brief Clip a line segment using the Liang-Barsky algorithm
 * @param[in] point1,point2 Input points
 * @param[in] box Bounding box
 * @param[in] hasz Has Z dimension?
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] point3,point4 Output points
 * @param[out] p3_inc,p4_inc Are the points included or not in the box?
 * These are only written/returned when @p border_inc is false
 * @param[out] param3,param4 Parameters in [0,1] at which the segment takes
 * @p point3 and @p point4. The clip solves the border in this parameter, so it
 * states the crossing to digits the coordinates of the clipped point no longer
 * carry
 * @return True if the line segment defined by p1,p2 intersects the bounding
 * box, false otherwise.
 * @note It is possible to mix 2D/3D geometries, the Z dimension is only
 * considered if both the temporal point and the box have Z dimension
 * @see The Liang-Barsky algorithm is explained in
 * https://www.researchgate.net/publication/255657434_Some_Improvements_to_a_Parametric_Line_Clipping_Algorithm
 */
static bool
liangBarskyClip(const GSERIALIZED *point1, const GSERIALIZED *point2, 
  const STBox *box, bool hasz, bool border_inc, GSERIALIZED **point3,
  GSERIALIZED **point4, bool *p3_inc, bool *p4_inc, double *param3,
  double *param4)
{
  assert(MEOS_FLAGS_GET_X(box->flags));
  assert(! geopoint_eq(point1, point2));
  assert(! hasz || (MEOS_FLAGS_GET_Z(box->flags) &&
    (bool) FLAGS_GET_Z(point1->gflags) && (bool) FLAGS_GET_Z(point2->gflags)));

  int32_t srid = box->srid;
  assert(srid == gserialized_get_srid(point1) &&
    srid == gserialized_get_srid(point2));

  /* Get the input points */
  double x1, y1, z1 = 0.0, x2, y2, z2 = 0.0;
  if (hasz)
  {
    const POINT3DZ *pt1 = GSERIALIZED_POINT3DZ_P(point1);
    const POINT3DZ *pt2 = GSERIALIZED_POINT3DZ_P(point2);
    x1 = pt1->x; x2 = pt2->x;
    y1 = pt1->y; y2 = pt2->y;
    z1 = pt1->z; z2 = pt2->z;
  }
  else
  {
    const POINT2D *pt1 = GSERIALIZED_POINT2D_P(point1);
    const POINT2D *pt2 = GSERIALIZED_POINT2D_P(point2);
    x1 = pt1->x; x2 = pt2->x;
    y1 = pt1->y; y2 = pt2->y;
  }

  if ((x1 < box->xmin && x2 < box->xmin) || (x1 > box->xmax && x2 > box->xmax) ||
      (y1 < box->ymin && y2 < box->ymin) || (y1 > box->ymax && y2 > box->ymax) ||
      (hasz && z1 < box->zmin && z2 < box->zmin) ||
      (hasz && z1 > box->zmax && z2 > box->zmax))
    /* trivial reject */
    return false;

  /* not trivial reject */
  double t0 = 0, t1 = 1;
  double dx = x2 - x1;
  if (clipt(-dx, x1 - box->xmin, &t0, &t1) && /* left */
      clipt(dx, box->xmax - x1, &t0, &t1)) /* right */
  {
    double dy = y2 - y1;
    if (clipt(-dy, y1 - box->ymin, &t0, &t1) && /* bottom */
        clipt(dy, box->ymax - y1, &t0, &t1)) /* top */
    {
      bool found = true;
      double dz = 0;
      if (hasz)
      {
        dz = z2 - z1;
        if (! clipt(-dz, z1 - box->zmin, &t0, &t1) || /* front */
            ! clipt(dz, box->zmax - z1, &t0, &t1)) /* back */
          found = false;
      }
      if (found)
      {
        /* compute coordinates */
        if (t1 < 1)
        {
          /* compute V1' */
          x2 = x1 + t1 * dx;
          y2 = y1 + t1 * dy;
          if (hasz)
            z2 = z1 + t1 * dz;
        }
        if (t0 > 0)
        {
          /* compute V0' */
          x1 = x1 + t0 * dx;
          y1 = y1 + t0 * dy;
          if (hasz)
            z1 = z1 + t0 * dz;
        }
        /* Remove the max border */
        if (! border_inc)
        {
          /* Compute max border codes for the input points */
          int max_code1 = computeMaxBorderCode(x1, y1, z1, hasz, box);
          int max_code2 = computeMaxBorderCode(x2, y2, z2, hasz, box);
          /* If the whole segment is on a max border, remove it */
          if (max_code1 & max_code2)
            return false;
          /* Point is included if its max_code is 0 */
          if (p3_inc && p4_inc)
          {
            *p3_inc = (max_code1 == 0);
            *p4_inc = (max_code2 == 0);
          }
        }
        if (point3 && point4)
        {
          *point3 = geopoint_make(x1, y1, z1, hasz, false, srid);
          *point4 = geopoint_make(x2, y2, z2, hasz, false, srid);
        }
        if (param3 && param4)
        {
          *param3 = t0;
          *param4 = t1;
        }
        return true;
      }
    }
  }
  return false;
}

/*****************************************************************************/

/**
 * @brief Return a temporal point instant restricted to (the complement of) a
 * spatiotemporal box (iterator function)
 * @pre The box has only spatial dimension and the arguments have the same SRID.
 * @note This function restricts only to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox
 */
static TInstant *
tpointinst_restrict_stbox_iter(const TInstant *inst, const STBox *box,
  bool border_inc, bool atfunc)
{
  assert(! MEOS_FLAGS_GET_T(box->flags));
  bool hasz = MEOS_FLAGS_GET_Z(inst->flags) && MEOS_FLAGS_GET_Z(box->flags);

  /* Restrict to the spatial dimension */
  Datum value = tinstant_value_p(inst);
  /* Get the input point */
  double x, y, z = 0.0;
  if (hasz)
  {
    const POINT3DZ *pt = DATUM_POINT3DZ_P(value);
    x = pt->x;
    y = pt->y;
    z = pt->z;
  }
  else
  {
    const POINT2D *pt = DATUM_POINT2D_P(value);
    x = pt->x;
    y = pt->y;
  }
  /* Compute region code for the input point */
  int reg_code = computeRegionCode(x, y, z, hasz, box);
  int max_code = 0;
  if (! border_inc)
    max_code = computeMaxBorderCode(x, y, z, hasz, box);
  if ((reg_code | max_code) != 0)
    return atfunc ? NULL : (TInstant *) inst;

  /* Point is inside the region */
    return atfunc ? (TInstant *) inst : NULL;
}

/**
 * @brief Return a temporal geo instant restricted to (the complement of) a
 * spatiotemporal box (iterator function)
 * @param[in] inst Temporal geo instant
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The box has only spatial dimension and the arguments have the same SRID.
 * @note This function restricts only to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox
 */
static TInstant *
tgeoinst_restrict_stbox_iter(const TInstant *inst, const STBox *box,
  bool border_inc UNUSED, bool atfunc)
{
  assert(! MEOS_FLAGS_GET_T(box->flags));

  /* Execute the specialized function for temporal points */
  if (tpoint_type(inst->temptype))
    return tpointinst_restrict_stbox_iter(inst, box, border_inc, atfunc);

  /* Get the geometry of the instant and convert the box to a geometry */
  const GSERIALIZED *gs1 = DatumGetGserializedP(tinstant_value_p(inst));
  GSERIALIZED *gs2 = stbox_geo(box);
  /* Restrict to the spatial dimension */
  GSERIALIZED *res = atfunc ? 
    geom_intersection2d(gs1, gs2) : geom_difference2d(gs1, gs2);
  pfree(gs2);
  TInstant *result = NULL;
  if (res)
  {
    if (! gserialized_is_empty(res))
      result = tinstant_make(PointerGetDatum(res), inst->temptype, inst->t);
    pfree(res);
  }
  return result;
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo instant restricted to (the complement of) a
 * spatiotemporal box
 * @param[in] inst temporal geo instant
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The box has only spatial dimension and the arguments have the same SRID.
 * @note This function restricts only to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox
 * @csqlfn #Tgeo_at_stbox(), #Tgeo_minus_stbox()
 */
TInstant *
tgeoinst_restrict_stbox(const TInstant *inst, const STBox *box,
  bool border_inc, bool atfunc)
{
  assert(inst); assert(box); assert(tgeo_type_all(inst->temptype));
  assert(! MEOS_FLAGS_GET_T(box->flags));
  TInstant *res = tgeoinst_restrict_stbox_iter(inst, box, border_inc, atfunc);
  if (! res)
    return NULL;
  return tpoint_type(inst->temptype) ? tinstant_copy(res) : res;
}

/**
 * @brief Return a temporal point discrete sequence restricted to (the
 * complement of) a spatiotemporal box
 * @param[in] seq Temporal point discrete sequence
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The box has only spatial dimension, the arguments have the same SRID, and
 * the sequence is not instantaneous.
 * @note The function only restricts to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox.
 */
TSequence *
tgeoseq_disc_restrict_stbox(const TSequence *seq, const STBox *box,
  bool border_inc, bool atfunc)
{
  assert(seq); assert(box); assert(tgeo_type_all(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert (seq->count > 1); assert(! MEOS_FLAGS_GET_T(box->flags));

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *res = tgeoinst_restrict_stbox_iter(TSEQUENCE_INST_N(seq, i),
      box, border_inc, atfunc);
    if (res)
      instants[ninsts++] = res;
  }
  TSequence *result = NULL;
  if (ninsts > 0)
    result = tsequence_make(instants, ninsts, true, true, DISCRETE,
      NORMALIZE_NO);
  /* Clean up and return */
  if (tpoint_type(seq->temptype))
    pfree(instants);
  else
    pfree_array((void **) instants, ninsts);
  return result;
}

/**
 * @brief Return a temporal point sequence with step interpolation restricted
 * to a spatiotemporal box
 * @param[in] seq Temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The box has only spatial dimension, the arguments have the same SRID, and
 * the sequence is not instantaneous.
 * @note This function restricts only to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox
 */
TSequenceSet *
tgeoseq_step_restrict_stbox(const TSequence *seq, const STBox *box,
  bool border_inc, bool atfunc)
{
  assert(seq); assert(box); assert(tgeo_type_all(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP); assert(seq->count > 1);
  assert(! MEOS_FLAGS_GET_T(box->flags));

  /* Restrict to the spatial dimension */
  bool lower_inc;
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TimestampTz start = DatumGetTimestampTz(seq->period.lower);
  int ninsts = 0, nseqs = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i); 
    TInstant *res = tgeoinst_restrict_stbox_iter(inst, box, border_inc,
      atfunc);
    if (res)
      instants[ninsts++] = res;
    else
    {
      if (ninsts > 0)
      {
        /* Continue the last instant of the sequence until the time of inst2
         * projected to the time dimension (if any) */
        Datum value = tinstant_value_p(instants[ninsts - 1]);
        bool tofree = false;
        bool upper_inc = false;
        /* Continue the last instant of the sequence until the time of inst2 */
        instants[ninsts++] = tinstant_make(value, seq->temptype, inst->t);
        tofree = true;
        lower_inc = (instants[0]->t == start) ? seq->period.lower_inc : true;
        sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc,
          upper_inc, STEP, NORMALIZE_NO);
        if (tofree)
          pfree(instants[ninsts - 1]);
        ninsts = 0;
      }
    }
  }
  /* Add a last sequence with the remaining instants */
  if (ninsts > 0)
  {
    lower_inc = (instants[0]->t == start) ? seq->period.lower_inc : true;
    TimestampTz end = DatumGetTimestampTz(seq->period.upper);
    bool upper_inc = (instants[ninsts - 1]->t == end) ?
      seq->period.upper_inc : false;
    sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc, upper_inc,
      STEP, NORMALIZE_NO);
  }

  /* Clean up and return if no sequences have been found */
  if (tpoint_type(seq->temptype))
    pfree(instants);
  else
    pfree_array((void **) instants, ninsts);
  if (nseqs == 0)
  {
    pfree(sequences);
    return NULL;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE_NO);
}

/*****************************************************************************/

/**
 * @brief Return the instant at which a geodetic segment reaches a parameter of
 * its path
 * @details The parameter states a position, and the position states the
 * instant through #tpointsegm_timestamp_at_value1_iter(), the dating every
 * clip of a segment in this file uses. A parameter at an end of the segment
 * states the instant of that end, and a position the dating does not find
 * falls back on the parameter itself
 * @param[in] inst1,inst2 Bounds of the segment
 * @param[in] arc Path of the segment
 * @param[in] param Parameter along the path
 * @param[in] srid SRID of the temporal point
 */
static TimestampTz
tgeogpointsegm_timestamp_at_param(const TInstant *inst1, const TInstant *inst2,
  const DggsArc *arc, double param, int32_t srid)
{
  if (param <= 0.0)
    return inst1->t;
  if (param >= 1.0)
    return inst2->t;
  double lon, lat;
  TimestampTz result;
  if (dggs_arc_point(arc, param, &lon, &lat))
  {
    GSERIALIZED *gs = geopoint_make(lon * 180.0 / M_PI, lat * 180.0 / M_PI,
      0.0, false, true, srid);
    bool found = tpointsegm_timestamp_at_value1_iter(inst1, inst2,
      PointerGetDatum(gs), &result);
    pfree(gs);
    if (found)
      return result;
  }
  return inst1->t +
    (TimestampTz) ((double) (inst2->t - inst1->t) * param);
}

/**
 * @brief Return true if a position lies in the spatial dimensions of a box
 * @param[in] lon,lat Position in degrees
 * @param[in] box Bounding box
 * @param[in] border_inc True when the box contains the upper border
 */
static bool
stbox_holds_lonlat(double lon, double lat, const STBox *box,
  bool border_inc)
{
  if (lon < box->xmin || lat < box->ymin)
    return false;
  return border_inc ? (lon <= box->xmax && lat <= box->ymax) :
    (lon < box->xmax && lat < box->ymax);
}

/**
 * @brief Restrict a geodetic temporal point to the spatial dimensions of a
 * spatiotemporal box
 * @details A geodetic point travels the great circle between two positions,
 * which is no straight line in longitude and latitude: a trip from
 * `Point(10 60)` to `Point(50 60)` passes through latitude 61.518762, and a
 * trip across the antimeridian sweeps the longitudes the other way round from
 * the ones a straight line between its endpoints passes. The box is therefore
 * met where #dggs_arc_lonlat_box_spans() states the path enters and leaves it,
 * and the trip is restricted to the periods those parameters give, as a cell
 * of a grid on the sphere is entered at the parameter of its own crossing.
 * @param[in] seq Temporal point sequence
 * @param[in] box Bounding box
 * @param[in] border_inc True when the box contains the upper border
 * @pre The box has only spatial dimension, the arguments have the same SRID,
 * and the sequence is not instantaneous
 */
static TSequenceSet *
tpointseq_linear_at_stbox_geodetic(const TSequence *seq, const STBox *box,
  bool border_inc)
{
  assert(seq); assert(box); assert(tpoint_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR); assert(seq->count > 1);

  bool hasz = MEOS_FLAGS_GET_Z(seq->flags) && MEOS_FLAGS_GET_Z(box->flags);
  int32_t srid = tspatial_srid((Temporal *) seq);
  Span *spans = palloc(sizeof(Span) * seq->count * 4);
  int nspans = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const GSERIALIZED *p1 = DatumGetGserializedP(tinstant_value_p(inst1));
    const GSERIALIZED *p2 = DatumGetGserializedP(tinstant_value_p(inst2));
    const POINT2D *q1 = GSERIALIZED_POINT2D_P(p1);
    const POINT2D *q2 = GSERIALIZED_POINT2D_P(p2);
    double lon1 = q1->x, lat1 = q1->y, lon2 = q2->x, lat2 = q2->y;
    DggsArc arc;
    if (geopoint_eq(p1, p2) || ! dggs_arc_init(lon1, lat1, lon2, lat2, &arc))
    {
      /* A segment holding one position is in the box when that position is */
      if (stbox_holds_lonlat(lon1, lat1, box, border_inc))
        span_set(TimestampTzGetDatum(inst1->t), TimestampTzGetDatum(inst2->t),
          true, (i == seq->count - 1) ? seq->period.upper_inc : false,
          T_TIMESTAMPTZ, T_TSTZSPAN, &spans[nspans++]);
      inst1 = inst2;
      continue;
    }
    double tin[4], tout[4];
    int count = dggs_arc_lonlat_box_spans(&arc, box->xmin, box->ymin,
      box->xmax, box->ymax, tin, tout, 4);
    /* The height of a geodetic point runs linearly in time, so the bounds of
     * the box admit one stretch of the segment, which every span meets in */
    double zlo = 0.0, zhi = 1.0;
    if (hasz)
    {
      const POINT3DZ *r1 = GSERIALIZED_POINT3DZ_P(p1);
      const POINT3DZ *r2 = GSERIALIZED_POINT3DZ_P(p2);
      if (r1->z == r2->z)
      {
        if (r1->z < box->zmin || (border_inc ? r1->z > box->zmax :
              r1->z >= box->zmax))
          count = 0;
      }
      else
      {
        double a = (box->zmin - r1->z) / (r2->z - r1->z);
        double b = (box->zmax - r1->z) / (r2->z - r1->z);
        zlo = Min(a, b); zhi = Max(a, b);
        if (zlo < 0.0) zlo = 0.0;
        if (zhi > 1.0) zhi = 1.0;
        if (zlo > zhi)
          count = 0;
      }
    }
    for (int k = 0; k < count; k++)
    {
      if (hasz)
      {
        if (tin[k] < zlo) tin[k] = zlo;
        if (tout[k] > zhi) tout[k] = zhi;
        if (tin[k] > tout[k])
          continue;
      }
      /* A crossing is dated from the position the path holds there, as the
       * clip of a segment by a geometry dates the points it answers, so two
       * boxes meeting at that position state the same instant and the pieces
       * of a trip split by a grid merge back into it. A parameter at an end of
       * the segment states the instant of that end */
      TimestampTz t1 = tgeogpointsegm_timestamp_at_param(inst1, inst2, &arc,
        tin[k], srid);
      TimestampTz t2 = tgeogpointsegm_timestamp_at_param(inst1, inst2, &arc,
        tout[k], srid);
      bool lower_inc = (t1 == inst1->t && i == 1) ?
        seq->period.lower_inc : true;
      bool upper_inc = (t2 == inst2->t) ?
        ((i == seq->count - 1) ? seq->period.upper_inc : false) : border_inc;
      if (t1 == t2)
      {
        if (! lower_inc || ! upper_inc)
          continue;          /* the path holds the box for no time at all */
        upper_inc = true;
      }
      span_set(TimestampTzGetDatum(t1), TimestampTzGetDatum(t2), lower_inc,
        upper_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &spans[nspans++]);
    }
    inst1 = inst2;
  }
  if (nspans == 0)
  {
    pfree(spans);
    return NULL;
  }
  /* The periods the trip holds the box, which the segments state one by one
   * and a span set states as a whole */
  SpanSet *ss = spanset_make_free(spans, nspans, NORMALIZE, ORDER);
  Temporal *res = temporal_restrict_tstzspanset((Temporal *) seq, ss, REST_AT);
  pfree(ss);
  if (! res)
    return NULL;
  if (res->subtype == TSEQUENCE)
    return tsequence_to_tsequenceset_free((TSequence *) res);
  return (TSequenceSet *) res;
}

/**
 * @brief Restrict the temporal point to the spatial dimensions of a
 * spatiotemporal box
 * @param[in] seq Temporal point sequence
 * @param[in] box Bounding box
 * @param[in] border_inc True when the box contains the upper border
 * @pre The box has only spatial dimension, the arguments have the same SRID,
 * and the sequence is not instantaneous.
 * @note The function only restricts to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox.
 */
TSequenceSet *
tpointseq_linear_at_stbox_xyz(const TSequence *seq, const STBox *box,
  bool border_inc)
{
  assert(seq); assert(box); assert(tpoint_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);
  assert(! MEOS_FLAGS_GET_T(box->flags)); assert(seq->count > 1);

  /* A geodetic point travels the great circle between two positions, which no
   * clip of a straight line in longitude and latitude follows */
  if (MEOS_FLAGS_GET_GEODETIC(seq->flags))
    return tpointseq_linear_at_stbox_geodetic(seq, box, border_inc);

  /* General case */
  bool hasz_seq = MEOS_FLAGS_GET_Z(seq->flags);
  bool hasz_box = MEOS_FLAGS_GET_Z(box->flags);
  bool hasz = hasz_seq && hasz_box;
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * 2);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const GSERIALIZED *p1 = DatumGetGserializedP(tinstant_value_p(inst1));
  bool lower_inc = seq->period.lower_inc;
  bool upper_inc;
  int ninsts = 0, nseqs = 0, nfree = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    const GSERIALIZED *p2 = DatumGetGserializedP(tinstant_value_p(inst2));
    GSERIALIZED *p3, *p4;
    bool makeseq = false;
    if (geopoint_eq(p1, p2))
    {
      /* Constant segment */
      if (tpointinst_restrict_stbox_iter(inst1, box, border_inc, REST_AT))
      {
        /* If ninsts > 0 the instant was added in the previous iteration */
        if (ninsts == 0)
          instants[ninsts++] = (TInstant *) inst1;
        instants[ninsts++] = (TInstant *) inst2;
      }
      else
        makeseq = true;
    }
    else
    {
      /* Clip the segment */
      bool p3_inc = true, p4_inc = true;
      double param3, param4;
      bool found = liangBarskyClip(p1, p2, box, hasz, border_inc, &p3, &p4,
        &p3_inc, &p4_inc, &param3, &param4);
      if (found)
      {
        TInstant *inst1_2d, *inst2_2d;
        /* If p2 != p4 or p4_inc is false, we exit the box,
         * so end the previous sequence and start a new one */
        if (! geopoint_eq(p2, p4) || ! p4_inc)
          makeseq = true;
        /* The clip states each crossing as the parameter at which the segment
         * reaches the border, which dates it as the cell walks date theirs.
         * Reading the parameter back out of the clipped point instead loses
         * the border's low digits to the coordinates that carry it, and the
         * crossing then falls a microsecond before the instant it happens */
        TimestampTz t1, t2;
        TimestampTz duration = inst2->t - inst1->t;
        if (hasz_seq && ! hasz)
        {
          /* Force the computation at 2D */
          inst1_2d = (TInstant *) tpoint_force2d((Temporal *) inst1);
          inst2_2d = (TInstant *) tpoint_force2d((Temporal *) inst2);
          p1 = DatumGetGserializedP(tinstant_value_p(inst1_2d));
          p2 = DatumGetGserializedP(tinstant_value_p(inst2_2d));
        }
        /* Compute timestamp t1 of point p3 */
        if (geopoint_eq(p1, p3))
          t1 = inst1->t;
        else if (geopoint_eq(p2, p3))
          t1 = inst2->t;
        else /* inst1->t < t1(p3) < inst2->t */
          t1 = inst1->t + (TimestampTz) ((double) duration * param3);
        /* Compute timestamp t2 of point p4  */
        if (geopoint_eq(p2, p4))
          t2 = inst2->t;
        else if (geopoint_eq(p3, p4))
          t2 = t1;
        else /* inst1->t < t2(p4) < inst2->t */
          t2 = inst1->t + (TimestampTz) ((double) duration * param4);
        if (hasz_seq && ! hasz)
        {
          pfree(inst1_2d); pfree(inst2_2d);
        }
        pfree(p3); pfree(p4);
        /* A chord of the box shorter than a microsecond dates both its
         * crossings at one instant, which may be the instant the segment
         * starts at though the chord starts inside it. Entered through a
         * border the box leaves out, it holds no instant: the truncated
         * instant dates the entry, at which the trip is not yet in the box,
         * and nothing of the chord is left after it. It is no part of the
         * answer, as a crossing landing on the instant already stated is no
         * crossing (#tsegment_intersection) */
        if (t1 == t2 && ! p3_inc)
        {
          assert(ninsts == 0);
          inst1 = inst2;
          p1 = DatumGetGserializedP(tinstant_value_p(inst2));
          continue;
        }
        /* Project the segment to the timestamps if necessary and add the
         * instants */
        Datum inter1 = 0, inter2; /* make compiler quiet */
        bool free1 = false, free2 = false;
        /* If ninsts > 0 the instant was added in the previous iteration */
        if (ninsts == 0)
        {
          if (t1 == inst1->t)
          {
            /* The trip holds the first instant of a segment as its own lower
             * bound states for the first segment alone: every later segment
             * starts at an instant inside the trip */
            instants[ninsts++] = (TInstant *) inst1;
            lower_inc = ((i == 1) ? seq->period.lower_inc : true) && p3_inc;
          }
          else if (t1 != inst2->t)
          {
            inter1 = tsegment_value_at_timestamptz(tinstant_value_p(inst1),
              tinstant_value_p(inst2), inst1->temptype, inst1->t, inst2->t, t1);
            free1 = true;
            instants[ninsts] = tinstant_make(inter1, inst1->temptype, t1);
            tofree[nfree++] = instants[ninsts++];
            lower_inc = p3_inc;
          }
          else /* t1 == inst2->t */
          {
            /* We have t1 == t2 == inst2->t and since found = true, we know
             * that we are on an inclusive border (p3_inc == p4_inc == true).
             * Thus, lower_inc is only false if we are at the last segment
             * and seq->period.upper_inc is false. */
            instants[ninsts++] = (TInstant *) inst2;
            lower_inc = (i == seq->count - 1) ? seq->period.upper_inc : true;
          }
        }
        if (t1 == t2)
        {
          /* If we are here: p3_inc == p4_inc == true, otherwise found would be false.
           * And we also know that p1 != p2, so segment has nonzero length.
           * Thus, we are in 1 of 3 cases: */
          if (t1 == inst1->t)
            /* We are exiting the box at the start of a segment
             * Start of segments are assumed inclusive except for the first one */
            upper_inc = (i == 1) ? seq->period.lower_inc : true;
          else if (t1 == inst2->t)
            /* We are entering the box at the end of a segment
             * End of segments are assumed exclusive except for the last one */
            upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
          else
            /* We clip the box at one of its inclusive corners */
            upper_inc = true;
        }
        else
        {
          if (t2 != inst2->t)
          {
            inter2 = tsegment_value_at_timestamptz(tinstant_value_p(inst1),
              tinstant_value_p(inst2), inst1->temptype, inst1->t, inst2->t, t2);
            free2 = true;
            /* Add the instant only if it is different from the previous one
             * Otherwise, assume that t1 == t2 and skip t2 */
            if (! free1 || ! geopoint_eq(DatumGetGserializedP(inter1),
                  DatumGetGserializedP(inter2)))
            {
              instants[ninsts] = tinstant_make(inter2, inst1->temptype, t2);
              tofree[nfree++] = instants[ninsts++];
              upper_inc = p4_inc;
            }
            else
              upper_inc = lower_inc;
          }
          else
          {
            instants[ninsts++] = (TInstant *) inst2;
            upper_inc &= p4_inc;
          }
        }
        if (free1)
          pfree(DatumGetPointer(inter1));
        if (free2)
          pfree(DatumGetPointer(inter2));
      }
      else
        makeseq = true;
    }
    if (makeseq && ninsts > 0)
    {
      /* We can occasionally have ninsts == 1 and lower_inc == upper_inc == false
       * These are cases where the sequence starts / ends on an inclusive border
       * and we have seq->lower_inc / seq->upper_inc being false.
       * Don't create a sequence, but still reset ninsts and lower_inc */
      if (ninsts > 1 || lower_inc || upper_inc)
        sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc,
          upper_inc, LINEAR, NORMALIZE_NO);
      ninsts = 0;
      lower_inc = true;
    }
    inst1 = inst2;
    p1 = DatumGetGserializedP(tinstant_value_p(inst2));
  }
  /* See above for explanation of condition */
  if (ninsts > 0 && (ninsts > 1 || lower_inc || upper_inc))
    sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc, upper_inc,
      LINEAR, NORMALIZE_NO);
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  if (nseqs == 0)
  {
    pfree(sequences);
    return NULL;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Return a temporal point sequence restricted to (the complement of) a
 * spatiotemporal box
 * @details The function computes the "at" restriction on all dimensions. Then,
 * for the "minus" restriction, it computes the complement of the "at"
 * restriction with respect to the time dimension.
 * The function first filters the temporal point wrt the time dimension
 * to reduce the number of instants before computing the restriction to the
 * spatial dimension.
 * @param[in] seq temporal point sequence
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The box has only spatial dimension, the arguments have the same SRID, and
 * the sequence is not instantaneous.
 * @note The function only restricts to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox.
 */
TSequenceSet *
tpointseq_linear_restrict_stbox(const TSequence *seq, const STBox *box,
  bool border_inc, bool atfunc)
{
  assert(seq); assert(box); assert(tpoint_type(seq->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags)); assert(seq->count > 1);
  assert(! MEOS_FLAGS_GET_T(box->flags));

  /* Compute the "at" restriction for the spatial dimension */
  TSequenceSet *res_at = tpointseq_linear_at_stbox_xyz(seq, box, border_inc);

  /* If "at" restriction, return */
  if (atfunc)
    return res_at;

  /* If "minus" restriction, compute the complement wrt time */
  if (! res_at)
    return tsequence_as_tsequenceset(seq);

  SpanSet *ss = tsequenceset_time(res_at);
  TSequenceSet *result = tcontseq_restrict_tstzspanset(seq, ss, atfunc);
  pfree(ss); pfree(res_at);
  return result;
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo sequence restricted to (the complement of) a
 * spatiotemporal box
 * @param[in] seq temporal geo sequence
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The box has only spatial dimension and the arguments have the same SRID.
 * @note The function only restricts to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox.
 * @csqlfn #Tgeo_at_stbox(), #Tgeo_minus_stbox()
 */
Temporal *
tgeoseq_restrict_stbox(const TSequence *seq, const STBox *box,
  bool border_inc, bool atfunc)
{
  assert(seq); assert(box); assert(tgeo_type_all(seq->temptype));
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(! MEOS_FLAGS_GET_T(box->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *res = tgeoinst_restrict_stbox_iter(TSEQUENCE_INST_N(seq, 0), box,
      border_inc, atfunc);
    if (! res)
      return NULL;
    if (tpoint_type(seq->temptype))
      return (interp == DISCRETE) ? (Temporal *) tsequence_copy(seq) :
          (Temporal *) tsequence_as_tsequenceset(seq);
    else
    {
      TSequence *res1 = tinstant_as_tsequence(res, interp);
      TSequenceSet *result = tsequence_as_tsequenceset(res1);
      pfree(res); pfree(res1);
      return (Temporal *) result;
    }
  }

  /* General case */
  if (interp == DISCRETE)
    return (Temporal *) tgeoseq_disc_restrict_stbox((TSequence *) seq, box,
      border_inc, atfunc);
  else if (interp == STEP)
    return (Temporal *) tgeoseq_step_restrict_stbox((TSequence *) seq, box,
      border_inc, atfunc);
  else /* interp == LINEAR */
    return (Temporal *) tpointseq_linear_restrict_stbox((TSequence *) seq, box,
      border_inc, atfunc);
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo sequence set restricted to (the complement of)
 * a spatiotemporal box
 * @param[in] ss Temporal geo sequence set
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The box has only spatial dimension and the arguments have the same SRID.
 * @note The function only restricts to the spatial dimension, the restriction
 * to the time dimension is made in function #tgeo_restrict_stbox.
 * @csqlfn #Tgeo_at_stbox(), #Tgeo_minus_stbox()
 */
TSequenceSet *
tgeoseqset_restrict_stbox(const TSequenceSet *ss, const STBox *box,
  bool border_inc, bool atfunc)
{
  assert(ss); assert(box); assert(tgeo_type_all(ss->temptype));
  assert(! MEOS_FLAGS_GET_T(box->flags));

  /* Singleton sequence set */
  if (ss->count == 1)
    /* We can safely cast since the composing sequences are continuous */
    return (TSequenceSet *) tgeoseq_restrict_stbox(TSEQUENCESET_SEQ_N(ss, 0), 
      box, border_inc, atfunc);

  /* General case */

  /* Initialize to 0 due to the bounding box test below */
  TSequenceSet **seqsets = palloc0(sizeof(TSequenceSet *) * ss->count);
  int totalseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    /* Bounding box test */
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    STBox box1;
    tspatialseq_set_stbox(seq, &box1);
    if (atfunc && ! overlaps_stbox_stbox(&box1, box))
      continue;
    else
    {
      /* We can safely cast since the composing sequences are continuous */
      seqsets[i] = (TSequenceSet *) tgeoseq_restrict_stbox(seq, box,
        border_inc, atfunc);
      if (seqsets[i])
        totalseqs += seqsets[i]->count;
    }
  }
  /* Assemble the sequences from all the sequence sets */
  TSequenceSet *result = NULL;
  if (totalseqs > 0)
    result = tseqsetarr_to_tseqset(seqsets, ss->count, totalseqs);
  pfree_array((void **) seqsets, ss->count);
  return result;
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo restricted to (the complement of) a
 * spatiotemporal box
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @note A temporal point is restricted in 3D: 2D/3D arguments can be mixed,
 * the Z dimension being considered only if both the temporal point and the box
 * have Z dimension. The body of a temporal geometry is clipped in 2D, so a
 * body having Z dimension is refused
 */
Temporal *
tgeo_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(box, NULL);
  /* At least one of MEOS_FLAGS_GET_X and MEOS_FLAGS_GET_T is true */
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  assert(hasx || hast);
  if (hasx && (! ensure_same_geodetic(temp->flags, box->flags) ||
      ! ensure_same_srid(tspatial_srid(temp), box->srid) ||
      /* Generic 3D geometries cannot be restricted to a spatiotemporal box */
      (tgeo_type(temp->temptype) &&
       ! ensure_has_not_Z(temp->temptype, temp->flags))))
    return NULL;

  /* Short-circuit restriction to only the time dimension */
  if (hast && ! hasx)
    return temporal_restrict_tstzspan(temp, &box->period, atfunc);

  /* For minus with a box having both the X and the T dimensions, the
   * result is the complement of the at restriction: at every instant the
   * value is atomically inside or outside the box, so the at and minus
   * restrictions partition the time domain */
  if (! atfunc && hast)
  {
    Temporal *atres = tgeo_restrict_stbox(temp, box, border_inc, REST_AT);
    if (! atres)
      return temporal_copy(temp);
    SpanSet *ss = temporal_time(atres);
    Temporal *result = temporal_restrict_tstzspanset(temp, ss, REST_MINUS);
    pfree(atres); pfree(ss);
    return result;
  }

  /* Bounding box test */
  STBox box1;
  tspatial_set_stbox(temp, &box1);
  if (! overlaps_stbox_stbox(&box1, box))
    return atfunc ? NULL : temporal_copy(temp);

  /* Restrict to the time dimension */
  Temporal *temp1;
  const STBox *box2;
  if (hast)
  {
    temp1 = temporal_restrict_tstzspan(temp, &box->period, atfunc);
    if (! temp1)
      return NULL;
    memcpy(&box1, box, sizeof(STBox));
    MEOS_FLAGS_SET_T(box1.flags, false);
    box2 = &box1;
  }
  else
  {
    temp1 = (Temporal *) temp;
    box2 = box;
  }

  assert(temptype_subtype(temp1->subtype));
  Temporal *result;
  switch (temp1->subtype)
  {
    case TINSTANT:
      result = (Temporal *) tgeoinst_restrict_stbox((TInstant *) temp1,
        box2, border_inc, atfunc);
      break;
    case TSEQUENCE:
      result = (Temporal *) tgeoseq_restrict_stbox((TSequence *) temp1,
        box2, border_inc, atfunc);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tgeoseqset_restrict_stbox((TSequenceSet *) temp1,
        box2, border_inc, atfunc);
  }
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to a spatiotemporal box
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tgeo_at_stbox()
 */
Temporal *
tgeo_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tgeo_restrict_stbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to the complement of a
 * spatiotemporal box
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Tgeo_minus_stbox()
 */
Temporal *
tgeo_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tgeo_restrict_stbox(temp, box, border_inc, REST_MINUS);
}

/*****************************************************************************
 * Restriction functions for a spatiotemporal box keeping the original
 * segments WITHOUT clipping
 *****************************************************************************/

/**
 * @brief Return a temporal point sequence with the segments that intersect
 * a spatiotemporal box in ONLY the spatial dimension
 * @param[in] seq temporal point sequence
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @pre The box has X dimension and the arguments have the same SRID.
 * This is verified in #tgeo_restrict_stbox
 * @note This function is ONLY called by the function atGeometry and thus the
 * stbox has NO temporal dimension
 */
static TSequenceSet *
tpointseq_at_stbox_segm(const TSequence *seq, const STBox *box,
  bool border_inc)
{
  assert(seq); assert(box); assert(tpoint_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);
  assert(! MEOS_FLAGS_GET_T(box->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    if (tpointinst_restrict_stbox_iter(TSEQUENCE_INST_N(seq, 0), box,
        border_inc, REST_AT))
      return tsequence_as_tsequenceset(seq);
    return NULL;
  }

  /* General case */
  bool hasz_seq = MEOS_FLAGS_GET_Z(seq->flags);
  bool hasz_box = MEOS_FLAGS_GET_Z(box->flags);
  bool hasz = hasz_seq && hasz_box;
  TSequence **sequences = palloc(sizeof(TSequence *) * (seq->count - 1));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const GSERIALIZED *p1 = DatumGetGserializedP(tinstant_value_p(inst1));
  bool lower_inc = seq->period.lower_inc;
  bool lower_inc_seq = lower_inc, upper_inc;
  int nseqs = 0, ninsts = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    const GSERIALIZED *p2 = DatumGetGserializedP(tinstant_value_p(inst2));
    /* Keep the segment if intersects the bounding box */
    bool inter = false;
    if (geopoint_eq(p1, p2))
    {
      /* Constant segment */
      if (tpointinst_restrict_stbox_iter(inst1, box, border_inc, REST_AT))
        inter = true;
    }
    else
    {
      /* Keep the segment if intersects the bounding box in the spatial
       * dimension */
      if (liangBarskyClip(p1, p2, box, hasz, border_inc, NULL, NULL, NULL,
          NULL, NULL, NULL))
        inter = true;
    }
    if (inter)
    {
      if (ninsts == 0)
        instants[ninsts++] = (TInstant *) inst1;
      instants[ninsts++] = (TInstant *) inst2;
    }
    else if (ninsts > 0)
    {
      sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc_seq,
        upper_inc, LINEAR, NORMALIZE_NO);
      ninsts = 0;
      lower_inc_seq = lower_inc;
    }
    lower_inc = true;
    inst1 = inst2;
    p1 = p2;
  }
  if (ninsts > 0)
    sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc_seq,
      upper_inc, LINEAR, NORMALIZE_NO);
  pfree(instants);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Return a temporal point sequence set with the segments that intersect
 * a spatiotemporal box in BOTH the spatial and the temporal dimension (if any)
 * @param[in] ss Temporal point sequence set
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @pre The box has X dimension and the arguments have the same SRID.
 * This is verified in #tgeo_restrict_stbox
 * @note This function is ONLY called by the function atGeometry and thus the
 * stbox has NO temporal dimension
 */
static TSequenceSet *
tpointseqset_at_stbox_segm(const TSequenceSet *ss, const STBox *box,
  bool border_inc)
{
  assert(ss); assert(box); assert(tpoint_type(ss->temptype));
  TSequenceSet *result = NULL;

  /* Singleton sequence set */
  if (ss->count == 1)
    /* We can safely cast since the composing sequences are continuous */
    return (TSequenceSet *) tpointseq_at_stbox_segm(TSEQUENCESET_SEQ_N(ss, 0),
      box, border_inc);

  /* General case */

  /* Initialize to 0 due to the bounding box test below */
  TSequenceSet **seqsets = palloc0(sizeof(TSequenceSet *) * ss->count);
  int totalseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    /* Bounding box test */
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    STBox box1;
    tspatialseq_set_stbox(seq, &box1);
    if (! overlaps_stbox_stbox(&box1, box))
      continue;
    else
    {
      /* We can safely cast since the composing sequences are continuous */
      seqsets[i] = (TSequenceSet *) tpointseq_at_stbox_segm(seq, box,
        border_inc);
      if (seqsets[i])
        totalseqs += seqsets[i]->count;
    }
  }
  /* Assemble the sequences from all the sequence sets */
  if (totalseqs > 0)
    result = tseqsetarr_to_tseqset(seqsets, ss->count, totalseqs);
  pfree_array((void **) seqsets, ss->count);
  return result;
}

/**
 * @brief Return a temporal point with the segments that intersect
 * a spatiotemporal box in ONLY the spatial dimension
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @note It is possible to mix 2D/3D geometries, the Z dimension is only
 * considered if both the temporal point and the box have Z dimension
 * @pre This function supposes all the checks have been done in the calling
 * function
 * @note This function is ONLY called by the function atGeometry and thus the
 * stbox has NO temporal dimension
 */
static Temporal *
tpoint_at_stbox_segm(const Temporal *temp, const STBox *box, bool border_inc)
{
  assert(temp); assert(box); assert(tpoint_type(temp->temptype));
  /* The following implies that temp->subtype != TINSTANT */
  assert(MEOS_FLAGS_GET_INTERP(temp->flags) == LINEAR);
  /* The stbox has ONLY spatial dimension */
  assert(MEOS_FLAGS_GET_X(box->flags));
  assert(! MEOS_FLAGS_GET_T(box->flags));
  /* Ensure the validity of the arguments */
  if (! ensure_same_geodetic(temp->flags, box->flags) ||
      (MEOS_FLAGS_GET_X(box->flags) &&
        ! ensure_same_srid(tspatial_srid(temp), stbox_srid(box))))
    return NULL;

  /* Parameter test */
  assert(tspatial_srid(temp) == stbox_srid(box));
  assert(MEOS_FLAGS_GET_GEODETIC(temp->flags) ==
    MEOS_FLAGS_GET_GEODETIC(box->flags));

  /* Bounding box test */
  STBox box1;
  tspatial_set_stbox(temp, &box1);
  if (! overlaps_stbox_stbox(&box1, box))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TSEQUENCE)
    return (Temporal *) tpointseq_at_stbox_segm((TSequence *) temp,
        box, border_inc);
  else /* temp->subtype == TSEQUENCESET */
    return (Temporal *) tpointseqset_at_stbox_segm((TSequenceSet *) temp,
      box, border_inc);
}

/*****************************************************************************
 * Restriction functions for geometry
 * N.B. In the current PostGIS version there is no true ST_Intersection
 * function for geography, it is implemented as ST_DWithin with tolerance 0
 *****************************************************************************/

/**
 * @brief Return a temporal point instant restricted to (the complement of) a
 * spatiotemporal box (iterator function)
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
TInstant *
tpointinst_restrict_geom_iter(const TInstant *inst, const GSERIALIZED *gs,
  bool atfunc)
{
  /* Restrict to the XY dimension */
  Datum value = tinstant_value_p(inst);
  if (! geom_intersects2d(DatumGetGserializedP(value), gs))
      /* For temporal point types we return the instants of the input value */
      return atfunc ? NULL : (TInstant *) inst;

  /* The point intersects the geometry */
  return atfunc ? (TInstant *) inst : NULL;
}

/**
 * @brief Return a temporal geo instant restricted to (the complement of) a
 * geometry (iterator function)
 * @param[in] inst Temporal geo instant
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The arguments have the same SRID. This is verified in
 * #tgeo_restrict_geom
 */
TInstant *
tgeoinst_restrict_geom_iter(const TInstant *inst, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(inst); assert(gs); assert(tgeo_type_all(inst->temptype));
  assert(! gserialized_is_empty(gs));
  /* Execute the specialized function for temporal points */
  if (tpoint_type(inst->temptype))
    return tpointinst_restrict_geom_iter(inst, gs, atfunc);

  /* Get the geometry of the instant */
  const GSERIALIZED *gs1 = DatumGetGserializedP(tinstant_value_p(inst));
  /* Restrict to the spatial dimension */
  GSERIALIZED *res = atfunc ? 
    geom_intersection2d(gs1, gs) : geom_difference2d(gs1, gs);
  TInstant *result = NULL;
  if (res)
  {
    if (! gserialized_is_empty(res))
      result = tinstant_make(PointerGetDatum(res), inst->temptype, inst->t);
    pfree(res);
  }
  return result;
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo instant restricted to (the complement of) a
 * geometry
 * @param[in] inst Temporal geo
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
TInstant *
tgeoinst_restrict_geom(const TInstant *inst, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(inst); assert(gs); assert(tgeo_type_all(inst->temptype));
  TInstant *res = tgeoinst_restrict_geom_iter(inst, gs, atfunc);
  if (! res)
    return NULL;
  return tpoint_type(inst->temptype) ? tinstant_copy(res) : res;
}

/**
 * @brief Return a temporal geo discrete sequence restricted to
 * (the complement of) a geometry
 * @param[in] seq Temporal geo
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre Instantaneous sequences have been managed in the calling function
 */
TSequence *
tgeoseq_disc_restrict_geom(const TSequence *seq, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(seq); assert(gs); assert(tgeo_type_all(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(seq->count > 1);

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *res = tgeoinst_restrict_geom_iter(TSEQUENCE_INST_N(seq, i), gs,
      atfunc);
    if (res)
      instants[ninsts++] = res;
  }
  TSequence *result = NULL;
  if (ninsts > 0)
    result = tsequence_make(instants, ninsts, true, true, DISCRETE,
      NORMALIZE_NO);
  /* Clean up and return */
  if (tpoint_type(seq->temptype))
    pfree(instants);
  else
    pfree_array((void **) instants, ninsts);
  return result;
}

/**
 * @brief Return a temporal geo sequence with step interpolation
 * restricted to a geometry
 * @param[in] seq Temporal geo
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre Instantaneous sequences have been managed in the calling function
 * @note The function computes the "at" restriction on all dimensions. Then,
 * for the "minus" restriction, it computes the complement of the "at"
 * restriction with respect to the time dimension.
 */
TSequenceSet *
tgeoseq_step_restrict_geom(const TSequence *seq, const GSERIALIZED *gs,
   bool atfunc)
{
  assert(seq); assert(gs); assert(tgeo_type_all(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);
  assert(seq->count > 1);

  /* Compute the sequences for "at" restriction */
  bool lower_inc;
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TimestampTz start = DatumGetTimestampTz(seq->period.lower);
  int ninsts = 0, nseqs = 0;
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    TInstant *res = tgeoinst_restrict_geom_iter(inst, gs, REST_AT);
    if (res)
      instants[ninsts++] = res;
    else
    {
      if (ninsts > 0)
      {
        /* Continue the last instant of the sequence until the time of inst2 */
        Datum value = tinstant_value_p(instants[ninsts - 1]);
        bool tofree = false;
        bool upper_inc = false;
        instants[ninsts++] = tinstant_make(value, seq->temptype, inst->t);
        tofree = true;
        lower_inc = (instants[0]->t == start) ? seq->period.lower_inc : true;
        sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc,
          upper_inc, STEP, NORMALIZE_NO);
        if (tofree)
          pfree(instants[ninsts - 1]);
        ninsts = 0;
      }
    }
  }
  /* Add a last sequence with the remaining instants */
  if (ninsts > 0)
  {
    lower_inc = (instants[0]->t == start) ? seq->period.lower_inc : true;
    TimestampTz end = DatumGetTimestampTz(seq->period.upper);
    bool upper_inc = (instants[ninsts - 1]->t == end) ?
      seq->period.upper_inc : false;
    sequences[nseqs++] = tsequence_make(instants, ninsts, lower_inc, upper_inc,
      STEP, NORMALIZE_NO);
  }
  if (tpoint_type(seq->temptype))
    pfree(instants);
  else
    pfree_array((void **) instants, ninsts);

  /* Clean up and return if no sequences have been found with "at" */
  if (nseqs == 0)
  {
    pfree(sequences);
    return atfunc ? NULL : tsequence_as_tsequenceset(seq);
  }

  /* Construct the result for "at" restriction */
  TSequenceSet *result_at = tsequenceset_make_free(sequences, nseqs,
    NORMALIZE_NO);
  /* If "at" restriction, return */
  if (atfunc)
    return result_at;

  /* If "minus" restriction, compute the complement wrt time */
  SpanSet *ss = tsequenceset_time(result_at);
  TSequenceSet *result = tcontseq_restrict_tstzspanset(seq, ss, atfunc);
  pfree(ss); pfree(result_at);
  return result;
}

/*****************************************************************************/

/**
 * @brief Get the periods at which a temporal point sequence with linear
 * interpolation intersects a geometry
 * @param[in] seq Temporal point
 * @param[in] gsinter Intersection of the temporal point and the geometry
 * @param[out] count Number of elements in the resulting array
 * @pre The temporal sequence is simple, that is, non self-intersecting and
 * the intersecting geometry is non empty
 */
Span *
tpointseq_interperiods(const TSequence *seq, const GSERIALIZED *gsinter,
  int *count)
{
  /* The temporal sequence has at least 2 instants since
   * (1) the test for instantaneous full sequence is done in the calling function
   * (2) the simple components of a non self-intersecting sequence have at least
   *     two instants */
  assert(seq->count > 1);
  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  const TInstant *end = TSEQUENCE_INST_N(seq, seq->count - 1);
  Span *result;

  /* If the sequence is stationary the whole sequence intersects with the
   * geometry since gsinter is not empty */
  if (seq->count == 2 &&
    datum_point_eq(tinstant_value_p(start), tinstant_value_p(end)))
  {
    result = palloc(sizeof(Span));
    result[0] = seq->period;
    *count = 1;
    return result;
  }

  /* General case */
  LWGEOM *geom_inter = lwgeom_from_gserialized(gsinter);
  int type = geom_inter->type;
  int ninter;
  LWPOINT *point_inter = NULL; /* make compiler quiet */
  LWLINE *line_inter = NULL; /* make compiler quiet */
  LWCOLLECTION *coll = NULL; /* make compiler quiet */
  if (type == POINTTYPE)
  {
    ninter = 1;
    point_inter = lwgeom_as_lwpoint(geom_inter);
  }
  else if (type == LINETYPE)
  {
    ninter = 1;
    line_inter = lwgeom_as_lwline(geom_inter);
  }
  else
  /* It is a collection of type MULTIPOINTTYPE, MULTILINETYPE, or
   * COLLECTIONTYPE */
  {
    coll = lwgeom_as_lwcollection(geom_inter);
    ninter = coll->ngeoms;
  }
  Span *periods = palloc(sizeof(Span) * ninter);
  int npers = 0;
  for (int i = 0; i < ninter; i++)
  {
    if (ninter > 1)
    {
      /* Find the i-th intersection */
      LWGEOM *subgeom = coll->geoms[i];
      if (subgeom->type == POINTTYPE)
        point_inter = lwgeom_as_lwpoint(subgeom);
      else /* type == LINETYPE */
        line_inter = lwgeom_as_lwline(subgeom);
      type = subgeom->type;
    }
    bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
    TimestampTz t1, t2;
    GSERIALIZED *gspoint;
    /* Each intersection is either a point or a linestring */
    if (type == POINTTYPE)
    {
      gspoint = geo_serialize((LWGEOM *) point_inter);
      if (geodetic)
        tpointseq_timestamp_at_value_2d(seq, gspoint, &t1);
      else
        tpointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &t1);
      pfree(gspoint);
      /* If the intersection is not at an exclusive bound */
      if ((seq->period.lower_inc || t1 > start->t) &&
          (seq->period.upper_inc || t1 < end->t))
        span_set(t1, t1, true, true, T_TIMESTAMPTZ, T_TSTZSPAN,
          &periods[npers++]);
    }
    else
    {
      /* Get the fraction of the start point of the intersecting line */
      LWPOINT *point = lwline_get_lwpoint(line_inter, 0);
      gspoint = geo_serialize((LWGEOM *) point);
      lwpoint_free(point);
      if (geodetic)
        tpointseq_timestamp_at_value_2d(seq, gspoint, &t1);
      else
        tpointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &t1);
      pfree(gspoint);
      /* Get the fraction of the end point of the intersecting line */
      point = lwline_get_lwpoint(line_inter, line_inter->points->npoints - 1);
      gspoint = geo_serialize((LWGEOM *) point);
      lwpoint_free(point);
      if (geodetic)
        tpointseq_timestamp_at_value_2d(seq, gspoint, &t2);
      else
        tpointseq_timestamp_at_value(seq, PointerGetDatum(gspoint), &t2);
      pfree(gspoint);
      /* If t1 == t2 and the intersection is not at an exclusive bound */
      if (t1 == t2)
      {
        if ((seq->period.lower_inc || t1 > start->t) &&
            (seq->period.upper_inc || t1 < end->t))
          span_set(t1, t1, true, true, T_TIMESTAMPTZ, T_TSTZSPAN,
            &periods[npers++]);
      }
      else
      {
        TimestampTz lower1 = Min(t1, t2);
        TimestampTz upper1 = Max(t1, t2);
        bool lower_inc1 = (lower1 == start->t) ? seq->period.lower_inc : true;
        bool upper_inc1 = (upper1 == end->t) ? seq->period.upper_inc : true;
        span_set(lower1, upper1, lower_inc1, upper_inc1, T_TIMESTAMPTZ,
          T_TSTZSPAN, &periods[npers++]);
      }
    }
  }
  lwgeom_free(geom_inter);

  if (npers == 0)
  {
    *count = npers;
    pfree(periods);
    return NULL;
  }
  if (npers == 1)
  {
    *count = npers;
    return periods;
  }

  int newcount;
  result = spanarr_normalize(periods, npers, ORDER, &newcount);
  *count = newcount;
  pfree(periods);
  return result;
}

/**
 * @brief Return the parameters at which a great-circle path enters and leaves
 * a polygon read on the sphere
 * @details An edge of a geography joins its two vertices along the great
 * circle through them, so it lies in the plane through the centre of the
 * sphere whose normal is the cross product of those vertices as unit vectors,
 * and the path meets the edge only where it meets that plane. Every such
 * parameter cuts the path, and a piece between two consecutive cuts lies
 * wholly inside the polygon or wholly outside it, which the position halfway
 * along it states. A parameter where the path crosses the plane of an edge
 * away from the edge itself only splits a piece in two, and both halves
 * answer the same, so the extent of an edge needs no test of its own.
 * @param[in] arc Great-circle path
 * @param[in] poly Polygon whose rings are read as geodetic
 * @param[out] tin,tout Parameters at which the path enters and leaves
 * @param[in] maxout Maximum number of spans to state
 * @return The number of spans stated
 */
static int
dggs_arc_geog_poly_spans(const DggsArc *arc, const LWPOLY *poly, double *tin,
  double *tout, int maxout)
{
  assert(arc); assert(poly); assert(tin); assert(tout);
  if (maxout < 1)
    return 0;
  /* The path is cut at its ends and wherever it crosses the plane of an edge */
  uint32_t npoints = 0;
  for (uint32_t r = 0; r < poly->nrings; r++)
    npoints += poly->rings[r]->npoints;
  double *cuts = palloc(sizeof(double) * (size_t) (2 * npoints + 2));
  int ncuts = 0;
  cuts[ncuts++] = 0.0;
  cuts[ncuts++] = 1.0;
  for (uint32_t r = 0; r < poly->nrings; r++)
  {
    const POINTARRAY *pa = poly->rings[r];
    for (uint32_t i = 0; i + 1 < pa->npoints; i++)
    {
      const POINT2D *q1 = getPoint2d_cp(pa, i);
      const POINT2D *q2 = getPoint2d_cp(pa, i + 1);
      GEOGRAPHIC_POINT g1, g2;
      POINT3D v1, v2;
      geographic_point_init(q1->x, q1->y, &g1);
      geographic_point_init(q2->x, q2->y, &g2);
      geog2cart(&g1, &v1);
      geog2cart(&g2, &v2);
      /* The unit normal of the plane the edge lies in */
      double m[3];
      m[0] = v1.y * v2.z - v1.z * v2.y;
      m[1] = v1.z * v2.x - v1.x * v2.z;
      m[2] = v1.x * v2.y - v1.y * v2.x;
      double norm = sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
      /* Two vertices at one position, or at opposite ends of a diameter,
       * state no single plane and therefore no edge to cross */
      if (norm <= MEOS_EPSILON)
        continue;
      m[0] /= norm; m[1] /= norm; m[2] /= norm;
      double params[2];
      int count = dggs_arc_plane_params(arc, m, 0.0, params);
      for (int k = 0; k < count; k++)
        cuts[ncuts++] = params[k];
    }
  }
  /* In ascending order, which is the order the path passes them */
  for (int i = 1; i < ncuts; i++)
  {
    double v = cuts[i];
    int j = i - 1;
    while (j >= 0 && cuts[j] > v)
    {
      cuts[j + 1] = cuts[j]; j--;
    }
    cuts[j + 1] = v;
  }
  /* A piece halfway inside the polygon is inside it, and a piece following one
   * that is inside extends its span */
  int nspans = 0;
  for (int i = 0; i + 1 < ncuts; i++)
  {
    if (cuts[i + 1] <= cuts[i])
      continue;
    double lon, lat;
    if (! dggs_arc_point(arc, (cuts[i] + cuts[i + 1]) / 2.0, &lon, &lat))
      continue;
    POINT2D pt;
    pt.x = lon * 180.0 / M_PI;
    pt.y = lat * 180.0 / M_PI;
    if (! lwpoly_covers_point2d(poly, &pt))
      continue;
    if (nspans > 0 && tout[nspans - 1] == cuts[i])
      tout[nspans - 1] = cuts[i + 1];
    else if (nspans < maxout)
    {
      tin[nspans] = cuts[i];
      tout[nspans++] = cuts[i + 1];
    }
    else
      break;
  }
  pfree(cuts);
  return nspans;
}

/**
 * @brief Return whether a geography holds a position
 * @param[in] polys Polygons the geography states
 * @param[in] npolys Number of polygons
 * @param[in] lon,lat Position, in degrees
 */
static bool
geog_polys_cover_lonlat(LWPOLY **polys, int npolys, double lon, double lat)
{
  POINT2D pt;
  pt.x = lon; pt.y = lat;
  for (int i = 0; i < npolys; i++)
    if (polys[i] && lwpoly_covers_point2d(polys[i], &pt))
      return true;
  return false;
}

/**
 * @brief Return the polygons a geography states
 * @param[in] lwgeom Geography
 * @param[out] count Number of polygons
 */
static LWPOLY **
geog_polys(LWGEOM *lwgeom, int *count)
{
  LWPOLY **result;
  if (lwgeom->type == MULTIPOLYGONTYPE)
  {
    LWCOLLECTION *coll = lwgeom_as_lwcollection(lwgeom);
    *count = (int) coll->ngeoms;
    result = palloc(sizeof(LWPOLY *) * (size_t) (*count));
    for (int i = 0; i < *count; i++)
      result[i] = lwgeom_as_lwpoly(coll->geoms[i]);
  }
  else
  {
    *count = 1;
    result = palloc(sizeof(LWPOLY *));
    result[0] = lwgeom_as_lwpoly(lwgeom);
  }
  return result;
}

/**
 * @brief Return the periods at which a temporal geodetic point sequence holds
 * a geography
 * @details A geodetic point travels the great circle between two positions and
 * a geography joins its own vertices the same way, so the periods the trip
 * holds the region are the ones between the parameters at which its path
 * crosses the edges, which #dggs_arc_geog_poly_spans() states. A segment of a
 * sequence that does not interpolate holds the position of its first instant
 * throughout, so it holds the region for its whole extent or not at all.
 * @param[in] seq Temporal point sequence
 * @param[in] polys,npolys Polygons the geography states
 * @param[in,out] spans Spans the sequence states, appended to
 * @param[in] nspans Number of spans already stated
 * @pre The sequence is geodetic and the arguments have the same SRID
 */
static int
tpointseq_at_geog_spans(const TSequence *seq, LWPOLY **polys, int npolys,
  Span *spans, int nspans)
{
  int32_t srid = tspatial_srid((Temporal *) seq);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  /* The instants of a sequence with discrete interpolation state positions of
   * their own and no path between them, so each one answers for itself */
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags) || seq->count == 1)
  {
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const POINT2D *q = GSERIALIZED_POINT2D_P(
        DatumGetGserializedP(tinstant_value_p(inst)));
      if (geog_polys_cover_lonlat(polys, npolys, q->x, q->y))
        span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
          true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &spans[nspans++]);
    }
    return nspans;
  }
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const GSERIALIZED *p1 = DatumGetGserializedP(tinstant_value_p(inst1));
    const GSERIALIZED *p2 = DatumGetGserializedP(tinstant_value_p(inst2));
    const POINT2D *q1 = GSERIALIZED_POINT2D_P(p1);
    const POINT2D *q2 = GSERIALIZED_POINT2D_P(p2);
    bool last = (i == seq->count - 1);
    DggsArc arc;
    if (! linear || geopoint_eq(p1, p2) ||
        ! dggs_arc_init(q1->x, q1->y, q2->x, q2->y, &arc))
    {
      /* A segment holding one position is in the region when that position
       * is */
      if (geog_polys_cover_lonlat(polys, npolys, q1->x, q1->y))
        span_set(TimestampTzGetDatum(inst1->t), TimestampTzGetDatum(inst2->t),
          true, last ? seq->period.upper_inc : false, T_TIMESTAMPTZ,
          T_TSTZSPAN, &spans[nspans++]);
      /* The last instant of a sequence that does not interpolate states its
       * own position rather than the one before it */
      if (! linear && last && seq->period.upper_inc &&
          geog_polys_cover_lonlat(polys, npolys, q2->x, q2->y))
        span_set(TimestampTzGetDatum(inst2->t), TimestampTzGetDatum(inst2->t),
          true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &spans[nspans++]);
      inst1 = inst2;
      continue;
    }
    for (int j = 0; j < npolys; j++)
    {
      if (! polys[j])
        continue;
      double tin[16], tout[16];
      int count = dggs_arc_geog_poly_spans(&arc, polys[j], tin, tout, 16);
      for (int k = 0; k < count; k++)
      {
        /* A crossing is dated from the position the path holds there, so two
         * regions meeting at that position state the same instant */
        TimestampTz t1 = tgeogpointsegm_timestamp_at_param(inst1, inst2, &arc,
          tin[k], srid);
        TimestampTz t2 = tgeogpointsegm_timestamp_at_param(inst1, inst2, &arc,
          tout[k], srid);
        bool lower_inc = (t1 == inst1->t && i == 1) ?
          seq->period.lower_inc : true;
        bool upper_inc = (t2 == inst2->t) ?
          (last ? seq->period.upper_inc : false) : true;
        if (t1 == t2)
        {
          if (! lower_inc || ! upper_inc)
            continue;        /* the path holds the region for no time at all */
          upper_inc = true;
        }
        span_set(TimestampTzGetDatum(t1), TimestampTzGetDatum(t2), lower_inc,
          upper_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &spans[nspans++]);
      }
    }
    inst1 = inst2;
  }
  return nspans;
}

/**
 * @brief Return a temporal geodetic point restricted to (the complement of) a
 * geography
 * @details The periods the value holds the region state the restriction whole:
 * the `at` restriction keeps them and the `minus` restriction keeps the rest,
 * so the two partition the time domain by construction. The value returns the
 * dimensions the path does not state, exactly as the restriction to a box
 * does.
 * @param[in] temp Temporal geodetic point
 * @param[in] gs Geography
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @pre The arguments have the same SRID and the geography is not empty
 */
static Temporal *
tpoint_geog_restrict_geom(const Temporal *temp, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(temp); assert(gs); assert(MEOS_FLAGS_GET_GEODETIC(temp->flags));

  /* A region on the sphere is bounded by great-circle edges, which only a
   * polygonal geography states */
  uint32_t gstype = gserialized_get_type(gs);
  if (gstype != POLYGONTYPE && gstype != MULTIPOLYGONTYPE)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Restriction of a temporal geodetic point to a geography of type %s is "
      "not implemented", geo_typename((int) gstype));
    return NULL;
  }

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return atfunc ? NULL : temporal_copy(temp);

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  int npolys;
  LWPOLY **polys = geog_polys(lwgeom, &npolys);

  /* The periods the value holds the region, which the segments state one by
   * one and a span set states as a whole */
  int maxspans = (temporal_num_instants(temp) + 1) * 4 * npolys;
  Span *spans = palloc(sizeof(Span) * (size_t) maxspans);
  int nspans = 0;
  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    const POINT2D *q = GSERIALIZED_POINT2D_P(
      DatumGetGserializedP(tinstant_value_p(inst)));
    if (geog_polys_cover_lonlat(polys, npolys, q->x, q->y))
      span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
        true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &spans[nspans++]);
  }
  else if (temp->subtype == TSEQUENCE)
    nspans = tpointseq_at_geog_spans((const TSequence *) temp, polys, npolys,
      spans, nspans);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    for (int i = 0; i < ss->count; i++)
      nspans = tpointseq_at_geog_spans(TSEQUENCESET_SEQ_N(ss, i), polys,
        npolys, spans, nspans);
  }
  pfree(polys);
  lwgeom_free(lwgeom);

  if (nspans == 0)
  {
    pfree(spans);
    return atfunc ? NULL : temporal_copy(temp);
  }
  SpanSet *ss = spanset_make_free(spans, nspans, NORMALIZE, ORDER);
  Temporal *result = temporal_restrict_tstzspanset(temp, ss,
    atfunc ? REST_AT : REST_MINUS);
  pfree(ss);
  return result;
}

/**
 * @brief Return a temporal point sequence with linear interpolation
 * restricted to a geometry
 * @details The computation is based on the PostGIS function @p ST_Intersection
 * which delegates the computation to GEOS. The geometry must be in 2D.
 * When computing the intersection the Z values of the temporal point must
 * be dropped since the Z values "are copied, averaged or interpolated"
 * as stated in https://postgis.net/docs/ST_Intersection.html
 * After this computation, the Z values are recovered by restricting the
 * original sequence to the time span of the 2D result.
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
static TSequenceSet *
tpointseq_linear_at_geom(const TSequence *seq, const GSERIALIZED *gs)
{
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags)); assert(seq->count > 1);

  /* Bounding box test */
  STBox box1, box2;
  tspatialseq_set_stbox(seq, &box1);
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return NULL;

  /* Fast path for (multi)polygon clip: route through Clipper2's open-path
   * intersection. Returns the inside-polygon time spans directly, which we
   * feed back through tcontseq_restrict_tstzspanset to inherit Z and other
   * dimensions from the original sequence (matching the GEOS path
   * structurally). Falls through to the GEOS path on Clipper2 failure. */
  uint32_t gs_type = gserialized_get_type(gs);
  if (gs_type == POLYGONTYPE || gs_type == MULTIPOLYGONTYPE)
  {
    int nperiods = 0;
    Span *periods = clipper2_traj_poly_periods(seq, gs, &nperiods);
    if (nperiods == 0)
      return NULL;
    SpanSet *ss = spanset_make_free(periods, nperiods, NORMALIZE, ORDER);
    TSequenceSet *result = tcontseq_restrict_tstzspanset(seq, ss, REST_AT);
    pfree(ss);
    return result;
  }

  /* Convert the point to 2D before computing the restriction to geometry */
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  TSequence *seq2d = hasz ?
    (TSequence *) tpoint_force2d((Temporal *) seq) : (TSequence *) seq;

  /* Split the temporal point in an array of non self-intersecting fragments
   * to be able to recover the time dimension after obtaining the spatial
   * intersection */
  int nsimple;
  TSequence **simpleseqs = tpointseq_make_simple(seq2d, &nsimple);
  Span *allperiods = NULL; /* make compiler quiet */
  int totalpers = 0;
  GSERIALIZED *traj, *inter;

  if (nsimple == 1)
  {
    /* Particular case when the input sequence is simple */
    pfree_array((void **) simpleseqs, nsimple);
    traj = tpointseq_linear_trajectory(seq2d, UNARY_UNION_NO);
    inter = geom_intersection2d(traj, gs);
    if (! gserialized_is_empty(inter))
      allperiods = tpointseq_interperiods(seq2d, inter, &totalpers);
    pfree(inter); pfree(traj);
    if (totalpers == 0)
    {
      if (hasz)
        pfree(seq2d);
      return NULL;
    }
  }
  else
  {
    /* General case */
    if (hasz)
      pfree(seq2d);
    Span **periods = palloc(sizeof(Span *) * nsimple);
    int *npers = palloc0(sizeof(int) * nsimple);
    /* Loop for every simple fragment of the sequence */
    for (int i = 0; i < nsimple; i++)
    {
      traj = tpointseq_linear_trajectory(simpleseqs[i], UNARY_UNION_NO);
      inter = geom_intersection2d(traj, gs);
      if (! gserialized_is_empty(inter))
      {
        periods[i] = tpointseq_interperiods(simpleseqs[i], inter, &npers[i]);
        totalpers += npers[i];
      }
      pfree(inter); pfree(traj);
    }
    pfree_array((void **) simpleseqs, nsimple);
    if (totalpers == 0)
    {
      pfree(periods); pfree(npers);
      return NULL;
    }

    /* Assemble the periods into a single array */
    allperiods = palloc(sizeof(Span) * totalpers);
    int k = 0;
    for (int i = 0; i < nsimple; i++)
    {
      for (int j = 0; j < npers[i]; j++)
        allperiods[k++] = periods[i][j];
      if (npers[i] != 0)
        pfree(periods[i]);
    }
    pfree(periods); pfree(npers);
    /* It is necessary to sort the periods */
    spanarr_sort(allperiods, totalpers);
  }
  /* Compute the periodset */
  assert(totalpers > 0);
  SpanSet *ss = spanset_make_free(allperiods, totalpers, NORMALIZE, ORDER);
  /* Recover the Z values from the original sequence */
  TSequenceSet *result = tcontseq_restrict_tstzspanset(seq, ss, REST_AT);
  pfree(ss);
  return result;
}

/**
 * @brief Return a temporal point sequence with linear interpolation
 * restricted to (the complement of) a geometry
 * @details The function first filters the temporal point wrt the time
 * dimension to reduce the number of instants before computing the restriction
 * to the geometry, which is an expensive operation. Notice that we need to
 * filter wrt the Z dimension after that since while doing this, the subtype of
 * the temporal point may change from a sequence to a sequence set.
 * @param[in] seq Temporal point
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @note The function computes the "at" restriction on all dimensions. Then,
 * for the "minus" restriction, it computes the complement of the "at"
 * restriction with respect to the time dimension.
 * @pre Instantaneous sequences have been managed in the calling function
 */
TSequenceSet *
tpointseq_linear_restrict_geom(const TSequence *seq, const GSERIALIZED *gs,
  bool atfunc)
{
  VALIDATE_TPOINT(seq, NULL); VALIDATE_NOT_NULL(gs, NULL); 
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  assert(seq->count > 1);

  /* Compute atGeometry for the sequence */
  TSequenceSet *result_at = tpointseq_linear_at_geom(seq, gs);

  /* If "at" restriction, return */
  if (atfunc)
    return result_at;

  /* If "minus" restriction, compute the complement wrt time */
  if (! result_at)
    return tsequence_as_tsequenceset(seq);

  SpanSet *ss = tsequenceset_time(result_at);
  TSequenceSet *result = tcontseq_restrict_tstzspanset(seq, ss, atfunc);
  pfree(ss); pfree(result_at);
  return result;
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo sequence restricted to (the complement of) a
 * geometry
 * @param[in] seq Temporal geo
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
tgeoseq_restrict_geom(const TSequence *seq, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(seq); assert(gs); assert(tgeo_type_all(seq->temptype));
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *res = tgeoinst_restrict_geom_iter(TSEQUENCE_INST_N(seq, 0), gs,
      atfunc);
    if (! res)
      return NULL;
    if (tpoint_type(seq->temptype))
      return (interp == DISCRETE) ? (Temporal *) tsequence_copy(seq) :
          (Temporal *) tsequence_as_tsequenceset(seq);
    else
    {
      TSequence *res1 = tinstant_as_tsequence(res, interp);
      TSequenceSet *result = tsequence_as_tsequenceset(res1);
      pfree(res); pfree(res1);
      return (Temporal *) result;
    }
  }

  /* General case */
  if (interp == DISCRETE)
    return (Temporal *) tgeoseq_disc_restrict_geom((TSequence *) seq, gs,
      atfunc);
  else if (interp == STEP)
    return (Temporal *) tgeoseq_step_restrict_geom((TSequence *) seq, gs,
      atfunc);
  else /* interp == LINEAR */
    return (Temporal *) tpointseq_linear_restrict_geom((TSequence *) seq, gs,
      atfunc);
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo sequence set restricted to (the complement
 * of) a geometry
 * @param[in] ss Temporal geo
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
TSequenceSet *
tgeoseqset_restrict_geom(const TSequenceSet *ss, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(ss); assert(gs); assert(tgeo_type_all(ss->temptype));

  /* Singleton sequence set */
  if (ss->count == 1)
    /* We can safely cast since the composing sequences are continuous */
    return (TSequenceSet *) tgeoseq_restrict_geom(TSEQUENCESET_SEQ_N(ss, 0), 
      gs, atfunc);

  /* General case */
  STBox box2;
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);

  /* Initialize to 0 due to the bounding box test below */
  TSequenceSet **seqsets = palloc0(sizeof(TSequenceSet *) * ss->count);
  int totalseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    /* Bounding box test */
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    STBox box1;
    tspatialseq_set_stbox(seq, &box1);
    if (atfunc && ! overlaps_stbox_stbox(&box1, &box2))
      continue;
    else
    {
      /* We can safely cast since the composing sequences are continuous */
      seqsets[i] = (TSequenceSet *) tgeoseq_restrict_geom(seq, gs, atfunc);
      if (seqsets[i])
        totalseqs += seqsets[i]->count;
    }
  }
  TSequenceSet *result = NULL;
  /* Assemble the sequences from all the sequence sets */
  if (totalseqs > 0)
    result = tseqsetarr_to_tseqset(seqsets, ss->count, totalseqs);
  pfree_array((void **) seqsets, ss->count);
  return result;
}

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo restricted to (the complement of) a geometry
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
tgeo_restrict_geom(const Temporal *temp, const GSERIALIZED *gs,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL); 
  /* Ensure the validity of the arguments */
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_has_not_Z_geo(gs) ||
      /* Generic 3D geometries cannot be restricted to a geometry */
      (tgeo_type(temp->temptype) &&
       ! ensure_has_not_Z(temp->temptype, temp->flags)))
    return NULL;

  /* Empty geometry */
  if (gserialized_is_empty(gs))
    return atfunc ? NULL : temporal_copy(temp);

  /* A geodetic point travels the great circle between two positions and a
   * geography is bounded by great-circle edges, neither of which a clip of
   * straight lines in longitude and latitude follows */
  if (MEOS_FLAGS_GET_GEODETIC(temp->flags))
  {
    if (tpoint_type(temp->temptype))
      return tpoint_geog_restrict_geom(temp, gs, atfunc);
    /* The body of a temporal geography covers a region of the sphere bounded
     * by great-circle edges of its own, and the region two such bodies share
     * is no path that a parameter dates */
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Restriction of a %s to a geography is not implemented",
      meostype_name(temp->temptype));
    return NULL;
  }

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return atfunc ? NULL : temporal_copy(temp);

  /* Native fast path for a temporal geometry point with linear interpolation
   * over a geometry the clip engine supports, avoiding the GEOS call done by
   * ST_Intersection. Instantaneous values, step/discrete interpolation,
   * geodetic points, non-point temporal geos, and geometries with types the
   * clip does not handle fall through to the general path below. */
  if (temp->temptype == T_TGEOMPOINT && temp->subtype != TINSTANT &&
      ! MEOS_FLAGS_GET_GEODETIC(temp->flags) &&
      MEOS_FLAGS_GET_INTERP(temp->flags) == LINEAR)
  {
    LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
    bool supported = geom_meos_coverage(lwgeom) == 1;
    lwgeom_free(lwgeom);
    if (supported)
      return tpoint_linear_restrict_geom(temp, gs, atfunc);
  }

  /* Restrict to atStbox prior to do atGeom to improve efficiency */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  Temporal *temp1;
  if (interp == LINEAR && atfunc)
  {
    temp1 = tpoint_at_stbox_segm(temp, &box2, BORDER_INC);
    /* This is not redundant with the bounding box check above */
    if (! temp1)
      return NULL;
  }
  else
    temp1 = (Temporal *) temp;

  Temporal *result;
  assert(temptype_subtype(temp1->subtype));
  switch (temp1->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeoinst_restrict_geom((TInstant *) temp1, gs,
        atfunc);
      break;
    case TSEQUENCE:
      result = tgeoseq_restrict_geom((TSequence *) temp1, gs, atfunc);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tgeoseqset_restrict_geom((TSequenceSet *) temp1,
         gs, atfunc);
  }
  if (interp == LINEAR && atfunc)
    pfree(temp1);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal point restricted to a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Tgeo_at_geom()
 * @note This function has a last parameter for the Z dimension which is not
 * available for temporal geometries
 */
Temporal *
tpoint_at_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return tgeo_restrict_geom(temp, gs, REST_AT);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to a geometry
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Tgeo_at_geom()
 */
Temporal *
tgeo_at_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return tgeo_restrict_geom(temp, gs, REST_AT);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal point restricted to the complement of a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Tgeo_minus_geom()
 * @note This function has a last parameter for the Z dimension which is not
 * available for temporal geometries
 */
Temporal *
tpoint_minus_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return tgeo_restrict_geom(temp, gs, REST_MINUS);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to the complement of a geometry
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Tgeo_minus_geom()
 */
Temporal *
tgeo_minus_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return tgeo_restrict_geom(temp, gs, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_restrict
 * @brief Return a temporal geo restricted to (the complement of) an elevation
 * span
 * @param[in] temp Temporal geo
 * @param[in] s Elevation span
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
tgeo_restrict_elevation(const Temporal *temp, const Span *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(s, NULL); 
  /* Ensure the validity of the arguments */
  if (! ensure_has_Z(temp->temptype, temp->flags))
    return NULL;

  /* Bounding box test */
  STBox box;
  tspatial_set_stbox(temp, &box);
  /* Non-empty geometries have a bounding box */
  Span s1;
  span_set(Float8GetDatum(box.zmin), Float8GetDatum(box.zmax), true, true,
    T_FLOAT8, T_FLOATSPAN, &s1);
  if (! overlaps_span_span(&s1, s))
    return atfunc ? NULL : temporal_copy(temp);

  /* Get the Z coordinate as a temporal float, project the temporal float to
     the span, and project the temporal geometry */
  Temporal *temp1 = tpoint_get_coord(temp, 2);
  Temporal *temp2 = tnumber_restrict_span(temp1, s, atfunc);
  Temporal *result = NULL;
  if (temp2)
  {
    SpanSet *ss = temporal_time(temp2);
    result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    pfree(ss); pfree(temp2);
  }
  /* Clean out and return */
  pfree(temp1);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal point restricted to an elevation span
 * @param[in] temp Temporal point
 * @param[in] s Elevation span
 * @csqlfn #Tgeo_at_elevation()
 */
Temporal *
tpoint_at_elevation(const Temporal *temp, const Span *s)
{
  return tgeo_restrict_elevation(temp, s, REST_AT);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to an elevation span
 * @param[in] temp Temporal geo
 * @param[in] s Elevation span
 * @csqlfn #Tgeo_at_elevation()
 */
Temporal *
tgeo_at_elevation(const Temporal *temp, const Span *s)
{
  return tgeo_restrict_elevation(temp, s, REST_AT);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal point restricted to the complement of a geometry
 * @param[in] temp Temporal point
 * @param[in] s Elevation span
 * @csqlfn #Tgeo_minus_elevation()
 * @note This function has a last parameter for the Z dimension which is not
 * available for temporal geometries
 */
Temporal *
tpoint_minus_elevation(const Temporal *temp, const Span *s)
{
  return tgeo_restrict_elevation(temp, s, REST_MINUS);
}

/**
 * @ingroup meos_geo_restrict
 * @brief Return a temporal geo restricted to the complement of a geometry
 * @param[in] temp Temporal geo
 * @param[in] s Elevation span
 * @csqlfn #Tgeo_minus_elevation()
 */
Temporal *
tgeo_minus_elevation(const Temporal *temp, const Span *s)
{
  return tgeo_restrict_elevation(temp, s, REST_MINUS);
}

/*****************************************************************************/

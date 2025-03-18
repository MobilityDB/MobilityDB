/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Spatial functons for temporal circular buffers
 */

// #include "cbuffer/tcbuffer_spatialfuncs.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "general/tsequence.h"
#include "geo/pgis_types.h"
#include "geo/tgeo_spatialfuncs.h"
#include "cbuffer/cbuffer.h"

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * @brief Return true if the geometry/geography is a circle
 */
bool
circle_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) != CURVEPOLYTYPE)
    return false;
  LWGEOM *geo = lwgeom_from_gserialized(gs);
  if (lwgeom_count_rings(geo) != 1)
  {
    pfree(geo); 
    return false;
  }
  LWCURVEPOLY *circle = (LWCURVEPOLY *) geo;
  LWCIRCSTRING *ring = (LWCIRCSTRING *) circle->rings[0];
  if (ring->points->npoints != 3 || ! ptarray_is_closed(ring->points))
  {
    pfree(geo); 
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the geometry/geography is a circle
 */
bool
ensure_circle_type(const GSERIALIZED *gs)
{
  if (! circle_type(gs))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only circle polygons accepted");
    return false;
  }
  return true;
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a circular buffer
 */
bool
ensure_valid_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  if (ensure_not_null((void *) temp) && ensure_not_null((void *) cbuf) &&
      ensure_temporal_isof_type(temp, T_TCBUFFER) &&
      ensure_same_srid(tspatial_srid(temp), cbuffer_srid(cbuf)))
    return true;
  return false;
}

/**
 * @brief Ensure the validity of two temporal circular buffers
 */
bool
ensure_valid_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  if (ensure_not_null((void *) temp1) && ensure_not_null((void *) temp2) &&
      ensure_temporal_isof_type(temp1, T_TCBUFFER) &&
      ensure_temporal_isof_type(temp2, T_TCBUFFER) &&
      ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return true;
  return false;
}

/**
 * @brief Ensure the validity of a spatiotemporal box and a geometry
 */
bool
ensure_valid_stbox_cbuffer(const STBox *box, const Cbuffer *cbuf)
{
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) cbuf) ||
      ! ensure_has_X(T_STBOX, box->flags) || 
      ! ensure_same_srid(box->srid, cbuffer_srid(cbuf)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a geometry
 * @note The geometry can be empty since some functions such atGeometry or
 * minusGeometry return different result on empty geometries.
 */
bool
ensure_valid_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_temporal_isof_type(temp, T_TCBUFFER) ||
      ! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return false;
  return true;
}

extern LWCIRCSTRING *lwcircstring_from_lwpointarray(int32_t srid, uint32_t npoints, LWPOINT **points);

/**
 * @brief Return -1, 0, or 1 depending on whether the first point is less than,
 * equal to, or greater than the second one
 */
int
geopoint_cmp(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    if (float8_lt(point1->x, point2->x) || float8_lt(point1->y, point2->y) || 
        float8_lt(point1->z, point2->z))
      return -1;
    if (float8_gt(point1->x, point2->x) || float8_gt(point1->y, point2->y) || 
        float8_gt(point1->z, point2->z))
      return 1;
    return 0;
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    if (float8_lt(point1->x, point2->x) || float8_lt(point1->y, point2->y))
      return -1;
    if (float8_gt(point1->x, point2->x) || float8_gt(point1->y, point2->y))
      return 1;
    return 0;
  }
}

/**
 * @brief Return a circle created from a central point and a radius
 */
GSERIALIZED *
geocircle_make(double x, double y, double radius, int32_t srid)
{
  LWPOINT *points[3];
  /* Shift the X coordinate of the point by +- radius */
  points[0] = points[2] = lwpoint_make2d(srid, x - radius, y);
  points[1] = lwpoint_make2d(srid, x + radius, y);
  /* Construct the circle */
  LWGEOM *ring = lwcircstring_as_lwgeom(
    lwcircstring_from_lwpointarray(srid, 3, points));
  LWCURVEPOLY *poly = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(poly, ring);
  GSERIALIZED *result = geom_serialize((LWGEOM *) poly);
  /* Clean up and return */
  lwpoint_free(points[0]); lwpoint_free(points[1]); lwgeom_free(ring);
  return result;
}

/*****************************************************************************
 * Interpolation functions defining functionality required by tsequence.c
 * that must be implemented by each temporal type
 *****************************************************************************/

/**
 * @brief Return a cbuffer interpolated from a cbuffer segment with respect to 
 * the fraction of its total length
 * @param[in] start,end Circular buffers defining the segment
 * @param[in] ratio Float between 0 and 1 representing the fraction of the
 * total length of the segment where the interpolated buffer must be located
 */
Datum
cbuffersegm_interpolate(Datum start, Datum end, long double ratio)
{
  Cbuffer *cbuf1 = DatumGetCbufferP(start);
  Cbuffer *cbuf2 = DatumGetCbufferP(end);
  Datum d1 = PointerGetDatum(&cbuf1->point);
  Datum d2 = PointerGetDatum(&cbuf2->point);
  GSERIALIZED *point = 
    DatumGetGserializedP(pointsegm_interpolate_point(d1, d2, ratio));
  double radius = cbuf1->radius + 
    (double) ((long double)(cbuf2->radius - cbuf1->radius) * ratio);
  Cbuffer *result = cbuffer_make(point, radius);
  return PointerGetDatum(result);
}

/**
 * @brief Return true if a segment of a temporal network point value intersects
 * a base value at the timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 */
bool
tcbuffersegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  Datum value1 = tinstant_val(inst1);
  Datum value2 = tinstant_val(inst2);
  Cbuffer *cbuf = DatumGetCbufferP(value);
  Cbuffer *cbuf1 = DatumGetCbufferP(value1);
  Cbuffer *cbuf2 = DatumGetCbufferP(value2);
  const GSERIALIZED *gs1 = cbuffer_point(cbuf1);
  const GSERIALIZED *gs2 = cbuffer_point(cbuf2);
  TimestampTz t1, t2;
  bool result1, result2;
  if (! datum_point_eq(PointerGetDatum(gs1), PointerGetDatum(gs2)))
  {
    TInstant *point1 = tcbufferinst_tgeompointinst(inst1);
    TInstant *point2 = tcbufferinst_tgeompointinst(inst2); 
    Datum point = PointerGetDatum(&cbuf->point);
    result1 = tpointsegm_intersection_value(point1, point2, point, &t1);
    pfree(point1); pfree(point2); 
    if (! result1)
      return false;
  }
  else
    result1 = false;
  if (! float8_eq(cbuf1->radius, cbuf2->radius))
  {
    TInstant *radius1 = tcbufferinst_tfloatinst(inst1);
    TInstant *radius2 = tcbufferinst_tfloatinst(inst2);   
    result2 = tfloatsegm_intersection_value(radius1, radius2, 
      Float8GetDatum(cbuf->radius), T_FLOAT8, &t2);
    pfree(radius1); pfree(radius2); 
    if (! result2)
      return false;
  }   
  else
    result2 = false;
  bool result;
  if (result1 && result2 & (t1 == t2))
  {
    *t = t1;
    result = true;
  }
  else if (! result1 && result2)
  {
    *t = t2;
    result = true;
  }
  else /* result1 && ! result2 */
  {
    *t = t1;
    result = true;
  }
  return result;
}

/*****************************************************************************
 * Traversed area 
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbuffer_traversed_area(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_temporal_isof_type(temp, T_TCBUFFER))
    return NULL;

  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return NULL;
}

/*****************************************************************************/

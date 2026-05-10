/*****************************************************************************
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
 * @brief Spatial functions for temporal circular buffers
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
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

/*****************************************************************************
 * Traversed area 
 *****************************************************************************/

/* The following function is not exported in PostGIS */
extern LWCIRCSTRING *lwcircstring_from_lwpointarray(int32_t srid,
  uint32_t npoints, LWPOINT **points);

/**
 * @brief Return a circle created from a central point and a radius
 */
LWGEOM *
lwcircle_make(double x, double y, double radius, int32_t srid)
{
  assert(radius > 0);
  LWPOINT *points[3];
  /* Shift the X coordinate of the point by +- radius */
  points[0] = lwpoint_make2d(srid, x - radius, y);
  points[1] = lwpoint_make2d(srid, x + radius, y);
  points[2] = lwpoint_make2d(srid, x - radius, y);
  /* Construct the circle */
  LWGEOM *ring = lwcircstring_as_lwgeom(
    lwcircstring_from_lwpointarray(srid, 3, points));
  LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(result, ring);
  /* Clean up and return */
  lwpoint_free(points[0]); lwpoint_free(points[1]); lwpoint_free(points[2]);
  /* We cannot lwgeom_free(ring); */
  return lwcurvepoly_as_lwgeom(result);
}

/**
 * @brief Return a circle created from a central point and a radius
 */
GSERIALIZED *
geocircle_make(double x, double y, double radius, int32_t srid)
{
  LWGEOM *res = lwcircle_make(x, y, radius, srid);
  GSERIALIZED *result = geo_serialize(res);
  lwgeom_free(res);
  return result;
}

/**
 * @brief Return a trapezoid created from two circular buffers
 * @details Given two circular buffers, c1 and c2, defined by their centres 
 * (x1,y1) and (x2,y2) and radii r1 and r2, respectively, the function computes
 * the tangents points between the two circular buffers, and the two points on
 * the two circular buffers that are farthest from each other as illustrated
 * next
 * @code
 *              T1a(x,y)               T2a(x,y)
 *                  |                      |
 *                  ^                      ^
 *                 (-)--------------------(-)
 *                (---)                  (---)
 *   A1(x,y) --> (-----)                (-----) <-- A2(x,y)
 *                (---)                  (---)
 *                 (-)--------------------(-)
 *                  ^                      ^
 *                  |                      |
 *              T1b(x,y)               T2b(x,y)
 * @endcode
 * In particular, two common external tangent lines, Ta and Tb, intersect both
 * circles at four points
 *    - T1a(x,y) and T1b(x,y): the points where Ta and Tb touch circle c1.
 *    - T2a(x,y) and T2b(x,y): the points where Ta and Tb touch circle c2.
 * Additionally, each circle has a point that is farthest from the other circle:
 *    - A1(x,y) on circle c1: the point between T1a and T1b that is farthest 
 *      from circle c2.
 *    - A2(x,y) on circle c2: the point between T2a and T2b that is farthest 
 *      from circle c1.
 * This function returns the geometry given by the circular bufferss and the
 * two tangent lines. 
 * 
 * An example of usage of the function is given next.
 * @code
 *  SELECT ST_AsText(traversedArea(tcbuffer '[Cbuffer(Point(1 1),1)@2000-01-01,
 *    Cbuffer(Point(3 2),2)@2000-01-02]'), 3);
 *  -- CURVEPOLYGON(COMPOUNDCURVE(
 *       CIRCULARSTRING(1.4 3.2,4.789 2.894,3 0),(3 0,1 0),
 *       CIRCULARSTRING(1 0,0.106 0.553,0.2 1.6),(0.2 1.6,1.4 3.2)))
 * @endcode
 */
LWGEOM *
trapezoid_make(const Cbuffer *c1, const Cbuffer *c2)
{
  assert(c1); assert(c2); assert(cbuffer_srid(c1) == cbuffer_srid(c2));
  const GSERIALIZED *gs1 = cbuffer_point_p(c1);
  const GSERIALIZED *gs2 = cbuffer_point_p(c2);
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(gs1);
  const POINT2D *p2 = GSERIALIZED_POINT2D_P(gs2);
  int32_t srid = cbuffer_srid(c1);

  /* Compute the Euclidean distance between the centers of the circles */
  double dx = p2->x - p1->x;
  double dy = p2->y - p1->y;
  double d = sqrt(dx * dx + dy * dy);

  /* Compute the angle θ between the line connecting the two centers and the
   * x-axis */
  double theta = atan2(p2->y - p1->y, p2->x - p1->x);

  /* Compute the angle δ between the line connecting the centers and the 
   * external tangent line */
  double delta = acos((c1->radius - c2->radius) / d);

  /* The tangent points on the circles are obtained by rotating the vector that
   * goes from the center of each circle towards the other center by an angle
   * δ with respect to the direction θ */
  double xT1a = p1->x + c1->radius * cos(theta + delta);
  double yT1a = p1->y + c1->radius * sin(theta + delta);
  double xT2a = p2->x + c2->radius * cos(theta + delta);
  double yT2a = p2->y + c2->radius * sin(theta + delta);

  double xT1b = p1->x + c1->radius * cos(theta - delta);
  double yT1b = p1->y + c1->radius * sin(theta - delta);
  double xT2b = p2->x + c2->radius * cos(theta - delta);
  double yT2b = p2->y + c2->radius * sin(theta - delta);

  double cos_theta = cos(theta);
  double sin_theta = sin(theta);

  /* Compute the farthest points on the circles */
  double dx1 = p1->x + c1->radius * cos_theta - p2->x - c2->radius * cos_theta;
  double dy1 = p1->y + c1->radius * sin_theta - p2->y - c2->radius * sin_theta;
  double dist1 = dx1 * dx1 + dy1 * dy1;
  double dx2 = p1->x - c1->radius * cos_theta - p2->x + c2->radius * cos_theta;
  double dy2 = p1->y - c1->radius * sin_theta - p2->y + c2->radius * sin_theta;
  double dist2 = dx2 * dx2 + dy2 * dy2;
  double dx3 = p1->x + c1->radius * cos_theta - p2->x + c2->radius * cos_theta;
  double dy3 = p1->y + c1->radius * sin_theta - p2->y + c2->radius * sin_theta;
  double dist3 = dx3 * dx3 + dy3 * dy3;
  double dx4 = p1->x - c1->radius * cos_theta - p2->x - c2->radius * cos_theta;
  double dy4 = p1->y - c1->radius * sin_theta - p2->y - c2->radius * sin_theta;
  double dist4 = dx4 * dx4 + dy4 * dy4;
  
  /* Determine the farthest points on the circles based on the distances */
  double xA1, yA1, xA2, yA2;
  if (dist1 > dist2 && dist1 > dist3 && dist1 > dist4)
  {
    xA1 = p1->x + c1->radius * cos_theta;
    yA1 = p1->y + c1->radius * sin_theta;
    xA2 = p2->x + c2->radius * cos_theta;
    yA2 = p2->y + c2->radius * sin_theta;
  }
  else if (dist2 > dist1 && dist2 > dist3 && dist2 > dist4)
  {
    xA1 = p1->x - c1->radius * cos_theta;
    yA1 = p1->y - c1->radius * sin_theta;
    xA2 = p2->x - c2->radius * cos_theta;
    yA2 = p2->y - c2->radius * sin_theta;
  }
  else if (dist3 > dist1 && dist3 > dist2 && dist3 > dist4)
  {
    xA1 = p1->x + c1->radius * cos_theta;
    yA1 = p1->y + c1->radius * sin_theta;
    xA2 = p2->x - c2->radius * cos_theta;
    yA2 = p2->y - c2->radius * sin_theta;
  }
  else
  {
    xA1 = p1->x - c1->radius * cos_theta;
    yA1 = p1->y - c1->radius * sin_theta;
    xA2 = p2->x + c2->radius * cos_theta;
    yA2 = p2->y + c2->radius * sin_theta;
  }

  /* Construct the points for the ring components */
  LWPOINT *points1[3];
  points1[0] = lwpoint_make2d(srid, xT2a, yT2a);
  points1[1] = lwpoint_make2d(srid, xA2, yA2);
  points1[2] = lwpoint_make2d(srid, xT2b, yT2b);
  LWPOINT *points2[2];
  points2[0] = lwpoint_make2d(srid, xT2b, yT2b);
  points2[1] = lwpoint_make2d(srid, xT1b, yT1b);
  LWPOINT *points3[3];
  points3[0] = lwpoint_make2d(srid, xT1b, yT1b);
  points3[1] = lwpoint_make2d(srid, xA1, yA1);
  points3[2] = lwpoint_make2d(srid, xT1a, yT1a);
  LWPOINT *points4[2];
  points4[0] = lwpoint_make2d(srid, xT1a, yT1a);
  points4[1] = lwpoint_make2d(srid, xT2a, yT2a);
  /* Construct the ring components */
  LWGEOM *circstr1 = lwcircstring_as_lwgeom(
    lwcircstring_from_lwpointarray(srid, 3, points1));
  LWGEOM *linestr1 = lwline_as_lwgeom(
    lwline_from_ptarray(srid, 2, points2));
  LWGEOM *circstr2 = lwcircstring_as_lwgeom(
    lwcircstring_from_lwpointarray(srid, 3, points3));
  LWGEOM *linestr2 = lwline_as_lwgeom(
    lwline_from_ptarray(srid, 2, points4));
  /* Construct the ring */
  LWCOMPOUND *ring = lwcompound_construct_empty(srid, 0, 0);
  lwcompound_add_lwgeom(ring, circstr1);
  lwcompound_add_lwgeom(ring, linestr1);
  lwcompound_add_lwgeom(ring, circstr2);
  lwcompound_add_lwgeom(ring, linestr2);
  /* Construct the trapezoid */
  LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(result, (LWGEOM *) ring);

  /* Clean up and return */
  lwpoint_free(points1[0]); lwpoint_free(points1[1]); lwpoint_free(points1[2]);
  lwpoint_free(points2[0]); lwpoint_free(points2[1]); lwpoint_free(points3[0]);
  lwpoint_free(points3[1]); lwpoint_free(points3[2]); lwpoint_free(points4[0]);
  lwpoint_free(points4[1]);
  return lwcurvepoly_as_lwgeom(result);
}

/*****************************************************************************/

/**
 * @brief Comparator function for components of the traversed area of a
 * temporal circular buffer, which can be points, circles, or trapezoids
 * @note Function inspired from PostGIS function gserialized_cmp
 */
static int
geoarr_sort_cmp(const GSERIALIZED **l, const GSERIALIZED **r)
{
  return gserialized_cmp((*l), (*r));
}

/**
 * @brief Sort function for trapezoids
 */
static void
geoarr_sort(GSERIALIZED **geoms, int count)
{
  qsort(geoms, (size_t) count, sizeof(GSERIALIZED *),
    (qsort_comparator) &geoarr_sort_cmp);
  return;
}

/**
 * @brief Remove duplicates from an array of components of the traversed area
 * or a temporal circular buffer
 * @pre The array has been sorted before
 * @note Since PostGIS does not provide function `lwgeom_eq`, we use the
 * function `lwgeom_same`
 */
static int
geoarr_remove_duplicates(GSERIALIZED **geoms, int count)
{
  assert(count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (! geo_same(geoms[newcount], geoms[i]))
      geoms[++ newcount] = geoms[i];
  return newcount + 1;
}

/*****************************************************************************/

/**
 * @brief Return the traversed area of a circular buffer
 * @param[in] cb Circular buffer
 */
GSERIALIZED *
cbuffer_traversed_area(const Cbuffer *cb)
{
  assert(cb);
  const GSERIALIZED *gs = cbuffer_point_p(cb);
  const POINT2D *p = GSERIALIZED_POINT2D_P(gs);
  int32_t srid = gserialized_get_srid(gs);
  LWGEOM *lwgeom;
  GSERIALIZED *result;
  /* If radius is 0 construct a point */
  if (cb->radius == 0.0)
  {
    lwgeom = (LWGEOM *) lwpoint_make2d(srid, p->x, p->y);
    result = geo_serialize(lwgeom);
    lwgeom_free(lwgeom);
    return result;
  }
  /* Construct the points defining the circle */
  LWPOINT *points[3];
  /* Shift the X coordinate of the point by +- radius */
  points[0] = points[2] = lwpoint_make2d(srid, p->x - cb->radius, p->y);
  points[1] = lwpoint_make2d(srid, p->x + cb->radius, p->y);
  /* Construct the circle */
  LWGEOM *ring = lwcircstring_as_lwgeom(
    lwcircstring_from_lwpointarray(srid, 3, points));
  LWCURVEPOLY *poly = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(poly, ring);
  lwgeom = lwcurvepoly_as_lwgeom(poly);
  result = geo_serialize(lwgeom);
  lwgeom_free(lwgeom);
  lwpoint_free(points[0]); lwpoint_free(points[1]);
  return result;
}

/**
 * @brief Return the traversed area of a temporal circular buffer with step or
 * discrete interpolation
 * @param[in] instants Array of instants of a temporal circular buffer
 * @param[in] count Number of instants in the array
 * @param[out] result Array of resulting geometries
 * @return Number of elements in the output array
 */
int
cbufferarr_circles(TInstant **instants, int count, GSERIALIZED **result)
{
  assert(instants); assert(count > 1);
  for (int i = 0; i < count; i++)
  {
    const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(instants[i]));
    result[i] = cbuffer_traversed_area(cb);
  }
  return count;
}

/**
 * @ingroup meos_internal_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer instant
 * @param[in] inst Temporal circular buffer
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferinst_traversed_area(const TInstant *inst)
{
  assert(inst);
  return cbuffer_traversed_area(DatumGetCbufferP(tinstant_value_p(inst)));
}

/**
 * @ingroup meos_interal_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence with
 * discrete or step interpolation
 * @param[in] seq Temporal circular buffer
 * @param[out] result Array of resulting geometries
 * @return Number of elements in the output array
 * @csqlfn #Tcbuffer_traversed_area()
 */
int
tcbufferseq_discstep_traversed_area(const TSequence *seq, GSERIALIZED **result)
{
  assert(seq); assert(seq->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != LINEAR);
  const TInstant **instants = tsequence_insts_p(seq);
  int res = cbufferarr_circles((TInstant **) instants, seq->count, result);
  pfree(instants);
  return res;
}

/**
 * @ingroup meos_internal_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer segment with
 * linear interpolation
 * @param[in] inst1,inst2 Temporal instants
 */
GSERIALIZED *
tcbuffersegm_traversed_area(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2); assert(inst1->temptype == T_TCBUFFER);
  assert(inst2->temptype == T_TCBUFFER);
  const Cbuffer *cb1 = DatumGetCbufferP(tinstant_value_p(inst1));
  const Cbuffer *cb2 = DatumGetCbufferP(tinstant_value_p(inst2));
  /* Order the circular buffers to be able to remove duplicates if any */
  const Cbuffer *cb_min, *cb_max;
  if (cbuffer_cmp(cb1, cb2) <= 0)
  {
    cb_min = cb1;
    cb_max = cb2;
  }
  else
  {
    cb_min = cb2;
    cb_max = cb1;
  }
  const GSERIALIZED *gs1 = cbuffer_point_p(cb_min);
  const GSERIALIZED *gs2 = cbuffer_point_p(cb_max);
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(gs1);
  const POINT2D *p2 = GSERIALIZED_POINT2D_P(gs2);
  int32_t srid = gserialized_get_srid(gs1);

  /* If the two points are equal compute the traversed area of the circular
   * buffer with the bigger radius */
  if (p1->x == p2->x && p1->y == p2->y)
  {
    return (cb_min->radius <= cb_max->radius) ?
      cbuffer_traversed_area(cb_min) : cbuffer_traversed_area(cb_max);
  }

  /* If the two radius are equal to 0 construct a line segment */
  LWGEOM *res;
  GSERIALIZED *result;
  if (cb_min->radius == 0.0 && cb_max->radius == 0.0)
  {
    res = (LWGEOM *) lwline_make(PointerGetDatum(gs1), PointerGetDatum(gs2));
    result = geo_serialize(res);
    lwgeom_free(res);
    return result;
  }

  /* Compute the distance between the two centroids */
  double d = sqrt((p2->x - p1->x) * (p2->x - p1->x) +
    (p2->y - p1->y) * (p2->y - p1->y));
  /* If the distance is less than the difference of the two radii,
   * no tangent line exists */
  if (d > fabs(cb_min->radius - cb_max->radius))
    res = trapezoid_make(cb_min, cb_max);
  else
  {
    if (cb_min->radius > cb_max->radius)
      res = lwcircle_make(p1->x, p1->y, cb_min->radius, srid);
    else
      res = lwcircle_make(p2->x, p2->y, cb_max->radius, srid);
  }
  result = geo_serialize(res);
  lwgeom_free(res);
  return result;
}

/**
 * @ingroup meos_internal_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence with
 * linear interpolation (iterator function)
 * @param[in] seq Temporal circular buffer
 * @param[out] result Array of output geometries
 * @csqlfn #Tcbuffer_traversed_area()
 */
int
tcbufferseq_linear_traversed_area(const TSequence *seq, GSERIALIZED **result)
{
  assert(seq); assert(result);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result[0] = tcbufferinst_traversed_area(TSEQUENCE_INST_N(seq, 0));
    return 1;
  }

  /* General case */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    result[i - 1] = tcbuffersegm_traversed_area(inst1, inst2);
    inst1 = inst2;
  }
  return (seq->count > 2) ? seq->count - 1 : 1;
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence
 * @param[in] seq Temporal circular buffer
 * @param[in] unary_union True when the traversed area is a single geometry
 * obtained by applying spatial union to the geometries of the segments
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseq_traversed_area(const TSequence *seq, bool unary_union)
{
  assert(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tcbufferinst_traversed_area(TSEQUENCE_INST_N(seq, 0));

  /* General case */
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * seq->count);
  int count = (MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR) ?
    tcbufferseq_linear_traversed_area(seq, geoms) :
    tcbufferseq_discstep_traversed_area(seq, geoms);

  /* Construct the result */
  GSERIALIZED *result;
  if (count == 1)
  {
    result = geoms[0];
    pfree(geoms);
    return result;
  }
  /* Remove duplicate geometries constructed from the segments */
  geoarr_sort(geoms, count);
  int newcount = geoarr_remove_duplicates(geoms, count);
  GSERIALIZED *res = geo_collect_garray(geoms, newcount);
  if (unary_union)
  {
    result = geom_unary_union(res, -1);
    pfree(res);
  }
  else
    result = res;
  pfree(geoms);
  return result;
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence set
 * with step interpolation
 * @param[in] ss Temporal circular buffer
 * @param[out] result Array of resulting geometries
 * @return Number of elements in the output array
 * @csqlfn #Tcbuffer_traversed_area()
 */
int
tcbufferseqset_step_traversed_area(const TSequenceSet *ss, GSERIALIZED **result)
{
  assert(ss); assert(ss->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(ss->flags) == STEP);
  const TInstant **instants = tsequenceset_insts_p(ss);
  return cbufferarr_circles((TInstant **) instants, ss->count, result);
}

/**
 * @ingroup meos_internal_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence set
 * with linear interpolation
 * @param[in] ss Temporal circular buffer
 * @param[out] result Array of resulting geometries
 * @return Number of elements in the output array
 * @csqlfn #Tcbuffer_traversed_area()
 */
int
tcbufferseqset_linear_traversed_area(const TSequenceSet *ss, GSERIALIZED **result)
{
  assert(ss); assert(ss->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(ss->flags) == LINEAR);
  int ngeoms = 0;
  for (int i = 0; i < ss->count; i++)
    /* Get the traversed area of the sequence */
    ngeoms += tcbufferseq_linear_traversed_area(TSEQUENCESET_SEQ_N(ss, i),
      &result[ngeoms]);
  return ngeoms;
}

/**
 * @ingroup meos_internal_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer
 * @param[in] ss Temporal circular buffer
 * @param[in] unary_union True when the traversed area is a single geometry
 * obtained by applying spatial union to the geometries of the segments
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseqset_traversed_area(const TSequenceSet *ss, bool unary_union)
{
  assert(ss); assert(MEOS_FLAGS_GET_INTERP(ss->flags) == LINEAR);
  
  /* Singleton sequence set */
  if (ss->count == 1)
    return tcbufferseq_traversed_area(TSEQUENCESET_SEQ_N(ss, 0), unary_union);

  /* General case */
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * ss->totalcount);
  int count = (MEOS_FLAGS_GET_INTERP(ss->flags) == LINEAR) ?
    tcbufferseqset_linear_traversed_area(ss, geoms) :
    tcbufferseqset_step_traversed_area(ss, geoms);

  /* Construct the result */
  GSERIALIZED *result;
  if (count == 1)
  {
    result = geoms[0];
    pfree(geoms);
    return result;
  }
  /* Remove duplicate geometries constructed from the segments */
  geoarr_sort(geoms, count);
  int newcount = geoarr_remove_duplicates(geoms, count);
  GSERIALIZED *res = geo_collect_garray(geoms, newcount);
  if (unary_union)
  {
    result = geom_unary_union(res, -1);
    pfree(res);
  }
  else
    result = res;
  pfree(geoms);
  return result;
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] unary_union True when the traversed area is a single geometry
 * obtained by applying spatial union to the geometries of the segments
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbuffer_traversed_area(const Temporal *temp, bool unary_union)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tcbufferinst_traversed_area((TInstant *) temp);
      break;
    case TSEQUENCE:
      return tcbufferseq_traversed_area((TSequence *) temp, unary_union);
      break;
    default: /* TSEQUENCESET */
      return tcbufferseqset_traversed_area((TSequenceSet *) temp, unary_union);
  }
}

/*****************************************************************************/

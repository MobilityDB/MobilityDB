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
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "general/tsequence.h"
#include "geo/postgis_funcs.h"
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

/*****************************************************************************/

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

/*****************************************************************************
 * Traversed area 
 *****************************************************************************/

/**
 * @brief Return a circle created from a central point and a radius
 */
LWGEOM *
lwcircle_make(double x, double y, double radius, int32_t srid)
{
  LWPOINT *points[3];
  /* Shift the X coordinate of the point by +- radius */
  points[0] = points[2] = lwpoint_make2d(srid, x - radius, y);
  points[1] = lwpoint_make2d(srid, x + radius, y);
  /* Construct the circle */
  LWGEOM *ring = lwcircstring_as_lwgeom(
    lwcircstring_from_lwpointarray(srid, 3, points));
  LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(result, ring);
  /* Clean up and return */
  lwpoint_free(points[0]); lwpoint_free(points[1]); 
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
  double d = sqrt(pow(p2->x - p1->x, 2) + pow(p2->y - p1->y, 2));

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
  double dist1 = 
    pow(p1->x + c1->radius * cos_theta - p2->x - c2->radius * cos_theta, 2) + 
    pow(p1->y + c1->radius * sin_theta - p2->y - c2->radius * sin_theta, 2);
  double dist2 = 
    pow(p1->x - c1->radius * cos_theta - p2->x + c2->radius * cos_theta, 2) + 
    pow(p1->y - c1->radius * sin_theta - p2->y + c2->radius * sin_theta, 2);
  double dist3 = 
    pow(p1->x + c1->radius * cos_theta - p2->x + c2->radius * cos_theta, 2) + 
    pow(p1->y + c1->radius * sin_theta - p2->y + c2->radius * sin_theta, 2);
  double dist4 = 
    pow(p1->x - c1->radius * cos_theta - p2->x - c2->radius * cos_theta, 2) + 
    pow(p1->y - c1->radius * sin_theta - p2->y - c2->radius * sin_theta, 2);
  
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
  // for (int i = 0; i < 10; i ++)
    // lwpoint_free(points[i]);
  return lwcurvepoly_as_lwgeom(result);
}

/*****************************************************************************/

/**
 * @brief Return the traversed area of a temporal circular buffer with step or
 * discrete interpolation
 * @param[in] instants Array of instants of a temporal circular buffer
 * @param[in] count Number of instants in the array
 * @param[in] srid SRID
 */
GSERIALIZED *
cbufferarr_circles(const TInstant **instants, int count, int32_t srid)
{
  assert(instants); assert(count > 1);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  for (int i = 0; i < count; i++)
  {
    const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(instants[i]));
    const GSERIALIZED *gs = cbuffer_point_p(cb);
    const POINT2D *p = GSERIALIZED_POINT2D_P(gs);
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
    geoms[i] = lwcurvepoly_as_lwgeom(poly);
  }
  // TODO add the bounding box instead of ask PostGIS to compute it again
  LWGEOM *result = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE, srid,
    NULL, (uint32_t) count, geoms);
  /* We cannot pfree(geoms); */
  return geo_serialize(result);
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer instant
 * @param[in] int Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferinst_trav_area(const TInstant *inst, int32_t srid)
{
  assert(inst);
  const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
  const GSERIALIZED *gs = cbuffer_point_p(cb);
  const POINT2D *p = GSERIALIZED_POINT2D_P(gs);
  LWGEOM *res = lwcircle_make(p->x, p->y, cb->radius, srid);
  return geo_serialize(res);
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence with
 * discrete or step interpolation
 * @param[in] seq Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseq_discstep_trav_area(const TSequence *seq, int32_t srid)
{
  assert(seq); assert(seq->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != LINEAR);
  const TInstant **instants = tsequence_instants_p(seq);
  return cbufferarr_circles(instants, seq->count, srid);
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence with
 * linear interpolation (iterator function)
 * @param[in] seq Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
int
tcbufferseq_linear_trav_area_iter(const TSequence *seq, int32_t srid,
  LWGEOM **result)
{
  assert(seq); assert(seq->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);

  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const Cbuffer *cb1 = DatumGetCbufferP(tinstant_value_p(inst1));
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *cb2 = DatumGetCbufferP(tinstant_value_p(inst2));
    const GSERIALIZED *gs1 = cbuffer_point_p(cb1);
    const POINT2D *p1 = GSERIALIZED_POINT2D_P(gs1);
    const GSERIALIZED *gs2 = cbuffer_point_p(cb2);
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(gs2);
    
    /* Compute the distance between the two centroids */
    double d = sqrt(pow(p2->x - p1->x, 2) + pow(p2->y - p1->y, 2));
    /* If the distance is less than the difference of the two radii, 
     * no tangent line exists */
    if (d <= fabs(cb1->radius - cb2->radius))
    {
      const GSERIALIZED *gs;
      const POINT2D *p;
      if (cb1->radius > cb2->radius)
      {
        gs = cbuffer_point_p(cb1);
        p = GSERIALIZED_POINT2D_P(gs);
        result[i - 1] = lwcircle_make(p->x, p->y, cb1->radius, srid);
      }
      else
      {
        gs = cbuffer_point_p(cb2);
        p = GSERIALIZED_POINT2D_P(gs2);
        result[i - 1] = lwcircle_make(p->x, p->y, cb2->radius, srid);
      }
    }
    else
      result[i - 1] = trapezoid_make(cb1, cb2);

    /* Prepare for next iteration */
    inst2 = inst1;
    cb1 = cb2;
  }
  
  return (seq->count > 2) ? seq->count - 1 : 1;
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence with
 * linear interpolation
 * @param[in] seq Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseq_linear_trav_area(const TSequence *seq, int32_t srid)
{
  assert(seq); assert(seq->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * (seq->count - 1));
  int count = tcbufferseq_linear_trav_area_iter(seq, srid, geoms);
  LWGEOM *res;
  if (count == 1)
    /* When there is a single trapezoid */
    res = geoms[0];
  else
    // TODO add the bounding box instead of ask PostGIS to compute it again
    res = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE, srid,
      NULL, (uint32_t) (seq->count - 1), geoms);
  GSERIALIZED *result = geo_serialize(res);
  /* We cannot pfree(geoms); */
  lwgeom_free(res);
  return result;
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence
 * @param[in] seq Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseq_trav_area(const TSequence *seq, int32_t srid)
{
  assert(seq);

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tcbufferinst_trav_area(TSEQUENCE_INST_N(seq, 0), srid);

  /* General case */
  return (MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR) ?
    tcbufferseq_linear_trav_area(seq, srid) :
    tcbufferseq_discstep_trav_area(seq, srid);
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence set
 * with step interpolation
 * @param[in] ss Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseqset_step_trav_area(const TSequenceSet *ss, int32_t srid)
{
  assert(ss); assert(ss->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(ss->flags) == STEP);
  const TInstant **instants = tsequenceset_insts(ss);
  return cbufferarr_circles(instants, ss->count, srid);
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer sequence set
 * with linear interpolation
 * @param[in] ss Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseqset_linear_trav_area(const TSequenceSet *ss, int32_t srid)
{
  assert(ss); assert(ss->count > 1);
  assert(MEOS_FLAGS_GET_INTERP(ss->flags) == LINEAR);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ss->totalcount);
  int ngeoms = 0;
  for (int i = 0; i < ss->count; i++)
  {
    /* Get the traversed area of the sequence */
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    ngeoms += tcbufferseq_linear_trav_area_iter(seq, srid, &geoms[ngeoms]);
  }
  // TODO add the bounding box instead of ask PostGIS to compute it again
  LWGEOM *res = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE, srid,
    NULL, (uint32_t) ngeoms, geoms);
  /* We cannot pfree(geoms); */
  return geo_serialize(res);
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer
 * @param[in] ss Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbufferseqset_trav_area(const TSequenceSet *ss, int32_t srid)
{
  assert(ss); assert(MEOS_FLAGS_GET_INTERP(ss->flags) == LINEAR);
  
  /* Singleton sequence set */
  if (ss->count == 1)
    return tcbufferseq_trav_area(TSEQUENCESET_SEQ_N(ss, 0), srid);

  /* General case */
  return (MEOS_FLAGS_GET_INTERP(ss->flags) == LINEAR) ?
    tcbufferseqset_linear_trav_area(ss, srid) :
    tcbufferseqset_step_trav_area(ss, srid);
}

/**
 * @ingroup meos_cbuffer_spatial_accessor
 * @brief Return the traversed area of a temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] srid SRID
 * @csqlfn #Tcbuffer_traversed_area()
 */
GSERIALIZED *
tcbuffer_traversed_area(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  int32_t srid = tspatial_srid(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tcbufferinst_trav_area((TInstant *) temp, srid);
      break;
    case TSEQUENCE:
      return tcbufferseq_trav_area((TSequence *) temp, srid);
      break;
    default: /* TSEQUENCESET */
      return tcbufferseqset_trav_area((TSequenceSet *) temp, srid);
  }
}

/*****************************************************************************/

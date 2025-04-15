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
  lwpoint_free(points[0]); lwpoint_free(points[1]); lwgeom_free(ring);
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
  /* We cannot free lwgeom_free(res); */
  return result;
}

/*****************************************************************************
 * Traversed area 
 *****************************************************************************/

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

  /* TODO */
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Function %s not implemented", __func__);
  return NULL;
}

/*****************************************************************************/

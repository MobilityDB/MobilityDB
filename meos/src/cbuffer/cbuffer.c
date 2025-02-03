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
 * @brief Static buffer type
 */

#include "cbuffer/tcbuffer.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/type_out.h"
#include "general/type_util.h"
#include "point/pgis_types.h"
#include "point/tpoint.h"
#include "point/tpoint_out.h"
#include "point/tpoint_spatialfuncs.h"
#include "general/type_parser.h"
#include "point/tpoint_parser.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_parser.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_types
 * @brief Return a circular buffer from its string representation
 * @param[in] str String
 * @csqlfn #Cbuffer_in()
 */
Cbuffer *
cbuffer_in(const char *str)
{
  return cbuffer_parse(&str, true);
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return the string representation of a circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_out()
 */
char *
cbuffer_out(const Cbuffer *cbuf, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_negative(maxdd))
    return NULL;
  
  Datum d = PointerGetDatum(&cbuf->point);
  char *point = basetype_out(d, T_GEOMETRY, maxdd);
  char *radius = float8_out(cbuf->radius, maxdd);
  size_t size = strlen(point) + strlen(radius) + 11; // Cbuffer(,) + end NULL
  char *result = palloc(size);
  snprintf(result, size, "Cbuffer(%s,%s)", point, radius);
  pfree(point); pfree(radius);
  return result; 
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_types
 * @brief Return a circular buffer from a point and a radius
 * @param[in] point Point
 * @param[in] radius Radius
 * @csqlfn #Cbuffer_constructor()
 */
Cbuffer *
cbuffer_make(const GSERIALIZED *point, double radius)
{
  size_t value_offset = sizeof(Cbuffer) - sizeof(Datum);
  size_t size = value_offset;
  /* Create the circular buffer */
  void *value_from = (void *) point;
  size_t value_size = DOUBLE_PAD(VARSIZE(value_from));
  size += value_size;
  Cbuffer *result = palloc0(size);
  void *value_to = ((char *) result) + value_offset;
  memcpy(value_to, value_from, value_size);
  /* Initialize the radius and the size of the variable-length structure */
  result->radius = radius;
  SET_VARSIZE(result, size);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_types
 * @brief Return the point of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_point()
 */
const GSERIALIZED *
cbuffer_point(const Cbuffer *cbuf)
{
  Datum d = PointerGetDatum(&cbuf->point);
  return DatumGetGserializedP(d);
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return the radius of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #CBuffer_position()
 */
double
cbuffer_radius(const Cbuffer *cbuf)
{
  return cbuf->radius;
}

/*****************************************************************************
 * Conversions between circular point and geometry
 *****************************************************************************/

extern LWCIRCSTRING *lwcircstring_from_lwpointarray(int32_t srid, uint32_t npoints, LWPOINT **points);

/**
 * @ingroup meos_cbuffer_types
 * @brief Transform a circular buffer into a geometry
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_to_geom()
 */
GSERIALIZED *
cbuffer_to_geom(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf))
    return NULL;

  Datum d = PointerGetDatum(&cbuf->point);
  GSERIALIZED *gs = DatumGetGserializedP(d);
  int32_t srid = gserialized_get_srid(gs);
  LWPOINT *points[3];
  points[1] = (LWPOINT *) lwgeom_from_gserialized(gs);
  /* Shift the X coordinate of cbuf->point by +- cbuf->radius */
  POINT2D *p = (POINT2D *) GS_POINT_PTR(gs);
  points[0] = points[2] = lwpoint_make2d(srid, p->x - cbuf->radius, 
    p->y - cbuf->radius);
  points[1] = lwpoint_make2d(srid, p->x + cbuf->radius, p->y + cbuf->radius);
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

/**
 * @ingroup meos_cbuffer_types
 * @brief Transform a geometry into a circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Geom_to_cbuffer()
 */
Cbuffer *
geom_to_cbuffer(const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_circle_type(gs))
    return NULL;

  int32_t srid = gserialized_get_srid(gs);
  LWCURVEPOLY *poly = (LWCURVEPOLY *) lwgeom_from_gserialized(gs);
  LWLINE *ring = (LWLINE *) poly->rings[0];
  POINT4D p1, p2;
  getPoint4d_p(ring->points, 0, &p1);
  getPoint4d_p(ring->points, 1, &p2);
  double radius = (p2.x - p1.x) / 2;
  LWGEOM *center = (LWGEOM *) lwpoint_make2d(srid, p1.x + radius, p1.y + radius);
  GSERIALIZED *gscenter = geom_serialize(center);
  lwgeom_free((LWGEOM *) poly);
  Cbuffer *result = cbuffer_make(gscenter, radius);
  lwgeom_free(center); pfree(gscenter);
  return cbuffer_make(gscenter, radius);
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_types
 * @brief Return the SRID of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_get_srid()
 */
int
cbuffer_srid(const Cbuffer *cbuf)
{
  Datum d = PointerGetDatum(&cbuf->point);
  return gserialized_get_srid(DatumGetGserializedP(d));
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @brief Return an array of circular buffers converted into a geometry
 * @param[in] points Array of circular buffers
 * @param[in] count Number of elements in the input array
 * @pre The argument @p count is greater than 1
 */
// GSERIALIZED *
// cbufferarr_geom(Cbuffer **points, int count)
// {
  // assert(count > 1);
  // LWGEOM **geoms = palloc(sizeof(LWGEOM *) * count);
  // for (int i = 0; i < count; i++)
  // {
    // GSERIALIZED *gsline = route_geom(points[i]->point);
    // int32_t srid = gserialized_get_srid(gsline);
    // LWGEOM *line = lwgeom_from_gserialized(gsline);
    // geoms[i] = lwgeom_line_interpolate_point(line, points[i]->radius, srid, 0);
    // pfree(gsline); pfree(line);
  // }
  // int newcount;
  // LWGEOM **newgeoms = lwpointarr_remove_duplicates(geoms, count, &newcount);
  // LWGEOM *geom = lwpointarr_make_trajectory(newgeoms, newcount, STEP);
  // GSERIALIZED *result = geo_serialize(geom);
  // pfree(newgeoms); pfree(geom);
  // pfree_array((void **) geoms, count);
  // return result;
// }

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_types
 * @brief Return true if the first buffer is equal to the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_eq()
 */
bool
cbuffer_eq(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  Datum d1 = PointerGetDatum(&cbuf1->point);
  Datum d2 = PointerGetDatum(&cbuf2->point);
  return datum_point_eq(d1, d2) && 
    fabs(cbuf1->radius - cbuf2->radius) < MEOS_EPSILON;
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return true if the first buffer is not equal to the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_ne()
 */
bool
cbuffer_ne(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  return (! cbuffer_eq(cbuf1, cbuf2));
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return -1, 0, or 1 depending on whether the first buffer
 * is less than, equal to, or greater than the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_cmp()
 */
int
cbuffer_cmp(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  GSERIALIZED *gs1 = (GSERIALIZED *) (&cbuf1->point);
  GSERIALIZED *gs2 = (GSERIALIZED *) (&cbuf2->point);
  int cmp = geopoint_cmp(gs1, gs2);
  if (cmp)
    return cmp;
  /* Both point are equal */
  else if(cbuf1->radius < cbuf2->radius)
    return -1;
  else if (cbuf1->radius > cbuf2->radius)
    return 1;
  return 0;
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return true if the first buffer is less than the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_lt()
 */
bool
cbuffer_lt(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  int cmp = cbuffer_cmp(cbuf1, cbuf2);
  return (cmp < 0);
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return true if the first buffer is less than or equal to the
 * second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_le()
 */
bool
cbuffer_le(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  int cmp = cbuffer_cmp(cbuf1, cbuf2);
  return (cmp <= 0);
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return true if the first buffer is greater than the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_gt()
 */
bool
cbuffer_gt(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  int cmp = cbuffer_cmp(cbuf1, cbuf2);
  return (cmp > 0);
}

/**
 * @ingroup meos_cbuffer_types
 * @brief Return true if the first buffer is greater than or equal to
 * the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_ge()
 */
bool
cbuffer_ge(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  int cmp = cbuffer_cmp(cbuf1, cbuf2);
  return (cmp >= 0);
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for span types for combining the hash of
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_types
 * @brief Return the 32-bit hash value of a circular buffer
 * @param[in] cbuf Circular buffer
 */
uint32
cbuffer_hash(const Cbuffer *cbuf)
{
  /* Compute hashes of value and radius */
  Datum d = PointerGetDatum(&cbuf->point);
  uint32 point_hash = gserialized_hash(DatumGetGserializedP(d));
  uint32 radius_hash = pg_hashfloat8(cbuf->radius);

  /* Merge hashes of value and radius */
  uint32 result = point_hash;
  result = (result << 1) | (result >> 31);
  result ^= radius_hash;
  return result;
}

/*****************************************************************************/

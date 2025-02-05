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
 * @brief Static circular buffer type
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
#include "general/set.h"
#include "general/tsequence.h"
#include "general/type_out.h"
#include "general/type_util.h"
#include "geo/pgis_types.h"
#include "geo/tgeo.h"
#include "geo/tgeo_out.h"
#include "geo/tgeo_spatialfuncs.h"
#include "general/type_parser.h"
#include "geo/tgeo_parser.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_parser.h"

/*****************************************************************************
 * Collinear function
 *****************************************************************************/

/**
 * @brief Return true if the three values are collinear
 * @param[in] cbuf1,cbuf2,cbuf3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `cbuf1` and `cbuf2` divided by the duration
 * of the timestamps associated to `cbuf1` and `cbuf3`
 */
bool
cbuffer_collinear(Cbuffer *cbuf1, Cbuffer *cbuf2, Cbuffer *cbuf3, double ratio)
{
  Datum value1 = PointerGetDatum(&cbuf1->point);
  Datum value2 = PointerGetDatum(&cbuf2->point);
  Datum value3 = PointerGetDatum(&cbuf3->point);
  if (! geopoint_collinear(value1, value2, value3, ratio, false, false))
    return false;
  return float_collinear(cbuf1->radius, cbuf2->radius, cbuf3->radius, ratio);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_inout
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
 * @ingroup meos_temporal_inout
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
 * @ingroup meos_temporal_constructor
 * @brief Return a circular buffer from a point and a radius
 * @param[in] point Point
 * @param[in] radius Radius
 * @csqlfn #Cbuffer_constructor()
 */
Cbuffer *
cbuffer_make(const GSERIALIZED *point, double radius)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) point) || ! ensure_point_type(point) ||
      ! ensure_not_empty(point) || ! ensure_has_not_Z_geo(point) || 
      ! ensure_has_not_M_geo(point) || ! ensure_not_geodetic_geo(point) ||
      ! ensure_not_negative_datum(Float8GetDatum(radius), T_FLOAT8))
    return NULL;

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

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a copy of a circular buffer
 * @param[in] cbuf Circular buffer
 */
Cbuffer *
cbuffer_copy(const Cbuffer *cbuf)
{
#if MEOS
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */

  Cbuffer *result = palloc(VARSIZE(cbuf));
  memcpy(result, cbuf, VARSIZE(cbuf));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
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
 * @ingroup meos_temporal_accessor
 * @brief Return the radius of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_radius()
 */
double
cbuffer_radius(const Cbuffer *cbuf)
{
  return cbuf->radius;
}

/*****************************************************************************
 * Conversions functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_conversion
 * @brief Transform a circular buffer into a geometry
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_to_geom()
 */
GSERIALIZED *
cbuffer_geom(const Cbuffer *cbuf)
{
#if MEOS
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */

  GSERIALIZED *gs = DatumGetGserializedP(PointerGetDatum(&cbuf->point));
  POINT2D *p = (POINT2D *) GS_POINT_PTR(gs);
  int32_t srid = gserialized_get_srid(gs);
  return geocircle_make(p->x, p->y, cbuf->radius, srid);
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Transform a geometry into a circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Geom_to_cbuffer()
 */
Cbuffer *
geom_cbuffer(const GSERIALIZED *gs)
{
#if MEOS
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_circle_type(gs))
    return NULL;
#else
  assert(gs); assert(circle_type(gs));
#endif /* MEOS */

  int32_t srid = gserialized_get_srid(gs);
  LWCURVEPOLY *poly = (LWCURVEPOLY *) lwgeom_from_gserialized(gs);
  LWLINE *ring = (LWLINE *) poly->rings[0];
  POINT4D p1, p2;
  getPoint4d_p(ring->points, 0, &p1);
  getPoint4d_p(ring->points, 1, &p2);
  /* Compute the radius. We cannot call the PostGIS function
   * interpolate_point4d(&p1, &p2, &p, ratio);
   * since it uses a double and not a long double for the interpolation */
  double x = p1.x + (double) ((long double) (p2.x - p1.x) * 0.5);
  double y = p1.y + (double) ((long double) (p2.y - p1.y) * 0.5);
  double radius = fabs(p2.x - p1.x) / 2;
  LWGEOM *center = (LWGEOM *) lwpoint_make2d(srid, x, y);
  GSERIALIZED *gscenter = geom_serialize(center);
  lwgeom_free((LWGEOM *) poly);
  Cbuffer *result = cbuffer_make(gscenter, radius);
  lwgeom_free(center); pfree(gscenter);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_conversion
 * @brief Return an array of circular buffers converted into a geometry
 * @param[in] cbufarr Array of circular buffers
 * @param[in] nelems Number of elements in the input array
 * @pre The argument @p count is greater than 1
 */
GSERIALIZED *
cbufferarr_geom(Cbuffer **cbufarr, int nelems)
{
  assert(cbufarr); assert(nelems > 1);
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * nelems);
  /* SRID of the first element of the array */
  int32_t srid = cbuffer_srid(cbufarr[0]);
  for (int i = 0; i < nelems; i++)
  {
    int32_t srid_elem = cbuffer_srid(cbufarr[i]);
    if (! ensure_same_srid(srid, srid_elem))
    {
      for (int j = 0; j < i; j++)
        pfree(geoms[i]);
      pfree(geoms);
      return NULL;
    }
    geoms[i] = cbuffer_geom(cbufarr[i]);
  }
  GSERIALIZED *result = geo_collect_garray(geoms, nelems);
  pfree_array((void **) geoms, nelems);
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_srid
 * @brief Return the SRID of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_srid()
 */
int32_t
cbuffer_srid(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf))
    return SRID_INVALID;
  Datum d = PointerGetDatum(&cbuf->point);
  return gserialized_get_srid(DatumGetGserializedP(d));
}

/**
 * @ingroup meos_temporal_spatial_srid
 * @brief Set the coordinates of the circular buffer to an SRID
 * @param[in] cbuf Circular buffer
 * @param[in] srid SRID
 * @csqlfn #Cbuffer_set_srid()
 */
void
cbuffer_set_srid(Cbuffer *cbuf, int32_t srid)
{
  assert(cbuf);
  GSERIALIZED *gs = DatumGetGserializedP(PointerGetDatum(&cbuf->point));
  gserialized_set_srid(gs, srid);
  return;
}

/*****************************************************************************
 * Approximate equality for circular buffers
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if two circular buffers are approximately equal with
 * respect to an epsilon value
 */
bool
cbuffer_same(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  /* Same radius */
  if (fabs(cbuf1->radius - cbuf2->radius) > MEOS_EPSILON)
    return false;
  /* Same points */
  Datum point1 = PointerGetDatum(&cbuf1->point);
  Datum point2 = PointerGetDatum(&cbuf2->point);
  return datum_point_same(point1, point2);
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_trad
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
 * @ingroup meos_temporal_comp_trad
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
 * @ingroup meos_temporal_comp_trad
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
 * @ingroup meos_temporal_comp_trad
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
 * @ingroup meos_temporal_comp_trad
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
 * @ingroup meos_temporal_comp_trad
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
 * @ingroup meos_temporal_comp_trad
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
 * the point and the hash.
 *****************************************************************************/

/**
 * @ingroup meos_temporal_acccessor
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

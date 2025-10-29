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
 * @brief Functions for the static circular buffer type
 */

#include "cbuffer/cbuffer.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "common/hashfn.h"
#include "port/pg_bitutils.h"
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>
#include <pgtypes.h>
#include "temporal/set.h"
#include "temporal/tsequence.h"
#include "temporal/type_inout.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/meos_transform.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial.h"
#include "geo/tspatial_parser.h"
#include "cbuffer/cbuffer.h"

/*****************************************************************************
 * Collinear and interpolation function
 *****************************************************************************/

/**
 * @brief Return true if the three values are collinear
 * @param[in] cb1,cb2,cb3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `cb1` and `cb2` divided by the duration
 * of the timestamps associated to `cb1` and `cb3`
 */
bool
cbuffer_collinear(const Cbuffer *cb1, const Cbuffer *cb2, const Cbuffer *cb3,
  double ratio)
{
  Datum value1 = PointerGetDatum(&cb1->point);
  Datum value2 = PointerGetDatum(&cb2->point);
  Datum value3 = PointerGetDatum(&cb3->point);
  if (! geopoint_collinear(value1, value2, value3, ratio, false, false))
    return false;
  return float_collinear(cb1->radius, cb2->radius, cb3->radius, ratio);
}

/**
 * @brief Return a float in [0,1] representing the location of the closest
 * location on the circular buffer segment to the given circular buffer,
 * as a fraction of the segment length
 * @param[in] start,end Circular buffers defining the segment
 * @param[in] value Circular buffer to locate
 */
long double
cbuffersegm_locate(const Cbuffer *start, const Cbuffer *end,
  const Cbuffer *value)
{
  const GSERIALIZED *gs1 = cbuffer_point_p(start);
  const GSERIALIZED *gs2 = cbuffer_point_p(end);
  const GSERIALIZED *gs = cbuffer_point_p(value);
  long double result1 = -1.0;
  long double result2 = -1.0;
  if (! geopoint_eq(gs1, gs2))
  {
    result1 = pointsegm_locate(PointerGetDatum(gs1), PointerGetDatum(gs2),
      PointerGetDatum(gs), NULL);
    if (result1 < 0.0)
      return -1.0;
  }
  else
  {
    /* If constant segment and the point of the value is different */
    if (! geopoint_eq(gs1, gs))
      return -1.0;
  }
  if (start->radius != end->radius)
  {
    result2 = floatsegm_locate(start->radius, end->radius, value->radius);
    if (result2 < 0.0)
      return -1.0;
  }
  else
  {
    /* If the radii are equal return result1 where
     * - result1 = -1.0 if gs1 == gs2 == gs, or
     * - result1 in [0,1] if gs1 != gs2 */
    return result1;
  }
  if (result1 >= 0.0 && result2 >= 0.0)
    return (fabsl(result1 - result2) <= MEOS_EPSILON) ? result1 : -1.0;
  if (result1 < 0.0 && result2 >= 0)
    return result2;
  else if(result1 >= 0 && result2 <= 0)
    return result1;
  else /* The three values are equal */
    return -1.0;
}

/**
 * @brief Return a circular buffer interpolated from a circular buffer segment
 * with respect to a fraction of its total length
 * @param[in] start,end Circular buffers defining the segment
 * @param[in] ratio Float in [0,1] representing the fraction of the total
 * length of the segment for locating the interpolated circular buffer
 */
Cbuffer *
cbuffersegm_interpolate(const Cbuffer *start, const Cbuffer *end,
  long double ratio)
{
  assert(ratio >= 0.0 && ratio <= 1.0);
  Datum value1 = PointerGetDatum(&start->point);
  Datum value2 = PointerGetDatum(&end->point);
  Datum value = pointsegm_interpolate(value1, value2, ratio);
  double radius = floatsegm_interpolate(start->radius, end->radius, ratio);
  Cbuffer *result = cbuffer_make(DatumGetGserializedP(value), radius);
  pfree(DatumGetPointer(value));
  return result;
}

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure the validity of a circular buffer and geometry
 */
bool
ensure_valid_cbuffer_geo(const Cbuffer *cb, const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(cb, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(cbuffer_srid(cb), gserialized_get_srid(gs)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a circular buffer and spatiotemporal box
 */
bool
ensure_valid_cbuffer_stbox(const Cbuffer *cb, const STBox *box)
{
  VALIDATE_NOT_NULL(cb, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_same_srid(cbuffer_srid(cb), box->srid))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two circular buffers
 */
bool
ensure_valid_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2)
{
  VALIDATE_NOT_NULL(cb1, false); VALIDATE_NOT_NULL(cb2, false);
  if (! ensure_same_srid(cbuffer_srid(cb1), cbuffer_srid(cb2)))
    return false;
  return true;
}

/**
 * @brief Return true if a set and a circular buffer are valid for set
 * operations
 * @param[in] s Set
 * @param[in] cb Value
 */
bool
ensure_valid_cbufferset_cbuffer(const Set *s, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_CBUFFERSET(s, false); VALIDATE_NOT_NULL(cb, false);
  if (! ensure_same_srid(spatialset_srid(s), cbuffer_srid(cb)))
    return false;
  return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @brief Parse a circular buffer from its string representation
 */
Cbuffer *
cbuffer_parse(const char **str, bool end)
{
  const char *type_str = meostype_name(T_CBUFFER);
  p_whitespace(str);

  /* Determine whether there is an SRID */
  int32_t srid;
  srid_parse(str, &srid);

  /* Parse prefix */
  if (pg_strncasecmp(*str, "CBUFFER", 7) != 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing prefix 'Cbuffer'", type_str);
    return NULL;
  }

  *str += 7;
  p_whitespace(str);

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse geo */
  p_whitespace(str);
  GSERIALIZED *gs;
  /* The following call consumes also the separator passed as parameter */
  if (! geo_parse(str, T_GEOMETRY, ',', &srid, &gs))
    return NULL;
  if (! ensure_point_type(gs) || ! ensure_not_empty(gs) ||
      ! ensure_has_not_Z_geo(gs) || ! ensure_has_not_M_geo(gs))
  {
    pfree(gs);
    return NULL;
  }

  p_comma(str);

  /* Parse radius */
  p_whitespace(str);
  Datum d;
  if (! basetype_parse(str, T_FLOAT8, ')', &d))
    return NULL;
  double radius = DatumGetFloat8(d);
  if (radius < 0)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "The radius must be a real number greater than or equal to 0");
    return NULL;
  }

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
        (end && ! ensure_end_input(str, type_str)))
    return NULL;

  Cbuffer *result = cbuffer_make(gs, radius);
  pfree(gs);
  return result;
}

#if MEOS
/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return a circular buffer from its string representation
 * @param[in] str String
 * @csqlfn #Cbuffer_in()
 */
Cbuffer *
cbuffer_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return cbuffer_parse(&str, true);
}
#endif /* MEOS */

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the string representation of a circular buffer
 * @param[in] cb Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_out()
 */
char *
cbuffer_out(const Cbuffer *cb, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  Datum d = PointerGetDatum(&cb->point);
  char *point = basetype_out(d, T_GEOMETRY, maxdd);
  char *radius = float8_out(cb->radius, maxdd);
  size_t size = strlen(point) + strlen(radius) + 11; // Cbuffer(,) + end NULL
  char *result = palloc(size);
  snprintf(result, size, "Cbuffer(%s,%s)", point, radius);
  pfree(point); pfree(radius);
  return result;
}

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @brief Return the Well-Known Text (WKT) representation of a circular buffer
 */
char *
cbuffer_wkt_out(Datum value, int maxdd, bool extended)
{
  Cbuffer *cb = DatumGetCbufferP(value);
  Datum d = PointerGetDatum(&cb->point);
  GSERIALIZED *gs = DatumGetGserializedP(d);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, extended ? WKT_EXTENDED : WKT_ISO, maxdd,
    &len);
  char *radius = float8_out(cb->radius, maxdd);
  len += strlen(radius) + 11; // Cbuffer(,) + end NULL
  char *result = palloc(len);
  snprintf(result, len, "Cbuffer(%s,%s)", wkt, radius);
  lwgeom_free(geom); pfree(wkt); pfree(radius);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a circular buffer
 * @param[in] cb Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_as_text()
 */
char *
cbuffer_as_text(const Cbuffer *cb, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;
  return cbuffer_wkt_out(PointerGetDatum(cb), maxdd, false);
}

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * circular buffer
 * @param[in] cb Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_as_ewkt()
 */
char *
cbuffer_as_ewkt(const Cbuffer *cb, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  return spatialbase_as_ewkt(PointerGetDatum(cb), T_CBUFFER, maxdd);
}

/*****************************************************************************
 * WKB and HexWKB input/output functions for circular buffers
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return a circular buffer from its Well-Known Binary (WKB)
 * representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Cbuffer_recv(), #Cbuffer_from_wkb()
 */
Cbuffer *
cbuffer_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return DatumGetCbufferP(type_from_wkb(wkb, size, T_CBUFFER));
}

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return a circular buffer from its ASCII hex-encoded Well-Known Binary
 * (WKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Cbuffer_from_hexwkb()
 */
Cbuffer *
cbuffer_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  return DatumGetCbufferP(type_from_hexwkb(hexwkb, size, T_CBUFFER));
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a circular
 * buffer
 * @param[in] cb Circular buffer
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Cbuffer_recv(), #Cbuffer_as_wkb()
 */
uint8_t *
cbuffer_as_wkb(const Cbuffer *cb, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(cb), T_CBUFFER, variant, size_out);
}

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a circular buffer
 * @param[in] cb Circular buffer
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Cbuffer_as_hexwkb()
 */
char *
cbuffer_as_hexwkb(const Cbuffer *cb, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(cb), T_CBUFFER,
    variant | (uint8_t) WKB_HEX, size_out);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_constructor
 * @brief Construct a circular buffer from a point and a radius
 * @param[in] point Point
 * @param[in] radius Radius
 * @csqlfn #Cbuffer_constructor()
 */
Cbuffer *
cbuffer_make(const GSERIALIZED *point, double radius)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(point, NULL);
  if (! ensure_point_type(point) || ! ensure_not_empty(point) ||
      ! ensure_has_not_Z_geo(point) || ! ensure_has_not_M_geo(point) ||
      ! ensure_not_geodetic_geo(point) ||
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
 * @ingroup meos_cbuffer_base_constructor
 * @brief Return a copy of a circular buffer
 * @param[in] cb Circular buffer
 */
Cbuffer *
cbuffer_copy(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  Cbuffer *result = palloc(VARSIZE(cb));
  memcpy(result, cb, VARSIZE(cb));
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_conversion
 * @brief Convert a circular buffer into a geometry
 * @param[in] cb Circular buffer
 * @csqlfn #Cbuffer_to_geom()
 */
GSERIALIZED *
cbuffer_to_geom(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  const GSERIALIZED *gs = DatumGetGserializedP(PointerGetDatum(&cb->point));
  const POINT2D *p = (POINT2D *) GS_POINT_PTR(gs);
  int32_t srid = gserialized_get_srid(gs);
  return geocircle_make(p->x, p->y, cb->radius, srid);
}

/**
 * @ingroup meos_cbuffer_base_conversion
 * @brief Convert a geometry into a circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Geom_to_cbuffer()
 */
Cbuffer *
geom_to_cbuffer(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  uint32_t type = gserialized_get_type(gs);

  /* POINTTYPE */
  if (type == POINTTYPE)
    return cbuffer_make(gs, 0.0);

  /* CURVEPOLYTYPE */
  GSERIALIZED *gscenter;
  double radius;
  if (type == POINTTYPE)
  {
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
    radius = fabs(p2.x - p1.x) / 2;
    LWGEOM *center = (LWGEOM *) lwpoint_make2d(srid, x, y);
    gscenter = geom_serialize(center);
    lwgeom_free((LWGEOM *) poly); lwgeom_free(center); 
  }
  else
  /* geotype != POINTTYPE && geotype != CURVEPOLYTYPE */
  {
    gscenter = geom_min_bounding_radius(gs, &radius);
  }
  Cbuffer *result = cbuffer_make(gscenter, radius);
  pfree(gscenter);
  return result;
  
}

/**
 * @ingroup meos_cbuffer_base_conversion
 * @brief Return a geometry converted from an array of circular buffers
 * @param[in] cbarr Array of circular buffers
 * @param[in] count Number of elements in the input array
 */
GSERIALIZED *
cbufferarr_to_geom(const Cbuffer **cbarr, int count)
{
  assert(cbarr); assert(count > 1);
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * count);
  /* SRID of the first element of the array */
  int32_t srid = cbuffer_srid(cbarr[0]);
  for (int i = 0; i < count; i++)
  {
    int32_t srid_elem = cbuffer_srid(cbarr[i]);
    if (! ensure_same_srid(srid, srid_elem))
    {
      for (int j = 0; j < i; j++)
        pfree(geoms[i]);
      pfree(geoms);
      return NULL;
    }
    geoms[i] = cbuffer_to_geom(cbarr[i]);
  }
  GSERIALIZED *result = geo_collect_garray(geoms, count);
  pfree_array((void **) geoms, count);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_set_conversion
 * @brief Convert a circular buffer into a circular buffer set
 * @param[in] cb Value
 * @csqlfn #Value_to_set()
 */
Set *
cbuffer_to_set(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  Datum v = PointerGetDatum(cb);
  return set_make_exp(&v, 1, 1, T_CBUFFER, ORDER_NO);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument the spatiotemporal box of a circular
 * buffer
 * @param[in] cb Circular buffer
 * @param[out] box Spatiotemporal box
 */
bool
cbuffer_set_stbox(const Cbuffer *cb, STBox *box)
{
  assert(cb); assert(box);
  const GSERIALIZED *point = cbuffer_point_p(cb);
  bool result = geo_set_stbox(point, box);
  /* Expand spatial coordinates with respect to radius */
  box->xmin -= cb->radius;
  box->ymin -= cb->radius;
  box->xmax += cb->radius;
  box->ymax += cb->radius;
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box contructed from
 * an array of circular buffers
 * @param[in] values Circular buffers
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
cbufferarr_set_stbox(const Datum *values, int count, STBox *box)
{
  cbuffer_set_stbox(DatumGetCbufferP(values[0]), box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    cbuffer_set_stbox(DatumGetCbufferP(values[i]), &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @ingroup meos_cbuffer_base_conversion
 * @brief Convert a circular buffer into a spatiotemporal box
 * @param[in] cb Circular buffer
 * @csqlfn #Cbuffer_to_stbox()
 */
STBox *
cbuffer_to_stbox(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  STBox box;
  if (! cbuffer_set_stbox(cb, &box))
    return NULL;
  return stbox_copy(&box);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_base_accessor
 * @brief Return a pointer to the point of a circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Cbuffer_point()
 */
const GSERIALIZED *
cbuffer_point_p(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  return (const GSERIALIZED *) (&cb->point);
}

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return a copy of the point of a circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Cbuffer_point()
 */
GSERIALIZED *
cbuffer_point(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  const GSERIALIZED *gs = (const GSERIALIZED *) (&cb->point);
  return geo_copy(gs);
}

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return the radius of a circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Cbuffer_radius()
 */
double
cbuffer_radius(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, DBL_MAX);
  return cb->radius;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_transf
 * @brief Return a circular buffer with the precision of the values set to a
 * number of decimal places
 */
Cbuffer *
cbuffer_round(const Cbuffer *cb, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  /* Set precision of the point and the radius */
  GSERIALIZED *point = point_round((GSERIALIZED *) (&cb->point), maxdd);
  double radius = float8_round(cb->radius, maxdd);
  Cbuffer *result = cbuffer_make(point, radius);
  pfree(point);
  return result;
}

/**
 * @brief Return a circular buffer with the precision of the values set to a
 * number of decimal places
 * @note Funcion used by the lifting infrastructure
 */
Datum
datum_cbuffer_round(Datum cbuffer, Datum size)
{
  /* Set precision of radius */
  return PointerGetDatum(cbuffer_round(DatumGetCbufferP(cbuffer),
    DatumGetInt32(size)));
}

/**
 * @ingroup meos_cbuffer_base_transf
 * @brief Return an array of circular buffers with the precision of the values
 * set to a number of decimal places
 * @param[in] cbarr Array of circular buffers
 * @param[in] count Number of elements in the array
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbufferarr_round()
 */
Cbuffer **
cbufferarr_round(const Cbuffer **cbarr, int count, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cbarr, NULL);
  if (! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  Cbuffer **result = palloc(sizeof(Cbuffer *) * count);
  for (int i = 0; i < count; i++)
    result[i] = cbuffer_round(cbarr[i], maxdd);
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_srid
 * @brief Return the SRID of a circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Cbuffer_srid()
 */
int32_t
cbuffer_srid(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, SRID_INVALID);
  return gserialized_get_srid(
    DatumGetGserializedP(PointerGetDatum(&cb->point)));
}

/**
 * @ingroup meos_cbuffer_base_srid
 * @brief Set the coordinates of a circular buffer to an SRID
 * @param[in] cb Circular buffer
 * @param[in] srid SRID
 * @csqlfn #Cbuffer_set_srid()
 */
void
cbuffer_set_srid(Cbuffer *cb, int32_t srid)
{
  /* Ensure the validity of the arguments */
  assert(cb);
  GSERIALIZED *gs = DatumGetGserializedP(PointerGetDatum(&cb->point));
  gserialized_set_srid(gs, srid);
  return;
}

/*****************************************************************************/

/**
 * @brief Return a circular buffer transformed to another SRID using a
 * pipeline
 * @param[in] cb Circular buffer
 * @param[in] srid_to Target SRID, may be `SRID_UNKNOWN` for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
Cbuffer *
cbuffer_transf_pj(const Cbuffer *cb, int32_t srid_to, const LWPROJ *pj)
{
  VALIDATE_NOT_NULL(cb, NULL); assert(pj);
  /* Copy the circular buffer to transform its point in place */
  Cbuffer *result = cbuffer_copy(cb);
  GSERIALIZED *gs = DatumGetGserializedP(PointerGetDatum(&result->point));
  if (! point_transf_pj(gs, srid_to, pj))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/**
 * @ingroup meos_cbuffer_base_srid
 * @brief Return a circular buffer transformed to another SRID
 * @param[in] cb Circular buffer
 * @param[in] srid_to Target SRID
 */
Cbuffer *
cbuffer_transform(const Cbuffer *cb, int32_t srid_to)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  int32_t srid_from = cbuffer_srid(cb);
  if (! ensure_srid_known(srid_from) || ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return cbuffer_copy(cb);

  /* Get the structure with information about the projection */
  LWPROJ *pj;
  if (! lwproj_lookup(srid_from, srid_to, &pj))
    return NULL;

  /* Transform the circular buffer */
  return cbuffer_transf_pj(cb, srid_to, pj);
}

/**
 * @ingroup meos_cbuffer_base_srid
 * @brief Return a circular buffer transformed to another SRID using a
 * pipeline
 * @param[in] cb Circular buffer
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be `SRID_UNKNOWN`
 * @param[in] is_forward True when the transformation is forward
 */
Cbuffer *
cbuffer_transform_pipeline(const Cbuffer *cb, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_NOT_NULL(pipeline, NULL);

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_from_str_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the circular buffer */
  Cbuffer *result = cbuffer_transf_pj(cb, srid_to, pj);

  /* Transform the circular buffer */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_dist
 * @brief Return the distance between two circular buffers
 * @param[in] cb1,cb2 Circular buffers
 * @note The function assumes that all validity tests have been previously done
 */
double
cbuffer_distance(const Cbuffer *cb1, const Cbuffer *cb2)
{
  const GSERIALIZED *gs1 = cbuffer_point_p(cb1);
  const GSERIALIZED *gs2 = cbuffer_point_p(cb2);
  double result = Max(geom_distance2d(gs1, gs2) - cb1->radius - cb2->radius, 0);
  return result;
}

/**
 * @ingroup meos_cbuffer_base_dist
 * @brief Return the distance between two circular buffers
 * @return On error return -1.0
 * @csqlfn #Distance_cbuffer_cbuffer()
 */
double
distance_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2)
{
  VALIDATE_NOT_NULL(cb1, -1.0); VALIDATE_NOT_NULL(cb2, -1.0);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_cbuffer_cbuffer(cb1, cb2))
    return -1.0;
  /* The following function assumes that all validity tests have been done */
  return cbuffer_distance(cb1, cb2);
}

/**
 * @ingroup meos_internal_cbuffer_dist
 * @brief Return the distance between two circular buffers
 * @param[in] cb1,cb2 Circular buffers
 * @note The function assumes that all validity tests have been previously done
 */
Datum
datum_cbuffer_distance(Datum cb1, Datum cb2)
{
  return Float8GetDatum(cbuffer_distance(DatumGetCbufferP(cb1), DatumGetCbufferP(cb2)));
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_dist
 * @brief Return the distance between a circular buffer and a geometry
 * @return On error return -1.0
 * @csqlfn #Distance_cbuffer_geo()
 */
double
distance_cbuffer_geo(const Cbuffer *cb, const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(cb, -1.0); VALIDATE_NOT_NULL(gs, -1.0);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_cbuffer_geo(cb, gs) || gserialized_is_empty(gs))
    return -1.0;

  GSERIALIZED *geo = cbuffer_to_geom(cb);
  double result = geom_distance2d(geo, gs);
  pfree(geo);
  return result;
}

/**
 * @ingroup meos_cbuffer_base_dist
 * @brief Return the distance between a circular buffer and a spatiotemporal box
 * @return On error return -1.0
 * @csqlfn #Distance_cbuffer_stbox()
 */
double
distance_cbuffer_stbox(const Cbuffer *cb, const STBox *box)
{
  VALIDATE_NOT_NULL(cb, -1.0); VALIDATE_NOT_NULL(box, -1.0);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_cbuffer_stbox(cb, box))
    return -1.0;

  GSERIALIZED *geo1 = cbuffer_to_geom(cb);
  GSERIALIZED *geo2 = stbox_geo(box);
  double result = geom_distance2d(geo1, geo2);
  pfree(geo1); pfree(geo2); 
  return result;
}

/*****************************************************************************
 * Auxiliary functions for spatial relationships
 *****************************************************************************/

/**
 * @brief Return 1 if a point is inside a circle or in the border, 0 otherwise
 * @note Inspired by
 * https://stackoverflow.com/questions/481144/equation-for-testing-if-a-point-is-inside-a-circle
 */
bool
point_in_circle(const POINT2D *center, double radius, double x, double y)
{ 
  double dx = fabs(x - center->x);
  if (dx > radius)
    return false;
  int dy = fabs(y - center->y);
  if (dy > radius)
    return false;
  if (dx + dy <= radius)
    return true;
  return (dx * dx + dy * dy <= radius * radius);
}

/**
 * @brief Return 1 if a point is inside a circle, 0 otherwise
 * @note Inspired by
 * https://stackoverflow.com/questions/481144/equation-for-testing-if-a-point-is-inside-a-circle
 */
bool
point_inside_circle(const POINT2D *center, double radius, double x, double y)
{ 
  double dx = fabs(x - center->x);
  if (dx >= radius)
    return false;
  int dy = fabs(y - center->y);
  if (dy >= radius)
    return false;
  if (dx + dy < radius)
    return true;
  return (dx * dx + dy * dy < radius * radius);
}

/*****************************************************************************
 * Spatial relationship functions
 * There are three versions of these functions
 * - Internal functions with the name cbuffer_<spatialrel> which suppose that
 *   the arguments are valid
 * - Internal functions with the name datum_cbuffer_<spatialrel> which are used
 *   int the lifting infrastructure and supposes that the arguments are valid
 * - External functions with the name <spatialrel>_cbuffer_cbuffer which must
 *   verify that the the arguments are valid
 * disjoint and intersects are inverse to each other
 *****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return true if the first circular buffer contains the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_contains()
 * @note The function assumes that all validity tests have been previously done
 */
int
cbuffer_contains(const Cbuffer *cb1, const Cbuffer *cb2)
{
  const GSERIALIZED *point1 = cbuffer_point_p(cb1);
  const GSERIALIZED *point2 = cbuffer_point_p(cb2);
  const POINT2D *pt1 = (POINT2D *) GS_POINT_PTR(point1);
  const POINT2D *pt2 = (POINT2D *) GS_POINT_PTR(point2);
  if (! point_inside_circle(pt1, cb1->radius, pt2->x - cb2->radius, pt2->y) ||
      ! point_inside_circle(pt1, cb1->radius, pt2->x + cb2->radius, pt2->y) ||
      ! point_inside_circle(pt1, cb1->radius, pt2->x, pt2->y - cb2->radius) ||
      ! point_inside_circle(pt1, cb1->radius, pt2->x, pt2->y + cb2->radius))
    return 0;
  return 1;
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return true if the first circular buffer covers the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_covers()
 * @note The function assumes that all validity tests have been previously done
 */
int
cbuffer_covers(const Cbuffer *cb1, const Cbuffer *cb2)
{
  const GSERIALIZED *point1 = cbuffer_point_p(cb1);
  const GSERIALIZED *point2 = cbuffer_point_p(cb2);
  const POINT2D *pt1 = (POINT2D *) GS_POINT_PTR(point1);
  const POINT2D *pt2 = (POINT2D *) GS_POINT_PTR(point2);
  if (! point_in_circle(pt1, cb1->radius, pt2->x - cb2->radius, pt2->y) ||
      ! point_in_circle(pt1, cb1->radius, pt2->x + cb2->radius, pt2->y) ||
      ! point_in_circle(pt1, cb1->radius, pt2->x, pt2->y - cb2->radius) ||
      ! point_in_circle(pt1, cb1->radius, pt2->x, pt2->y + cb2->radius))
    return 0;
  return 1;
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return true if two circular buffers are disjoint in 2D
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_disjoint()
 * @note The function assumes that all validity tests have been previously done
 */
int
cbuffer_disjoint(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return ! cbuffer_intersects(cb1, cb2);
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return true if two circular buffers intersect in 2D
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_intersects()
 * @note The function assumes that all validity tests have been previously done
 */
int
cbuffer_intersects(const Cbuffer *cb1, const Cbuffer *cb2)
{
  double dist = cbuffer_distance(cb1, cb2);
  return (dist == 0) ? 1 : 0;
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return true if the first circular buffer touches the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_touches()
 * @note The function assumes that all validity tests have been previously done
 */
int
cbuffer_touches(const Cbuffer *cb1, const Cbuffer *cb2)
{
  Datum d1 = PointerGetDatum(&cb1->point);
  Datum d2 = PointerGetDatum(&cb2->point);
  double dist1 = DatumGetFloat8(datum_pt_distance2d(d1, d2));
  return (dist1 == cb1->radius + cb2->radius) ? 1 : 0;
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return true if two 2D circular buffers are within a distance
 * @param[in] cb1,cb2 Circular buffers
 * @param[in] dist Distance
 * @note The function assumes that all validity tests have been previously done
 */
int
cbuffer_dwithin(const Cbuffer *cb1, const Cbuffer *cb2, double dist)
{
  double dist1 = cbuffer_distance(cb1, cb2);
  return (dist1 <= dist) ? 1 : 0;
}


/*****************************************************************************/

/**
 * @brief Return true if two circular buffers satisfy a spatial relationship
 * @param[in] cb1,cb2 Circular buffers
 * @param[in] func Function computing the spatial relationship
 */
int
spatialrel_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2,
  int (*func)(const Cbuffer *, const Cbuffer *))
{
  VALIDATE_NOT_NULL(cb1, -1); VALIDATE_NOT_NULL(cb2, -1);
  if (! ensure_valid_cbuffer_cbuffer(cb1, cb2))
    return -1;
  return func(cb1, cb2);
}

/**
 * @ingroup meos_cbuffer_base_rel
 * @brief Return true if the first circular buffer contains the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_contains()
 */
int
contains_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return spatialrel_cbuffer(cb1, cb2, &cbuffer_contains);
}

/**
 * @ingroup meos_cbuffer_base_rel
 * @brief Return true if the first circular buffer covers the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_covers()
 */
int
covers_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return spatialrel_cbuffer(cb1, cb2, &cbuffer_covers);
}

/**
 * @ingroup meos_cbuffer_base_rel
 * @brief Return true if two circular buffers are disjoint in 2D
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_intersects()
 */
int
disjoint_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return spatialrel_cbuffer(cb1, cb2, &cbuffer_disjoint);
}

/**
 * @ingroup meos_cbuffer_base_rel
 * @brief Return true if two circular buffers intersect in 2D
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_intersects()
 */
int
intersects_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return spatialrel_cbuffer(cb1, cb2, &cbuffer_intersects);
}

/**
 * @ingroup meos_cbuffer_base_rel
 * @brief Return true if the first circular buffer touches the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_touches()
 */
int
touches_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return spatialrel_cbuffer(cb1, cb2, &cbuffer_touches);
}

/**
 * @ingroup meos_cbuffer_base_rel
 * @brief Return true if two 2D circular buffers are within a distance
 * @param[in] cb1,cb2 Circular buffers
 * @param[in] dist Distance
 * @csqlfn #Cbuffer_dwithin()
 */
int
dwithin_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2, double dist)
{
  VALIDATE_NOT_NULL(cb1, -1); VALIDATE_NOT_NULL(cb2, -1);
  if (! ensure_valid_cbuffer_cbuffer(cb1, cb2))
    return -1;
  return cbuffer_dwithin(cb1, cb2, dist);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return a Datum true if the first circular buffer contains the second
 * one
 * @param[in] cb1,cb2 Circular buffers
 */
Datum
datum_cbuffer_contains(Datum cb1, Datum cb2)
{
  return BoolGetDatum(cbuffer_contains(DatumGetCbufferP(cb1),
    DatumGetCbufferP(cb2)));
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return a Datum true if the first circular buffer covers the second
 * one
 * @param[in] cb1,cb2 Circular buffers
 */
Datum
datum_cbuffer_covers(Datum cb1, Datum cb2)
{
  return BoolGetDatum(cbuffer_covers(DatumGetCbufferP(cb1),
    DatumGetCbufferP(cb2)));
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return a Datum true if two circular buffers are disjoint in 2D
 * @param[in] cb1,cb2 Circular buffers
 */
Datum
datum_cbuffer_disjoint(Datum cb1, Datum cb2)
{
  return BoolGetDatum(! cbuffer_intersects(DatumGetCbufferP(cb1),
    DatumGetCbufferP(cb2)));
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return a Datum true if two circular buffers intersect in 2D
 * @param[in] cb1,cb2 Circular buffers
 */
Datum
datum_cbuffer_intersects(Datum cb1, Datum cb2)
{
  return BoolGetDatum(cbuffer_intersects(DatumGetCbufferP(cb1),
    DatumGetCbufferP(cb2)));
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return a Datum true if the first circular buffer touches the second
 * one
 * @param[in] cb1,cb2 Circular buffers
 */
Datum
datum_cbuffer_touches(Datum cb1, Datum cb2)
{
  return BoolGetDatum(cbuffer_touches(DatumGetCbufferP(cb1),
    DatumGetCbufferP(cb2)));
}

/**
 * @ingroup meos_internal_cbuffer_base_rel
 * @brief Return a Datum true if two 2D circular buffers are within a distance
 * @param[in] cb1,cb2 Circular buffers
 * @param[in] dist Distance
 */
Datum
datum_cbuffer_dwithin(Datum cb1, Datum cb2, Datum dist)
{
  return BoolGetDatum(cbuffer_dwithin(DatumGetCbufferP(cb1),
    DatumGetCbufferP(cb2), DatumGetFloat8(dist)));
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is equal to the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_eq()
 */
bool
cbuffer_eq(const Cbuffer *cb1, const Cbuffer *cb2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb1, false); VALIDATE_NOT_NULL(cb2, false);
  Datum d1 = PointerGetDatum(&cb1->point);
  Datum d2 = PointerGetDatum(&cb2->point);
  return datum_point_eq(d1, d2) &&
    fabs(cb1->radius - cb2->radius) < MEOS_EPSILON;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is not equal to the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_ne()
 */
inline bool
cbuffer_ne(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return (! cbuffer_eq(cb1, cb2));
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if two circular buffers are approximately equal with
 * respect to an epsilon value
 */
bool
cbuffer_same(const Cbuffer *cb1, const Cbuffer *cb2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb1, false); VALIDATE_NOT_NULL(cb2, false);

  /* Same radius */
  if (fabs(cb1->radius - cb2->radius) > MEOS_EPSILON)
    return false;
  /* Same points */
  return datum_point_same(PointerGetDatum(&cb1->point),
    PointerGetDatum(&cb2->point));
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if two circular buffers are approximately equal with
 * respect to an epsilon value
 */
inline bool
cbuffer_nsame(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return ! cbuffer_same(cb1, cb2);
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first buffer
 * is less than, equal to, or greater than the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_cmp()
 */
int
cbuffer_cmp(const Cbuffer *cb1, const Cbuffer *cb2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb1, false); VALIDATE_NOT_NULL(cb2, false);

  GSERIALIZED *gs1 = (GSERIALIZED *) (&cb1->point);
  GSERIALIZED *gs2 = (GSERIALIZED *) (&cb2->point);
  int32_t srid1 = gserialized_get_srid(gs1);
  int32_t srid2 = gserialized_get_srid(gs2);
  /* Compare SRID */
  if (srid1 < srid2)
    return -1;
  if (srid1 > srid2)
    return 1;
  /* Compare coordinates */
  const POINT2D *pt1 = (POINT2D *) GS_POINT_PTR(gs1);
  const POINT2D *pt2 = (POINT2D *) GS_POINT_PTR(gs2);
  if (pt1->x < pt2->x)
    return -1;
  if (pt1->x > pt2->x)
    return 1;
  if (pt1->y < pt2->y)
    return -1;
  if (pt1->y > pt2->y)
    return 1;
  /* Compare radius */
  if(cb1->radius < cb2->radius)
    return -1;
  if (cb1->radius > cb2->radius)
    return 1;
  return 0;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is less than the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_lt()
 */
inline bool
cbuffer_lt(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return cbuffer_cmp(cb1, cb2) < 0;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is less than or equal to the
 * second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_le()
 */
inline bool
cbuffer_le(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return cbuffer_cmp(cb1, cb2) <= 0;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is greater than the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_gt()
 */
inline bool
cbuffer_gt(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return cbuffer_cmp(cb1, cb2) > 0;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is greater than or equal to
 * the second one
 * @param[in] cb1,cb2 Circular buffers
 * @csqlfn #Cbuffer_ge()
 */
inline bool
cbuffer_ge(const Cbuffer *cb1, const Cbuffer *cb2)
{
  return cbuffer_cmp(cb1, cb2) >= 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for span types for combining the hash of
 * the point and the hash.
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return the 32-bit hash value of a circular buffer
 * @param[in] cb Circular buffer
 */
uint32
cbuffer_hash(const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, INT_MAX);

  /* Compute hashes of value and radius */
  Datum d = PointerGetDatum(&cb->point);
  uint32 point_hash = gserialized_hash(DatumGetGserializedP(d));
  uint32 radius_hash = float8_hash(cb->radius);

  /* Merge hashes of value and radius */
  uint32 result = point_hash;
#if POSTGRESQL_VERSION_NUMBER >= 150000
  result = pg_rotate_left32(result, 1);
#else
  result =  (result << 1) | (result >> 31);
#endif
  result ^= radius_hash;
  return result;
}

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return the 64-bit hash value of a circular buffer using a seed
 * @param[in] cb Circular buffer
 * @param[in] seed Seed
 * csqlfn hash_extended
 */
uint64
cbuffer_hash_extended(const Cbuffer *cb, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, LONG_MAX);
  /* PostGIS currently does not provide an extended hash function, */
  return DatumGetUInt64(hash_any_extended(
    (unsigned char *) VARDATA_ANY(cb), VARSIZE_ANY_EXHDR(cb), seed));
}

/*****************************************************************************/

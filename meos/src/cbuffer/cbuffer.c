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
 * @brief Static circular buffer type
 */

#include "cbuffer/cbuffer.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include <common/hashfn.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/tsequence.h"
#include "general/type_inout.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tspatial.h"
#include "geo/tgeo.h"
#include "geo/tgeo_out.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "cbuffer/cbuffer.h"

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
 * @brief Parse a spatial value from its string representation
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

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return a circular buffer from its string representation
 * @param[in] str String
 * @csqlfn #Cbuffer_in()
 */
Cbuffer *
cbuffer_in(const char *str)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) str))
    return NULL;
#else
  assert(str);
#endif /* MEOS */
  return cbuffer_parse(&str, true);
}

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the string representation of a circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_out()
 */
char *
cbuffer_out(const Cbuffer *cbuf, int maxdd)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */
  if (! ensure_not_negative(maxdd))
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
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @brief Output a circular buffer in the Well-Known Text (WKT) representation
 */
char *
cbuffer_wkt_out(Datum value, int maxdd, bool extended)
{
  Cbuffer *cbuf = DatumGetCbufferP(value);
  Datum d = PointerGetDatum(&cbuf->point);
  GSERIALIZED *gs = DatumGetGserializedP(d);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, extended ? WKT_EXTENDED : WKT_ISO, maxdd, 
    &len);
  char *radius = float8_out(cbuf->radius, maxdd);
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
 * @param[in] cbuf Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_as_text()
 */
char *
cbuffer_as_text(const Cbuffer *cbuf, int maxdd)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */
  if (! ensure_not_negative(maxdd))
    return NULL;
  return cbuffer_wkt_out(PointerGetDatum(cbuf), maxdd, false);
}

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbuffer_as_ewkt()
 */
char *
cbuffer_as_ewkt(const Cbuffer *cbuf, int maxdd)
{
  return spatialbase_as_ewkt(PointerGetDatum(cbuf), T_CBUFFER, maxdd);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) wkb))
    return NULL;
  return DatumGetCbufferP(type_from_wkb(wkb, size, T_CBUFFER));
}

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return a circular buffer from its hex-encoded ASCII Well-Known Binary
 * (WKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Cbuffer_from_hexwkb()
 */
Cbuffer *
cbuffer_from_hexwkb(const char *hexwkb)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) hexwkb))
    return NULL;
  size_t size = strlen(hexwkb);
  return DatumGetCbufferP(type_from_hexwkb(hexwkb, size, T_CBUFFER));
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a circular
 * buffer
 * @param[in] cbuf Circular buffer
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Cbuffer_recv(), #Cbuffer_as_wkb()
 */
uint8_t *
cbuffer_as_wkb(const Cbuffer *cbuf, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_null((void *) size_out))
    return NULL;
#else
  assert(cbuf); assert(size_out);
#endif /* MEOS */
  return datum_as_wkb(PointerGetDatum(cbuf), T_CBUFFER, variant, size_out);
}

/**
 * @ingroup meos_cbuffer_base_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Cbuffer_as_hexwkb()
 */
char *
cbuffer_as_hexwkb(const Cbuffer *cbuf, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_null((void *) size_out))
    return NULL;
#else
  assert(cbuf); assert(size_out);
#endif /* MEOS */
  return (char *) datum_as_wkb(PointerGetDatum(cbuf), T_CBUFFER,
    variant | (uint8_t) WKB_HEX, size_out);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_constructor
 * @brief Return a circular buffer from a point and a radius
 * @param[in] point Point
 * @param[in] radius Radius
 * @csqlfn #Cbuffer_constructor()
 */
Cbuffer *
cbuffer_make(const GSERIALIZED *point, double radius)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) point))
    return NULL;
#else
  assert(point);
#endif /* MEOS */
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
 * @param[in] cbuf Circular buffer
 */
Cbuffer *
cbuffer_copy(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
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
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_conversion
 * @brief Transform a circular buffer into a geometry
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_to_geom()
 */
GSERIALIZED *
cbuffer_geom(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
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
 * @ingroup meos_cbuffer_base_conversion
 * @brief Transform a geometry into a circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Geom_to_cbuffer()
 */
Cbuffer *
geom_cbuffer(const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) gs))
    return NULL;
#else
  assert(gs);
#endif /* MEOS */
  if (! ensure_circle_type(gs))
    return NULL;

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
 * @ingroup meos_internal_base_conversion
 * @brief Return an array of circular buffers converted into a geometry
 * @param[in] cbufarr Array of circular buffers
 * @param[in] count Number of elements in the input array
 * @pre The argument @p count is greater than 1
 */
GSERIALIZED *
cbufferarr_geom(Cbuffer **cbufarr, int count)
{
  assert(cbufarr); assert(count > 1);
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * count);
  /* SRID of the first element of the array */
  int32_t srid = cbuffer_srid(cbufarr[0]);
  for (int i = 0; i < count; i++)
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
  GSERIALIZED *result = geo_collect_garray(geoms, count);
  pfree_array((void **) geoms, count);
  return result;
}

/*****************************************************************************
 * Transform a temporal circular buffer to a STBox
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument the spatiotemporal box of a circular
 * buffer
 * @param[in] cbuf Circular buffer
 * @param[out] box Spatiotemporal box
 */
bool
cbuffer_set_stbox(const Cbuffer *cbuf, STBox *box)
{
  assert(cbuf); assert(box);
  const GSERIALIZED *point = cbuffer_point_p(cbuf);
  bool result = geo_set_stbox(point, box);
  /* Expand spatial coordinates with respect to radius */
  box->xmin -= cbuf->radius;
  box->ymin -= cbuf->radius;
  box->xmax += cbuf->radius;
  box->ymax += cbuf->radius;
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
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box contructed from
 * an array of circular buffers
 * @param[in] values Circular buffers
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
cbufferset_stbox(const Datum *values, int count, STBox *box)
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
 * @brief Return a circular buffer converted to a spatiotemporal box
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_to_stbox()
 */
STBox *
cbuffer_stbox(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */
  STBox box;
  if (! cbuffer_set_stbox(cbuf, &box))
    return NULL;
  return stbox_copy(&box);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_base_accessor
 * @brief Return a pointer to the point of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_point()
 */
const GSERIALIZED *
cbuffer_point_p(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */
  return (const GSERIALIZED *) (&cbuf->point);
}

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return a copy of the point of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_point()
 */
GSERIALIZED *
cbuffer_point(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return NULL;
#else
  assert(cbuf);
#endif /* MEOS */
  const GSERIALIZED *gs = (const GSERIALIZED *) (&cbuf->point);
  return geo_copy(gs);
}

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return the radius of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_radius()
 */
double
cbuffer_radius(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return DBL_MAX;
#else
  assert(cbuf);
#endif /* MEOS */
  return cbuf->radius;
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
cbuffer_round(const Cbuffer *cbuf, int maxdd)
{
  /* Set precision of the point and the radius */
  GSERIALIZED *point = point_round((GSERIALIZED *) (&cbuf->point), maxdd);
  double radius = float_round(cbuf->radius, maxdd);
  return cbuffer_make(point, radius);
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
 * @brief Return an array of circular buffers with the precision of the
 * vales set to a number of decimal places
 * @param[in] cbufarr Array of circular buffers
 * @param[in] count Number of elements in the array
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Cbufferarr_round()
 */
Cbuffer **
cbufferarr_round(const Cbuffer **cbufarr, int count, int maxdd)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbufarr))
    return NULL;
#else
  assert(cbufarr);
#endif /* MEOS */
  if (! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  Cbuffer **result = palloc(sizeof(Cbuffer *) * count);
  for (int i = 0; i < count; i++)
    result[i] = cbuffer_round(cbufarr[i], maxdd);
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_srid
 * @brief Return the SRID of a circular buffer
 * @param[in] cbuf Circular buffer
 * @csqlfn #Cbuffer_srid()
 */
int32_t
cbuffer_srid(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return SRID_INVALID;
#else
  assert(cbuf);
#endif /* MEOS */

  return gserialized_get_srid(
    DatumGetGserializedP(PointerGetDatum(&cbuf->point)));
}

/**
 * @ingroup meos_cbuffer_base_srid
 * @brief Set the coordinates of the circular buffer to an SRID
 * @param[in] cbuf Circular buffer
 * @param[in] srid SRID
 * @csqlfn #Cbuffer_set_srid()
 */
void
cbuffer_set_srid(Cbuffer *cbuf, int32_t srid)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
  {
    ;
  }
#else
  assert(cbuf);
#endif /* MEOS */

  GSERIALIZED *gs = DatumGetGserializedP(PointerGetDatum(&cbuf->point));
  gserialized_set_srid(gs, srid);
  return;
}

/*****************************************************************************/

/**
 * @brief Return a circular buffer transformed to another SRID using a
 * pipeline
 * @param[in] cbuf Circular buffer
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
Cbuffer *
cbuffer_transf_pj(const Cbuffer *cbuf, int32_t srid_to, const LWPROJ *pj)
{
  assert(cbuf); assert(pj);
  /* Copy the circular buffer to transform its point in place */
  Cbuffer *result = cbuffer_copy(cbuf);
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
 * @param[in] cbuf Circular buffer
 * @param[in] srid_to Target SRID
 */
Cbuffer *
cbuffer_transform(const Cbuffer *cbuf, int32_t srid_to)
{
  int32_t srid_from;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_srid_known(srid_to) ||
      ! ensure_srid_known(srid_from = cbuffer_srid(cbuf)))
    return NULL;
    
  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return cbuffer_copy(cbuf);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get(srid_from, srid_to);
  if (! pj)
    return NULL;

  /* Transform the circular buffer */
  Cbuffer *result = cbuffer_transf_pj(cbuf, srid_to, pj);

  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_cbuffer_base_srid
 * @brief Return a circular buffer transformed to another SRID using a
 * pipeline
 * @param[in] cbuf Circular buffer
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Cbuffer *
cbuffer_transform_pipeline(const Cbuffer *cbuf, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_null((void *) pipeline))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the circular buffer */
  Cbuffer *result = cbuffer_transf_pj(cbuf, srid_to, pj);

  /* Transform the circular buffer */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/*****************************************************************************
 * Distance function
 *****************************************************************************/

/**
 * @brief Return the distance between the two poses
 */
Datum
cbuffer_distance(Datum pose1, Datum pose2)
{
  Datum geom1 = CbufferPGetDatum(cbuffer_geom(DatumGetCbufferP(pose1)));
  Datum geom2 = CbufferPGetDatum(cbuffer_geom(DatumGetCbufferP(pose2)));
  return datum_geom_distance2d(geom1, geom2);
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is equal to the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_eq()
 */
bool
cbuffer_eq(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf1) || ! ensure_not_null((void *) cbuf2))
    return false;
#else
  assert(cbuf1); assert(cbuf2);
#endif /* MEOS */

  Datum d1 = PointerGetDatum(&cbuf1->point);
  Datum d2 = PointerGetDatum(&cbuf2->point);
  return datum_point_eq(d1, d2) && 
    fabs(cbuf1->radius - cbuf2->radius) < MEOS_EPSILON;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is not equal to the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_ne()
 */
inline bool
cbuffer_ne(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  return (! cbuffer_eq(cbuf1, cbuf2));
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if two circular buffers are approximately equal with
 * respect to an epsilon value
 */
bool
cbuffer_same(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf1) || ! ensure_not_null((void *) cbuf2))
    return false;
#else
  assert(cbuf1); assert(cbuf2);
#endif /* MEOS */

  /* Same radius */
  if (fabs(cbuf1->radius - cbuf2->radius) > MEOS_EPSILON)
    return false;
  /* Same points */
  return datum_point_same(PointerGetDatum(&cbuf1->point),
    PointerGetDatum(&cbuf2->point));
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if two circular buffers are approximately equal with
 * respect to an epsilon value
 */
inline bool
cbuffer_nsame(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  return ! cbuffer_same(cbuf1, cbuf2);
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first buffer
 * is less than, equal to, or greater than the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_cmp()
 */
int
cbuffer_cmp(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf1) || ! ensure_not_null((void *) cbuf2))
    return false;
#else
  assert(cbuf1); assert(cbuf2);
#endif /* MEOS */

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
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is less than the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_lt()
 */
inline bool
cbuffer_lt(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  return cbuffer_cmp(cbuf1, cbuf2) < 0;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is less than or equal to the
 * second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_le()
 */
inline bool
cbuffer_le(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  return cbuffer_cmp(cbuf1, cbuf2) <= 0;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is greater than the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_gt()
 */
inline bool
cbuffer_gt(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  return cbuffer_cmp(cbuf1, cbuf2) > 0;
}

/**
 * @ingroup meos_cbuffer_base_comp
 * @brief Return true if the first buffer is greater than or equal to
 * the second one
 * @param[in] cbuf1,cbuf2 Buffers
 * @csqlfn #Cbuffer_ge()
 */
inline bool
cbuffer_ge(const Cbuffer *cbuf1, const Cbuffer *cbuf2)
{
  return cbuffer_cmp(cbuf1, cbuf2) >= 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for span types for combining the hash of
 * the point and the hash.
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return the 32-bit hash value of a circular buffer
 * @param[in] cbuf Circular buffer
 */
uint32
cbuffer_hash(const Cbuffer *cbuf)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) cbuf))
    return false;
#else
  assert(cbuf);
#endif /* MEOS */

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

/**
 * @ingroup meos_cbuffer_base_accessor
 * @brief Return the 64-bit hash value of a circular buffer using a seed
 * @param[in] cbuf Circular buffer
 * @param[in] seed Seed
 * csqlfn hash_extended
 */
uint64
cbuffer_hash_extended(const Cbuffer *cbuf, uint64 seed)
{
  /* PostGIS currently does not provide an extended hash function, */
  return DatumGetUInt64(hash_any_extended(
    (unsigned char *) VARDATA_ANY(cbuf), VARSIZE_ANY_EXHDR(cbuf), seed));
}

/*****************************************************************************/

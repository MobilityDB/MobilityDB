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
 * @brief Functions for spatiotemporal bounding boxes
 */

#include "geo/stbox.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include "utils/timestamp.h"
/* PostGIS */
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_inout.h"
#include "general/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial.h"
#include "geo/tspatial_parser.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
  #include "cbuffer/tcbuffer_boxops.h"
#endif
#if NPOINT
  #include "npoint/tnpoint_boxops.h"
#endif
#if POSE
  #include "pose/tpose_boxops.h"
#endif
#if RGEO
  #include "rgeo/trgeo_boxops.h"
#endif

/* Buffer size for input and output of STBox */
#define MAXGBOXLEN     256
#define MAXSTBOXLEN    256

/* PostGIS prototype */
extern void ll2cart(const POINT2D *g, POINT3D *p);

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_transf
 * @brief Return the second spatiotemporal box expanded with the first one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @pre No tests are made concerning the SRID, dimensionality, etc.
 * This should be ensured by the calling function.
 */
void
stbox_expand(const STBox *box1, STBox *box2)
{
  assert(box1); assert(box2);
  if (MEOS_FLAGS_GET_X(box2->flags))
  {
    box2->xmin = Min(box1->xmin, box2->xmin);
    box2->xmax = Max(box1->xmax, box2->xmax);
    box2->ymin = Min(box1->ymin, box2->ymin);
    box2->ymax = Max(box1->ymax, box2->ymax);
    if (MEOS_FLAGS_GET_Z(box2->flags) ||
      MEOS_FLAGS_GET_GEODETIC(box2->flags))
    {
      box2->zmin = Min(box1->zmin, box2->zmin);
      box2->zmax = Max(box1->zmax, box2->zmax);
    }
  }
  if (MEOS_FLAGS_GET_T(box2->flags))
    span_expand(&box1->period, &box2->period);
  return;
}

/*****************************************************************************
 * Input/ouput functions in string format
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_geo_box_inout
 * @brief Return a spatiotemporal box from its Well-Known Text (WKT)
 * representation
 * @details Examples of input are as follows
 * @code
 * STBOX X((1.0, 2.0), (3.0, 4.0)) -> only spatial 2D
 * STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial 3D
 * STBOX XT(((1.0, 2.0), (3.0, 4.0)),[2001-01-01, 2001-01-02]) -> spatiotemporal 2D+T
 * STBOX ZT(((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)),[2001-01-01, 2001-01-02]) -> spatiotemporal 3D+T
 * STBOX T([2001-01-01, 2001-01-02]) -> only temporal
 * SRID=xxxx;STBOX... -> Any of the above with SRID
 * GEODSTBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial
 * GEODSTBOX T([2001-01-01, 2001-01-02]) -> only temporal
 * GEODSTBOX ZT(((1.0, 2.0, 3.0),(4.0, 5.0, 6.0)),[2001-01-01, 2001-01-02]) -> spatiotemporal
 * SRID=xxxx;GEODSTBOX... -> Any of the above with SRID
 * @endcode
 * where the commas are optional and the SRID is optional. If the SRID is not
 * stated it is by default 0 for non geodetic boxes and 4326 for geodetic boxes
 * @param[in] str String
 */
STBox *
stbox_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return stbox_parse(&str);
}
#endif /* MEOS */

/**
 * @ingroup meos_geo_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
stbox_out(const STBox *box, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  static size_t size = MAXSTBOXLEN + 1;
  char *xmin = NULL, *xmax = NULL, *ymin = NULL, *ymax = NULL, *zmin = NULL,
    *zmax = NULL, *period = NULL;
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);
  assert(hasx || hast);

  char *str = palloc(size);
  char srid[18];
  if (hasx && box->srid > 0)
    /* SRID_MAXIMUM is defined by PostGIS as 999999 */
    snprintf(srid, sizeof(srid), "SRID=%d;", box->srid);
  else
    srid[0] = '\0';
  char *boxtype = geodetic ? "GEODSTBOX" : "STBOX";
  if (hast)
    /* The second argument is not used for periods */
    period = span_out(&box->period, maxdd);

  if (hasx && hast)
  {
    xmin = float8_out(box->xmin, maxdd);
    xmax = float8_out(box->xmax, maxdd);
    ymin = float8_out(box->ymin, maxdd);
    ymax = float8_out(box->ymax, maxdd);
    if (hasz)
    {
      zmin = float8_out(box->zmin, maxdd);
      zmax = float8_out(box->zmax, maxdd);
      snprintf(str, size, "%s%s ZT(((%s,%s,%s),(%s,%s,%s)),%s)",
        srid, boxtype, xmin, ymin, zmin, xmax, ymax, zmax, period);
    }
    else
      snprintf(str, size, "%s%s XT(((%s,%s),(%s,%s)),%s)",
        srid, boxtype, xmin, ymin, xmax, ymax, period);
  }
  else if (hasx)
  {
    xmin = float8_out(box->xmin, maxdd);
    xmax = float8_out(box->xmax, maxdd);
    ymin = float8_out(box->ymin, maxdd);
    ymax = float8_out(box->ymax, maxdd);
    if (hasz)
    {
      zmin = float8_out(box->zmin, maxdd);
      zmax = float8_out(box->zmax, maxdd);
      snprintf(str, size, "%s%s Z((%s,%s,%s),(%s,%s,%s))",
        srid, boxtype, xmin, ymin, zmin, xmax, ymax, zmax);
    }
    else
      snprintf(str, size, "%s%s X((%s,%s),(%s,%s))",
        srid, boxtype, xmin, ymin, xmax, ymax);
  }
  else /* hast */
    snprintf(str, size, "%s%s T(%s)", srid, boxtype, period);

  if (hasx)
  {
    pfree(xmin); pfree(xmax);
    pfree(ymin); pfree(ymax);
    if (hasz)
    {
      pfree(zmin); pfree(zmax);
    }
  }
  if (hast)
    pfree(period);
  return str;
}

/*****************************************************************************
 * WKB and HexWKB input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_box_inout
 * @brief Return a spatiotemporal box from its Well-Known Binary (WKB)
 * representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Stbox_recv(), #Stbox_from_wkb()
 */
STBox *
stbox_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return DatumGetSTboxP(type_from_wkb(wkb, size, T_STBOX));
}

/**
 * @ingroup meos_geo_box_inout
 * @brief Return a spatiotemporal box from its hex-encoded ASCII Well-Known
 * Binary (WKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Stbox_from_hexwkb()
 */
STBox *
stbox_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
 size_t size = strlen(hexwkb);
  return DatumGetSTboxP(type_from_hexwkb(hexwkb, size, T_STBOX));
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_box_inout
 * @brief Return the Well-Known Binary (WKB) representation of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Stbox_recv(), #Stbox_as_wkb()
 */
uint8_t *
stbox_as_wkb(const STBox *box, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(box), T_STBOX, variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_geo_box_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Stbox_as_hexwkb()
 */
char *
stbox_as_hexwkb(const STBox *box, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(box), T_STBOX,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_box_constructor
 * @brief Return a spatiotemporal box from the arguments
 * @param[in] hasx True if the values for the spatial dimension are givne
 * @param[in] hasz True if there is a Z dimension
 * @param[in] geodetic True if geodetic
 * @param[in] srid SRID
 * @param[in] xmin,ymin,zmin Minimum bounds for the spatial dimension
 * @param[in] xmax,ymax,zmax Maximum bounds for the spatial dimension
 * @param[in] s Span, may be `NULL`
 * @csqlfn #Stbox_constructor()
 */
STBox *
stbox_make(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin,
  double xmax, double ymin, double ymax, double zmin, double zmax,
  const Span *s)
{
#if MEOS
  if (s && s->spantype != T_TSTZSPAN)
    return NULL;
#else
  assert(! s || s->spantype == T_TSTZSPAN);
#endif /* MEOS */

  /* Note: zero-fill is done in function stbox_set */
  STBox *result = palloc(sizeof(STBox));
  stbox_set(hasx, hasz, geodetic, srid, xmin, xmax, ymin, ymax, zmin, zmax, s,
    result);
  return result;
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from the
 * given arguments
 * @param[in] hasx True if the values for the spatial dimension are given
 * @param[in] hasz True if there is a Z dimension
 * @param[in] geodetic True if geodetic
 * @param[in] srid SRID
 * @param[in] xmin,ymin,zmin Minimum bounds for the spatial dimension
 * @param[in] xmax,ymax,zmax Maximum bounds for the spatial dimension
 * @param[in] s Span
 * @param[out] box Resulting box
 * @note This function is equivalent to #stbox_make without memory allocation
 */
void
stbox_set(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin,
  double xmax, double ymin, double ymax, double zmin, double zmax,
  const Span *s, STBox *box)
{
  assert(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  MEOS_FLAGS_SET_X(box->flags, hasx);
  MEOS_FLAGS_SET_Z(box->flags, hasz);
  MEOS_FLAGS_SET_GEODETIC(box->flags, geodetic);
  box->srid = srid;

  if (s)
  {
    /* Process T min/max */
    memcpy(&box->period, s, sizeof(Span));
    MEOS_FLAGS_SET_T(box->flags, true);
  }
  if (hasx)
  {
    /* Process X min/max */
    box->xmin = Min(xmin, xmax);
    box->xmax = Max(xmin, xmax);
    /* Process Y min/max */
    box->ymin = Min(ymin, ymax);
    box->ymax = Max(ymin, ymax);
    if (hasz)
    {
      /* Process Z min/max */
      box->zmin = Min(zmin, zmax);
      box->zmax = Max(zmin, zmax);
    }
  }
  return;
}

/**
 * @ingroup meos_geo_box_constructor
 * @brief Return a copy of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 */
STBox *
stbox_copy(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  STBox *result = palloc(sizeof(STBox));
  memcpy(result, box, sizeof(STBox));
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_box_constructor
 * @brief Return a spatiotemporal box from a geometry/geography and a
 * timestamptz
 * @param[in] gs Geometry/geography
 * @param[in] t Timestamp
 * @csqlfn #Stbox_constructor()
 */
STBox *
geo_timestamptz_to_stbox(const GSERIALIZED *gs, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (gserialized_is_empty(gs))
    return NULL;

  STBox *result = palloc(sizeof(STBox));
  geo_set_stbox(gs, result);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &result->period);
  MEOS_FLAGS_SET_T(result->flags, true);
  return result;
}

/**
 * @ingroup meos_geo_box_constructor
 * @brief Return a spatiotemporal box from a geometry/geography and a
 * timestamptz span
 * @param[in] gs Geometry/geography
 * @param[in] s Span
 * @csqlfn #Stbox_constructor()
 */
STBox *
geo_tstzspan_to_stbox(const GSERIALIZED *gs, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TSTZSPAN(s, NULL);
  if (gserialized_is_empty(gs))
    return NULL;

  STBox *result = palloc(sizeof(STBox));
  geo_set_stbox(gs, result);
  memcpy(&result->period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(result->flags, true);
  return result;
}

/*****************************************************************************
 * Conversion functions
 * The interal *_set_* functions initialize the output argument to 0
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a `GBOX` constructed from a 
 * spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @param[out] gbox GBOX
 */
void
stbox_set_gbox(const STBox *box, GBOX *gbox)
{
  assert(box); assert(gbox); assert(MEOS_FLAGS_GET_X(box->flags));
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(gbox, 0, sizeof(GBOX));
  /* Initialize existing dimensions */
  gbox->xmin = box->xmin;
  gbox->xmax = box->xmax;
  gbox->ymin = box->ymin;
  gbox->ymax = box->ymax;
  if (MEOS_FLAGS_GET_Z(box->flags))
  {
    gbox->zmin = box->zmin;
    gbox->zmax = box->zmax;
  }
  FLAGS_SET_Z(gbox->flags, MEOS_FLAGS_GET_Z(box->flags));
  FLAGS_SET_M(gbox->flags, false);
  FLAGS_SET_GEODETIC(gbox->flags, MEOS_FLAGS_GET_GEODETIC(box->flags));
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a `BOX3D` constructed from a
 * spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @param[out] box3d BOX3D
 */
void
stbox_set_box3d(const STBox *box, BOX3D *box3d)
{
  assert(box); assert(box3d); assert(MEOS_FLAGS_GET_X(box->flags));
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box3d, 0, sizeof(BOX3D));
  /* Initialize existing dimensions */
  box3d->xmin = box->xmin;
  box3d->xmax = box->xmax;
  box3d->ymin = box->ymin;
  box3d->ymax = box->ymax;
  if (MEOS_FLAGS_GET_Z(box->flags))
  {
    box3d->zmin = box->zmin;
    box3d->zmax = box->zmax;
  }
  box3d->srid = box->srid;
  /* box3d does not have a flags attribute */
  return;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a spatiotemporal box into a `GBOX`
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_box2d()
 */
GBOX *
stbox_gbox(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_has_X(T_STBOX, box->flags))
    return NULL;

  GBOX *result = palloc(sizeof(GBOX));
  stbox_set_gbox(box, result);
  return result;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a spatiotemporal box into a `BOX3D`
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_box3d()
 */
BOX3D *
stbox_box3d(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_has_X(T_STBOX, box->flags) ||
      /* box3d does not have flags */
      ! ensure_not_geodetic(box->flags))
    return NULL;

  BOX3D *result = palloc(sizeof(BOX3D));
  stbox_set_box3d(box, result);
  return result;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Return a spatiotemporal box converted as a geometry/geography
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_geo()
 */
GSERIALIZED *
stbox_geo(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_has_X(T_STBOX, box->flags))
    return NULL;

  LWGEOM *geo;
  GSERIALIZED *result;
  BOX3D box3d;
  STBox box1;
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);
  if (geodetic)
  {
    /* Transform the stbox coordinates, expressed as Cartesian coordinates on
     * unit sphere back to lon/lat coordinates */
    POINT2D min2d, max2d;
    POINT3D p1, p2;
    GEOGRAPHIC_POINT g1, g2;
    min2d.x = box->xmin;
    min2d.y = box->ymin;
    max2d.x = box->xmax;
    max2d.y = box->ymax;
    ll2cart(&min2d, &p1);
    ll2cart(&max2d, &p2);
    cart2geog(&p1, &g1);
    cart2geog(&p2, &g2);
    memset(&box1, 0, sizeof(STBox));
    box1.xmin = rad2deg(g1.lon);
    box1.ymin = rad2deg(g1.lat);
    box1.xmax = rad2deg(g2.lon);
    box1.ymax = rad2deg(g2.lat);
    box1.flags = box->flags;
    box1.srid = box->srid;
  }
  else
    memcpy(&box1, box, sizeof(STBox));
  if (hasz)
  {
    stbox_set_box3d(&box1, &box3d);
    geo = box3d_to_lwgeom(&box3d);
  }
  else
  {
    GBOX box2d;
    stbox_set_gbox(&box1, &box2d);
    geo = box2d_to_lwgeom(&box2d, box->srid);
  }
  FLAGS_SET_Z(geo->flags, hasz);
  FLAGS_SET_GEODETIC(geo->flags, geodetic);
  result = geo_serialize(geo);
  lwgeom_free(geo);
  return result;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a spatiotemporal box into a timestamptz span
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_tstzspan()
 */
Span *
stbox_tstzspan(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_has_T(T_STBOX, box->flags))
    return NULL;
  return span_copy(&box->period);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Set the a spatiotemporal box in the last argument from the `GBOX`
 * and the SRID
 * @param[in] box GBOX
 * @param[in] srid SRID
 * @param[out] result Spatiotemporal box
 */
void
gbox_set_stbox(const GBOX *box, int32_t srid, STBox *result)
{
  assert(box);
  bool hasz = (bool) FLAGS_GET_Z(box->flags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(box->flags);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, hasz);
  MEOS_FLAGS_SET_T(result->flags, false);
  MEOS_FLAGS_SET_GEODETIC(result->flags, geodetic);

  result->xmin = box->xmin;
  result->xmax = box->xmax;
  result->ymin = box->ymin;
  result->ymax = box->ymax;
  if (hasz)
  {
    result->zmin = box->zmin;
    result->zmax = box->zmax;
  }
  result->srid = srid;
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Convert a `GBOX` into a spatiotemporal box
 * @param[in] box GBOX
 */
STBox *
gbox_stbox(const GBOX *box)
{
  assert(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  STBox *result = palloc0(sizeof(STBox));
  gbox_set_stbox(box, 0, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Convert a `BOX3D` into a spatiotemporal box
 * @param[in] box BOX3D
 */
STBox *
box3d_stbox(const BOX3D *box)
{
  assert(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  STBox *result = palloc0(sizeof(STBox));
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, false);
  MEOS_FLAGS_SET_GEODETIC(result->flags, false);

  result->xmin = box->xmin;
  result->xmax = box->xmax;
  result->ymin = box->ymin;
  result->ymax = box->ymax;
  result->zmin = box->zmin;
  result->zmax = box->zmax;
  result->srid = box->srid;
  return result;
}

/**
 * @brief Get the coordinates from a geometry/geography point
 * @note This function is called for the points composing a temporal point
 * @pre The point is not empty
 */
void
point_get_coords(const GSERIALIZED *point, bool hasz, double *x, double *y,
  double *z)
{
  if (hasz)
  {
    const POINT3DZ *p = GSERIALIZED_POINT3DZ_P(point);
    *x = p->x;
    *y = p->y;
    *z = p->z;
  }
  else
  {
    const POINT2D *p = GSERIALIZED_POINT2D_P(point);
    *x = p->x;
    *y = p->y;
  }
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument the spatiotemporal box of a
 * geometry/geography
 * @param[in] gs Geometry/geography
 * @param[out] box Spatiotemporal box
 */
bool
geo_set_stbox(const GSERIALIZED *gs, STBox *box)
{
  assert(gs); assert(box);
  if (gserialized_is_empty(gs))
    return false;

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(gs->gflags);
  box->srid = gserialized_get_srid(gs);
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, hasz);
  MEOS_FLAGS_SET_T(box->flags, false);
  MEOS_FLAGS_SET_GEODETIC(box->flags, geodetic);

  /* Short-circuit the case where the geometry/geography is a point */
  if (gserialized_get_type(gs) == POINTTYPE)
  {
    double x, y, z;
    point_get_coords(gs, hasz, &x, &y, &z);
    box->xmin = box->xmax = x;
    box->ymin = box->ymax = y;
    if (hasz)
      box->zmin = box->zmax = z;
    return true;
  }

  /* General case for arbitrary geometry/geography */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  GBOX gbox;
  memset(&gbox, 0, sizeof(GBOX));
  /* We are sure that the geometry/geography is not empty
   * We cannot use `lwgeom_calculate_gbox` since for geography it calculates
   * a geodetic box where the coordinates are expressed in the unit sphere
   */
  lwgeom_calculate_gbox_cartesian(geom, &gbox);
  lwgeom_free(geom);
  box->xmin = gbox.xmin;
  box->xmax = gbox.xmax;
  box->ymin = gbox.ymin;
  box->ymax = gbox.ymax;
  if (hasz)
  {
    box->zmin = gbox.zmin;
    box->zmax = gbox.zmax;
  }
  return true;
}

#if MEOS
/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a geometry/geography into a spatiotemporal box
 * @param[in] gs Geometry/geography
 * @csqlfn #Geo_to_stbox()
 */
STBox *
geo_stbox(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (ensure_not_empty(gs))
    return NULL;

  STBox *result = palloc(sizeof(STBox));
  geo_set_stbox(gs, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from
 * an array of geometries/geographies
 * @param[in] values Values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
geoarr_set_stbox(const Datum *values, int count, STBox *box)
{
  assert(values); assert(box);
  geo_set_stbox(DatumGetGserializedP(values[0]), box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    geo_set_stbox(DatumGetGserializedP(values[i]), &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * timestamptz
 * @param[in] t Timestamp
 * @param[out] box Spatiotemporal box
 */
void
timestamptz_set_stbox(TimestampTz t, STBox *box)
{
  assert(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  Datum dt = TimestampTzGetDatum(t);
  span_set(dt, dt, true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a timestamptz into a spatiotemporal box
 * @param[in] t Timestamp
 * @csqlfn #Timestamptz_to_stbox()
 */
STBox *
timestamptz_stbox(TimestampTz t)
{
  STBox *result = palloc(sizeof(STBox));
  timestamptz_set_stbox(t, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from a 
 * timestamptz set
 * @param[in] s Set
 * @param[out] box Spatiotemporal box
 */
void
tstzset_set_stbox(const Set *s, STBox *box)
{
  assert(s); assert(box); assert(s->settype == T_TSTZSET);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  set_set_span(s, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a timestamptz set into a spatiotemporal box
 * @param[in] s Set
 * @csqlfn #Tstzset_to_stbox()
 */
STBox *
tstzset_stbox(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  STBox *result = palloc(sizeof(STBox));
  tstzset_set_stbox(s, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * timestamptz span
 * @param[in] s Span
 * @param[out] box Spatiotemporal box
 */
void
tstzspan_set_stbox(const Span *s, STBox *box)
{
  assert(s); assert(box); assert(s->spantype == T_TSTZSPAN);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  memcpy(&box->period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a timestamptz span into a spatiotemporal box
 * @param[in] s Span
 * @csqlfn #Tstzspan_to_stbox()
 */
STBox *
tstzspan_stbox(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL);
  STBox *result = palloc(sizeof(STBox));
  tstzspan_set_stbox(s, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * timestamptz span set
 * @param[in] ss Span set
 * @param[out] box Spatiotemporal box
 */
void
tstzspanset_set_stbox(const SpanSet *ss, STBox *box)
{
  assert(ss); assert(box); assert(ss->spansettype == T_TSTZSPANSET);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  memcpy(&box->period, &ss->span, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup meos_geo_box_conversion
 * @brief Convert a timestamptz span set into a spatiotemporal box
 * @param[in] ss Span set
 * @csqlfn #Tstzspanset_to_stbox()
 */
STBox *
tstzspanset_stbox(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  STBox *result = palloc(sizeof(STBox));
  tstzspanset_set_stbox(ss, result);
  return result;
}
#endif /* MEOS */

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * spatial base value
 * @param[in] d Value
 * @param[in] basetype Type of the value
 * @param[out] box Spatiotemporal box
 */
bool
spatial_set_stbox(Datum d, meosType basetype, STBox *box)
{
  assert(spatial_basetype(basetype));
  switch (basetype)
  {
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return geo_set_stbox(DatumGetGserializedP(d), box);
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_set_stbox(DatumGetCbufferP(d), box);
#endif
#if NPOINT
    case T_NPOINT:
      return npoint_set_stbox(DatumGetNpointP(d), box);
#endif
#if POSE || RGEO
    case T_POSE:
      return pose_set_stbox(DatumGetPoseP(d), box);
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown set stbox function for type: %s", meostype_name(basetype));
    return false;
  }
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return true if a spatiotemporal box has value dimension
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_hasx()
 */
bool
stbox_hasx(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false);
  return MEOS_FLAGS_GET_X(box->flags);
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return true if a spatiotemporal box has Z dimension
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_hasz()
 */
bool
stbox_hasz(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false);
  return MEOS_FLAGS_GET_Z(box->flags);
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return true if a spatiotemporal box has time dimension
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_hast()
 */
bool
stbox_hast(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false);
  return MEOS_FLAGS_GET_T(box->flags);
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return true if a spatiotemporal box is geodetic
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_isgeodetic()
 */
bool
stbox_isgeodetic(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false);
  return MEOS_FLAGS_GET_GEODETIC(box->flags);
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the minimum X value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_xmin()
 */
bool
stbox_xmin(const STBox *box, double *result)
{
  /* Ensure the validity of the arguments */

  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmin;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the maximum X value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_xmax()
 */
bool
stbox_xmax(const STBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmax;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the minimum Y value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_ymin()
 */
bool
stbox_ymin(const STBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymin;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the maximum Y value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_ymax()
 */
bool
stbox_ymax(const STBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymax;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the minimum Z value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_zmin()
 */
bool
stbox_zmin(const STBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmin;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the maximum Z value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_zmax()
 */
bool
stbox_zmax(const STBox *box, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmax;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the minimum T value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_tmin()
 */
bool
stbox_tmin(const STBox *box, TimestampTz *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.lower);
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument whether the maximum T value of a
 * spatiotemporal box is inclusive
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_tmin_inc()
 */
bool
stbox_tmin_inc(const STBox *box, bool *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = box->period.lower_inc;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument the maximum T value of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_tmax()
 */
bool
stbox_tmax(const STBox *box, TimestampTz *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.upper);
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return in the last argument whether the maximum T value of a
 * spatiotemporal box is inclusive
 * @param[in] box Spatiotemporal box
 * @param[out] result Result
 * @return On error return false, otherwise return true
 * @csqlfn #Stbox_tmax_inc()
 */
bool
stbox_tmax_inc(const STBox *box, bool *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(result, false);
  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = box->period.upper_inc;
  return true;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return the area of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @param[in] spheroid When true, the calculation uses the WGS 84 spheroid,
 * otherwise it uses a faster spherical calculation
 * @return On error, return -1.0
 * @csqlfn #Stbox_area()
 */
double
stbox_area(const STBox *box, bool spheroid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, -1.0); 
  if (! ensure_has_X(T_STBOX, box->flags))
    return -1.0;

  if (! MEOS_FLAGS_GET_GEODETIC(box->flags))
    return (box->xmax - box->xmin) * (box->ymax - box->ymin);
  
  GSERIALIZED *geo = stbox_geo(box);
  double result = geog_area(geo, spheroid);
  pfree(geo);
  return result;
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return the volume of a 3D spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @result On error return -1.0
 * @csqlfn #Stbox_volume()
 */
double
stbox_volume(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, -1.0); 
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_has_Z(T_STBOX, box->flags) || ! ensure_not_geodetic(box->flags))
    return -1.0;
  return (box->xmax - box->xmin) * (box->ymax - box->ymin) * 
    (box->zmax - box->zmin);
}

/**
 * @ingroup meos_geo_box_accessor
 * @brief Return the permieter of the spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @param[in] spheroid When true, the calculation uses the WGS 84 spheroid,
 * otherwise it uses a faster spherical calculation
 * @result On error return -1.0
 * @csqlfn #Stbox_perimeter()
 */
double
stbox_perimeter(const STBox *box, bool spheroid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, -1.0); 
  if (! ensure_has_X(T_STBOX, box->flags))
    return -1.0;

  GSERIALIZED *geo = stbox_geo(box);
  double result = MEOS_FLAGS_GET_GEODETIC(box->flags) ?
    geog_perimeter(geo, spheroid) : geom_perimeter(geo);
  pfree(geo);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_transf
 * @brief Return in the last argument a spatiotemporal box with the precision
 * set to a number of decimal places
 * @param[in] box Spatiotemporal box
 * @param[in] maxdd Maximum number of decimal digits
 * @param[out] result Result box
 */
void
stbox_round_set(const STBox *box, int maxdd, STBox *result)
{
  assert(box); assert(result); assert(MEOS_FLAGS_GET_X(box->flags));
  assert(maxdd >=0);

  result->xmin = float_round(box->xmin, maxdd);
  result->xmax = float_round(box->xmax, maxdd);
  result->ymin = float_round(box->ymin, maxdd);
  result->ymax = float_round(box->ymax, maxdd);
  if (MEOS_FLAGS_GET_Z(box->flags) || MEOS_FLAGS_GET_GEODETIC(box->flags))
  {
    result->zmin = float_round(box->zmin, maxdd);
    result->zmax = float_round(box->zmax, maxdd);
  }
  return;
}

/**
 * @ingroup meos_geo_box_transf
 * @brief Return a spatiotemporal box with the precision of the coordinates set
 * to a number of decimal places
 * @param[in] box Spatiotemporal box
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Stbox_round()
 */
STBox *
stbox_round(const STBox *box, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); 
  if (! ensure_has_X(T_STBOX, box->flags) || ! ensure_not_negative(maxdd))
    return NULL;

  STBox *result = stbox_copy(box);
  stbox_round_set(box, maxdd, result);
  return result;
}

/**
 * @ingroup meos_geo_box_transf
 * @brief Return an array of spatiotemporal boxes with the precision of the
 * coordinates set to a number of decimal places
 * @param[in] boxarr Array of spatiotemporal boxes
 * @param[in] count Number of elements in the array
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Stboxarr_round()
 */
STBox *
stboxarr_round(const STBox *boxarr, int count, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(boxarr, NULL); 
  if (! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  STBox *result = palloc(sizeof(STBox) * count);
  memcpy(result, boxarr, sizeof(STBox) * count);
  for (int i = 0; i < count; i++)
    stbox_round_set(&boxarr[i], maxdd, &result[i]);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_box_transf
 * @brief Return a spatiotemporal box with the time span expanded and/or scaled
 * by two intervals
 * @param[in] box Spatiotemporal box
 * @param[in] shift Interval to shift the value span, may be `NULL`
 * @param[in] duration Duration of the result, may be `NULL`
 * @csqlfn #Stbox_shift_time(), #Stbox_scale_time(), #Stbox_shift_scale_time()
 */
STBox *
stbox_shift_scale_time(const STBox *box, const Interval *shift,
  const Interval *duration)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_has_T(T_STBOX, box->flags) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Copy the input period to the result */
  STBox *result = stbox_copy(box);
  /* Shift and/or scale the resulting period */
  TimestampTz lower = DatumGetTimestampTz(box->period.lower);
  TimestampTz upper = DatumGetTimestampTz(box->period.upper);
  span_bounds_shift_scale_time(shift, duration, &lower, &upper);
  result->period.lower = TimestampTzGetDatum(lower);
  result->period.upper = TimestampTzGetDatum(upper);
  return result;
}

/**
 * @ingroup meos_geo_box_transf
 * @brief Return a spatiotemporal box with only the space dimension
 * @csqlfn #Stbox_get_space()
 * @param[in] box Spatiotemporal box
 */
STBox *
stbox_get_space(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); 
  if (! ensure_has_X(T_STBOX, box->flags))
    return NULL;

  STBox *result = palloc(sizeof(STBox));
  stbox_set(true, MEOS_FLAGS_GET_Z(box->flags),
    MEOS_FLAGS_GET_GEODETIC(box->flags), box->srid, box->xmin, box->xmax,
    box->ymin, box->ymax, box->zmin, box->zmax, NULL, result);
  return result;
}

/**
 * @ingroup meos_geo_box_transf
 * @brief Return a spatiotemporal box with the space bounds expanded/decreased
 * by a double
 * @param[in] box Spatiotemporal box
 * @param[in] d Value for expanding
 * @csqlfn #Stbox_expand_space()
 */
STBox *
stbox_expand_space(const STBox *box, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); 
  if (! ensure_has_X(T_STBOX, box->flags))
    return NULL;
  /* When the value is negative, ensure that its absolute value is less than
   * the size of all spatial dimensions */ 
  bool hasz = MEOS_FLAGS_GET_Z(box->flags) ||
    MEOS_FLAGS_GET_GEODETIC(box->flags);
  if (d < 0 && (
       fabs(d) >= (box->xmax - box->xmin) ||
       fabs(d) >= (box->ymax - box->ymin) ||
       (hasz && (fabs(d) >= (box->zmax - box->zmin)))))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The value to decrease must be smaller than the size of the stbox: %d", d);
    return NULL;
  }

  STBox *result = stbox_copy(box);
  result->xmin -= d;
  result->ymin -= d;
  result->xmax += d;
  result->ymax += d;
  if (hasz)
  {
    result->zmin -= d;
    result->zmax += d;
  }
  return result;
}

/**
 * @ingroup meos_geo_box_transf
 * @brief Return a spatiotemporal box with the time span expanded/decreased by
 * an interval
 * @param[in] box Spatiotemporal box
 * @param[in] interv Interval for expanding
 * @csqlfn #Stbox_expand_time()
 */
STBox *
stbox_expand_time(const STBox *box, const Interval *interv)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(interv, NULL);
  if (! ensure_has_T(T_STBOX, box->flags))
    return NULL;
  /* When the interval is negative, ensure that its absolute value is less than
   * the duration of the spatiotemporal box */ 
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  bool negative = pg_interval_cmp(interv, &intervalzero) <= 0;
  Interval *duration = tstzspan_duration(&box->period);
  bool smaller = pg_interval_cmp(interv, duration) < 0;
  pfree(duration);
  if (negative && ! smaller)
  {
    char *str = pg_interval_out(interv);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The interval to decrease must be smaller than the time span of the stbox: %s",
      str);
    pfree(str);
    return NULL;
  }

  STBox *result = stbox_copy(box);
  TimestampTz tmin = minus_timestamptz_interval(DatumGetTimestampTz(
    box->period.lower), interv);
  TimestampTz tmax = add_timestamptz_interval(DatumGetTimestampTz(
    box->period.upper), interv);
  result->period.lower = TimestampTzGetDatum(tmin);
  result->period.upper = TimestampTzGetDatum(tmax);
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_box_srid
 * @brief Return the SRID of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_srid()
 */
int32_t
stbox_srid(const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, SRID_INVALID);
  if (! ensure_has_X(T_STBOX, box->flags))
    return SRID_INVALID;
  return box->srid;
}

/**
 * @ingroup meos_geo_box_srid
 * @brief Return a spatiotemporal box with the coordinates set to an SRID
 * @param[in] box Spatiotemporal box
 * @param[in] srid SRID
 * @csqlfn #Stbox_set_srid()
 */
STBox *
stbox_set_srid(const STBox *box, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_has_X(T_STBOX, box->flags))
    return NULL;
  STBox *result = stbox_copy(box);
  result->srid = srid;
  return result;
}

/**
 * @brief Return a spatiotemporal box transformed to another SRID using a
 * pipeline
 * @param[in] box Spatiotemporal box
 * @param[in] srid_to Target SRID, may be `SRID_UNKNOWN` for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
static STBox *
stbox_transf_pj(const STBox *box, int32_t srid_to, const LWPROJ *pj)
{
  assert(box); assert(pj);
  /* Create the points corresponding to the bounds */
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);
  GSERIALIZED *min = geopoint_make(box->xmin, box->ymin, box->zmin,
    hasz, geodetic, box->srid);
  GSERIALIZED *max = geopoint_make(box->xmax, box->ymax, box->zmax,
    hasz, geodetic, box->srid);

  /* Transform the points */
  if (! point_transf_pj(min, srid_to, pj) ||
      ! point_transf_pj(max, srid_to, pj))
  {
    pfree(min); pfree(max);
    return NULL;
  }

  STBox *result = stbox_copy(box);
  /* Set the bounds of the box from the transformed points */
  result->srid = srid_to;
  if (hasz)
  {
    const POINT3DZ *ptmin = GSERIALIZED_POINT3DZ_P(min);
    const POINT3DZ *ptmax = GSERIALIZED_POINT3DZ_P(max);
    result->xmin = ptmin->x;
    result->ymin = ptmin->y;
    result->zmin = ptmin->z;
    result->xmax = ptmax->x;
    result->ymax = ptmax->y;
    result->zmax = ptmax->z;
  }
  else
  {
    const POINT2D *ptmin = GSERIALIZED_POINT2D_P(min);
    const POINT2D *ptmax = GSERIALIZED_POINT2D_P(max);
    result->xmin = ptmin->x;
    result->ymin = ptmin->y;
    result->xmax = ptmax->x;
    result->ymax = ptmax->y;
  }

  /* Clean up and return */
  pfree(min); pfree(max);
  return result;
}

/**
 * @ingroup meos_geo_box_srid
 * @brief Return a spatiotemporal box transformed to another SRID
 * @param[in] box Spatiotemporal box
 * @param[in] srid_to Target SRID
 */
STBox *
stbox_transform(const STBox *box, int32_t srid_to)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_srid_known(box->srid) || ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (box->srid == srid_to)
    return stbox_copy(box);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get(box->srid, srid_to);
  if (! pj)
    return NULL;

  /* Transform the temporal point */
  STBox *result = stbox_copy(box);
  if (! stbox_transf_pj(stbox_copy(box), srid_to, pj))
  {
    pfree(result); return NULL;
  }
  return result;
}

/**
 * @ingroup meos_geo_box_srid
 * @brief Return a spatiotemporal box transformed to another SRID using a
 * pipeline
 * @param[in] box Spatiotemporal box
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be `SRID_UNKNOWN`
 * @param[in] is_forward True when the transformation is forward
 */
STBox *
stbox_transform_pipeline(const STBox *box, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(pipeline, NULL);
  if (! ensure_srid_known(box->srid))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the spatiotemporal box */
  STBox *result = stbox_transf_pj(box, srid_to, pj);

  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/*****************************************************************************
 * Topological functions
 *****************************************************************************/

/**
 * @brief Return the ouput variables initialized with the flag values of two
 * boxes
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hasz,hast,geodetic Boolean variables
 */
static void
stbox_stbox_flags(const STBox *box1, const STBox *box2, bool *hasx,
  bool *hasz, bool *hast, bool *geodetic)
{
  *hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  *hasz = MEOS_FLAGS_GET_Z(box1->flags) && MEOS_FLAGS_GET_Z(box2->flags);
  *hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  *geodetic = MEOS_FLAGS_GET_GEODETIC(box1->flags) &&
    MEOS_FLAGS_GET_GEODETIC(box2->flags);
  return;
}

/**
 * @brief Verify the conditions and set the ouput variables with the values of
 * the flags of the boxes
 *
 * Mixing 2D/3D is enabled to compute, for example, 2.5D operations
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hasz,hast,geodetic Boolean variables
 */
static bool
topo_stbox_stbox_init(const STBox *box1, const STBox *box2, bool *hasx,
  bool *hasz, bool *hast, bool *geodetic)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, false); VALIDATE_NOT_NULL(box2, false);
  if (! ensure_common_dimension(box1->flags, box2->flags))
    return false;
  if (MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags) &&
     (! ensure_same_geodetic(box1->flags, box2->flags) ||
      ! ensure_same_srid(stbox_srid(box1), stbox_srid(box2))))
    return false;
  stbox_stbox_flags(box1, box2, hasx, hasz, hast, geodetic);
  return true;
}

/**
 * @ingroup meos_geo_box_topo
 * @brief Return true if the first spatiotemporal box contains the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Contains_stbox_stbox()
 */
bool
contains_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  bool hasx, hasz, hast, geodetic;
  if (! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
    return false;

  if (hasx && (box2->xmin < box1->xmin || box2->xmax > box1->xmax ||
    box2->ymin < box1->ymin || box2->ymax > box1->ymax))
      return false;
  if (hasz && (box2->zmin < box1->zmin || box2->zmax > box1->zmax))
      return false;
  if (hast && (
    datum_lt(box2->period.lower, box1->period.lower, T_TIMESTAMPTZ) ||
    datum_gt(box2->period.upper, box1->period.upper, T_TIMESTAMPTZ)))
      return false;
  return true;
}

/**
 * @ingroup meos_geo_box_topo
 * @brief Return true if the first spatiotemporal box is contained in the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Contained_stbox_stbox()
 */
inline bool
contained_stbox_stbox(const STBox *box1, const STBox *box2)
{
  return contains_stbox_stbox(box2, box1);
}

/**
 * @ingroup meos_geo_box_topo
 * @brief Return true if the spatiotemporal boxes overlap
 * @csqlfn #Overlaps_stbox_stbox()
 */
bool
overlaps_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  bool hasx, hasz, hast, geodetic;
  if (! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
    return false;

  if (hasx && (box1->xmax < box2->xmin || box1->xmin > box2->xmax ||
    box1->ymax < box2->ymin || box1->ymin > box2->ymax))
    return false;
  if (hasz && (box1->zmax < box2->zmin || box1->zmin > box2->zmax))
    return false;
  if (hast && (
    datum_lt(box1->period.upper, box2->period.lower, T_TIMESTAMPTZ) ||
    datum_gt(box1->period.lower, box2->period.upper, T_TIMESTAMPTZ)))
    return false;
  return true;
}

/**
 * @ingroup meos_geo_box_topo
 * @brief Return true if the spatiotemporal boxes are equal in the common
 * dimensions
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Same_stbox_stbox()
 */
bool
same_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  bool hasx, hasz, hast, geodetic;
  if (! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
    return false;

  if (hasx && (box1->xmin != box2->xmin || box1->xmax != box2->xmax ||
    box1->ymin != box2->ymin || box1->ymax != box2->ymax))
    return false;
  if (hasz && (box1->zmin != box2->zmin || box1->zmax != box2->zmax))
    return false;
  if (hast && (box1->period.lower != box2->period.lower ||
               box1->period.upper != box2->period.upper))
    return false;
  return true;
}

/**
 * @ingroup meos_geo_box_topo
 * @brief Return true if the spatiotemporal boxes are adjacent
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Adjacent_stbox_stbox()
 */
bool
adjacent_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  bool hasx, hasz, hast, geodetic;
  if (! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
    return false;

  STBox inter;
  if (! inter_stbox_stbox(box1, box2, &inter))
    return false;

  /* Boxes are adjacent if they share n dimensions and their intersection is
   * at most of n-1 dimensions */
  if (! hasx && hast)
    return (inter.period.lower == inter.period.upper);
  if (hasx && ! hast)
  {
    if (hasz)
      return (inter.xmin == inter.xmax || inter.ymin == inter.ymax ||
           inter.zmin == inter.zmax);
    else
      return (inter.xmin == inter.xmax || inter.ymin == inter.ymax);
  }
  else
  {
    if (hasz)
      return (inter.xmin == inter.xmax || inter.ymin == inter.ymax ||
           inter.zmin == inter.zmax ||
           inter.period.lower == inter.period.upper);
    else
      return (inter.xmin == inter.xmax || inter.ymin == inter.ymax ||
           inter.period.lower == inter.period.upper);
  }
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

/**
 * @brief Verify the conditions for a position operator
 * @param[in] box1,box2 Spatiotemporal boxes
 */
static bool
ensure_valid_pos_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, NULL); VALIDATE_NOT_NULL(box2, NULL);
  if (MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags) && (
       ! ensure_same_srid(stbox_srid(box1), stbox_srid(box2)) ||
       ! ensure_same_geodetic(box1->flags, box2->flags)))
    return false;
  return true;
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is to the left of the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Left_stbox_stbox()
 */
bool
left_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->xmax < box2->xmin);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * right of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overleft_stbox_stbox()
 */
bool
overleft_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->xmax <= box2->xmax);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is to the right of the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Right_stbox_stbox()
 */
bool
right_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->xmin > box2->xmax);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * left of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overright_stbox_stbox()
 */
bool
overright_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->xmin >= box2->xmin);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is below the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Below_stbox_stbox()
 */
bool
below_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->ymax < box2->ymin);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend above the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overbelow_stbox_stbox()
 */
bool
overbelow_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->ymax <= box2->ymax);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is above the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Above_stbox_stbox()
 */
bool
above_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->ymin > box2->ymax);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend below the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overabove_stbox_stbox()
 */
bool
overabove_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags))
    return false;
  return (box1->ymin >= box2->ymin);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is in front of the
 * the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Front_stbox_stbox()
 */
bool
front_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_Z(T_STBOX, box1->flags) ||
      ! ensure_has_Z(T_STBOX, box2->flags))
    return false;
  return (box1->zmax < box2->zmin);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * back of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overfront_stbox_stbox()
 */
bool
overfront_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_Z(T_STBOX, box1->flags) ||
      ! ensure_has_Z(T_STBOX, box2->flags))
    return false;
  return (box1->zmax <= box2->zmax);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is at the back of the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Back_stbox_stbox()
 */
bool
back_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_Z(T_STBOX, box1->flags) ||
      ! ensure_has_Z(T_STBOX, box2->flags))
    return false;
  return (box1->zmin > box2->zmax);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * front of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overback_stbox_stbox()
 */
bool
overback_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_Z(T_STBOX, box1->flags) ||
      ! ensure_has_Z(T_STBOX, box2->flags))
    return false;
  return (box1->zmin >= box2->zmin);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is before the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Before_stbox_stbox()
 */
bool
before_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_T(T_STBOX, box1->flags) ||
      ! ensure_has_T(T_STBOX, box2->flags))
    return false;
  return left_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is not after the second
 * one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overbefore_stbox_stbox()
 */
bool
overbefore_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_T(T_STBOX, box1->flags) ||
      ! ensure_has_T(T_STBOX, box2->flags))
    return false;
  return overleft_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is after the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #After_stbox_stbox()
 */
bool
after_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_T(T_STBOX, box1->flags) ||
      ! ensure_has_T(T_STBOX, box2->flags))
    return false;
  return right_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_geo_box_pos
 * @brief Return true if the first spatiotemporal box is not before the second
 * one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overafter_stbox_stbox()
 */
bool
overafter_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pos_stbox_stbox(box1, box2) ||
      ! ensure_has_T(T_STBOX, box1->flags) ||
      ! ensure_has_T(T_STBOX, box2->flags))
    return false;
  return overright_span_span(&box1->period, &box2->period);
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * @ingroup meos_geo_box_set
 * @brief Return the union of the spatiotemporal boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @param[in] strict True when the boxes must overlap
 * @csqlfn #Union_stbox_stbox()
 */
STBox *
union_stbox_stbox(const STBox *box1, const STBox *box2, bool strict)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, NULL); VALIDATE_NOT_NULL(box2, NULL);
  if (! ensure_same_geodetic(box1->flags, box2->flags) ||
      ! ensure_same_dimensionality(box1->flags, box2->flags) ||
      ! ensure_same_srid(box1->srid, box2->srid))
    return NULL;
  /* If the strict parameter is true, we need to ensure that the boxes
   * intersect, otherwise their union cannot be represented by a box */
  if (strict && ! overlaps_stbox_stbox(box1, box2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Result of box union would not be contiguous");
    return NULL;
  }

  STBox *result = stbox_copy(box1);
  stbox_expand(box2, result);
  return result;
}

/**
 * @ingroup meos_internal_box_set
 * @brief Return in the last argument the intersection of two
 * spatiotemporal boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @param[out] result Result
 * @note This function is equivalent to @ref intersection_stbox_stbox without
 * memory allocation
 */
bool
inter_stbox_stbox(const STBox *box1, const STBox *box2, STBox *result)
{
  assert(box1); assert(box2);
  bool hasx = MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags);
  bool hasz = MEOS_FLAGS_GET_Z(box1->flags) && MEOS_FLAGS_GET_Z(box2->flags);
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box1->flags) && MEOS_FLAGS_GET_GEODETIC(box2->flags);
  /* If there is no common dimension */
  if ((! hasx && ! hast) ||
    /* If they do no intersect in one common dimension */
    (hasx && (box1->xmin > box2->xmax || box2->xmin > box1->xmax ||
      box1->ymin > box2->ymax || box2->ymin > box1->ymax)) ||
    (hasz && (box1->zmin > box2->zmax || box2->zmin > box1->zmax)) ||
    (hast && ! overlaps_span_span(&box1->period, &box2->period)))
    return false;

  if (hasx)
  {
    assert(MEOS_FLAGS_GET_GEODETIC(box1->flags) ==
      MEOS_FLAGS_GET_GEODETIC(box2->flags));
    assert(stbox_srid(box1) == stbox_srid(box2));
  }
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  Span period;
  if (hasx)
  {
    xmin = Max(box1->xmin, box2->xmin);
    xmax = Min(box1->xmax, box2->xmax);
    ymin = Max(box1->ymin, box2->ymin);
    ymax = Min(box1->ymax, box2->ymax);
    if (hasz)
    {
      zmin = Max(box1->zmin, box2->zmin);
      zmax = Min(box1->zmax, box2->zmax);
    }
  }
  /* We are sure that the intersection is not NULL */
  if (hast)
    inter_span_span(&box1->period, &box2->period, &period);

  stbox_set(hasx, hasz, geodetic, box1->srid, xmin, xmax, ymin, ymax,
     zmin, zmax, hast ? &period : NULL, result);
  return true;
}

/**
 * @ingroup meos_geo_box_set
 * @brief Return the intersection of the spatiotemporal boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Intersection_stbox_stbox()
 */
STBox *
intersection_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, NULL); VALIDATE_NOT_NULL(box2, NULL);
  if (! ensure_same_geodetic(box1->flags, box2->flags) ||
      // ! ensure_same_dimensionality(box1->flags, box2->flags) ||
      ! ensure_same_srid(box1->srid, box2->srid))
    return NULL;

  STBox *result = palloc(sizeof(STBox));
  if (! inter_stbox_stbox(box1, box2, result))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_box_transf
 * @brief Return a spatiotemporal box split with respect to its space bounds
 * in four quadrants (2D) or eight octants (3D)
 * @details
 * The quadrants/octants are numbered as follows
 * @code
 *   (front)        (back if has Z dimension)
 * -------------   -------------
 * |  2  |  3  |   |  6  |  7  |
 * ------------- + -------------
 * |  0  |  1  |   |  4  |  5  |
 * -------------   -------------
 * @endcode
 * @param[in] box Spatiotemporal box
 * @param[out] count Number of elements in the output array
 * @csqlfn #Stbox_quad_split()
 */
STBox *
stbox_quad_split(const STBox *box, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_has_X(T_STBOX, box->flags))
    return NULL;

  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);
  Span *period = hast ? (Span *) &box->period : NULL;
  *count = hasz ? 8 : 4;
  STBox *result = palloc(sizeof(STBox) * (*count));
  double deltax = (box->xmax - box->xmin) / 2.0;
  double deltay = (box->ymax - box->ymin) / 2.0;
  double deltaz = hasz ? (box->zmax - box->zmin) / 2.0 : 0.0;
  if (hasz)
  {
    /* Front */
    stbox_set(true, hasz, geodetic, box->srid, box->xmin, box->xmin + deltax,
      box->ymin, box->ymin + deltay, box->zmin, box->zmin + deltaz,
      period, &result[0]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin + deltax,
      box->xmax, box->ymin, box->ymin + deltay, box->zmin, box->zmin + deltaz,
      period, &result[1]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin, box->xmin + deltax,
      box->ymin + deltay, box->ymax, box->zmin, box->zmin + deltaz,
      period, &result[2]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin + deltax, box->xmax,
      box->ymin + deltay, box->ymax, box->zmin, box->zmin + deltaz,
      period, &result[3]);
    /* Back */
    stbox_set(true, hasz, geodetic, box->srid, box->xmin, box->xmin + deltax,
      box->ymin, box->ymin + deltay, box->zmin + deltaz, box->zmax,
      period, &result[4]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin + deltax,
      box->xmax, box->ymin, box->ymin + deltay, box->zmin + deltaz, box->zmax,
      period, &result[5]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin, box->xmin + deltax,
      box->ymin + deltay, box->ymax, box->zmin + deltaz, box->zmax,
      period, &result[6]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin + deltax, box->xmax,
      box->ymin + deltay, box->ymax, box->zmin + deltaz, box->zmax,
      period , &result[7]);
  }
  else
  {
    stbox_set(true, hasz, geodetic, box->srid, box->xmin, box->xmin + deltax,
      box->ymin, box->ymin + deltay, 0.0, 0.0, period, &result[0]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin + deltax,
      box->xmax, box->ymin, box->ymin + deltay, 0.0, 0.0, period, &result[1]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin, box->xmin + deltax,
      box->ymin + deltay, box->ymax, 0.0, 0.0, period, &result[2]);
    stbox_set(true, hasz, geodetic, box->srid, box->xmin + deltax, box->xmax,
      box->ymin + deltay, box->ymax, 0.0, 0.0, period , &result[3]);
  }
  return result;
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_geo_box_comp
 * @brief Return true if the spatiotemporal boxes are equal
 * @param[in] box1,box2 Spatiotemporal boxes
 * @note The function #stbox_cmp is not used to increase efficiency
 * @csqlfn #Stbox_eq()
 */
bool
stbox_eq(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, false); VALIDATE_NOT_NULL(box2, false);
  if (box1->flags != box2->flags ||
      box1->xmin != box2->xmin || box1->ymin != box2->ymin ||
      box1->zmin != box2->zmin || box1->xmax != box2->xmax ||
      box1->ymax != box2->ymax || box1->zmax != box2->zmax ||
      box1->srid != box2->srid || ! span_eq(&box1->period, &box2->period))
    return false;
  /* The two boxes are equal */
  return true;
}

/**
 * @ingroup meos_geo_box_comp
 * @brief Return true if the spatiotemporal boxes are different
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_ne()
 */
inline bool
stbox_ne(const STBox *box1, const STBox *box2)
{
  return ! stbox_eq(box1, box2);
}

/**
 * @ingroup meos_geo_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first spatiotemporal
 * box is less than, equal to, or greater than the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_cmp()
 */
int
stbox_cmp(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  assert(box1); assert(box2);

  /* Compare the SRID */
  if (box1->srid < box2->srid)
    return -1;
  if (box1->srid > box2->srid)
    return 1;

  bool hasx, hasz, hast, geodetic;
  stbox_stbox_flags(box1, box2, &hasx, &hasz, &hast, &geodetic);
  if (hast)
  {
    int cmp = span_cmp(&box1->period, &box2->period);
    /* Compare the box minima */
    if (cmp != 0)
      return cmp;
  }
  if (hasx)
  {
    /* Compare the box minima */
    if (box1->xmin < box2->xmin)
      return -1;
    if (box1->xmin > box2->xmin)
      return 1;
    if (box1->ymin < box2->ymin)
      return -1;
    if (box1->ymin > box2->ymin)
      return 1;
    if (hasz)
    {
      if (box1->zmin < box2->zmin)
        return -1;
      if (box1->zmin > box2->zmin)
        return 1;
    }
    /* Compare the box maxima */
    if (box1->xmax < box2->xmax)
      return -1;
    if (box1->xmax > box2->xmax)
      return 1;
    if (box1->ymax < box2->ymax)
      return -1;
    if (box1->ymax > box2->ymax)
      return 1;
    if (hasz)
    {
      if (box1->zmax < box2->zmax)
        return -1;
      if (box1->zmax > box2->zmax)
        return 1;
    }
  }
  /* Finally compare the flags */
  if (box1->flags < box2->flags)
    return -1;
  if (box1->flags > box2->flags)
    return 1;
  /* The two boxes are equal */
  return 0;
}

/**
 * @ingroup meos_geo_box_comp
 * @brief Return true if the first spatiotemporal box is less than the second
 * one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_lt()
 */
inline bool
stbox_lt(const STBox *box1, const STBox *box2)
{
  return stbox_cmp(box1, box2) < 0;
}

/**
 * @ingroup meos_geo_box_comp
 * @brief Return true if the first spatiotemporal box is less than or equal to
 * the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_le()
 */
inline bool
stbox_le(const STBox *box1, const STBox *box2)
{
  return stbox_cmp(box1, box2) <= 0;
}

/**
 * @ingroup meos_geo_box_comp
 * @brief Return true if the first spatiotemporal box is greater than or equal
 * to the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_ge()
 */
inline bool
stbox_ge(const STBox *box1, const STBox *box2)
{
  return stbox_cmp(box1, box2) >= 0;
}

/**
 * @ingroup meos_geo_box_comp
 * @brief Return true if the first spatiotemporal box is greater than the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_gt()
 */
inline bool
stbox_gt(const STBox *box1, const STBox *box2)
{
  return stbox_cmp(box1, box2) > 0;
}

/*****************************************************************************/

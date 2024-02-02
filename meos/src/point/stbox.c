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
 * @brief Functions for spatiotemporal bounding boxes
 */

#include "point/stbox.h"

/* C */
#include <assert.h>
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
#include "general/type_util.h"
#include "point/pgis_types.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"

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
 * @pre No tests are made concerning the srid, dimensionality, etc.
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
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that the spatiotemporal box has XY dimension
 */
bool
ensure_has_X_stbox(const STBox *box)
{
  if (! MEOS_FLAGS_GET_X(box->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The box must have space dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the spatiotemporal box has T dimension
 */
bool
ensure_has_T_stbox(const STBox *box)
{
  if (! MEOS_FLAGS_GET_T(box->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The box must have time dimension");
    return false;
  }
  return true;
}


/*****************************************************************************
 * Input/ouput functions in string format
 *****************************************************************************/

/**
 * @ingroup meos_box_inout
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return stbox_parse(&str);
}

/**
 * @ingroup meos_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
stbox_out(const STBox *box, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_negative(maxdd))
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
  char srid[20];
  if (hasx && box->srid > 0)
    sprintf(srid, "SRID=%d;", box->srid);
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
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_box_constructor
 * @brief Return a spatiotemporal box from the arguments
 * @param[in] hasx True if the values for the spatial dimension are givne
 * @param[in] hasz True if there is a Z dimension
 * @param[in] geodetic True if geodetic
 * @param[in] srid SRID
 * @param[in] xmin,ymin,zmin Minimum bounds for the spatial dimension
 * @param[in] xmax,ymax,zmax Maximum bounds for the spatial dimension
 * @param[in] s Span
 * @csqlfn #Stbox_constructor()
 */
STBox *
stbox_make(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin,
  double xmax, double ymin, double ymax, double zmin, double zmax,
  const Span *s)
{
  /* Note: zero-fill is done in function stbox_set */
  STBox *result = palloc(sizeof(STBox));
  stbox_set(hasx, hasz, geodetic, srid, xmin, xmax, ymin, ymax, zmin, zmax, s,
    result);
  return result;
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return the last argument initialized with a spatiotemporal box
 * constructed from the other arguments
 * @param[in] hasx True if the values for the spatial dimension are givne
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
 * @ingroup meos_internal_box_constructor
 * @brief Return a copy of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 */
STBox *
stbox_cp(const STBox *box)
{
  assert(box);
  STBox *result = palloc(sizeof(STBox));
  memcpy(result, box, sizeof(STBox));
  return result;
}

#if MEOS
/**
 * @ingroup meos_box_constructor
 * @brief Return a copy of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 */
STBox *
stbox_copy(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return NULL;
  return stbox_cp(box);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_box_constructor
 * @brief Return a spatiotemporal box from a geometry/geography and a
 * timestamptz
 * @param[in] gs Geometry/geography
 * @param[in] t Timestamp
 * @csqlfn #Stbox_constructor()
 */
STBox *
geo_timestamptz_to_stbox(const GSERIALIZED *gs, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs))
    return NULL;

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
 * @ingroup meos_box_constructor
 * @brief Return a spatiotemporal box from a geometry/geography and a
 * timestamptz span
 * @param[in] gs Geometry/geography
 * @param[in] s Span
 * @csqlfn #Stbox_constructor()
 */
STBox *
geo_tstzspan_to_stbox(const GSERIALIZED *gs, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;

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
 * @brief Return the last argument initialized with a @p GBOX contructed from a
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
 * @brief Return the last argument initialized with a @p BOX3D contructed from
 * a spatiotemporal box
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
 * @ingroup meos_box_conversion
 * @brief Return a spatiotemporal box converted to a @p GBOX
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_box2d()
 */
GBOX *
stbox_to_gbox(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box))
    return NULL;

  GBOX *result = palloc(sizeof(GBOX));
  stbox_set_gbox(box, result);
  return result;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a spatiotemporal box converted to a @p BOX3D
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_box3d()
 */
BOX3D *
stbox_to_box3d(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box) ||
      /* box3d does not have flags */
      ! ensure_not_geodetic(box->flags))
    return NULL;

  BOX3D *result = palloc(sizeof(BOX3D));
  stbox_set_box3d(box, result);
  return result;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a spatiotemporal box converted as a geometry/geography
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_geo()
 */
GSERIALIZED *
stbox_to_geo(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box))
    return NULL;

  LWGEOM *geo;
  GSERIALIZED *result;
  BOX3D box3d;
  STBox box1;
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);
  if (geodetic)
  {
    /* Transform the stbox coordinates, expressed as cartesian coordinates on
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
 * @ingroup meos_box_conversion
 * @brief Return a spatiotemporal box converted to a timestamptz span
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_to_tstzspan()
 */
Span *
stbox_to_tstzspan(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_T_stbox(box))
    return NULL;
  return span_cp(&box->period);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with the bounding box of a
 * spatial set
 * @param[in] s Set
 * @param[out] box Spatiotemporal box
 */
void
spatialset_set_stbox(const Set *s, STBox *box)
{
  assert(s); assert(box); assert(spatialset_type(s->settype));
  memset(box, 0, sizeof(STBox));
  memcpy(box, SET_BBOX_PTR(s), sizeof(STBox));
  return;
}

/**
 * @ingroup meos_box_conversion
 * @brief Return a spatial set converted to a spatiotemporal box
 * @param[in] s Set
 * @csqlfn #Geoset_to_stbox(), #Npointset_to_stbox()
 */
STBox *
spatialset_to_stbox(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_spatialset_type(s->settype))
    return NULL;
  STBox *result = palloc(sizeof(STBox));
  spatialset_set_stbox(s, result);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return a @p GBOX converted to a spatiotemporal box
 * @param[in] box GBOX
 */
STBox *
gbox_to_stbox(const GBOX *box)
{
  assert(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  STBox *result = palloc0(sizeof(STBox));
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
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return a @p BOX3D converted to a spatiotemporal box
 * @param[out] box BOX3D
 */
STBox *
box3d_to_stbox(const BOX3D *box)
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
 * @brief Return the last argument initialized with a spatiotemporal box
 * constructed from a geometry/geography
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
 * @ingroup meos_box_conversion
 * @brief Return a geometry/geography converted to a spatiotemporal box
 * @param[in] gs Geometry/geography
 * @csqlfn #Geo_to_stbox()
 */
STBox *
geo_to_stbox(const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_empty(gs))
    return NULL;

  STBox *result = palloc(sizeof(STBox));
  geo_set_stbox(gs, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a spatiotemporal box
 * constructed from an array of geometries/geographies
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
 * @brief Return the last argument initialized with a spatiotemporal box
 * constructed from a timestamptz
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
 * @ingroup meos_box_conversion
 * @brief return a timestamptz converted to a spatiotemporal box
 * @param[in] t Timestamp
 * @csqlfn #Timestamptz_to_stbox()
 */
STBox *
timestamptz_to_stbox(TimestampTz t)
{
  STBox *result = palloc(sizeof(STBox));
  timestamptz_set_stbox(t, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a spatiotemporal box
 * constructed from a timestamptz set
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
 * @ingroup meos_box_conversion
 * @brief Return a timestamptz set converted to a spatiotemporal box
 * @param[in] s Set
 * @csqlfn #Tstzset_to_stbox()
 */
STBox *
tstzset_to_stbox(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  STBox *result = palloc(sizeof(STBox));
  tstzset_set_stbox(s, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a spatiotemporal box
 * constructed from a timestamptz span
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
 * @ingroup meos_box_conversion
 * @brief return a timestamptz span converted to a spatiotemporal box
 * @param[in] s Span
 * @csqlfn #Tstzspan_to_stbox()
 */
STBox *
tstzspan_to_stbox(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  STBox *result = palloc(sizeof(STBox));
  tstzspan_set_stbox(s, result);
  return result;
}

/**
 * @ingroup meos_internal_box_conversion
 * @brief Return the last argument initialized with a spatiotemporal box
 * constructed from a timestamptz span set
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
 * @ingroup meos_box_conversion
 * @brief Return a timestamptz span set converted to a spatiotemporal box
 * @param[in] ss Span set
 * @csqlfn #Tstzspanset_to_stbox()
 */
STBox *
tstzspanset_to_stbox(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  STBox *result = palloc(sizeof(STBox));
  tstzspanset_set_stbox(ss, result);
  return result;
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_box_accessor
 * @brief Return true if a spatiotemporal box has value dimension
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_hasx()
 */
bool
stbox_hasx(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  return MEOS_FLAGS_GET_X(box->flags);
}

/**
 * @ingroup meos_box_accessor
 * @brief Return true if a spatiotemporal box has Z dimension
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_hasz()
 */
bool
stbox_hasz(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  return MEOS_FLAGS_GET_Z(box->flags);
}

/**
 * @ingroup meos_box_accessor
 * @brief Return true if a spatiotemporal box has time dimension
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_hast()
 */
bool
stbox_hast(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  return MEOS_FLAGS_GET_T(box->flags);
}

/**
 * @ingroup meos_box_accessor
 * @brief Return true if a spatiotemporal box is geodetic
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_isgeodetic()
 */
bool
stbox_isgeodetic(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box))
    return false;
  return MEOS_FLAGS_GET_GEODETIC(box->flags);
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmin;
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmax;
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymin;
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymax;
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmin;
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmax;
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.lower);
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = box->period.lower_inc;
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.upper);
  return true;
}

/**
 * @ingroup meos_box_accessor
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) result))
    return false;

  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = box->period.upper_inc;
  return true;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_box_transf
 * @brief Return a spatiotemporal box with the time span expanded and/or scaled
 * by two intervals
 * @param[in] box Spatiotemporal box
 * @param[in] shift Interval to shift the value span, may be @p NULL
 * @param[in] duration Duration of the result, may be @p NULL
 * @csqlfn #Stbox_shift_time(), #Stbox_scale_time(), #Stbox_shift_scale_time()
 */
STBox *
stbox_shift_scale_time(const STBox *box, const Interval *shift,
  const Interval *duration)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_T_stbox(box) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Copy the input period to the result */
  STBox *result = stbox_cp(box);
  /* Shift and/or scale the resulting period */
  TimestampTz lower = DatumGetTimestampTz(box->period.lower);
  TimestampTz upper = DatumGetTimestampTz(box->period.upper);
  lower_upper_shift_scale_time(shift, duration, &lower, &upper);
  result->period.lower = TimestampTzGetDatum(lower);
  result->period.upper = TimestampTzGetDatum(upper);
  return result;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a spatiotemporal box with only the space dimension
 * @csqlfn #Stbox_get_space()
 * @param[in] box Spatiotemporal box
 */
STBox *
stbox_get_space(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box))
    return NULL;

  STBox *result = palloc(sizeof(STBox));
  stbox_set(true, MEOS_FLAGS_GET_Z(box->flags),
    MEOS_FLAGS_GET_GEODETIC(box->flags), box->srid, box->xmin, box->xmax,
    box->ymin, box->ymax, box->zmin, box->zmax, NULL, result);
  return result;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a spatiotemporal box with the space bounds expanded by a
 * double
 * @param[in] box Spatiotemporal box
 * @param[in] d Value for expanding
 * @csqlfn #Stbox_expand_space()
 */
STBox *
stbox_expand_space(const STBox *box, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box))
    return NULL;

  STBox *result = stbox_cp(box);
  result->xmin -= d;
  result->ymin -= d;
  result->xmax += d;
  result->ymax += d;
  if (MEOS_FLAGS_GET_Z(box->flags) || MEOS_FLAGS_GET_GEODETIC(box->flags))
  {
    result->zmin -= d;
    result->zmax += d;
  }
  return result;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a spatiotemporal box with the time span expanded by an
 * interval
 * @param[in] box Spatiotemporal box
 * @param[in] interv Interval for expanding
 * @csqlfn #Stbox_expand_time()
 */
STBox *
stbox_expand_time(const STBox *box, const Interval *interv)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) interv) ||
      ! ensure_has_T_stbox(box))
    return NULL;

  STBox *result = stbox_cp(box);
  TimestampTz tmin = minus_timestamptz_interval(DatumGetTimestampTz(
    box->period.lower), interv);
  TimestampTz tmax = add_timestamptz_interval(DatumGetTimestampTz(
    box->period.upper), interv);
  result->period.lower = TimestampTzGetDatum(tmin);
  result->period.upper = TimestampTzGetDatum(tmax);
  return result;
}

/*****************************************************************************
 * Topological operators
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
  /* Ensure validity of the arguments */
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
 * @ingroup meos_box_bbox_topo
 * @brief Return true if the first spatiotemporal box contains the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Contains_stbox_stbox()
 */
bool
contains_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
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
 * @ingroup meos_box_bbox_topo
 * @brief Return true if the first spatiotemporal box is contained in the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Contained_stbox_stbox()
 */
bool
contained_stbox_stbox(const STBox *box1, const STBox *box2)
{
  return contains_stbox_stbox(box2, box1);
}

/**
 * @ingroup meos_box_bbox_topo
 * @brief Return true if the spatiotemporal boxes overlap
 * @csqlfn #Overlaps_stbox_stbox()
 */
bool
overlaps_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
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
 * @ingroup meos_box_bbox_topo
 * @brief Return true if the spatiotemporal boxes are equal in the common
 * dimensions
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Same_stbox_stbox()
 */
bool
same_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
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
 * @ingroup meos_box_bbox_topo
 * @brief Return true if the spatiotemporal boxes are adjacent
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Adjacent_stbox_stbox()
 */
bool
adjacent_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic))
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
pos_stbox_stbox_test(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_same_geodetic(box1->flags, box2->flags) ||
      ! ensure_same_srid(stbox_srid(box1), stbox_srid(box2)))
    return false;
  return true;
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is to the left of the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Left_stbox_stbox()
 */
bool
left_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->xmax < box2->xmin);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * right of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overleft_stbox_stbox()
 */
bool
overleft_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->xmax <= box2->xmax);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is to the right of the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Right_stbox_stbox()
 */
bool
right_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->xmin > box2->xmax);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * left of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overright_stbox_stbox()
 */
bool
overright_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->xmin >= box2->xmin);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is below the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Below_stbox_stbox()
 */
bool
below_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->ymax < box2->ymin);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box does not extend above the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overbelow_stbox_stbox()
 */
bool
overbelow_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->ymax <= box2->ymax);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is above the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Above_stbox_stbox()
 */
bool
above_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->ymin > box2->ymax);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box does not extend below the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overabove_stbox_stbox()
 */
bool
overabove_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->ymin >= box2->ymin);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is in front of the
 * the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Front_stbox_stbox()
 */
bool
front_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_Z(box1->flags) || ! ensure_has_Z(box2->flags) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->zmax < box2->zmin);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * back of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overfront_stbox_stbox()
 */
bool
overfront_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_Z(box1->flags) || ! ensure_has_Z(box2->flags) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->zmax <= box2->zmax);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is at the back of the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Back_stbox_stbox()
 */
bool
back_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_Z(box1->flags) || ! ensure_has_Z(box2->flags) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->zmin > box2->zmax);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * front of the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overback_stbox_stbox()
 */
bool
overback_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_Z(box1->flags) || ! ensure_has_Z(box2->flags) ||
      ! pos_stbox_stbox_test(box1, box2))
    return false;
  return (box1->zmin >= box2->zmin);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is before the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Before_stbox_stbox()
 */
bool
before_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_stbox(box1) || ! ensure_has_T_stbox(box2))
    return false;
  return lf_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is not after the second
 * one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overbefore_stbox_stbox()
 */
bool
overbefore_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_stbox(box1) || ! ensure_has_T_stbox(box2))
    return false;
  return ovlf_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is after the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #After_stbox_stbox()
 */
bool
after_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_stbox(box1) || ! ensure_has_T_stbox(box2))
    return false;
  return ri_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup meos_box_bbox_pos
 * @brief Return true if the first spatiotemporal box is not before the second
 * one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Overafter_stbox_stbox()
 */
bool
overafter_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_T_stbox(box1) || ! ensure_has_T_stbox(box2))
    return false;
  return ovri_span_span(&box1->period, &box2->period);
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * @ingroup meos_box_set
 * @brief Return the union of the spatiotemporal boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @param[in] strict True when the boxes must overlap
 * @csqlfn #Union_stbox_stbox()
 */
STBox *
union_stbox_stbox(const STBox *box1, const STBox *box2, bool strict)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_same_geodetic(box1->flags, box2->flags) ||
      ! ensure_same_dimensionality(box1->flags, box2->flags) ||
      ! ensure_same_srid_stbox(box1, box2))
    return NULL;
  /* If the strict parameter is true, we need to ensure that the boxes
   * intersect, otherwise their union cannot be represented by a box */
  if (strict && ! overlaps_stbox_stbox(box1, box2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Result of box union would not be contiguous");
    return NULL;
  }

  STBox *result = stbox_cp(box1);
  stbox_expand(box2, result);
  return result;
}

/**
 * @ingroup meos_internal_box_set
 * @brief Return the last argument initialized with the intersection of two
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
    (hast && ! over_span_span(&box1->period, &box2->period)))
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
 * @ingroup meos_box_set
 * @brief Return the intersection of the spatiotemporal boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Intersection_stbox_stbox()
 */
STBox *
intersection_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_same_geodetic(box1->flags, box2->flags) ||
      // ! ensure_same_dimensionality(box1->flags, box2->flags) ||
      ! ensure_same_srid_stbox(box1, box2))
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
 * @ingroup meos_box_transf
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) count) ||
      ! ensure_has_X_stbox(box))
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
 * @ingroup meos_box_comp
 * @brief Return true if the spatiotemporal boxes are equal
 * @param[in] box1,box2 Spatiotemporal boxes
 * @note The function #stbox_cmp is not used to increase efficiency
 * @csqlfn #Stbox_eq()
 */
bool
stbox_eq(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2))
    return false;

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
 * @ingroup meos_box_comp
 * @brief Return true if the spatiotemporal boxes are different
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_ne()
 */
bool
stbox_ne(const STBox *box1, const STBox *box2)
{
  return ! stbox_eq(box1, box2);
}

/**
 * @ingroup meos_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first spatiotemporal
 * box is less than, equal to, or greater than the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_cmp()
 */
int
stbox_cmp(const STBox *box1, const STBox *box2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2))
    return INT_MAX;

  /* Compare the SRID */
  if (box1->srid < box2->srid)
    return -1;
  if (box1->srid > box2->srid)
    return 1;

  bool hasx, hasz, hast, geodetic;
  stbox_stbox_flags(box1, box2, &hasx, &hasz, &hast, &geodetic);
  if (hast)
  {
    int cmp = span_cmp_int(&box1->period, &box2->period);
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
 * @ingroup meos_box_comp
 * @brief Return true if the first spatiotemporal box is less than the second
 * one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_lt()
 */
bool
stbox_lt(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp < 0;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if the first spatiotemporal box is less than or equal to
 * the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_le()
 */
bool
stbox_le(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp <= 0;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if the first spatiotemporal box is greater than or equal
 * to the second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_ge()
 */
bool
stbox_ge(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp >= 0;
}

/**
 * @ingroup meos_box_comp
 * @brief Return true if the first spatiotemporal box is greater than the
 * second one
 * @param[in] box1,box2 Spatiotemporal boxes
 * @csqlfn #Stbox_gt()
 */
bool
stbox_gt(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp > 0;
}

/*****************************************************************************/

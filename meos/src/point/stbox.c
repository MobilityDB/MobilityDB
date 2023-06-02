/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Functions for spatiotemporal bounding boxes.
 */

#include "point/stbox.h"

/* C */
#include <assert.h>
/* PostGIS */
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/temporal.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_util.h"
#include "point/pgis_call.h"
#include "point/tpoint.h"
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
 * @ingroup libmeos_box_transf
 * @brief Expand the second spatiotemporal box with the first one.
 * @pre No tests are made concerning the srid, dimensionality, etc.
 * This should be ensured by the calling function.
 */
void
stbox_expand(const STBox *box1, STBox *box2)
{
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
 * @brief Ensure that the temporal value has XY dimension
 */
void
ensure_has_X_stbox(const STBox *box)
{
  if (! MEOS_FLAGS_GET_X(box->flags))
    elog(ERROR, "The box must have space dimension");
  return;
}

/**
 * @brief Ensure that the temporal value has T dimension
 */
void
ensure_has_T_stbox(const STBox *box)
{
  if (! MEOS_FLAGS_GET_T(box->flags))
    elog(ERROR, "The box must have time dimension");
  return;
}


/*****************************************************************************
 * Input/ouput functions in string format
 *****************************************************************************/

/**
 * @ingroup libmeos_box_inout
 * @brief Return a spatiotemporal box from its Well-Known Text (WKT) representation.
 *
 * Examples of input:
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
 */
STBox *
stbox_in(const char *str)
{
  return stbox_parse(&str);
}

/**
 * @ingroup libmeos_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatiotemporal box.
 */
char *
stbox_out(const STBox *box, int maxdd)
{
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
 * @ingroup libmeos_box_constructor
 * @brief Construct a spatiotemporal box from the arguments.
 * @sqlfunc stbox()
 */
STBox *
stbox_make(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin,
  double xmax, double ymin, double ymax, double zmin, double zmax,
  const Span *p)
{
  /* Note: zero-fill is done in function stbox_set */
  STBox *result = palloc(sizeof(STBox));
  stbox_set(hasx, hasz, geodetic, srid, xmin, xmax, ymin, ymax, zmin, zmax, p,
    result);
  return result;
}

/**
 * @ingroup libmeos_internal_box_constructor
 * @brief Set a spatiotemporal box from the arguments.
 * @note This function is equivalent to @ref stbox_make without memory
 * allocation
 */
void
stbox_set(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin,
  double xmax, double ymin, double ymax, double zmin, double zmax,
  const Span *p, STBox *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  MEOS_FLAGS_SET_X(box->flags, hasx);
  MEOS_FLAGS_SET_Z(box->flags, hasz);
  MEOS_FLAGS_SET_GEODETIC(box->flags, geodetic);
  box->srid = srid;

  if (p)
  {
    /* Process T min/max */
    memcpy(&box->period, p, sizeof(Span));
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
 * @ingroup libmeos_box_constructor
 * @brief Return a copy of a spatiotemporal box.
 */
STBox *
stbox_copy(const STBox *box)
{
  STBox *result = palloc(sizeof(STBox));
  memcpy(result, box, sizeof(STBox));
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a spatiotemporal box from a geometry/geography and a timestamp.
 * @sqlfunc stbox()
 */
STBox *
geo_timestamp_to_stbox(const GSERIALIZED *gs, TimestampTz t)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBox *result = palloc(sizeof(STBox));
  geo_set_stbox(gs, result);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, &result->period);
  MEOS_FLAGS_SET_T(result->flags, true);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a spatiotemporal box from a geometry/geography and a period
 * @sqlfunc stbox()
 */
STBox *
geo_period_to_stbox(const GSERIALIZED *gs, const Span *p)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBox *result = palloc(sizeof(STBox));
  geo_set_stbox(gs, result);
  memcpy(&result->period, p, sizeof(Span));
  MEOS_FLAGS_SET_T(result->flags, true);
  return result;
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_box_cast
 * @brief Set a PostGIS GBOX from a spatiotemporal box.
 * @sqlop @p ::
 */
void
stbox_set_gbox(const STBox *box, GBOX *gbox)
{
  ensure_has_X_stbox(box);
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
 * @ingroup libmeos_internal_box_cast
 * @brief Set a PostGIS BOX3D from a spatiotemporal box
 * @sqlop @p ::
 */
void
stbox_set_box3d(const STBox *box, BOX3D *box3d)
{
  ensure_has_X_stbox(box);
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
 * @ingroup libmeos_box_cast
 * @brief Cast a spatiotemporal box as a PostGIS geometry
 * @sqlop @p ::
 */
GSERIALIZED *
stbox_to_geo(const STBox *box)
{
  ensure_has_X_stbox(box);
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
 * @ingroup libmeos_box_cast
 * @brief Cast a temporal box as a period
 * @sqlop @p ::
 */
Span *
stbox_to_period(const STBox *box)
{
  if (! MEOS_FLAGS_GET_T(box->flags))
    return NULL;
  return span_copy(&box->period);
}

/*****************************************************************************
 * Transform a <Type> to a STBox
 * The functions assume set the argument box to 0
 *****************************************************************************/

/**
 * @brief Get the coordinates from a geometry/geography point.
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
 * @ingroup libmeos_internal_box_cast
 * @brief Set a spatiotemporal box from a geometry/geography.
 */
bool
geo_set_stbox(const GSERIALIZED *gs, STBox *box)
{
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
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  GBOX gbox;
  memset(&gbox, 0, sizeof(GBOX));
  /* We are sure that the geometry/geography is not empty
   * We cannot use `lwgeom_calculate_gbox` since for geography it calculates
   * a geodetic box where the coordinates are expressed in the unit sphere
   */
  lwgeom_calculate_gbox_cartesian(lwgeom, &gbox);
  lwgeom_free(lwgeom);
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
 * @ingroup libmeos_box_cast
 * @brief Cast a geometry/geography to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBox *
geo_to_stbox(const GSERIALIZED *gs)
{
  STBox *result = palloc(sizeof(STBox));
  geo_set_stbox(gs, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_cast
 * @brief Set the spatiotemporal box from an array of geometries/geographies
 * @param[in] values Values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
geoarr_set_stbox(const Datum *values, int count, STBox *box)
{
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
 * @ingroup libmeos_internal_box_cast
 * @brief Set a spatiotemporal box from a timestamp.
 */
void
timestamp_set_stbox(TimestampTz t, STBox *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, &box->period);
  MEOS_FLAGS_SET_X(box->flags, false);
  MEOS_FLAGS_SET_Z(box->flags, false);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a timestamp to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBox *
timestamp_to_stbox(TimestampTz t)
{
  STBox *result = palloc(sizeof(STBox));
  timestamp_set_stbox(t, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_cast
 * @brief Set a spatiotemporal box from a timestamp set.
 */
void
timestampset_set_stbox(const Set *ts, STBox *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  set_set_span(ts, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a timestamp set to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBox *
timestampset_to_stbox(const Set *ts)
{
  STBox *result = palloc(sizeof(STBox));
  timestampset_set_stbox(ts, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_cast
 * @brief Set a spatiotemporal box from a period.
 */
void
period_set_stbox(const Span *p, STBox *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  memcpy(&box->period, p, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a period to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBox *
period_to_stbox(const Span *p)
{
  STBox *result = palloc(sizeof(STBox));
  period_set_stbox(p, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_box_cast
 * @brief Set a spatiotemporal box from a period set.
 */
void
periodset_set_stbox(const SpanSet *ps, STBox *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  memcpy(&box->period, &ps->span, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a period set to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBox *
periodset_to_stbox(const SpanSet *ps)
{
  STBox *result = palloc(sizeof(STBox));
  periodset_set_stbox(ps, result);
  return result;
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box has value dimension
 * @sqlfunc hasX()
 */
bool
stbox_hasx(const STBox *box)
{
  bool result = MEOS_FLAGS_GET_X(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box has Z dimension
 * @sqlfunc hasZ()
 */
bool
stbox_hasz(const STBox *box)
{
  bool result = MEOS_FLAGS_GET_Z(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box has time dimension
 * @sqlfunc hasT()
 */
bool
stbox_hast(const STBox *box)
{
  bool result = MEOS_FLAGS_GET_T(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box is geodetic
 * @sqlfunc isGeodetic()
 * @pymeosfunc geodetic()
 */
bool
stbox_isgeodetic(const STBox *box)
{
  bool result = MEOS_FLAGS_GET_GEODETIC(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the minimum X value of a spatiotemporal box
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmin()
 * @pymeosfunc xmin()
 */
bool
stbox_xmin(const STBox *box, double *result)
{
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the maximum X value of a spatiotemporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmax()
 * @pymeosfunc xmax()
 */
bool
stbox_xmax(const STBox *box, double *result)
{
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmax;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the minimum Y value of a spatiotemporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Ymin()
 * @pymeosfunc ymin()
 */
bool
stbox_ymin(const STBox *box, double *result)
{
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the maximum Y value of a spatiotemporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Ymax()
 * @pymeosfunc ymax()
 */
bool
stbox_ymax(const STBox *box, double *result)
{
  if (! MEOS_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymax;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the minimum Z value of a spatiotemporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Zmin()
 * @pymeosfunc zmin()
 */
bool
stbox_zmin(const STBox *box, double *result)
{
  if (! MEOS_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the maximum Z value of a spatiotemporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Zmax()
 * @pymeosfunc zmax()
 */
bool
stbox_zmax(const STBox *box, double *result)
{
  if (! MEOS_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmax;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the minimum timestamp of a spatiotemporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmin()
 * @pymeosfunc tmin()
 */
bool
stbox_tmin(const STBox *box, TimestampTz *result)
{
  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.lower);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Compute the maximum timestamp of a spatiotemporal box.
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmax()
 * @pymeosfunc tmax()
 */
bool
stbox_tmax(const STBox *box, TimestampTz *result)
{
  if (! MEOS_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.upper);
  return true;
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the SRID of a spatiotemporal box.
 * @sqlfunc SRID()
 * @pymeosfunc srid()
 */
int32
stbox_srid(const STBox *box)
{
  return box->srid;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Set the SRID of a spatiotemporal box.
 * @sqlfunc setSRID()
 */
STBox *
stbox_set_srid(const STBox *box, int32 srid)
{
  STBox *result = stbox_copy(box);
  result->srid = srid;
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_box_transf
 * @brief Return a copy of the spatiotemporal box keeping only the space
 * dimension
 * @sqlfunc getSpace()
 */
STBox *
stbox_get_space(const STBox *box)
{
  ensure_has_X_stbox(box);
  STBox *result = palloc(sizeof(STBox));
  stbox_set(true, MEOS_FLAGS_GET_Z(box->flags),
    MEOS_FLAGS_GET_GEODETIC(box->flags), box->srid, box->xmin, box->xmax,
    box->ymin, box->ymax, box->zmin, box->zmax, NULL, result);
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Return a spatiotemporal box expanded in the spatial dimension by a
 * double.
 * @sqlfunc expandSpace()
 */
STBox *
stbox_expand_space(const STBox *box, double d)
{
  ensure_has_X_stbox(box);
  STBox *result = stbox_copy(box);
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
 * @ingroup libmeos_box_transf
 * @brief Return a spatiotemporal box expanded in the temporal dimension by
 * an interval
 * @sqlfunc expandTime()
 */
STBox *
stbox_expand_time(const STBox *box, const Interval *interval)
{
  ensure_has_T_stbox(box);
  STBox *result = stbox_copy(box);
  TimestampTz tmin = pg_timestamp_mi_interval(DatumGetTimestampTz(
    box->period.lower), interval);
  TimestampTz tmax = pg_timestamp_pl_interval(DatumGetTimestampTz(
    box->period.upper), interval);
  result->period.lower = TimestampTzGetDatum(tmin);
  result->period.upper = TimestampTzGetDatum(tmax);
  return result;
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

/**
 * @brief Set the ouput variables with the values of the flags of the boxes.
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
 * the flags of the boxes.
 *
 * Mixing 2D/3D is enabled to compute, for example, 2.5D operations
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hasz,hast,geodetic Boolean variables
 */
static void
topo_stbox_stbox_init(const STBox *box1, const STBox *box2, bool *hasx,
  bool *hasz, bool *hast, bool *geodetic)
{
  ensure_common_dimension(box1->flags, box2->flags);
  if (MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags))
  {
    ensure_same_geodetic(box1->flags, box2->flags);
    ensure_same_srid(stbox_srid(box1), stbox_srid(box2));
  }
  stbox_stbox_flags(box1, box2, hasx, hasz, hast, geodetic);
  return;
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the first spatiotemporal box contains the second one.
 * @sqlop @p \@>
 */
bool
contains_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
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
 * @ingroup libmeos_box_topo
 * @brief Return true if the first spatiotemporal box is contained in the
 * second one
 * @sqlop @p <@
 */
bool
contained_stbox_stbox(const STBox *box1, const STBox *box2)
{
  return contains_stbox_stbox(box2, box1);
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the spatiotemporal boxes overlap
 * @sqlop @p &&
 */
bool
overlaps_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
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
 * @ingroup libmeos_box_topo
 * @brief Return true if the spatiotemporal boxes are equal in the common
 * dimensions.
 * @sqlop @p ~=
 */
bool
same_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
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
 * @ingroup libmeos_box_topo
 * @brief Return true if the spatiotemporal boxes are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_stbox_stbox(const STBox *box1, const STBox *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
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
 * @param[in] box1,box2 Input boxes
 */
static void
pos_stbox_stbox_test(const STBox *box1, const STBox *box2)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_srid(stbox_srid(box1), stbox_srid(box2));
  return;
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly to the
 * left of the second one
 * @sqlop @p <<
 */
bool
left_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmax < box2->xmin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * right of the second one
 * @sqlop @p &<
 */
bool
overleft_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmax <= box2->xmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly to the right
 * of the second one
 * @sqlop @p >>
 */
bool
right_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmin > box2->xmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatio temporal box does not extend to the
 * left of the second one.
 * @sqlop @p &>
 */
bool
overright_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmin >= box2->xmin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly below of
 * the second one.
 * @sqlop @p <<|
 */
bool
below_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymax < box2->ymin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box does not extend above of
 * the second one.
 * @sqlop @p &<|
 */
bool
overbelow_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymax <= box2->ymax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly above of the
 * second one.
 * @sqlop @p |>>
 */
bool
above_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymin > box2->ymax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box does not extend below of
 * the second one.
 * @sqlop @p |&>
 */
bool
overabove_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymin >= box2->ymin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly in front of
 * the second one.
 * @sqlop @p <</
 */
bool
front_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmax < box2->zmin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * back of the second one.
 * @sqlop @p &</
 */
bool
overfront_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmax <= box2->zmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly back of the
 * second one
 * @sqlop @p />>
 */
bool
back_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmin > box2->zmax);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * front of the second one.
 * @sqlop @p /&>
 */
bool
overback_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmin >= box2->zmin);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly before the
 * second one
 * @sqlop @p <<#
 */
bool
before_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return left_span_span(&box1->period, &box2->period);

}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box does not extend after the
 * second one
 * @sqlop @p &<#
 */
bool
overbefore_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return overleft_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly after
 * the second one.
 * @sqlop @p #>>
 */
bool
after_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return right_span_span(&box1->period, &box2->period);
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first temporal box does not extend before the
 * second one.
 * @sqlop @p #&>
 */
bool
overafter_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return overright_span_span(&box1->period, &box2->period);
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * @ingroup libmeos_box_set
 * @brief Return the union of the spatiotemporal boxes.
 * @sqlop @p +
 */
STBox *
union_stbox_stbox(const STBox *box1, const STBox *box2, bool strict)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_dimensionality(box1->flags, box2->flags);
  ensure_same_srid(stbox_srid(box1), stbox_srid(box2));
  /* If the strict parameter is true, we need to ensure that the boxes
   * intersect, otherwise their union cannot be represented by a box */
  if (strict && ! overlaps_stbox_stbox(box1, box2))
    elog(ERROR, "Result of box union would not be contiguous");

  STBox *result = stbox_copy(box1);
  stbox_expand(box2, result);
  return result;
}

/**
 * @ingroup libmeos_internal_box_set
 * @brief Set a spatiotemporal box with the result of the intersection of the
 * first two boxes
 * @note This function is equivalent to @ref intersection_stbox_stbox without
 * memory allocation
 */
bool
inter_stbox_stbox(const STBox *box1, const STBox *box2, STBox *result)
{
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
    ensure_same_geodetic(box1->flags, box2->flags);
    ensure_same_srid(stbox_srid(box1), stbox_srid(box2));
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
 * @ingroup libmeos_box_set
 * @brief Return the intersection of the spatiotemporal boxes.
 * @sqlop @p *
 */
STBox *
intersection_stbox_stbox(const STBox *box1, const STBox *box2)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  // ensure_same_dimensionality(box1->flags, box2->flags);
  ensure_same_srid(stbox_srid(box1), stbox_srid(box2));
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
 * @ingroup libmeos_box_transf
 * @brief Split the spatiotemporal box with respect to its space dimension in
 * four quadrants/octants. The quadrants/octants are numbered as follows
 * @code
 *   (front)        (back if has Z dimension)
 * -------------   -------------
 * |  2  |  3  |   |  6  |  7  |
 * ------------- + -------------
 * |  0  |  1  |   |  4  |  5  |
 * -------------   -------------
 * @endcode
 * @sqlfunc quadSplit
 */
STBox *
stbox_quad_split(const STBox *box, int *count)
{
  ensure_has_X_stbox(box);
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
 * Comparison functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the spatiotemporal boxes are equal.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
stbox_eq(const STBox *box1, const STBox *box2)
{
  if (box1->xmin != box2->xmin || box1->ymin != box2->ymin ||
      box1->zmin != box2->zmin || box1->xmax != box2->xmax ||
      box1->ymax != box2->ymax || box1->zmax != box2->zmax ||
      box1->srid != box2->srid || ! span_eq(&box1->period, &box2->period))
    return false;
  /* The two boxes are equal */
  return true;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the spatiotemporal boxes are different
 * @sqlop @p <>
 */
bool
stbox_ne(const STBox *box1, const STBox *box2)
{
  return ! stbox_eq(box1, box2);
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first spatiotemporal
 * box is less than, equal, or greater than the second one
 * @sqlfunc stbox_cmp()
 */
int
stbox_cmp(const STBox *box1, const STBox *box2)
{
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
 * @ingroup libmeos_box_comp
 * @brief Return true if the first spatiotemporal box is less than the second one
 * @sqlop @p <
 */
bool
stbox_lt(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first spatiotemporal box is less than or equal to
 * the second one
 * @sqlop @p <=
 */
bool
stbox_le(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first spatiotemporal box is greater than or equal to
 * the second one
 * @sqlop @p >=
 */
bool
stbox_ge(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_box_comp
 * @brief Return true if the first spatiotemporal box is greater than the second one
 * @sqlop @p >
 */
bool
stbox_gt(const STBox *box1, const STBox *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp > 0;
}

/*****************************************************************************/

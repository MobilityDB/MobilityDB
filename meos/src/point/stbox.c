/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Functions for spatiotemporal bounding boxes.
 */

#include "point/stbox.h"

/* C */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"
#include "point/pgis_call.h"
#include "point/tpoint.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"

/* Buffer size for input and output of STBOX */
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
stbox_expand(const STBOX *box1, STBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box2->flags))
  {
    box2->xmin = Min(box1->xmin, box2->xmin);
    box2->xmax = Max(box1->xmax, box2->xmax);
    box2->ymin = Min(box1->ymin, box2->ymin);
    box2->ymax = Max(box1->ymax, box2->ymax);
    if (MOBDB_FLAGS_GET_Z(box2->flags) ||
      MOBDB_FLAGS_GET_GEODETIC(box2->flags))
    {
      box2->zmin = Min(box1->zmin, box2->zmin);
      box2->zmax = Max(box1->zmax, box2->zmax);
    }
  }
  if (MOBDB_FLAGS_GET_T(box2->flags))
    span_expand(&box1->period, &box2->period);
  return;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Shift and/or scale a spatiotemporal box by the intervals
 * @sqlfunc shift(), tscale(), shiftTscale()
 */
void
stbox_shift_tscale(const Interval *shift, const Interval *duration, STBOX *box)
{
  period_shift_tscale(shift, duration, &box->period);
  return;
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensure that the temporal value has XY dimension
 */
void
ensure_has_X_stbox(const STBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    elog(ERROR, "The box must have XY(Z) dimension");
  return;
}

/**
 * Ensure that the temporal value has T dimension
 */
void
ensure_has_T_stbox(const STBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    elog(ERROR, "The box must have time dimension");
  return;
}


/*****************************************************************************
 * Input/ouput functions in string format
 *****************************************************************************/

/**
 * @ingroup libmeos_box_in_out
 * @brief Return a spatiotemporal box from its Well-Known Text (WKT) representation.
 *
 * Examples of input:
 * @code
 * STBOX((1.0, 2.0), (3.0, 4.0)) -> only spatial
 * STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial
 * STBOX T([2001-01-01, 2001-01-02], ((1.0, 2.0), (3.0, 4.0))) -> spatiotemporal
 * STBOX ZT([2001-01-01, 2001-01-02], ((1.0, 2.0, 3.0), (4.0, 5.0, 6.0))) -> spatiotemporal
 * STBOX T([2001-01-01, 2001-01-02]) -> only temporal
 * SRID=xxxx;STBOX... (any of the above)
 * GEODSTBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial
 * GEODSTBOX T([2001-01-01, 2001-01-02]) -> only temporal
 * GEODSTBOX ZT([2001-01-01, 2001-01-02], ((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> spatiotemporal
 * SRID=xxxx;GEODSTBOX... (any of the above)
 * @endcode
 * where the commas are optional and the SRID is optional. If the SRID is not
 * stated it is by default 0 for non geodetic boxes and 4326 for geodetic boxes
 */
STBOX *
stbox_in(const char *str)
{
  return stbox_parse(&str);
}

/**
 * @ingroup libmeos_box_in_out
 * @brief Return the Well-Known Text (WKT) representation of a spatiotemporal box.
 */
char *
stbox_out(const STBOX *box, int maxdd)
{
  static size_t size = MAXSTBOXLEN + 1;
  char *xmin = NULL, *xmax = NULL, *ymin = NULL, *ymax = NULL, *zmin = NULL,
    *zmax = NULL, *period = NULL;
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);
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
    period = span_out(&box->period, Int32GetDatum(maxdd));

  if (hasx && hast)
  {
    xmin = float8_out(box->xmin, maxdd);
    xmax = float8_out(box->xmax, maxdd);
    ymin = float8_out(box->ymin, maxdd);
    ymax = float8_out(box->ymax, maxdd);
    if (geodetic || hasz)
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
    if (geodetic || hasz)
    {
      zmin = float8_out(box->zmin, maxdd);
      zmax = float8_out(box->zmax, maxdd);
      snprintf(str, size, "%s%s Z((%s,%s,%s),(%s,%s,%s))",
        srid, boxtype, xmin, ymin, zmin, xmax, ymax, zmax);
    }
    else
      snprintf(str, size, "%s%s X(((%s,%s),(%s,%s)))",
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
STBOX *
stbox_make(const Period *p, bool hasx, bool hasz, bool geodetic, int32 srid,
  double xmin, double xmax, double ymin, double ymax, double zmin,
  double zmax)
{
  /* Note: zero-fill is done in function stbox_set */
  STBOX *result = palloc(sizeof(STBOX));
  stbox_set(p, hasx, hasz, geodetic, srid, xmin, xmax, ymin, ymax, zmin, zmax,
    result);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Set a spatiotemporal box from the arguments.
 * @note This function is equivalent to @ref stbox_make without memory
 * allocation
 */
void
stbox_set(const Period *p, bool hasx, bool hasz, bool geodetic, int32 srid,
  double xmin, double xmax, double ymin, double ymax, double zmin, double zmax,
  STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  MOBDB_FLAGS_SET_X(box->flags, hasx);
  MOBDB_FLAGS_SET_Z(box->flags, hasz);
  MOBDB_FLAGS_SET_GEODETIC(box->flags, geodetic);
  box->srid = srid;

  if (p)
  {
    /* Process T min/max */
    memcpy(&box->period, p, sizeof(Span));
    MOBDB_FLAGS_SET_T(box->flags, true);
  }
  if (hasx)
  {
    /* Process X min/max */
    box->xmin = Min(xmin, xmax);
    box->xmax = Max(xmin, xmax);
    /* Process Y min/max */
    box->ymin = Min(ymin, ymax);
    box->ymax = Max(ymin, ymax);
    if (hasz || geodetic)
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
STBOX *
stbox_copy(const STBOX *box)
{
  STBOX *result = palloc(sizeof(STBOX));
  memcpy(result, box, sizeof(STBOX));
  return result;
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * @ingroup libmeos_int_box_cast
 * @brief Set a PostGIS GBOX from a spatiotemporal box.
 * @sqlop @p ::
 */
void
stbox_set_gbox(const STBOX *box, GBOX *gbox)
{
  ensure_has_X_stbox(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(gbox, 0, sizeof(GBOX));
  /* Initialize existing dimensions */
  gbox->xmin = box->xmin;
  gbox->xmax = box->xmax;
  gbox->ymin = box->ymin;
  gbox->ymax = box->ymax;
  if (MOBDB_FLAGS_GET_Z(box->flags))
  {
    gbox->zmin = box->zmin;
    gbox->zmax = box->zmax;
  }
  FLAGS_SET_Z(gbox->flags, MOBDB_FLAGS_GET_Z(box->flags));
  FLAGS_SET_M(gbox->flags, false);
  FLAGS_SET_GEODETIC(gbox->flags, MOBDB_FLAGS_GET_GEODETIC(box->flags));
  return;
}

/**
 * @ingroup libmeos_int_box_cast
 * @brief Set a PostGIS BOX3D from a spatiotemporal box
 * @sqlop @p ::
 */
void
stbox_set_box3d(const STBOX *box, BOX3D *box3d)
{
  ensure_has_X_stbox(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box3d, 0, sizeof(BOX3D));
  /* Initialize existing dimensions */
  box3d->xmin = box->xmin;
  box3d->xmax = box->xmax;
  box3d->ymin = box->ymin;
  box3d->ymax = box->ymax;
  if (MOBDB_FLAGS_GET_Z(box->flags))
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
stbox_to_geo(const STBOX *box)
{
  ensure_has_X_stbox(box);
  LWGEOM *geo;
  GSERIALIZED *result;
  if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
  {
    BOX3D box3d;
    stbox_set_box3d(box, &box3d);
    geo = box3d_to_lwgeom(&box3d);
  }
  else
  {
    GBOX box2d;
    stbox_set_gbox(box, &box2d);
    geo = box2d_to_lwgeom(&box2d, box->srid);
  }
  FLAGS_SET_GEODETIC(geo->flags, MOBDB_FLAGS_GET_GEODETIC(box->flags));
  result = geo_serialize(geo);
  lwgeom_free(geo);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Cast a temporal box as a period
 * @sqlop @p ::
 */
Period *
stbox_to_period(const STBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return NULL;
  return span_copy(&box->period);
}

/*****************************************************************************
 * Transform a <Type> to a STBOX
 * The functions assume set the argument box to 0
 *****************************************************************************/

/**
 * @ingroup libmeos_int_box_cast
 * @brief Set a spatiotemporal box from a geometry/geography.
 */
bool
geo_set_stbox(const GSERIALIZED *gs, STBOX *box)
{
  if (gserialized_is_empty(gs))
    return false;

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(gs->gflags);
  box->srid = gserialized_get_srid(gs);
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_Z(box->flags, hasz);
  MOBDB_FLAGS_SET_T(box->flags, false);
  MOBDB_FLAGS_SET_GEODETIC(box->flags, geodetic);

  /* Short-circuit the case where the geometry/geography is a point */
  if (gserialized_get_type(gs) == POINTTYPE)
  {
    if (geodetic)
    {
      POINT3D A1;
      const POINT2D *p = datum_point2d_p(PointerGetDatum(gs));
      ll2cart(p, &A1);
      box->xmin = box->xmax = A1.x;
      box->ymin = box->ymax = A1.y;
      box->zmin = box->zmax = A1.z;
    }
    else
    {
      if (hasz)
      {
        const POINT3DZ *p = datum_point3dz_p(PointerGetDatum(gs));
        box->xmin = box->xmax = p->x;
        box->ymin = box->ymax = p->y;
        box->zmin = box->zmax = p->z;
      }
      else
      {
        const POINT2D *p = datum_point2d_p(PointerGetDatum(gs));
        box->xmin = box->xmax = p->x;
        box->ymin = box->ymax = p->y;
      }
    }
    return true;
  }

  /* General case for arbitrary geometry/geography */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  GBOX gbox;
  memset(&gbox, 0, sizeof(GBOX));
  /* We are sure that the geometry/geography is not empty */
  lwgeom_calculate_gbox(lwgeom, &gbox);
  lwgeom_free(lwgeom);
  box->xmin = gbox.xmin;
  box->xmax = gbox.xmax;
  box->ymin = gbox.ymin;
  box->ymax = gbox.ymax;
  if (hasz || geodetic)
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
STBOX *
geo_to_stbox(const GSERIALIZED *gs)
{
  STBOX *result = palloc(sizeof(STBOX));
  geo_set_stbox(gs, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_box_cast
 * @brief Set a spatiotemporal box from a timestamp.
 */
void
timestamp_set_stbox(TimestampTz t, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, &box->period);
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_Z(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a timestamp to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBOX *
timestamp_to_stbox(TimestampTz t)
{
  STBOX *result = palloc(sizeof(STBOX));
  timestamp_set_stbox(t, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_box_cast
 * @brief Set a spatiotemporal box from a timestamp set.
 */
void
timestampset_set_stbox(const TimestampSet *ts, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  memcpy(&box->period, &ts->period, sizeof(Span));
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a timestamp set to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBOX *
timestampset_to_stbox(const TimestampSet *ts)
{
  STBOX *result = palloc(sizeof(STBOX));
  timestampset_set_stbox(ts, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_box_cast
 * @brief Set a spatiotemporal box from a period.
 */
void
period_set_stbox(const Period *p, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  memcpy(&box->period, p, sizeof(Span));
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a period to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBOX *
period_to_stbox(const Period *p)
{
  STBOX *result = palloc(sizeof(STBOX));
  period_set_stbox(p, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_box_cast
 * @brief Set a spatiotemporal box from a period set.
 */
void
periodset_set_stbox(const PeriodSet *ps, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  memcpy(&box->period, &ps->period, sizeof(Span));
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Cast a period set to a spatiotemporal box.
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBOX *
periodset_to_stbox(const PeriodSet *ps)
{
  STBOX *result = palloc(sizeof(STBOX));
  periodset_set_stbox(ps, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_box_cast
 * @brief Return a spatiotemporal box from a geometry/geography and a timestamp.
 * @sqlfunc stbox()
 */
STBOX *
geo_timestamp_to_stbox(const GSERIALIZED *gs, TimestampTz t)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBOX *result = palloc(sizeof(STBOX));
  geo_set_stbox(gs, result);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, &result->period);
  MOBDB_FLAGS_SET_T(result->flags, true);
  return result;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Return a spatiotemporal box from a geometry/geography and a period
 * @sqlfunc stbox()
 */
STBOX *
geo_period_to_stbox(const GSERIALIZED *gs, const Period *p)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBOX *result = palloc(sizeof(STBOX));
  geo_set_stbox(gs, result);
  memcpy(&result->period, p, sizeof(Span));
  MOBDB_FLAGS_SET_T(result->flags, true);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box has value dimension
 * @sqlfunc hasX()
 */
bool
stbox_hasx(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_X(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box has Z dimension
 * @sqlfunc hasZ()
 */
bool
stbox_hasz(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_Z(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box has time dimension
 * @sqlfunc hasT()
 */
bool
stbox_hast(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_T(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if a spatiotemporal box is geodetic
 * @sqlfunc isGeodetic()
 * @pymeosfunc geodetic()
 */
bool
stbox_isgeodetic(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has value dimension. In that
 * case, the minimum X value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmin()
 * @pymeosfunc xmin()
 */
bool
stbox_xmin(const STBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has value dimension. In that
 * case, the maximum X value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Xmax()
 * @pymeosfunc xmax()
 */
bool
stbox_xmax(const STBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return false;
  *result = box->xmax;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has value dimension. In that
 * case, the minimum Y value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Ymin()
 * @pymeosfunc ymin()
 */
bool
stbox_ymin(const STBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has value dimension. In that
 * case, the maximum Y value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Ymax()
 * @pymeosfunc ymax()
 */
bool
stbox_ymax(const STBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    return false;
  *result = box->ymax;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has Z dimension. In that
 * case, the minimum Z value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Zmin()
 * @pymeosfunc zmin()
 */
bool
stbox_zmin(const STBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has Z dimension. In that
 * case, the maximum Z value is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Zmax()
 * @pymeosfunc zmax()
 */
bool
stbox_zmax(const STBOX *box, double *result)
{
  if (! MOBDB_FLAGS_GET_Z(box->flags))
    return false;
  *result = box->zmax;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has time dimension. In that
 * case, the minimum timestamp is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmin()
 * @pymeosfunc tmin()
 */
bool
stbox_tmin(const STBOX *box, TimestampTz *result)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return false;
  *result = DatumGetTimestampTz(box->period.lower);
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has time dimension. In that
 * case, the maximum timestamp is returned in the output argument.
 *
 * @param[in] box Box
 * @param[out] result Result
 * @sqlfunc Tmax()
 * @pymeosfunc tmax()
 */
bool
stbox_tmax(const STBOX *box, TimestampTz *result)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
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
stbox_srid(const STBOX *box)
{
  return box->srid;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Set the SRID of a spatiotemporal box.
 * @sqlfunc setSRID()
 */
STBOX *
stbox_set_srid(const STBOX *box, int32 srid)
{
  STBOX *result = stbox_copy(box);
  result->srid = srid;
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_transf
 * @brief Return a spatiotemporal box expanded in the spatial dimension by a
 * double.
 * @sqlfunc expandSpatial()
 */
STBOX *
stbox_expand_spatial(const STBOX *box, double d)
{
  ensure_has_X_stbox(box);
  STBOX *result = stbox_copy(box);
  result->xmin -= d;
  result->ymin -= d;
  result->xmax += d;
  result->ymax += d;
  if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
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
 * @sqlfunc expandTemporal()
 */
STBOX *
stbox_expand_temporal(const STBOX *box, const Interval *interval)
{
  ensure_has_T_stbox(box);
  STBOX *result = stbox_copy(box);
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
 * Set the ouput variables with the values of the flags of the boxes.
 *
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hasz,hast,geodetic Boolean variables
 */
static void
stbox_stbox_flags(const STBOX *box1, const STBOX *box2, bool *hasx,
  bool *hasz, bool *hast, bool *geodetic)
{
  *hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  *hasz = MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags);
  *hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  *geodetic = MOBDB_FLAGS_GET_GEODETIC(box1->flags) &&
    MOBDB_FLAGS_GET_GEODETIC(box2->flags);
  return;
}

/**
 * Verify the conditions and set the ouput variables with the values of the
 * flags of the boxes.
 *
 * Mixing 2D/3D is enabled to compute, for example, 2.5D operations
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hasz,hast,geodetic Boolean variables
 */
static void
topo_stbox_stbox_init(const STBOX *box1, const STBOX *box2, bool *hasx,
  bool *hasz, bool *hast, bool *geodetic)
{
  ensure_common_dimension(box1->flags, box2->flags);
  if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags))
  {
    ensure_same_geodetic(box1->flags, box2->flags);
    ensure_same_srid_stbox(box1, box2);
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
contains_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
  if (hasx && (box2->xmin < box1->xmin || box2->xmax > box1->xmax ||
    box2->ymin < box1->ymin || box2->ymax > box1->ymax))
      return false;
  if ((hasz || geodetic) && (box2->zmin < box1->zmin || box2->zmax > box1->zmax))
      return false;
  if (hast && (
    datum_lt(box2->period.lower, box1->period.lower, T_TIMESTAMPTZ) ||
    datum_gt(box2->period.upper, box1->period.upper, T_TIMESTAMPTZ)))
      return false;
  return true;
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the first spatiotemporal box is contained by the
 * second one
 * @sqlop @p <@
 */
bool
contained_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  return contains_stbox_stbox(box2, box1);
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the spatiotemporal boxes overlap
 * @sqlop @p &&
 */
bool
overlaps_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
  if (hasx && (box1->xmax < box2->xmin || box1->xmin > box2->xmax ||
    box1->ymax < box2->ymin || box1->ymin > box2->ymax))
    return false;
  if ((hasz || geodetic) && (box1->zmax < box2->zmin || box1->zmin > box2->zmax))
    return false;
  if (hast && (
    datum_lt(box1->period.upper, box2->period.lower, T_TIMESTAMPTZ) ||
    datum_gt(box1->period.lower, box2->period.upper, T_TIMESTAMPTZ)))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_topo
 * @brief Return true if the spatiotemporal boxes are equal on the common
 * dimensions.
 * @sqlop @p ~=
 */
bool
same_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
  if (hasx && (box1->xmin != box2->xmin || box1->xmax != box2->xmax ||
    box1->ymin != box2->ymin || box1->ymax != box2->ymax))
    return false;
  if ((hasz || geodetic) && (box1->zmin != box2->zmin || box1->zmax != box2->zmax))
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
adjacent_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
  STBOX inter;
  if (! inter_stbox_stbox(box1, box2, &inter))
    return false;

  /* Boxes are adjacent if they share n dimensions and their intersection is
   * at most of n-1 dimensions */
  if (! hasx && hast)
    return (inter.period.lower == inter.period.upper);
  if (hasx && !hast)
  {
    if (hasz || geodetic)
      return (inter.xmin == inter.xmax || inter.ymin == inter.ymax ||
           inter.zmin == inter.zmax);
    else
      return (inter.xmin == inter.xmax || inter.ymin == inter.ymax);
  }
  else
  {
    if (hasz || geodetic)
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
 * Verify the conditions for a position operator
 *
 * @param[in] box1,box2 Input boxes
 */
static void
pos_stbox_stbox_test(const STBOX *box1, const STBOX *box2)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_srid_stbox(box1, box2);
  return;
}

/**
 * @ingroup libmeos_box_pos
 * @brief Return true if the first spatiotemporal box is strictly to the
 * left of the second one
 * @sqlop @p <<
 */
bool
left_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overleft_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
right_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overright_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
below_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overbelow_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
above_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overabove_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
front_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overfront_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
back_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overback_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
before_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overbefore_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
after_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
overafter_stbox_stbox(const STBOX *box1, const STBOX *box2)
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
STBOX *
union_stbox_stbox(const STBOX *box1, const STBOX *box2, bool strict)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_dimensionality(box1->flags, box2->flags);
  ensure_same_srid_stbox(box1, box2);
  /* If the strict parameter is true, we need to ensure that the boxes
   * intersect, otherwise their union cannot be represented by a box */
  if (strict && ! overlaps_stbox_stbox(box1, box2))
    elog(ERROR, "Result of box union would not be contiguous");

  STBOX *result = stbox_copy(box1);
  stbox_expand(box2, result);
  return result;
}

/**
 * @ingroup libmeos_box_set
 * @brief Set a spatiotemporal box with the result of the intersection of the
 * first two boxes
 * @note This function is equivalent to @ref intersection_stbox_stbox without
 * memory allocation
 */
bool
inter_stbox_stbox(const STBOX *box1, const STBOX *box2, STBOX *result)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_srid_stbox(box1, box2);

  bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags);
  bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box1->flags) && MOBDB_FLAGS_GET_GEODETIC(box2->flags);
  /* If there is no common dimension */
  if ((! hasx && ! hast) ||
    /* If they do no intersect in one common dimension */
    (hasx && (box1->xmin > box2->xmax || box2->xmin > box1->xmax ||
      box1->ymin > box2->ymax || box2->ymin > box1->ymax)) ||
    ((hasz || geodetic) && (box1->zmin > box2->zmax || box2->zmin > box1->zmax)) ||
    (hast && ! overlaps_span_span(&box1->period, &box2->period)))
    return false;

  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  Period period;
  if (hasx)
  {
    xmin = Max(box1->xmin, box2->xmin);
    xmax = Min(box1->xmax, box2->xmax);
    ymin = Max(box1->ymin, box2->ymin);
    ymax = Min(box1->ymax, box2->ymax);
    if (hasz || geodetic)
      {
      zmin = Max(box1->zmin, box2->zmin);
      zmax = Min(box1->zmax, box2->zmax);
      }
  }
  /* We are sure that the intersection is not NULL */
  if (hast)
    inter_span_span(&box1->period, &box2->period, &period);

  stbox_set(hast ? &period : NULL, hasx, hasz, geodetic, box1->srid, xmin,
    xmax, ymin, ymax, zmin, zmax, result);
  return true;
}

/**
 * @ingroup libmeos_box_set
 * @brief Return the intersection of the spatiotemporal boxes.
 * @sqlop @p *
 */
STBOX *
intersection_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  // ensure_same_dimensionality(box1->flags, box2->flags);
  ensure_same_srid_stbox(box1, box2);
  STBOX *result = palloc(sizeof(STBOX));
  if (! inter_stbox_stbox(box1, box2, result))
  {
    pfree(result);
    return NULL;
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
stbox_eq(const STBOX *box1, const STBOX *box2)
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
stbox_ne(const STBOX *box1, const STBOX *box2)
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
stbox_cmp(const STBOX *box1, const STBOX *box2)
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
stbox_lt(const STBOX *box1, const STBOX *box2)
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
stbox_le(const STBOX *box1, const STBOX *box2)
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
stbox_ge(const STBOX *box1, const STBOX *box2)
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
stbox_gt(const STBOX *box1, const STBOX *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp > 0;
}

/*****************************************************************************/

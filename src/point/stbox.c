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
 * @file stbox.c
 * @brief Functions for spatiotemporal bounding boxes.
 */

#include "point/stbox.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/builtins.h>
/* MobilityDB */
#include "general/period.h"
#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"
#include "point/tpoint.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"

/* Buffer size for input and output of STBOX */
#define MAXSTBOXLEN    256

/* PostGIS prototype */
extern void ll2cart(const POINT2D *g, POINT3D *p);

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_constructor
 * @brief Constructs a newly allocated spatiotemporal box.
 */
STBOX *
stbox_make(bool hasx, bool hasz, bool hast, bool geodetic, int32 srid,
  double xmin, double xmax, double ymin, double ymax, double zmin,
  double zmax, TimestampTz tmin, TimestampTz tmax)
{
  /* Note: zero-fill is done in function stbox_set */
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  stbox_set(hasx, hasz, hast, geodetic, srid, xmin, xmax, ymin, ymax,
    zmin, zmax, tmin, tmax, result);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Set the spatiotemporal box from the argument values
 */
void
stbox_set(bool hasx, bool hasz, bool hast, bool geodetic, int32 srid,
  double xmin, double xmax, double ymin, double ymax, double zmin,
  double zmax, TimestampTz tmin, TimestampTz tmax, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  MOBDB_FLAGS_SET_X(box->flags, hasx);
  MOBDB_FLAGS_SET_Z(box->flags, hasz);
  MOBDB_FLAGS_SET_T(box->flags, hast);
  MOBDB_FLAGS_SET_GEODETIC(box->flags, geodetic);
  box->srid = srid;

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
  if (hast)
  {
    /* Process T min/max */
    box->tmin = Min(tmin, tmax);
    box->tmax = Max(tmin, tmax);
  }
  return;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Return a copy of the spatiotemporal box.
 */
STBOX *
stbox_copy(const STBOX *box)
{
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  memcpy(result, box, sizeof(STBOX));
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Expand the second spatiotemporal box with the first one
 *
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
  {
    box2->tmin = Min(box1->tmin, box2->tmin);
    box2->tmax = Max(box1->tmax, box2->tmax);
  }
  return;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Shift and/or scale the time span of the spatiotemporal box by the interval
 */
void
stbox_shift_tscale(const Interval *start, const Interval *duration, STBOX *box)
{
  assert(start != NULL || duration != NULL);
  if (start != NULL)
    box->tmin = DatumGetTimestampTz(DirectFunctionCall2(
      timestamptz_pl_interval, TimestampTzGetDatum(box->tmin),
      PointerGetDatum(start)));
  box->tmax = (duration == NULL) ?
    DatumGetTimestampTz(DirectFunctionCall2(timestamptz_pl_interval,
      TimestampTzGetDatum(box->tmax), PointerGetDatum(start))) :
    DatumGetTimestampTz(DirectFunctionCall2(timestamptz_pl_interval,
       TimestampTzGetDatum(box->tmin), PointerGetDatum(duration)));
  return;
}

/**
 * Set the values of a GBOX
 */
static void
gbox_set(bool hasz, bool hasm, bool geodetic, double xmin, double xmax,
  double ymin, double ymax, double zmin, double zmax, GBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(GBOX));
  box->xmin = xmin;
  box->xmax = xmax;
  box->ymin = ymin;
  box->ymax = ymax;
  box->zmin = zmin;
  box->zmax = zmax;
  FLAGS_SET_Z(box->flags, hasz);
  FLAGS_SET_M(box->flags, hasm);
  FLAGS_SET_GEODETIC(box->flags, geodetic);
  return;
}

/**
 * @ingroup libmeos_box_oper_set
 * @brief Return the intersection of the spatiotemporal boxes in the third argument
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
    (hast && (box1->tmin > box2->tmax || box2->tmin > box1->tmax)))
    return NULL;

  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  TimestampTz tmin = 0, tmax = 0;
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
  if (hast)
  {
    tmin = Max(box1->tmin, box2->tmin);
    tmax = Min(box1->tmax, box2->tmax);
  }
  stbox_set(hasx, hasz, hast, geodetic, box1->srid, xmin, xmax, ymin,
    ymax, zmin, zmax, tmin, tmax, result);
  return true;
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
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have XY(Z) dimension")));
  return;
}

/**
 * Ensure that the temporal value has T dimension
 */
void
ensure_has_T_stbox(const STBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have time dimension")));
  return;
}

/*****************************************************************************
 * Input/Ouput functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_input_output
 * @brief Return the string representation of the spatiotemporal box.
 */
char *
stbox_to_string(const STBOX *box)
{
  static size_t size = MAXSTBOXLEN + 1;
  char *str, *xmin = NULL, *xmax = NULL, *ymin = NULL, *ymax = NULL,
    *zmin = NULL, *zmax = NULL, *tmin = NULL, *tmax = NULL;
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);

  str = (char *) palloc(size);
  char srid[20];
  if (hasx && box->srid > 0)
    sprintf(srid, "SRID=%d;", box->srid);
  else
    srid[0] = '\0';
  char *boxtype = geodetic ? "GEODSTBOX" : "STBOX";
  assert(hasx || hast);
  if (hasx)
  {
    xmin = call_output(FLOAT8OID, Float8GetDatum(box->xmin));
    xmax = call_output(FLOAT8OID, Float8GetDatum(box->xmax));
    ymin = call_output(FLOAT8OID, Float8GetDatum(box->ymin));
    ymax = call_output(FLOAT8OID, Float8GetDatum(box->ymax));
    if (geodetic || hasz)
    {
      zmin = call_output(FLOAT8OID, Float8GetDatum(box->zmin));
      zmax = call_output(FLOAT8OID, Float8GetDatum(box->zmax));
    }
  }
  if (hast)
  {
    tmin = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmin));
    tmax = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmax));
  }
  if (hasx)
  {
    if (geodetic)
    {
      char *Z;
      if (hast)
      {
        Z = hasz ? "Z" : "";
        snprintf(str, size, "%s%s %sT((%s,%s,%s,%s),(%s,%s,%s,%s))",
          srid, boxtype, Z, xmin, ymin, zmin, tmin, xmax, ymax, zmax, tmax);
      }
      else
      {
        Z = hasz ? " Z" : "";
        snprintf(str, size, "%s%s%s((%s,%s,%s),(%s,%s,%s))",
          srid, boxtype, Z, xmin, ymin, zmin, xmax, ymax, zmax);
      }
    }
    else if (hasz && hast)
      snprintf(str, size, "%s%s ZT((%s,%s,%s,%s),(%s,%s,%s,%s))",
        srid, boxtype, xmin, ymin, zmin, tmin, xmax, ymax, zmax, tmax);
    else if (hasz)
      snprintf(str, size, "%s%s Z((%s,%s,%s),(%s,%s,%s))",
        srid, boxtype, xmin, ymin, zmin, xmax, ymax, zmax);
    else if (hast)
      snprintf(str, size, "%s%s T((%s,%s,%s),(%s,%s,%s))",
        srid, boxtype, xmin, ymin, tmin, xmax, ymax, tmax);
    else
      snprintf(str, size, "%s%s((%s,%s),(%s,%s))",
        srid, boxtype, xmin, ymin, xmax, ymax);
  }
  else
    /* Missing spatial dimension */
    snprintf(str, size, "%s%s T((,,%s),(,,%s))", srid, boxtype, tmin, tmax);
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
  {
    pfree(tmin); pfree(tmax);
  }
  return str;
}

/**
 * @ingroup libmeos_box_input_output
 * @brief Write the binary representation of the box value into the buffer.
 */
void
stbox_write(const STBOX *box, StringInfo buf)
{
  pq_sendint32(buf, box->flags);
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    pq_sendfloat8(buf, box->xmin);
    pq_sendfloat8(buf, box->xmax);
    pq_sendfloat8(buf, box->ymin);
    pq_sendfloat8(buf, box->ymax);
    if (MOBDB_FLAGS_GET_Z(box->flags))
    {
      pq_sendfloat8(buf, box->zmin);
      pq_sendfloat8(buf, box->zmax);
    }
    pq_sendint32(buf, box->srid);
  }
  if (MOBDB_FLAGS_GET_T(box->flags))
  {
    bytea *tmin = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmin));
    bytea *tmax = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmax));
    pq_sendbytes(buf, VARDATA(tmin), VARSIZE(tmin) - VARHDRSZ);
    pq_sendbytes(buf, VARDATA(tmax), VARSIZE(tmax) - VARHDRSZ);
    pfree(tmin); pfree(tmax);
  }
  return;
}

/**
 * @ingroup libmeos_box_input_output
 * @brief Return a new box value from its binary representation read from
 * the buffer.
 */
STBOX *
stbox_read(StringInfo buf)
{
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  result->flags = (int) pq_getmsgint(buf, 4);
  if (MOBDB_FLAGS_GET_X(result->flags))
  {
    result->xmin = pq_getmsgfloat8(buf);
    result->xmax = pq_getmsgfloat8(buf);
    result->ymin = pq_getmsgfloat8(buf);
    result->ymax = pq_getmsgfloat8(buf);
    if (MOBDB_FLAGS_GET_Z(result->flags))
    {
      result->zmin = pq_getmsgfloat8(buf);
      result->zmax = pq_getmsgfloat8(buf);
    }
    result->srid = (int) pq_getmsgint(buf, 4);
  }
  if (MOBDB_FLAGS_GET_T(result->flags))
  {
    result->tmin = call_recv(TIMESTAMPTZOID, buf);
    result->tmax = call_recv(TIMESTAMPTZOID, buf);
  }
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/*****************************************************************************
 * Casting
 *****************************************************************************/


/**
 * @ingroup libmeos_box_cast
 * @brief Cast the spatiotemporal box as a GBOX value for PostGIS
 */
void
stbox_gbox(const STBOX *box, GBOX *gbox)
{
  assert(MOBDB_FLAGS_GET_X(box->flags));
  gbox_set(MOBDB_FLAGS_GET_Z(box->flags), false,
    MOBDB_FLAGS_GET_GEODETIC(box->flags), box->xmin, box->xmax,
    box->ymin, box->ymax, box->zmin, box->zmax, gbox);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Cast the spatiotemporal box as a BOX3D value for PostGIS
 */
void
stbox_box3d(const STBOX *box, BOX3D *box3d)
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
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Cast the spatiotemporal box as a GBOX value for PostGIS
 */
Datum
stbox_geometry(const STBOX *box)
{
  ensure_has_X_stbox(box);
  Datum result;
  if (MOBDB_FLAGS_GET_Z(box->flags))
  {
    BOX3D box3d;
    stbox_box3d(box, &box3d);
    result = DirectFunctionCall1(BOX3D_to_LWGEOM, STboxPGetDatum(&box3d));
  }
  else
  {
    GBOX box2d;
    stbox_gbox(box, &box2d);
    Datum geom = DirectFunctionCall1(BOX2D_to_LWGEOM, PointerGetDatum(&box2d));
    GSERIALIZED *g = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
    gserialized_set_srid(g, box->srid);
    result = PointerGetDatum(g);
    PG_FREE_IF_COPY_P(g, DatumGetPointer(geom));
    pfree(DatumGetPointer(geom));
  }
  return result;
}

/*****************************************************************************
 * Transform a <Type> to a STBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Transform a geometry/geography to a spatiotemporal box.
 */
bool
geo_stbox(const GSERIALIZED *gs, STBOX *box)
{
  if (gserialized_is_empty(gs))
    return false;

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  bool hasz = (bool) FLAGS_GET_Z(GS_FLAGS(gs));
  bool geodetic = (bool) FLAGS_GET_GEODETIC(GS_FLAGS(gs));
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

/**
 * @ingroup libmeos_box_cast
 * @brief Transform a timestamptz to a spatiotemporal box.
 */
void
timestamp_stbox(TimestampTz t, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  box->tmin = box->tmax = t;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_Z(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform a timestamp set to a spatiotemporal box.
 */
void
timestampset_stbox(const TimestampSet *ts, STBOX *box)
{
  const Period *p = timestampset_bbox_ptr(ts);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform a period to a spatiotemporal box.
 */
void
period_stbox(const Period *p, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_cast
 * @brief Transform a period set to a spatiotemporal box.
 */
void
periodset_stbox(const PeriodSet *ps, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  const Period *p = periodset_bbox_ptr(ps);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform a geometry/geography and a timestamp to a spatiotemporal box
 */
STBOX *
geo_timestamp_to_stbox(const GSERIALIZED *gs, TimestampTz t)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  geo_stbox(gs, result);
  result->tmin = result->tmax = t;
  MOBDB_FLAGS_SET_T(result->flags, true);
  return result;
}

/**
 * @ingroup libmeos_box_constructor
 * @brief Transform a geometry/geography and a period to a spatiotemporal box
 */
STBOX *
geo_period_to_stbox(const GSERIALIZED *gs, const Period *p)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  geo_stbox(gs, result);
  result->tmin = p->lower;
  result->tmax = p->upper;
  MOBDB_FLAGS_SET_T(result->flags, true);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has X dimension
 */
bool
stbox_hasx(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_X(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has Z dimension
 */
bool
stbox_hasz(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_Z(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box has T dimension
 */
bool
stbox_hast(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_T(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return true if the spatiotemporal box is geodetic
 */
bool
stbox_isgeodetic(const STBOX *box)
{
  bool result = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  return result;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the minimum X value of the spatiotemporal box value, if any.
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
 * @brief Return the maximum X value of the spatiotemporal box value, if any.
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
 * @brief Return the minimum Y value of the spatiotemporal box value, if any.
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
 * @brief Return the maximum Y value of the spatiotemporal box value, if any.
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
 * @brief Return the minimum Z value of the spatiotemporal box value, if any.
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
 * @brief Return the maximum X value of the spatiotemporal box value, if any.
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
 * @brief Return the minimum T value of the spatiotemporal box value, if any.
 */
bool
stbox_tmin(const STBOX *box, TimestampTz *result)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return false;
  *result = box->tmin;
  return true;
}

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the maximum T value of the spatiotemporal box value, if any.
 */
bool
stbox_tmax(const STBOX *box, TimestampTz *result)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    return false;
  *result = box->tmax;
  return true;
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * @ingroup libmeos_box_accessor
 * @brief Return the SRID of the spatiotemporal box.
 */
int32
stbox_get_srid(const STBOX *box)
{
  PG_RETURN_INT32(box->srid);
}

/**
 * @ingroup libmeos_box_transf
 * @brief Sets the SRID of the spatiotemporal box.
 */
STBOX *
stbox_set_srid(const STBOX *box, int32 srid)
{
  STBOX *result = stbox_copy(box);
  result->srid = srid;
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Transform a spatiotemporal box into another spatial reference system
 */
STBOX *
stbox_transform(const STBOX *box, int32 srid)
{
  ensure_has_X_stbox(box);
  STBOX *result = stbox_copy(box);
  result->srid = DatumGetInt32(srid);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  Datum min = point_make(box->xmin, box->ymin, box->zmin, hasz, geodetic,
    box->srid);
  Datum max = point_make(box->xmax, box->ymax, box->zmax, hasz, geodetic,
    box->srid);
  Datum min1 = datum_transform(min, srid);
  Datum max1 = datum_transform(max, srid);
  if (hasz)
  {
    const POINT3DZ *ptmin1 = datum_point3dz_p(min1);
    const POINT3DZ *ptmax1 = datum_point3dz_p(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->zmin = ptmin1->z;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
    result->zmax = ptmax1->z;
  }
  else
  {
    const POINT2D *ptmin1 = datum_point2d_p(min1);
    const POINT2D *ptmax1 = datum_point2d_p(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
  }
  pfree(DatumGetPointer(min)); pfree(DatumGetPointer(max));
  pfree(DatumGetPointer(min1)); pfree(DatumGetPointer(max1));
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_transf
 * @brief Expand the spatial dimension of the spatiotemporal box with the
 * double value.
 */
STBOX *
stbox_expand_spatial(const STBOX *box, double d)
{
  ensure_has_X_stbox(box);
  STBOX *result = stbox_copy(box);
  result->xmin = box->xmin - d;
  result->xmax = box->xmax + d;
  result->ymin = box->ymin - d;
  result->ymax = box->ymax + d;
  if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
  {
    result->zmin = box->zmin - d;
    result->zmax = box->zmax + d;
  }
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Expand the temporal dimension of the spatiotemporal box with the
 * interval value
 */
STBOX *
stbox_expand_temporal(const STBOX *box, Datum interval)
{
  ensure_has_T_stbox(box);
  STBOX *result = stbox_copy(box);
  result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval,
    TimestampTzGetDatum(box->tmin), interval));
  result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval,
    TimestampTzGetDatum(box->tmax), interval));
  return result;
}

/**
 * @ingroup libmeos_box_transf
 * @brief Sets the precision of the coordinates of the spatiotemporal box.
 */
STBOX *
stbox_round(const STBOX *box, Datum prec)
{
  ensure_has_X_stbox(box);
  STBOX *result = stbox_copy(box);
  result->xmin = DatumGetFloat8(datum_round_float(Float8GetDatum(box->xmin), prec));
  result->xmax = DatumGetFloat8(datum_round_float(Float8GetDatum(box->xmax), prec));
  result->ymin = DatumGetFloat8(datum_round_float(Float8GetDatum(box->ymin), prec));
  result->ymax = DatumGetFloat8(datum_round_float(Float8GetDatum(box->ymax), prec));
  if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
  {
    result->zmin = DatumGetFloat8(datum_round_float(Float8GetDatum(box->zmin), prec));
    result->zmax = DatumGetFloat8(datum_round_float(Float8GetDatum(box->zmax), prec));
  }
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
 * @ingroup libmeos_box_oper_topo
 * @brief Return true if the first spatiotemporal box contains the second one.
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
  if (hast && (box2->tmin < box1->tmin || box2->tmax > box1->tmax))
      return false;
  return true;
}

/**
 * @ingroup libmeos_box_oper_topo
 * @brief Return true if the first spatiotemporal box is contained by the
 * second one
 */
bool
contained_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  return contains_stbox_stbox(box2, box1);
}

/**
 * @ingroup libmeos_box_oper_topo
 * @brief Return true if the spatiotemporal boxes overlap
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
  if (hast && (box1->tmax < box2->tmin || box1->tmin > box2->tmax))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_oper_topo
 * @brief Return true if the spatiotemporal boxes are equal on the common
 * dimensions.
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
  if (hast && (box1->tmin != box2->tmin || box1->tmax != box2->tmax))
    return false;
  return true;
}

/**
 * @ingroup libmeos_box_oper_topo
 * @brief Return true if the spatiotemporal boxes are adjacent.
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
    return (inter.tmin == inter.tmax);
  else if (hasx && !hast)
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
           inter.zmin == inter.zmax || inter.tmin == inter.tmax);
    else
      return (inter.xmin == inter.xmax || inter.ymin == inter.ymax ||
           inter.tmin == inter.tmax);
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly to the
 * left of the second one
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * right of the second one
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly to the right
 * of the second one
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatio temporal box does not extend to the
 * left of the second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly below of
 * the second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box does not extend above of
 * the second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly above of the
 * second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box does not extend below of
 * the second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly in front of
 * the second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * back of the second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly back of the
 * second one
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * front of the second one.
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
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly before the
 * second one
 */
bool
before_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmax < box2->tmin);
}

/**
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first temporal box does not extend after the
 * second one
 */
bool
overbefore_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmax <= box2->tmax);
}

/**
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first spatiotemporal box is strictly after
 * the second one.
 */
bool
after_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmin > box2->tmax);
}

/**
 * @ingroup libmeos_box_oper_pos
 * @brief Return true if the first temporal box does not extend before the
 * second one.
 */
bool
overafter_stbox_stbox(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmin >= box2->tmin);
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * @ingroup libmeos_box_oper_set
 * @brief Return the union of the spatiotemporal boxes.
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
 * @ingroup libmeos_box_oper_set
 * @brief Return the union of the spatiotemporal boxes.
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
 * @ingroup libmeos_box_oper_comp
 * @brief Return -1, 0, or 1 depending on whether the first spatiotemporal
 * box is less than, equal, or greater than the second one
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
    /* Compare the box minima */
    if (box1->tmin < box2->tmin)
      return -1;
    if (box1->tmin > box2->tmin)
      return 1;
    /* Compare the box maxima */
    if (box1->tmax < box2->tmax)
      return -1;
    if (box1->tmax > box2->tmax)
      return 1;
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
 * @ingroup libmeos_box_oper_comp
 * @brief Return true if the first spatiotemporal box is less than the second one
 */
bool
stbox_lt(const STBOX *box1, const STBOX *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_box_oper_comp
 * @brief Return true if the first spatiotemporal box is less than or equal to
 * the second one
 */
bool
stbox_le(const STBOX *box1, const STBOX *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_box_oper_comp
 * @brief Return true if the first spatiotemporal box is greater than or equal to
 * the second one
 */
bool
stbox_ge(const STBOX *box1, const STBOX *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_box_oper_comp
 * @brief Return true if the first spatiotemporal box is greater than the second one
 */
bool
stbox_gt(const STBOX *box1, const STBOX *box2)
{
  int cmp = stbox_cmp(box1, box2);
  return cmp > 0;
}

/**
 * @ingroup libmeos_box_oper_comp
 * @brief Return true if the two spatiotemporal boxes are equal.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
stbox_eq(const STBOX *box1, const STBOX *box2)
{
  if (box1->xmin != box2->xmin || box1->ymin != box2->ymin ||
      box1->zmin != box2->zmin || box1->tmin != box2->tmin ||
      box1->xmax != box2->xmax || box1->ymax != box2->ymax ||
      box1->zmax != box2->zmax || box1->tmax != box2->tmax ||
      box1->flags != box2->flags || box1->srid != box2->srid)
    return false;
  /* The two boxes are equal */
  return true;
}

/**
 * @ingroup libmeos_box_oper_comp
 * @brief Return true if the two spatiotemporal boxes are different
 */
bool
stbox_ne(const STBOX *box1, const STBOX *box2)
{
  return ! stbox_eq(box1, box2);
}

/*****************************************************************************/

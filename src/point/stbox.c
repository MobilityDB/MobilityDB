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
 * Functions for spatiotemporal bounding boxes.
 */

#include "point/stbox.h"

/* PostgreSQL */
#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
/* MobilityDB */
#include "general/period.h"
#include "general/timestampset.h"
#include "general/periodset.h"
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
 * Constructs a newly allocated spatiotemporal box
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
 * Set the spatiotemporal box from the argument values
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
 * Returns a copy of the spatiotemporal box
 */
STBOX *
stbox_copy(const STBOX *box)
{
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  memcpy(result, box, sizeof(STBOX));
  return result;
}

/**
 * Expand the second spatiotemporal box with the first one
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
 * Shift and/or scale the time span of the spatiotemporal box by the interval
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
 * Returns the intersection of the spatiotemporal boxes in the third argument
 */
static bool
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

PG_FUNCTION_INFO_V1(stbox_in);
/**
 * Input function for spatiotemporal boxes.
 *
 * Examples of input:
 * @code
 * STBOX((1.0, 2.0), (3.0, 4.0)) -> only spatial
 * STBOX Z((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial
 * STBOX T((1.0, 2.0, 2001-01-01), (3.0, 4.0, 2001-01-02)) -> spatiotemporal
 * STBOX ZT((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02)) -> spatiotemporal
 * STBOX T(( , , 2001-01-01), ( , , 2001-01-02)) -> only temporal
 * SRID=xxxx;STBOX... (any of the above)
 * GEODSTBOX((1.0, 2.0, 3.0), (4.0, 5.0, 6.0)) -> only spatial
 * GEODSTBOX T((1.0, 2.0, 3.0, 2001-01-01), (4.0, 5.0, 6.0, 2001-01-02)) -> spatiotemporal
 * GEODSTBOX T(( , , 2001-01-01), ( , , 2001-01-02)) -> only temporal
 * SRID=xxxx;GEODSTBOX... (any of the above)
 * @endcode
 * where the commas are optional and the SRID is optional. If the SRID is not
 * stated it is by default 0 for non geodetic boxes and 4326 for geodetic boxes
 */
PGDLLEXPORT Datum
stbox_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  STBOX *result = stbox_parse(&input);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the string representation of the spatiotemporal box
 */
static char *
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

PG_FUNCTION_INFO_V1(stbox_out);
/**
 * Output function for spatiotemporal boxes.
 */
PGDLLEXPORT Datum
stbox_out(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  char *result = stbox_to_string(box);
  PG_RETURN_CSTRING(result);
}

/**
 * Send function for STBOX (internal function)
 */
static void
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

PG_FUNCTION_INFO_V1(stbox_send);
/**
 * Send function for STBOX
 */
PGDLLEXPORT Datum
stbox_send(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  stbox_write(box, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/**
 * Receive function for STBOX (internal function)
 */
static STBOX *
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

PG_FUNCTION_INFO_V1(stbox_recv);
/**
 * Receive function for STBOX
 */
PGDLLEXPORT Datum
stbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(stbox_read(buf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * Construct a spatiotemporal box from the arguments
 */
static Datum
stbox_constructor1(FunctionCallInfo fcinfo, bool hasx, bool hasz, bool hast,
  bool geodetic)
{
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  int srid = 0; /* make Codacy quiet */

  if (!hasx && hast)
  {
    tmin = PG_GETARG_TIMESTAMPTZ(0);
    tmax = PG_GETARG_TIMESTAMPTZ(1);
    srid = PG_GETARG_INT32(2);
  }
  else if (hasx && !hasz && !geodetic && !hast)
  {
    xmin = PG_GETARG_FLOAT8(0);
    ymin = PG_GETARG_FLOAT8(1);
    xmax = PG_GETARG_FLOAT8(2);
    ymax = PG_GETARG_FLOAT8(3);
    srid = PG_GETARG_INT32(4);
  }
  else if (hasx && (hasz || geodetic) && !hast)
  {
    xmin = PG_GETARG_FLOAT8(0);
    ymin = PG_GETARG_FLOAT8(1);
    zmin = PG_GETARG_FLOAT8(2);
    xmax = PG_GETARG_FLOAT8(3);
    ymax = PG_GETARG_FLOAT8(4);
    zmax = PG_GETARG_FLOAT8(5);
    srid = PG_GETARG_INT32(6);
  }
  else if (hasx && !hasz && !geodetic && hast)
  {
    xmin = PG_GETARG_FLOAT8(0);
    ymin = PG_GETARG_FLOAT8(1);
    tmin = PG_GETARG_TIMESTAMPTZ(2);
    xmax = PG_GETARG_FLOAT8(3);
    ymax = PG_GETARG_FLOAT8(4);
    tmax = PG_GETARG_TIMESTAMPTZ(5);
    srid = PG_GETARG_INT32(6);
  }
  else /* hasx && (hasz || geodetic) && hast) */
  {
    xmin = PG_GETARG_FLOAT8(0);
    ymin = PG_GETARG_FLOAT8(1);
    zmin = PG_GETARG_FLOAT8(2);
    tmin = PG_GETARG_TIMESTAMPTZ(3);
    xmax = PG_GETARG_FLOAT8(4);
    ymax = PG_GETARG_FLOAT8(5);
    zmax = PG_GETARG_FLOAT8(6);
    tmax = PG_GETARG_TIMESTAMPTZ(7);
    srid = PG_GETARG_INT32(8);
  }

  /* Construct the box */
  STBOX *result = stbox_make(hasx, hasz, hast, geodetic, srid,
    xmin, xmax, ymin, ymax, zmin, zmax, tmin, tmax);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_constructor_t);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor_t(PG_FUNCTION_ARGS)
{
  if (PG_NARGS() > 3)
    return stbox_constructor1(fcinfo, true, false, true, false);
  return stbox_constructor1(fcinfo, false, false, true, false);
}

PG_FUNCTION_INFO_V1(stbox_constructor);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor(PG_FUNCTION_ARGS)
{
  return stbox_constructor1(fcinfo, true, false, false, false);
}

PG_FUNCTION_INFO_V1(stbox_constructor_z);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor1(fcinfo, true, true, false, false);
}

PG_FUNCTION_INFO_V1(stbox_constructor_zt);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
stbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return stbox_constructor1(fcinfo, true, true, true, false);
}

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to ::timestamptz is needed */

PG_FUNCTION_INFO_V1(geodstbox_constructor_t);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor_t(PG_FUNCTION_ARGS)
{
  if (PG_NARGS() > 3)
    return stbox_constructor1(fcinfo, true, false, true, true);
  return stbox_constructor1(fcinfo, false, false, true, true);
}

PG_FUNCTION_INFO_V1(geodstbox_constructor);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor(PG_FUNCTION_ARGS)
{
  return stbox_constructor1(fcinfo, true, false, false, true);
}

PG_FUNCTION_INFO_V1(geodstbox_constructor_z);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor1(fcinfo, true, true, false, true);
}

PG_FUNCTION_INFO_V1(geodstbox_constructor_zt);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
geodstbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return stbox_constructor1(fcinfo, true, true, true, true);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_to_period);
/**
 * Cast the spatiotemporal box as a period
 */
PGDLLEXPORT Datum
stbox_to_period(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  ensure_has_T_stbox(box);
  Period *result = period_make(box->tmin, box->tmax, true, true);
  PG_RETURN_POINTER(result);
}

/**
 * Cast the spatiotemporal box as a GBOX value for PostGIS
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

PG_FUNCTION_INFO_V1(stbox_to_box2d);
/**
 * Cast the spatiotemporal box as a GBOX value for PostGIS
 */
PGDLLEXPORT Datum
stbox_to_box2d(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  ensure_has_X_stbox(box);
  GBOX *result = (GBOX *) palloc0(sizeof(GBOX));
  stbox_gbox(box, result);
  PG_RETURN_POINTER(result);
}

/**
 * Cast the spatiotemporal box as a BOX3D value for PostGIS
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

PG_FUNCTION_INFO_V1(stbox_to_box3d);
/**
 * Cast the spatiotemporal box as a BOX3D value for PostGIS
 */
PGDLLEXPORT Datum
stbox_to_box3d(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  ensure_has_X_stbox(box);
  BOX3D *result = (BOX3D *) palloc0(sizeof(BOX3D));
  stbox_box3d(box, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(stbox_to_geometry);
/**
 * Cast the spatiotemporal box as a GBOX value for PostGIS
 */
PGDLLEXPORT Datum
stbox_to_geometry(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
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
    PG_FREE_IF_COPY_P(DatumGetPointer(geom), g);
  }
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Transform a <Type> to a STBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

PG_FUNCTION_INFO_V1(box2d_to_stbox);
/**
 * Transform a box2d to a spatiotemporal box
 */
PGDLLEXPORT Datum
box2d_to_stbox(PG_FUNCTION_ARGS)
{
  GBOX *box = (GBOX *) PG_GETARG_POINTER(0);
  STBOX *result = stbox_make(true, false, false, false, 0,
    box->xmin, box->xmax, box->ymin, box->ymax, 0, 0, 0, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(box3d_to_stbox);
/**
 * Transform a box3d to a spatiotemporal box
 */
PGDLLEXPORT Datum
box3d_to_stbox(PG_FUNCTION_ARGS)
{
  BOX3D *box = (BOX3D *) PG_GETARG_POINTER(0);
  STBOX *result = stbox_make(true, true, false, false, box->srid, box->xmin,
    box->xmax, box->ymin, box->ymax, box->zmin, box->zmax, 0, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Transform a geometry/geography to a spatiotemporal box
 * (internal function)
 */
bool
geo_stbox(const GSERIALIZED *gs, STBOX *box)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBOX));
  if (gserialized_is_empty(gs))
  {
    /* Spatial dimensions are set as missing for the SP-GiST index */
    MOBDB_FLAGS_SET_X(box->flags, false);
    MOBDB_FLAGS_SET_Z(box->flags, false);
    MOBDB_FLAGS_SET_T(box->flags, false);
    return false;
  }

  bool hasz = (bool) FLAGS_GET_Z(GS_FLAGS(gs));
  bool geodetic = (bool) FLAGS_GET_GEODETIC(GS_FLAGS(gs));
  box->srid = gserialized_get_srid(gs);
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_Z(box->flags, hasz);
  MOBDB_FLAGS_SET_T(box->flags, false);
  MOBDB_FLAGS_SET_GEODETIC(box->flags, geodetic);

  /* Short-circuit the case where the geometry is a geometric point */
  if (gserialized_get_type(gs) == POINTTYPE && ! geodetic)
  {
    if (hasz)
    {
      const POINT3DZ *p = datum_get_point3dz_p(PointerGetDatum(gs));
      box->xmin = box->xmax = p->x;
      box->ymin = box->ymax = p->y;
      box->zmin = box->zmax = p->z;
    }
    else
    {
      const POINT2D *p = datum_get_point2d_p(PointerGetDatum(gs));
      box->xmin = box->xmax = p->x;
      box->ymin = box->ymax = p->y;
    }
    return true;
  }

  /* General case for arbitrary geometry/geography */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  GBOX gbox;
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

PG_FUNCTION_INFO_V1(geo_to_stbox);
/**
 * Transform a geometry/geography to a spatiotemporal box
 */
PGDLLEXPORT Datum
geo_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  geo_stbox(gs, result);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Transform a timestampt to a spatiotemporal box
 * (internal function)
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

PG_FUNCTION_INFO_V1(timestamp_to_stbox);
/**
 * Transform a timestampt to a spatiotemporal box
 */
PGDLLEXPORT Datum
timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  timestamp_stbox(t, result);
  PG_RETURN_POINTER(result);
}

/**
 * Transform a timestamp set to a spatiotemporal box
 * (internal function)
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
 * Peak into a timestamp set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
timestampset_stbox_slice(Datum tsdatum, STBOX *box)
{
  TimestampSet *ts = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) tsdatum))
    ts = (TimestampSet *) PG_DETOAST_DATUM_SLICE(tsdatum, 0,
      time_max_header_size());
  else
    ts = (TimestampSet *) tsdatum;
  timestampset_stbox(ts, box);
  POSTGIS_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(timestampset_to_stbox);
/**
 * Transform a timestamp set to a spatiotemporal box
 */
PGDLLEXPORT Datum
timestampset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  timestampset_stbox_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

/**
 * Transform a period to a spatiotemporal box
 * (internal function)
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

PG_FUNCTION_INFO_V1(period_to_stbox);
/**
 * Transform a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
period_to_stbox(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  period_stbox(p, result);
  PG_RETURN_POINTER(result);
}

/**
 * Transform a period set to a spatiotemporal box
 * (internal function)
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
 * Peak into a period set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
periodset_stbox_slice(Datum psdatum, STBOX *box)
{
  PeriodSet *ps = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) psdatum))
    ps = (PeriodSet *) PG_DETOAST_DATUM_SLICE(psdatum, 0,
      time_max_header_size());
  else
    ps = (PeriodSet *) psdatum;
  periodset_stbox(ps, box);
  POSTGIS_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PG_FUNCTION_INFO_V1(periodset_to_stbox);
/**
 * Transform a period set to a spatiotemporal box
 */
PGDLLEXPORT Datum
periodset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  periodset_stbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geo_timestamp_to_stbox);
/**
 * Transform a geometry/geography and a timestamp to a spatiotemporal box
 */
PGDLLEXPORT Datum
geo_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  geo_stbox(gs, result);
  result->tmin = result->tmax = t;
  MOBDB_FLAGS_SET_T(result->flags, true);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geo_period_to_stbox);
/**
 * Transform a geometry/geography and a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
geo_period_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Period *p = PG_GETARG_PERIOD_P(1);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  geo_stbox(gs, result);
  result->tmin = p->lower;
  result->tmax = p->upper;
  MOBDB_FLAGS_SET_T(result->flags, true);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_hasx);
/**
 * Returns true if the spatiotemporal box has X dimension
 */
PGDLLEXPORT Datum
stbox_hasx(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  bool result = MOBDB_FLAGS_GET_X(box->flags);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_hasz);
/**
 * Returns true if the spatiotemporal box has Z dimension
 */
PGDLLEXPORT Datum
stbox_hasz(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  bool result = MOBDB_FLAGS_GET_Z(box->flags);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_hast);
/**
 * Returns true if the spatiotemporal box has T dimension
 */
PGDLLEXPORT Datum
stbox_hast(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  bool result = MOBDB_FLAGS_GET_T(box->flags);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_isgeodetic);
/**
 * Returns true if the spatiotemporal box is geodetic
 */
PGDLLEXPORT Datum
stbox_isgeodetic(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  bool result = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(stbox_xmin);
/**
 * Returns the minimum X value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_xmin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->xmin);
}

PG_FUNCTION_INFO_V1(stbox_xmax);
/**
 * Returns the maximum X value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_xmax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->xmax);
}

PG_FUNCTION_INFO_V1(stbox_ymin);
/**
 * Returns the minimum Y value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_ymin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->ymin);
}

PG_FUNCTION_INFO_V1(stbox_ymax);
/**
 * Returns the maximum Y value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_ymax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->ymax);
}

PG_FUNCTION_INFO_V1(stbox_zmin);
/**
 * Returns the minimum Z value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_zmin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_Z(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->zmin);
}

PG_FUNCTION_INFO_V1(stbox_zmax);
/**
 * Returns the maximum Z value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_zmax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_Z(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->zmax);
}

PG_FUNCTION_INFO_V1(stbox_tmin);
/**
 * Returns the minimum timestamp value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_tmin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_T(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(box->tmin);
}

PG_FUNCTION_INFO_V1(stbox_tmax);
/**
 * Returns the maximum timestamp value of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_tmax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (!MOBDB_FLAGS_GET_T(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(box->tmax);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_srid);
/**
 * Returns the SRID of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_INT32(box->srid);
}

PG_FUNCTION_INFO_V1(stbox_set_srid);
/**
 * Sets the SRID of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_set_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  STBOX *result = stbox_copy(box);
  result->srid = srid;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(stbox_transform);
/**
 * Transform a spatiotemporal box into another spatial reference system
 */
PGDLLEXPORT Datum
stbox_transform(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum srid = PG_GETARG_DATUM(1);
  ensure_has_X_stbox(box);
  STBOX *result = stbox_copy(box);
  result->srid = DatumGetInt32(srid);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  Datum min = point_make(box->xmin, box->ymin, box->zmin, hasz, geodetic,
    box->srid);
  Datum max = point_make(box->xmax, box->ymax, box->zmax, hasz, geodetic,
    box->srid);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum min1 = datum_transform(min, srid);
  Datum max1 = datum_transform(max, srid);
  if (hasz)
  {
    const POINT3DZ *ptmin1 = datum_get_point3dz_p(min1);
    const POINT3DZ *ptmax1 = datum_get_point3dz_p(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->zmin = ptmin1->z;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
    result->zmax = ptmax1->z;
  }
  else
  {
    const POINT2D *ptmin1 = datum_get_point2d_p(min1);
    const POINT2D *ptmax1 = datum_get_point2d_p(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
  }
  pfree(DatumGetPointer(min)); pfree(DatumGetPointer(max));
  pfree(DatumGetPointer(min1)); pfree(DatumGetPointer(max1));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * Expand the spatial dimension of the spatiotemporal box with the double value
 * (internal function)
 */
STBOX *
stbox_expand_spatial_internal(const STBOX *box, double d)
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

PG_FUNCTION_INFO_V1(stbox_expand_spatial);
/**
 * Expand the spatial dimension of the spatiotemporal box with the double value
 */
PGDLLEXPORT Datum
stbox_expand_spatial(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(stbox_expand_spatial_internal(box, d));
}

/**
 * Expand the temporal dimension of the spatiotemporal box with the interval value
 * (internal function)
 */
STBOX *
stbox_expand_temporal_internal(const STBOX *box, Datum interval)
{
  ensure_has_T_stbox(box);
  STBOX *result = stbox_copy(box);
  result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval,
    TimestampTzGetDatum(box->tmin), interval));
  result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval,
    TimestampTzGetDatum(box->tmax), interval));
  return result;
}

PG_FUNCTION_INFO_V1(stbox_expand_temporal);
/**
 * Expand the temporal dimension of the spatiotemporal box with the interval value
 */
PGDLLEXPORT Datum
stbox_expand_temporal(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum interval = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(stbox_expand_temporal_internal(box, interval));
}

PG_FUNCTION_INFO_V1(stbox_round);
/**
 * Sets the precision of the coordinates of the spatiotemporal box
 */
PGDLLEXPORT Datum
stbox_round(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum prec = PG_GETARG_DATUM(1);
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
  PG_RETURN_POINTER(result);
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
 * Returns true if the first spatiotemporal box contains the second one
 * (internal function)
 */
bool
contains_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
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

PG_FUNCTION_INFO_V1(contains_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box contains the second one
 */
PGDLLEXPORT Datum
contains_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contains_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is contained by the second one
 * (internal function)
 */
bool
contained_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  return contains_stbox_stbox_internal(box2, box1);
}

PG_FUNCTION_INFO_V1(contained_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is contained by the second one
 */
PGDLLEXPORT Datum
contained_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contained_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the spatiotemporal boxes overlap
 * (internal function)
 */
bool
overlaps_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
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

PG_FUNCTION_INFO_V1(overlaps_stbox_stbox);
/**
 * Returns true if the spatiotemporal boxes overlap
 */
PGDLLEXPORT Datum
overlaps_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overlaps_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the spatiotemporal boxes are equal on the common dimensions
 * (internal function)
 */
bool
same_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
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

PG_FUNCTION_INFO_V1(same_stbox_stbox);
/**
 * Returns true if the spatiotemporal boxes are equal on the common dimensions
 */
PGDLLEXPORT Datum
same_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(same_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the spatiotemporal boxes are adjacent
 * (internal function)
 */
bool
adjacent_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  bool hasx, hasz, hast, geodetic;
  topo_stbox_stbox_init(box1, box2, &hasx, &hasz, &hast, &geodetic);
  STBOX inter;
  if (! inter_stbox_stbox(box1, box2, &inter))
    return false;
  /* Boxes are adjacent if they share n dimensions and their intersection is
   * at most of n-1 dimensions */
  if (!hasx && hast)
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

PG_FUNCTION_INFO_V1(adjacent_stbox_stbox);
/**
 * Returns true if the spatiotemporal boxes are adjacent
 */
PGDLLEXPORT Datum
adjacent_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(adjacent_stbox_stbox_internal(box1, box2));
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
 * Returns true if the first spatiotemporal box is strictly to the left of the second one
 * (internal function)
 */
bool
left_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmax < box2->xmin);
}

PG_FUNCTION_INFO_V1(left_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly to the left of the second one
 */
PGDLLEXPORT Datum
left_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(left_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend to the right of the second one
 * (internal function)
 */
bool
overleft_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmax <= box2->xmax);
}

PG_FUNCTION_INFO_V1(overleft_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend to the right of the second one
 */
PGDLLEXPORT Datum
overleft_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overleft_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly to the right of the second one
 * (internal function)
 */
bool
right_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmin > box2->xmax);
}

PG_FUNCTION_INFO_V1(right_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly to the right of the second one
 */
PGDLLEXPORT Datum
right_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(right_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatio temporal box does not extend to the left of the second one
 * (internal function)
 */
bool
overright_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->xmin >= box2->xmin);
}

PG_FUNCTION_INFO_V1(overright_stbox_stbox);
/**
 * Returns true if the first spatio temporal box does not extend to the left of the second one
 */
PGDLLEXPORT Datum
overright_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overright_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly below of the second one
 * (internal function)
 */
bool
below_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymax < box2->ymin);
}

PG_FUNCTION_INFO_V1(below_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly below of the second one
 */
PGDLLEXPORT Datum
below_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(below_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend above of the second one
 * (internal function)
 */
bool
overbelow_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymax <= box2->ymax);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend above of the second one
 */
PGDLLEXPORT Datum
overbelow_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbelow_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly above of the second one
 * (internal function)
 */
bool
above_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymin > box2->ymax);
}

PG_FUNCTION_INFO_V1(above_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly above of the second one
 */
PGDLLEXPORT Datum
above_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(above_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend below of the second one
 * (internal function)
 */
bool
overabove_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_X_stbox(box1);
  ensure_has_X_stbox(box2);
  pos_stbox_stbox_test(box1, box2);
  return (box1->ymin >= box2->ymin);
}

PG_FUNCTION_INFO_V1(overabove_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend below of the second one
 */
PGDLLEXPORT Datum
overabove_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overabove_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly in front of the second one
 * (internal function)
 */
bool
front_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmax < box2->zmin);
}

PG_FUNCTION_INFO_V1(front_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly in front of the second one
 */
PGDLLEXPORT Datum
front_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(front_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend to the back of the second one
 * (internal function)
 */
bool
overfront_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmax <= box2->zmax);
}

PG_FUNCTION_INFO_V1(overfront_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend to the back of the second one
 */
PGDLLEXPORT Datum
overfront_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overfront_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly back of the second one
 * (internal function)
 */
bool
back_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmin > box2->zmax);
}

PG_FUNCTION_INFO_V1(back_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly back of the second one
 */
PGDLLEXPORT Datum
back_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(back_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box does not extend to the front of the second one
 * (internal function)
 */
bool
overback_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_Z(box1->flags);
  ensure_has_Z(box2->flags);
  pos_stbox_stbox_test(box1, box2);
  return (box1->zmin >= box2->zmin);
}

PG_FUNCTION_INFO_V1(overback_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box does not extend to the front of the second one
 */
PGDLLEXPORT Datum
overback_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overback_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly before the second one
 * (internal function)
 */
bool
before_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmax < box2->tmin);
}

PG_FUNCTION_INFO_V1(before_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly before the second one
 */
PGDLLEXPORT Datum
before_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(before_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend after the second one
 * (internal function)
 */
bool
overbefore_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmax <= box2->tmax);
}

PG_FUNCTION_INFO_V1(overbefore_stbox_stbox);
/**
 * Returns true if the first temporal box does not extend after the second one
 */
PGDLLEXPORT Datum
overbefore_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbefore_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first spatiotemporal box is strictly after the second one
 * (internal function)
 */
bool
after_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmin > box2->tmax);
}

PG_FUNCTION_INFO_V1(after_stbox_stbox);
/**
 * Returns true if the first spatiotemporal box is strictly after the second one
 */
PGDLLEXPORT Datum
after_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(after_stbox_stbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend before the second one
 * (internal function)
 */
bool
overafter_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
  ensure_has_T_stbox(box1);
  ensure_has_T_stbox(box2);
  return (box1->tmin >= box2->tmin);
}

PG_FUNCTION_INFO_V1(overafter_stbox_stbox);
/**
 * Returns true if the first temporal box does not extend before the second one
 */
PGDLLEXPORT Datum
overafter_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overafter_stbox_stbox_internal(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * Returns the union of the spatiotemporal boxes
 * (internal function)
 */
static STBOX *
union_stbox_stbox_internal(const STBOX *box1, const STBOX *box2, bool strict)
{
  ensure_same_geodetic(box1->flags, box2->flags);
  ensure_same_dimensionality(box1->flags, box2->flags);
  ensure_same_srid_stbox(box1, box2);
  /* If the strict parameter is true, we need to ensure that the boxes
   * intersect, otherwise their union cannot be represented by a box */
  if (strict && ! overlaps_stbox_stbox_internal(box1, box2))
    elog(ERROR, "Result of box union would not be contiguous");

  STBOX *result = stbox_copy(box1);
  stbox_expand(box2, result);
  return result;
}

PG_FUNCTION_INFO_V1(union_stbox_stbox);
/**
 * Returns the union of the spatiotemporal boxes
 */
PGDLLEXPORT Datum
union_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  STBOX *result = union_stbox_stbox_internal(box1, box2, true);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(intersection_stbox_stbox);
/**
 * Returns the intersection of the spatiotemporal boxes
 */
PGDLLEXPORT Datum
intersection_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  STBOX *result = palloc(sizeof(STBOX));
  if (! inter_stbox_stbox(box1, box2, result))
  {
    pfree(result);
    PG_RETURN_NULL();
  }
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(stbox_extent_transfn);
/**
 * Transition function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
stbox_extent_transfn(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);

  /* Can't do anything with null inputs */
  if (!box1 && !box2)
    PG_RETURN_NULL();
  STBOX *result = (STBOX *) palloc0(sizeof(STBOX));
  /* One of the boxes is null, return the other one */
  if (!box1)
  {
    memcpy(result, box2, sizeof(STBOX));
    PG_RETURN_POINTER(result);
  }
  if (!box2)
  {
    memcpy(result, box1, sizeof(STBOX));
    PG_RETURN_POINTER(result);
  }

  /* Both boxes are not null */
  memcpy(result, box1, sizeof(STBOX));
  stbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(stbox_extent_combinefn);
/**
 * Combine function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
stbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);

  if (!box2 && !box1)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (box2 && !box1)
    PG_RETURN_POINTER(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality(box1->flags, box2->flags);
  STBOX *result = stbox_copy(box1);
  stbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * Returns -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal, or greater than the second one
 * (internal function)
 *
 * @note Function used for B-tree comparison
 */
int
stbox_cmp_internal(const STBOX *box1, const STBOX *box2)
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

PG_FUNCTION_INFO_V1(stbox_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
stbox_cmp(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  int cmp = stbox_cmp_internal(box1, box2);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(stbox_lt);
/**
 * Returns true if the first spatiotemporal box is less than the second one
 */
PGDLLEXPORT Datum
stbox_lt(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  int cmp = stbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(stbox_le);
/**
 * Returns true if the first spatiotemporal box is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
stbox_le(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  int cmp = stbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(stbox_ge);
/**
 * Returns true if the first spatiotemporal box is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
stbox_ge(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  int cmp = stbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(stbox_gt);
/**
 * Returns true if the first spatiotemporal box is greater than the second one
 */
PGDLLEXPORT Datum
stbox_gt(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  int cmp = stbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp > 0);
}

/**
 * Returns true if the two spatiotemporal boxes are equal
 * (internal function)
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
stbox_eq_internal(const STBOX *box1, const STBOX *box2)
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

PG_FUNCTION_INFO_V1(stbox_eq);
/**
 * Returns true if the two spatiotemporal boxes are equal
 */
PGDLLEXPORT Datum
stbox_eq(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_eq_internal(box1, box2));
}

PG_FUNCTION_INFO_V1(stbox_ne);
/**
 * Returns true if the two spatiotemporal boxes are different
 */
PGDLLEXPORT Datum
stbox_ne(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(! stbox_eq_internal(box1, box2));
}

/*****************************************************************************/


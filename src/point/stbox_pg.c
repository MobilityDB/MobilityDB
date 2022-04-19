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

/*****************************************************************************
 * Input/Ouput functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_in);
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
Stbox_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  STBOX *result = stbox_parse(&input);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Stbox_out);
/**
 * Output function for spatiotemporal boxes.
 */
PGDLLEXPORT Datum
Stbox_out(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  char *result = stbox_to_string(box);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Stbox_send);
/**
 * Send function for STBOX
 */
PGDLLEXPORT Datum
Stbox_send(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  stbox_write(box, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Stbox_recv);
/**
 * Receive function for STBOX
 */
PGDLLEXPORT Datum
Stbox_recv(PG_FUNCTION_ARGS)
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
stbox_constructor_ext(FunctionCallInfo fcinfo, bool hasx, bool hasz,
  bool hast, bool geodetic)
{
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  int srid = 0; /* make Codacy quiet */

  if (! hasx && hast)
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

PG_FUNCTION_INFO_V1(Stbox_constructor_t);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Stbox_constructor_t(PG_FUNCTION_ARGS)
{
  if (PG_NARGS() > 3)
    return stbox_constructor_ext(fcinfo, true, false, true, false);
  return stbox_constructor_ext(fcinfo, false, false, true, false);
}

PG_FUNCTION_INFO_V1(Stbox_constructor);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Stbox_constructor(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, false, false, false);
}

PG_FUNCTION_INFO_V1(Stbox_constructor_z);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Stbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, false, false);
}

PG_FUNCTION_INFO_V1(Stbox_constructor_zt);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Stbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, true, false);
}

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to ::timestamptz is needed */

PG_FUNCTION_INFO_V1(Geodstbox_constructor_t);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Geodstbox_constructor_t(PG_FUNCTION_ARGS)
{
  if (PG_NARGS() > 3)
    return stbox_constructor_ext(fcinfo, true, false, true, true);
  return stbox_constructor_ext(fcinfo, false, false, true, true);
}

PG_FUNCTION_INFO_V1(Geodstbox_constructor);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Geodstbox_constructor(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, false, false, true);
}

PG_FUNCTION_INFO_V1(Geodstbox_constructor_z);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Geodstbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, false, true);
}

PG_FUNCTION_INFO_V1(Geodstbox_constructor_zt);
/**
 * Construct a spatiotemporal box from the arguments
 */
PGDLLEXPORT Datum
Geodstbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, true, true);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_to_period);
/**
 * Cast the spatiotemporal box as a period
 */
PGDLLEXPORT Datum
Stbox_to_period(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  ensure_has_T_stbox(box);
  Period *result = period_make(box->tmin, box->tmax, true, true);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Stbox_to_box2d);
/**
 * Cast the spatiotemporal box as a GBOX value for PostGIS
 */
PGDLLEXPORT Datum
Stbox_to_box2d(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  ensure_has_X_stbox(box);
  GBOX *result = (GBOX *) palloc0(sizeof(GBOX));
  stbox_gbox(box, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Stbox_to_box3d);
/**
 * Cast the spatiotemporal box as a BOX3D value for PostGIS
 */
PGDLLEXPORT Datum
Stbox_to_box3d(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  ensure_has_X_stbox(box);
  BOX3D *result = (BOX3D *) palloc0(sizeof(BOX3D));
  stbox_box3d(box, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Stbox_to_geometry);
/**
 * Cast the spatiotemporal box as a GBOX value for PostGIS
 */
PGDLLEXPORT Datum
Stbox_to_geometry(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum result = stbox_geometry(box);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Transform a <Type> to a STBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Box2d_to_stbox);
/**
 * Transform a box2d to a spatiotemporal box
 */
PGDLLEXPORT Datum
Box2d_to_stbox(PG_FUNCTION_ARGS)
{
  GBOX *box = (GBOX *) PG_GETARG_POINTER(0);
  STBOX *result = stbox_make(true, false, false, false, 0,
    box->xmin, box->xmax, box->ymin, box->ymax, 0, 0, 0, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Box3d_to_stbox);
/**
 * Transform a box3d to a spatiotemporal box
 */
PGDLLEXPORT Datum
Box3d_to_stbox(PG_FUNCTION_ARGS)
{
  BOX3D *box = (BOX3D *) PG_GETARG_POINTER(0);
  STBOX *result = stbox_make(true, true, false, false, box->srid, box->xmin,
    box->xmax, box->ymin, box->ymax, box->zmin, box->zmax, 0, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Geo_to_stbox);
/**
 * Transform a geometry/geography to a spatiotemporal box
 */
PGDLLEXPORT Datum
Geo_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  bool found = geo_stbox(gs, result);
  PG_FREE_IF_COPY(gs, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestamp_to_stbox);
/**
 * Transform a timestampt to a spatiotemporal box
 */
PGDLLEXPORT Datum
Timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  timestamp_stbox(t, result);
  PG_RETURN_POINTER(result);
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
  PG_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_to_stbox);
/**
 * Transform a timestamp set to a spatiotemporal box
 */
PGDLLEXPORT Datum
Timestampset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  timestampset_stbox_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_stbox);
/**
 * Transform a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
Period_to_stbox(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  period_stbox(p, result);
  PG_RETURN_POINTER(result);
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
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PG_FUNCTION_INFO_V1(Periodset_to_stbox);
/**
 * Transform a period set to a spatiotemporal box
 */
PGDLLEXPORT Datum
Periodset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  periodset_stbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Geo_timestamp_to_stbox);
/**
 * Transform a geometry/geography and a timestamp to a spatiotemporal box
 */
PGDLLEXPORT Datum
Geo_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBOX *result = geo_timestamp_to_stbox(gs, t);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Geo_period_to_stbox);
/**
 * Transform a geometry/geography and a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
Geo_period_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  STBOX *result = geo_period_to_stbox(gs, p);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_hasx);
/**
 * Return true if the spatiotemporal box has X dimension
 */
PGDLLEXPORT Datum
Stbox_hasx(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasx(box));
}

PG_FUNCTION_INFO_V1(Stbox_hasz);
/**
 * Return true if the spatiotemporal box has Z dimension
 */
PGDLLEXPORT Datum
Stbox_hasz(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasz(box));
}

PG_FUNCTION_INFO_V1(Stbox_hast);
/**
 * Return true if the spatiotemporal box has T dimension
 */
PGDLLEXPORT Datum
Stbox_hast(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hast(box));
}

PG_FUNCTION_INFO_V1(Stbox_isgeodetic);
/**
 * Return true if the spatiotemporal box is geodetic
 */
PGDLLEXPORT Datum
Stbox_isgeodetic(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_isgeodetic(box));
}

PG_FUNCTION_INFO_V1(Stbox_xmin);
/**
 * Return the minimum X value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_xmin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_xmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Stbox_xmax);
/**
 * Return the maximum X value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_xmax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_xmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Stbox_ymin);
/**
 * Return the minimum Y value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_ymin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_ymin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Stbox_ymax);
/**
 * Return the maximum Y value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_ymax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_ymax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Stbox_zmin);
/**
 * Return the minimum Z value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_zmin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_zmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Stbox_zmax);
/**
 * Return the maximum Z value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_zmax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_zmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Stbox_tmin);
/**
 * Return the minimum T value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_tmin(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  TimestampTz result;
  if (! stbox_tmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Stbox_tmax);
/**
 * Return the maximum T value of the spatiotemporal box, if any.
 */
PGDLLEXPORT Datum
Stbox_tmax(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  TimestampTz result;
  if (! stbox_tmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_get_srid);
/**
 * Return the SRID of the spatiotemporal box
 */
PGDLLEXPORT Datum
Stbox_get_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_INT32(stbox_get_srid(box));
}

PG_FUNCTION_INFO_V1(Stbox_set_srid);
/**
 * Sets the SRID of the spatiotemporal box
 */
PGDLLEXPORT Datum
Stbox_set_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  STBOX *result = stbox_set_srid(box, srid);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Stbox_transform);
/**
 * Transform a spatiotemporal box into another spatial reference system
 */
PGDLLEXPORT Datum
Stbox_transform(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  STBOX *result = stbox_transform(box, srid);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_expand_spatial);
/**
 * Expand the spatial dimension of the spatiotemporal box with the double value
 */
PGDLLEXPORT Datum
Stbox_expand_spatial(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(stbox_expand_spatial(box, d));
}

PG_FUNCTION_INFO_V1(Stbox_expand_temporal);
/**
 * Expand the temporal dimension of the spatiotemporal box with the interval value
 */
PGDLLEXPORT Datum
Stbox_expand_temporal(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum interval = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(stbox_expand_temporal(box, interval));
}

PG_FUNCTION_INFO_V1(Stbox_round);
/**
 * Sets the precision of the coordinates of the spatiotemporal box
 */
PGDLLEXPORT Datum
Stbox_round(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum prec = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(stbox_round(box, prec));
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_stbox_stbox);
/**
 * Return true if the first spatiotemporal box contains the second one
 */
PGDLLEXPORT Datum
Contains_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contains_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Contained_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is contained by the second one
 */
PGDLLEXPORT Datum
Contained_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contained_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overlaps_stbox_stbox);
/**
 * Return true if the spatiotemporal boxes overlap
 */
PGDLLEXPORT Datum
Overlaps_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overlaps_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Same_stbox_stbox);
/**
 * Return true if the spatiotemporal boxes are equal on the common dimensions
 */
PGDLLEXPORT Datum
Same_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(same_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Adjacent_stbox_stbox);
/**
 * Return true if the spatiotemporal boxes are adjacent
 */
PGDLLEXPORT Datum
Adjacent_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(adjacent_stbox_stbox(box1, box2));
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Left_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly to the left of the second one
 */
PGDLLEXPORT Datum
Left_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(left_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overleft_stbox_stbox);
/**
 * Return true if the first spatiotemporal box does not extend to the right of the second one
 */
PGDLLEXPORT Datum
Overleft_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overleft_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Right_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly to the right of the second one
 */
PGDLLEXPORT Datum
Right_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(right_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overright_stbox_stbox);
/**
 * Return true if the first spatio temporal box does not extend to the left of the second one
 */
PGDLLEXPORT Datum
Overright_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overright_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Below_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly below of the second one
 */
PGDLLEXPORT Datum
Below_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(below_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overbelow_stbox_stbox);
/**
 * Return true if the first spatiotemporal box does not extend above of the second one
 */
PGDLLEXPORT Datum
Overbelow_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbelow_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Above_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly above of the second one
 */
PGDLLEXPORT Datum
Above_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(above_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overabove_stbox_stbox);
/**
 * Return true if the first spatiotemporal box does not extend below of the second one
 */
PGDLLEXPORT Datum
Overabove_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overabove_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Front_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly in front of the second one
 */
PGDLLEXPORT Datum
Front_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(front_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overfront_stbox_stbox);
/**
 * Return true if the first spatiotemporal box does not extend to the back of the second one
 */
PGDLLEXPORT Datum
Overfront_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overfront_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Back_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly back of the second one
 */
PGDLLEXPORT Datum
Back_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(back_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overback_stbox_stbox);
/**
 * Return true if the first spatiotemporal box does not extend to the front of the second one
 */
PGDLLEXPORT Datum
Overback_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overback_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Before_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly before the second one
 */
PGDLLEXPORT Datum
Before_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(before_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overbefore_stbox_stbox);
/**
 * Return true if the first temporal box does not extend after the second one
 */
PGDLLEXPORT Datum
Overbefore_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbefore_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(After_stbox_stbox);
/**
 * Return true if the first spatiotemporal box is strictly after the second one
 */
PGDLLEXPORT Datum
After_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(after_stbox_stbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overafter_stbox_stbox);
/**
 * Return true if the first temporal box does not extend before the second one
 */
PGDLLEXPORT Datum
Overafter_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overafter_stbox_stbox(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_stbox_stbox);
/**
 * Return the union of the spatiotemporal boxes
 */
PGDLLEXPORT Datum
Union_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  STBOX *result = union_stbox_stbox(box1, box2, true);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_stbox_stbox);
/**
 * Return the intersection of the spatiotemporal boxes
 */
PGDLLEXPORT Datum
Intersection_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  STBOX *result = intersection_stbox_stbox(box1, box2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_extent_transfn);
/**
 * Transition function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Stbox_extent_transfn(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(Stbox_extent_combinefn);
/**
 * Combine function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Stbox_extent_combinefn(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(Stbox_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
Stbox_cmp(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_INT32(stbox_cmp(box1, box2));
}

PG_FUNCTION_INFO_V1(Stbox_lt);
/**
 * Return true if the first spatiotemporal box is less than the second one
 */
PGDLLEXPORT Datum
Stbox_lt(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_lt(box1, box2));
}

PG_FUNCTION_INFO_V1(Stbox_le);
/**
 * Return true if the first spatiotemporal box is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Stbox_le(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_le(box1, box2));
}

PG_FUNCTION_INFO_V1(Stbox_ge);
/**
 * Return true if the first spatiotemporal box is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Stbox_ge(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_ge(box1, box2));
}

PG_FUNCTION_INFO_V1(Stbox_gt);
/**
 * Return true if the first spatiotemporal box is greater than the second one
 */
PGDLLEXPORT Datum
Stbox_gt(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_gt(box1, box2));
}

PG_FUNCTION_INFO_V1(Stbox_eq);
/**
 * Return true if the two spatiotemporal boxes are equal
 */
PGDLLEXPORT Datum
Stbox_eq(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_eq(box1, box2));
}

PG_FUNCTION_INFO_V1(Stbox_ne);
/**
 * Return true if the two spatiotemporal boxes are different
 */
PGDLLEXPORT Datum
Stbox_ne(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_ne(box1, box2));
}

/*****************************************************************************/


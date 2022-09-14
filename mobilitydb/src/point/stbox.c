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
/* PostgreSQL */
#include <lib/stringinfo.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporal_out.h"
#include "general/temporal_util.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/tnumber_mathfuncs.h"
#include "pg_point/postgis.h"
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Input/Ouput functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_in);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Input function for spatiotemporal boxes.
 * @sqlfunc stbox_in()
 */
PGDLLEXPORT Datum
Stbox_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(stbox_in(input));
}

PG_FUNCTION_INFO_V1(Stbox_out);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Output function for spatiotemporal boxes.
 * @sqlfunc stbox_out()
 */
PGDLLEXPORT Datum
Stbox_out(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_CSTRING(stbox_out(box, OUT_DEFAULT_DECIMAL_DIGITS));
}

PG_FUNCTION_INFO_V1(Stbox_recv);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Receive function for STBOX
 * @sqlfunc stbox_recv()
 */
PGDLLEXPORT Datum
Stbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  STBOX *result = stbox_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Stbox_send);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Send function for STBOX
 * @sqlfunc stbox_send()
 */
PGDLLEXPORT Datum
Stbox_send(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(box);
  uint8_t *wkb = stbox_as_wkb(box, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Output in WKT format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_as_text);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Output function for spatiotemporal boxes.
 * @sqlfunc asText()
 */
PGDLLEXPORT Datum
Stbox_as_text(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = stbox_out(box, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
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
  int srid = 0; /* make Codacy quiet */
  Span *period = NULL;

  int i = 0;
  if (hast)
  {
    period = PG_GETARG_SPAN_P(0);
    i++;
  }

  if (hasx)
  {
    if (! hasz && ! geodetic)
    {
      xmin = PG_GETARG_FLOAT8(i++);
      ymin = PG_GETARG_FLOAT8(i++);
      xmax = PG_GETARG_FLOAT8(i++);
      ymax = PG_GETARG_FLOAT8(i++);
      srid = PG_GETARG_INT32(i++);
    }
    else /* hasz || geodetic */
    {
      xmin = PG_GETARG_FLOAT8(i++);
      ymin = PG_GETARG_FLOAT8(i++);
      zmin = PG_GETARG_FLOAT8(i++);
      xmax = PG_GETARG_FLOAT8(i++);
      ymax = PG_GETARG_FLOAT8(i++);
      zmax = PG_GETARG_FLOAT8(i++);
      srid = PG_GETARG_INT32(i++);
    }
  }

  /* Construct the box */
  STBOX *result = stbox_make(period, hasx, hasz, geodetic, srid, xmin, xmax,
    ymin, ymax, zmin, zmax);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_constructor_t);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox_t()
 */
PGDLLEXPORT Datum
Stbox_constructor_t(PG_FUNCTION_ARGS)
{
  if (PG_NARGS() > 1)
    return stbox_constructor_ext(fcinfo, true, false, true, false);
  return stbox_constructor_ext(fcinfo, false, false, true, false);
}

PG_FUNCTION_INFO_V1(Stbox_constructor);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox()
 */
PGDLLEXPORT Datum
Stbox_constructor(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, false, false, false);
}

PG_FUNCTION_INFO_V1(Stbox_constructor_z);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox_z()
 */
PGDLLEXPORT Datum
Stbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, false, false);
}

PG_FUNCTION_INFO_V1(Stbox_constructor_zt);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox_zt()
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
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc geodstbox_t()
 */
PGDLLEXPORT Datum
Geodstbox_constructor_t(PG_FUNCTION_ARGS)
{
  if (PG_NARGS() > 1)
    return stbox_constructor_ext(fcinfo, true, false, true, true);
  return stbox_constructor_ext(fcinfo, false, false, true, true);
}

PG_FUNCTION_INFO_V1(Geodstbox_constructor_z);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc geodstbox_z()
 */
PGDLLEXPORT Datum
Geodstbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, false, true);
}

PG_FUNCTION_INFO_V1(Geodstbox_constructor_zt);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc geodstbox_zt()
 */
PGDLLEXPORT Datum
Geodstbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, true, true);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_to_geo);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast the spatiotemporal box as a PostGIS GBOX
 * @sqlfunc geometry()
 * @sqlfunc @p ::
 */
PGDLLEXPORT Datum
Stbox_to_geo(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Datum result = PointerGetDatum(stbox_to_geo(box));
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Stbox_to_period);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast a spatiotemporal box as a period
 * @sqlfunc period()
 * @sqlfunc @p ::
 */
PGDLLEXPORT Datum
Stbox_to_period(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Period *result = stbox_to_period(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transform a <Type> to a STBOX
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Geo_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a geometry/geography to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
PGDLLEXPORT Datum
Geo_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  STBOX *result = palloc(sizeof(STBOX));
  bool found = geo_set_stbox(gs, result);
  PG_FREE_IF_COPY(gs, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestamp_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a timestampt to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
PGDLLEXPORT Datum
Timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  STBOX *result = palloc(sizeof(STBOX));
  timestamp_set_stbox(t, result);
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
  timestampset_set_stbox(ts, box);
  PG_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a timestamp set to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
PGDLLEXPORT Datum
Timestampset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  STBOX *result = palloc(sizeof(STBOX));
  timestampset_stbox_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a period to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
PGDLLEXPORT Datum
Period_to_stbox(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  STBOX *result = palloc(sizeof(STBOX));
  period_set_stbox(p, result);
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
  periodset_set_stbox(ps, box);
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PG_FUNCTION_INFO_V1(Periodset_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a period set to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
PGDLLEXPORT Datum
Periodset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  STBOX *result = palloc(sizeof(STBOX));
  periodset_stbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Geo_timestamp_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a geometry/geography and a timestamp to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
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
 * @ingroup mobilitydb_box_cast
 * @brief Transform a geometry/geography and a period to a spatiotemporal box
 * @sqlfunc stbox()
 */
PGDLLEXPORT Datum
Geo_period_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box has value dimension
 * @sqlfunc hasX()
 */
PGDLLEXPORT Datum
Stbox_hasx(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasx(box));
}

PG_FUNCTION_INFO_V1(Stbox_hasz);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box has Z dimension
 * @sqlfunc hasZ()
 */
PGDLLEXPORT Datum
Stbox_hasz(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasz(box));
}

PG_FUNCTION_INFO_V1(Stbox_hast);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box has time dimension
 * @sqlfunc hasT()
 */
PGDLLEXPORT Datum
Stbox_hast(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hast(box));
}

PG_FUNCTION_INFO_V1(Stbox_isgeodetic);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box is geodetic
 * @sqlfunc isGeodetic()
 */
PGDLLEXPORT Datum
Stbox_isgeodetic(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_isgeodetic(box));
}

PG_FUNCTION_INFO_V1(Stbox_xmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum X value of a spatiotemporal box, if any.
 * @sqlfunc Xmin()
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum X value of a spatiotemporal box, if any.
 * @sqlfunc Xmax()
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum Y value of a spatiotemporal box, if any.
 * @sqlfunc Ymin()
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum Y value of a spatiotemporal box, if any.
 * @sqlfunc Ymax()
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum Z value of a spatiotemporal box, if any.
 * @sqlfunc Zmin()
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum Z value of a spatiotemporal box, if any.
 * @sqlfunc Zmax()
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum T value of a spatiotemporal box, if any.
 * @sqlfunc Tmin()
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
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum T value of a spatiotemporal box, if any.
 * @sqlfunc Tmax()
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
 * @ingroup mobilitydb_box_spatial
 * @brief Return the SRID of a spatiotemporal box
 * @sqlfunc SRID()
 */
PGDLLEXPORT Datum
Stbox_get_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_INT32(stbox_srid(box));
}

PG_FUNCTION_INFO_V1(Stbox_set_srid);
/**
 * @ingroup mobilitydb_box_spatial
 * @brief Sets the SRID of a spatiotemporal box
 * @sqlfunc setSRID()
 */
PGDLLEXPORT Datum
Stbox_set_srid(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  STBOX *result = stbox_set_srid(box, srid);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Transform a spatiotemporal box into another spatial reference system
 */
static STBOX *
stbox_transform(const STBOX *box, int32 srid)
{
  ensure_has_X_stbox(box);
  STBOX *result = stbox_copy(box);
  result->srid = DatumGetInt32(srid);
  bool hasz = MOBDB_FLAGS_GET_Z(box->flags);
  bool geodetic = MOBDB_FLAGS_GET_GEODETIC(box->flags);
  Datum min = PointerGetDatum(gspoint_make(box->xmin, box->ymin, box->zmin,
    hasz, geodetic, box->srid));
  Datum max = PointerGetDatum(gspoint_make(box->xmax, box->ymax, box->zmax,
    hasz, geodetic, box->srid));
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

PG_FUNCTION_INFO_V1(Stbox_transform);
/**
 * @ingroup mobilitydb_box_spatial
 * @brief Transform a spatiotemporal box into another spatial reference system
 * @sqlfunc transform()
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
 * @ingroup mobilitydb_box_transf
 * @brief Return a spatiotemporal box expanded in the spatial dimension of by a double
 * @sqlfunc expandSpatial()
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
 * @ingroup mobilitydb_box_transf
 * @brief Return a spatiotemporal box expanded in the temporal dimension by an interval
 * @sqlfunc Stbox_expand_temporal()
 */
PGDLLEXPORT Datum
Stbox_expand_temporal(PG_FUNCTION_ARGS)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_POINTER(stbox_expand_temporal(box, interval));
}

/**
 * @brief Sets the precision of the coordinates of the spatiotemporal box.
 */
static STBOX *
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

PG_FUNCTION_INFO_V1(Stbox_round);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Sets the precision of the coordinates of a spatiotemporal box
 * @sqlfunc round()
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
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first spatiotemporal box contains the second one
 * @sqlfunc stbox_contains()
 * @sqlop @p @>
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
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first spatiotemporal box is contained by the second one
 * @sqlfunc stbox_contained()
 * @sqlop @p <@
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
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the spatiotemporal boxes overlap
 * @sqlfunc stbox_overlaps()
 * @sqlop @p &&
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
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the spatiotemporal boxes are equal on the common dimensions
 * @sqlfunc stbox_same()
 * @sqlop @p ~=
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
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the spatiotemporal boxes are adjacent
 * @sqlfunc stbox_adjacent()
 * @sqlop @p -|-
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly to the left of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p >>
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the right of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &>
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly to the right of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatio temporal box does not extend to the left of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &<
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly below of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend above of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &<|
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly above of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p |>>
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend below of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p |&>
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly in front of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <</
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the back of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &</
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly back of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p />>
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the front of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p /&>
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly before the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<#
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend after the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &<#
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly after the second one
 * @sqlfunc temporal_below()
 * @sqlop @p #>>
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
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend before the second one
 * @sqlfunc temporal_below()
 * @sqlop @p #&>
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
 * @ingroup mobilitydb_box_set
 * @brief Return the union of the spatiotemporal boxes
 * @sqlfunc stbox_union()
 * @sqlop @p +
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
 * @ingroup mobilitydb_box_set
 * @brief Return the intersection of the spatiotemporal boxes
 * @sqlfunc stbox_intersection()
 * @sqlop @p *
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
 * @brief Transition function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Stbox_extent_transfn(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);

  /* Can't do anything with null inputs */
  if (! box1 && ! box2)
    PG_RETURN_NULL();
  STBOX *result = palloc(sizeof(STBOX));
  /* One of the boxes is null, return the other one */
  if (! box1)
  {
    memcpy(result, box2, sizeof(STBOX));
    PG_RETURN_POINTER(result);
  }
  if (! box2)
  {
    memcpy(result, box1, sizeof(STBOX));
    PG_RETURN_POINTER(result);
  }

  /* Both boxes are not null */
  ensure_same_srid_stbox(box1, box2);
  ensure_same_dimensionality(box1->flags, box2->flags);
  ensure_same_geodetic(box1->flags, box2->flags);
  memcpy(result, box1, sizeof(STBOX));
  stbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Stbox_extent_combinefn);
/**
 * @brief Combine function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Stbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);
  if (!box1 && !box2)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (!box1 && box2)
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
 * @ingroup mobilitydb_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfunc stbox_cmp()
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
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is less than the second one
 * @sqlfunc stbox_lt()
 * @sqlop @p <
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
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is less than or equal to
 * the second one
 * @sqlfunc stbox_le()
 * @sqlop @p <=
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
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is greater than or equal to
 * the second one
 * @sqlfunc stbox_ge()
 * @sqlop @p >=
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
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is greater than the second one
 * @sqlfunc stbox_gt()
 * @sqlop @p >
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
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the spatiotemporal boxes are equal
 * @sqlfunc stbox_eq()
 * @sqlop @p =
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
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the spatiotemporal boxes are different
 * @sqlfunc stbox_ne()
 * @sqlop @p <>
 */
PGDLLEXPORT Datum
Stbox_ne(PG_FUNCTION_ARGS)
{
  STBOX *box1 = PG_GETARG_STBOX_P(0);
  STBOX *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_ne(box1, box2));
}

/*****************************************************************************/

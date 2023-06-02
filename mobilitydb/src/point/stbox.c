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
/* PostgreSQL */
#include <lib/stringinfo.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/type_out.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/tnumber_mathfuncs.h"
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Input/Ouput functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_in);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Input function for spatiotemporal boxes.
 * @sqlfunc stbox_in()
 */
Datum
Stbox_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(stbox_in(input));
}

PGDLLEXPORT Datum Stbox_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_out);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Output function for spatiotemporal boxes.
 * @sqlfunc stbox_out()
 */
Datum
Stbox_out(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_CSTRING(stbox_out(box, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Stbox_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_recv);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Receive function for STBox
 * @sqlfunc stbox_recv()
 */
Datum
Stbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  STBox *result = stbox_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Stbox_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_send);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Send function for STBox
 * @sqlfunc stbox_send()
 */
Datum
Stbox_send(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  /* A spatiotemporal box always outputs the SRID */
  uint8_t variant = WKB_EXTENDED;
  size_t wkb_size = VARSIZE_ANY_EXHDR(box);
  uint8_t *wkb = stbox_as_wkb(box, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Output in WKT format
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_as_text);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Output function for spatiotemporal boxes.
 * @sqlfunc asText()
 */
Datum
Stbox_as_text(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
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
 * @brief Construct a spatiotemporal box from the arguments
 */
static Datum
stbox_constructor_ext(FunctionCallInfo fcinfo, bool hasx, bool hasz,
  bool hast, bool geodetic)
{
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  int srid = 0; /* make Codacy quiet */
  Span *period = NULL;

  int i = 0;
  if (hasx)
  {
    if (! hasz)
    {
      xmin = PG_GETARG_FLOAT8(i++);
      ymin = PG_GETARG_FLOAT8(i++);
      xmax = PG_GETARG_FLOAT8(i++);
      ymax = PG_GETARG_FLOAT8(i++);
    }
    else /* hasz */
    {
      xmin = PG_GETARG_FLOAT8(i++);
      ymin = PG_GETARG_FLOAT8(i++);
      zmin = PG_GETARG_FLOAT8(i++);
      xmax = PG_GETARG_FLOAT8(i++);
      ymax = PG_GETARG_FLOAT8(i++);
      zmax = PG_GETARG_FLOAT8(i++);
    }
  }
  if (hast)
  {
    period = PG_GETARG_SPAN_P(i++);
  }
  if (hasx)
    srid = PG_GETARG_INT32(i++);

  /* Construct the box */
  STBox *result = stbox_make(hasx, hasz, geodetic, srid, xmin, xmax,
    ymin, ymax, zmin, zmax, period);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_constructor_t(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_t);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox_t()
 */
Datum
Stbox_constructor_t(PG_FUNCTION_ARGS)
{
  if (PG_NARGS() > 1)
    return stbox_constructor_ext(fcinfo, true, false, true, false);
  return stbox_constructor_ext(fcinfo, false, false, true, false);
}

PGDLLEXPORT Datum Stbox_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox()
 */
Datum
Stbox_constructor(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, false, false, false);
}

PGDLLEXPORT Datum Stbox_constructor_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_z);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox_z()
 */
Datum
Stbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, false, false);
}

PGDLLEXPORT Datum Stbox_constructor_zt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_zt);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc stbox_zt()
 */
Datum
Stbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, true, false);
}

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to ::timestamptz is needed */

PGDLLEXPORT Datum Geodstbox_constructor_t(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geodstbox_constructor_t);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc geodstbox_t()
 */
Datum
Geodstbox_constructor_t(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, false, false, true, true);
}

PGDLLEXPORT Datum Geodstbox_constructor_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geodstbox_constructor_z);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc geodstbox_z()
 */
Datum
Geodstbox_constructor_z(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, false, true);
}

PGDLLEXPORT Datum Geodstbox_constructor_zt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geodstbox_constructor_zt);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Construct a spatiotemporal box from the arguments
 * @sqlfunc geodstbox_zt()
 */
Datum
Geodstbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return stbox_constructor_ext(fcinfo, true, true, true, true);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_to_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_to_geo);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast the spatiotemporal box as a PostGIS GBOX
 * @sqlfunc geometry()
 * @sqlfunc @p ::
 */
Datum
Stbox_to_geo(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Datum result = PointerGetDatum(stbox_to_geo(box));
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Stbox_to_period(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_to_period);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast a spatiotemporal box as a period
 * @sqlfunc period()
 * @sqlfunc @p ::
 */
Datum
Stbox_to_period(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Span *result = stbox_to_period(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transform a <Type> to a STBox
 *****************************************************************************/

PGDLLEXPORT Datum Geo_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a geometry/geography to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
Datum
Geo_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  STBox *result = palloc(sizeof(STBox));
  bool found = geo_set_stbox(gs, result);
  PG_FREE_IF_COPY(gs, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Geoset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a geometry/geography to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
Datum
Geoset_to_stbox(PG_FUNCTION_ARGS)
{
  Set *set = PG_GETARG_SET_P(0);
  STBox *result = palloc(sizeof(STBox));
  spatialset_set_stbox(set, result);
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Timestamp_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamp_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a timestampt to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
Datum
Timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  STBox *result = palloc(sizeof(STBox));
  timestamp_set_stbox(t, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Timestampset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestampset_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a timestamp set to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
Datum
Timestampset_to_stbox(PG_FUNCTION_ARGS)
{
  Set *ts = PG_GETARG_SET_P(0);
  STBox *result = palloc(sizeof(STBox));
  timestampset_set_stbox(ts, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Period_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Period_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a period to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
Datum
Period_to_stbox(PG_FUNCTION_ARGS)
{
  Span *p = PG_GETARG_SPAN_P(0);
  STBox *result = palloc(sizeof(STBox));
  period_set_stbox(p, result);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Peak into a period set datum to find the bounding box. If the datum
 * needs to be detoasted, extract only the header and not the full object.
 */
void
periodset_stbox_slice(Datum psdatum, STBox *box)
{
  SpanSet *ps = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) psdatum))
    ps = (SpanSet *) PG_DETOAST_DATUM_SLICE(psdatum, 0,
      time_max_header_size());
  else
    ps = (SpanSet *) psdatum;
  periodset_set_stbox(ps, box);
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PGDLLEXPORT Datum Periodset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a period set to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
Datum
Periodset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  STBox *result = palloc(sizeof(STBox));
  periodset_stbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Geo_timestamp_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_timestamp_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a geometry/geography and a timestamp to a spatiotemporal box
 * @sqlfunc stbox()
 * @sqlfunc @p ::
 */
Datum
Geo_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBox *result = geo_timestamp_to_stbox(gs, t);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Geo_period_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_period_to_stbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform a geometry/geography and a period to a spatiotemporal box
 * @sqlfunc stbox()
 */
Datum
Geo_period_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Span *p = PG_GETARG_SPAN_P(1);
  STBox *result = geo_period_to_stbox(gs, p);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_hasx(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_hasx);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box has value dimension
 * @sqlfunc hasX()
 */
Datum
Stbox_hasx(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasx(box));
}

PGDLLEXPORT Datum Stbox_hasz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_hasz);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box has Z dimension
 * @sqlfunc hasZ()
 */
Datum
Stbox_hasz(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasz(box));
}

PGDLLEXPORT Datum Stbox_hast(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_hast);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box has time dimension
 * @sqlfunc hasT()
 */
Datum
Stbox_hast(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hast(box));
}

PGDLLEXPORT Datum Stbox_isgeodetic(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_isgeodetic);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a spatiotemporal box is geodetic
 * @sqlfunc isGeodetic()
 */
Datum
Stbox_isgeodetic(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_isgeodetic(box));
}

PGDLLEXPORT Datum Stbox_xmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_xmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum X value of a spatiotemporal box, if any.
 * @sqlfunc Xmin()
 */
Datum
Stbox_xmin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_xmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_xmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_xmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum X value of a spatiotemporal box, if any.
 * @sqlfunc Xmax()
 */
Datum
Stbox_xmax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_xmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_ymin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ymin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum Y value of a spatiotemporal box, if any.
 * @sqlfunc Ymin()
 */
Datum
Stbox_ymin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_ymin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_ymax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ymax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum Y value of a spatiotemporal box, if any.
 * @sqlfunc Ymax()
 */
Datum
Stbox_ymax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_ymax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_zmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_zmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum Z value of a spatiotemporal box, if any.
 * @sqlfunc Zmin()
 */
Datum
Stbox_zmin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_zmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_zmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_zmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum Z value of a spatiotemporal box, if any.
 * @sqlfunc Zmax()
 */
Datum
Stbox_zmax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_zmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_tmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum T value of a spatiotemporal box, if any.
 * @sqlfunc Tmin()
 */
Datum
Stbox_tmin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  TimestampTz result;
  if (! stbox_tmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Stbox_tmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum T value of a spatiotemporal box, if any.
 * @sqlfunc Tmax()
 */
Datum
Stbox_tmax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  TimestampTz result;
  if (! stbox_tmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_srid);
/**
 * @ingroup mobilitydb_box_spatial
 * @brief Return the SRID of a spatiotemporal box
 * @sqlfunc SRID()
 */
Datum
Stbox_get_srid(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_INT32(stbox_srid(box));
}

PGDLLEXPORT Datum Stbox_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_set_srid);
/**
 * @ingroup mobilitydb_box_spatial
 * @brief Sets the SRID of a spatiotemporal box
 * @sqlfunc setSRID()
 */
Datum
Stbox_set_srid(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  STBox *result = stbox_set_srid(box, srid);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Transform a spatiotemporal box into another spatial reference system
 */
static STBox *
stbox_transform(const STBox *box, int32 srid)
{
  ensure_has_X_stbox(box);
  STBox *result = stbox_copy(box);
  result->srid = DatumGetInt32(srid);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);
  Datum min = PointerGetDatum(gspoint_make(box->xmin, box->ymin, box->zmin,
    hasz, geodetic, box->srid));
  Datum max = PointerGetDatum(gspoint_make(box->xmax, box->ymax, box->zmax,
    hasz, geodetic, box->srid));
  Datum min1 = datum_transform(min, srid);
  Datum max1 = datum_transform(max, srid);
  if (hasz)
  {
    const POINT3DZ *ptmin1 = DATUM_POINT3DZ_P(min1);
    const POINT3DZ *ptmax1 = DATUM_POINT3DZ_P(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->zmin = ptmin1->z;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
    result->zmax = ptmax1->z;
  }
  else
  {
    const POINT2D *ptmin1 = DATUM_POINT2D_P(min1);
    const POINT2D *ptmax1 = DATUM_POINT2D_P(max1);
    result->xmin = ptmin1->x;
    result->ymin = ptmin1->y;
    result->xmax = ptmax1->x;
    result->ymax = ptmax1->y;
  }
  pfree(DatumGetPointer(min)); pfree(DatumGetPointer(max));
  pfree(DatumGetPointer(min1)); pfree(DatumGetPointer(max1));
  return result;
}

PGDLLEXPORT Datum Stbox_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_transform);
/**
 * @ingroup mobilitydb_box_spatial
 * @brief Transform a spatiotemporal box into another spatial reference system
 * @sqlfunc transform()
 */
Datum
Stbox_transform(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  STBox *result = stbox_transform(box, srid);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_get_space(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_space);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a copy of the spatiotemporal box keeping only the space
 * dimension
 * @sqlfunc getSpace()
 */
Datum
Stbox_get_space(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_POINTER(stbox_get_space(box));
}

PGDLLEXPORT Datum Stbox_expand_space(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_expand_space);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a spatiotemporal box expanded in the spatial dimension of by a double
 * @sqlfunc expandSpace()
 */
Datum
Stbox_expand_space(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(stbox_expand_space(box, d));
}

PGDLLEXPORT Datum Stbox_expand_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_expand_time);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a spatiotemporal box expanded in the temporal dimension by an interval
 * @sqlfunc Stbox_expand_time()
 */
Datum
Stbox_expand_time(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_POINTER(stbox_expand_time(box, interval));
}

/**
 * @brief Sets the precision of the coordinates of the spatiotemporal box.
 */
static STBox *
stbox_round(const STBox *box, Datum size)
{
  ensure_has_X_stbox(box);
  STBox *result = stbox_copy(box);
  result->xmin = DatumGetFloat8(datum_round_float(Float8GetDatum(box->xmin), size));
  result->xmax = DatumGetFloat8(datum_round_float(Float8GetDatum(box->xmax), size));
  result->ymin = DatumGetFloat8(datum_round_float(Float8GetDatum(box->ymin), size));
  result->ymax = DatumGetFloat8(datum_round_float(Float8GetDatum(box->ymax), size));
  if (MEOS_FLAGS_GET_Z(box->flags) || MEOS_FLAGS_GET_GEODETIC(box->flags))
  {
    result->zmin = DatumGetFloat8(datum_round_float(Float8GetDatum(box->zmin), size));
    result->zmax = DatumGetFloat8(datum_round_float(Float8GetDatum(box->zmax), size));
  }
  return result;
}

PGDLLEXPORT Datum Stbox_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_round);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Sets the precision of the coordinates of a spatiotemporal box
 * @sqlfunc round()
 */
Datum
Stbox_round(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(stbox_round(box, size));
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PGDLLEXPORT Datum Contains_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_stbox_stbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first spatiotemporal box contains the second one
 * @sqlfunc stbox_contains()
 * @sqlop @p \@>
 */
Datum
Contains_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contains_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Contained_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_stbox_stbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first spatiotemporal box is contained in the second one
 * @sqlfunc stbox_contained()
 * @sqlop @p <@
 */
Datum
Contained_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contained_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overlaps_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_stbox_stbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the spatiotemporal boxes overlap
 * @sqlfunc stbox_overlaps()
 * @sqlop @p &&
 */
Datum
Overlaps_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overlaps_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Same_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_stbox_stbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the spatiotemporal boxes are equal in the common
 * dimensions
 * @sqlfunc stbox_same()
 * @sqlop @p ~=
 */
Datum
Same_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(same_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Adjacent_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_stbox_stbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the spatiotemporal boxes are adjacent
 * @sqlfunc stbox_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(adjacent_stbox_stbox(box1, box2));
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

PGDLLEXPORT Datum Left_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly to the left of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p >>
 */
Datum
Left_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(left_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overleft_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the right of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &>
 */
Datum
Overleft_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overleft_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Right_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly to the right of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<
 */
Datum
Right_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(right_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overright_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatio temporal box does not extend to the left of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &<
 */
Datum
Overright_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overright_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Below_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly below of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(below_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overbelow_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend above of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &<|
 */
Datum
Overbelow_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbelow_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Above_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly above of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p |>>
 */
Datum
Above_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(above_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overabove_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend below of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p |&>
 */
Datum
Overabove_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overabove_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Front_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly in front of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <</
 */
Datum
Front_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(front_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overfront_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the back of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &</
 */
Datum
Overfront_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overfront_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Back_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly back of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p />>
 */
Datum
Back_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(back_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overback_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the front of the second one
 * @sqlfunc temporal_below()
 * @sqlop @p /&>
 */
Datum
Overback_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overback_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Before_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly before the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<#
 */
Datum
Before_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(before_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overbefore_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend after the second one
 * @sqlfunc temporal_below()
 * @sqlop @p &<#
 */
Datum
Overbefore_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbefore_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum After_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first spatiotemporal box is strictly after the second one
 * @sqlfunc temporal_below()
 * @sqlop @p #>>
 */
Datum
After_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(after_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overafter_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_stbox_stbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend before the second one
 * @sqlfunc temporal_below()
 * @sqlop @p #&>
 */
Datum
Overafter_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overafter_stbox_stbox(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

PGDLLEXPORT Datum Union_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_stbox_stbox);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the union of the spatiotemporal boxes
 * @sqlfunc stbox_union()
 * @sqlop @p +
 */
Datum
Union_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  STBox *result = union_stbox_stbox(box1, box2, true);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intersection_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_stbox_stbox);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the intersection of the spatiotemporal boxes
 * @sqlfunc stbox_intersection()
 * @sqlop @p *
 */
Datum
Intersection_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  STBox *result = intersection_stbox_stbox(box1, box2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_quad_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_quad_split);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the intersection of the spatiotemporal boxes
 * @sqlfunc stbox_intersection()
 * @sqlop @p *
 */
Datum
Stbox_quad_split(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int count;
  STBox *boxes = stbox_quad_split(box, &count);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_extent_transfn);
/**
 * @brief Transition function for extent aggregation for boxes
 */
Datum
Stbox_extent_transfn(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);

  /* Can't do anything with null inputs */
  if (! box1 && ! box2)
    PG_RETURN_NULL();
  STBox *result = palloc(sizeof(STBox));
  /* One of the boxes is null, return the other one */
  if (! box1)
  {
    memcpy(result, box2, sizeof(STBox));
    PG_RETURN_POINTER(result);
  }
  if (! box2)
  {
    memcpy(result, box1, sizeof(STBox));
    PG_RETURN_POINTER(result);
  }

  /* Both boxes are not null */
  ensure_same_srid(stbox_srid(box1), stbox_srid(box2));
  ensure_same_dimensionality(box1->flags, box2->flags);
  ensure_same_geodetic(box1->flags, box2->flags);
  memcpy(result, box1, sizeof(STBox));
  stbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Stbox_extent_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_extent_combinefn);
/**
 * @brief Combine function for extent aggregation for boxes
 */
Datum
Stbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);
  if (!box1 && !box2)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (!box1 && box2)
    PG_RETURN_POINTER(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality(box1->flags, box2->flags);
  STBox *result = stbox_copy(box1);
  stbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_cmp);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfunc stbox_cmp()
 */
Datum
Stbox_cmp(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_INT32(stbox_cmp(box1, box2));
}

PGDLLEXPORT Datum Stbox_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_lt);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is less than the second one
 * @sqlfunc stbox_lt()
 * @sqlop @p <
 */
Datum
Stbox_lt(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_lt(box1, box2));
}

PGDLLEXPORT Datum Stbox_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_le);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is less than or equal to
 * the second one
 * @sqlfunc stbox_le()
 * @sqlop @p <=
 */
Datum
Stbox_le(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_le(box1, box2));
}

PGDLLEXPORT Datum Stbox_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ge);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is greater than or equal to
 * the second one
 * @sqlfunc stbox_ge()
 * @sqlop @p >=
 */
Datum
Stbox_ge(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_ge(box1, box2));
}

PGDLLEXPORT Datum Stbox_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gt);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first spatiotemporal box is greater than the second one
 * @sqlfunc stbox_gt()
 * @sqlop @p >
 */
Datum
Stbox_gt(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_gt(box1, box2));
}

PGDLLEXPORT Datum Stbox_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_eq);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the spatiotemporal boxes are equal
 * @sqlfunc stbox_eq()
 * @sqlop @p =
 */
Datum
Stbox_eq(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_eq(box1, box2));
}

PGDLLEXPORT Datum Stbox_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ne);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the spatiotemporal boxes are different
 * @sqlfunc stbox_ne()
 * @sqlop @p <>
 */
Datum
Stbox_ne(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_ne(box1, box2));
}

/*****************************************************************************/

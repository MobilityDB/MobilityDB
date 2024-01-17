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
 * @brief Functions for temporal bounding boxes
 */

#include "general/tbox.h"

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/temporal.h"
#include "general/type_out.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_in);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return a temporal box from its Well-Known Text (WKT) representation
 *
 * Examples of input:
 * @code
 * TBOX XT([1.0, 3.0), [2020-01-01, 2020-01-03])   -- Both X and T dimensions
 * TBOXINT X([1, 3))     -- Only X dimension, integer span
 * TBOXFLOAT X([1, 3))   -- Only X dimension, float span
 * TBOX X([1.0, 3.0))    -- Only X dimension, float span is assumed
 * TBOX T([2020-01-01, 2020-01-03])  -- Only T dimension
 * @endcode
 * @sqlfn tbox_in()
 */
Datum
Tbox_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  PG_RETURN_TBOX_P(tbox_in(input));
}

PGDLLEXPORT Datum Tbox_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_out);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal box
 * @sqlfn tbox_out()
 */
Datum
Tbox_out(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_CSTRING(tbox_out(box, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Tbox_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_recv);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return a temporal box from its Well-Known Binary (WKB) representation
 * @sqlfn tbox_recv()
 */
Datum
Tbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  TBox *result = tbox_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_TBOX_P(result);
}

PGDLLEXPORT Datum Tbox_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_send);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return the Well-Known Binary (WKB) representation of a temporal box
 * @sqlfn tbox_send()
 */
Datum
Tbox_send(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  uint8_t variant = 0;
  size_t wkb_size;
  uint8_t *wkb = tbox_as_wkb(box, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Output in WKT format
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_as_text);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal box
 * @sqlfn asText()
 */
Datum
Tbox_as_text(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = tbox_out(box, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Number_timestamptz_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_timestamptz_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Return a temporal box constructed from a number and a timestamptz
 * @sqlfn tbox()
 */
Datum
Number_timestamptz_to_tbox(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_TBOX_P(number_timestamptz_to_tbox(value, basetype, t));
}

PGDLLEXPORT Datum Number_tstzspan_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_tstzspan_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Return a temporal box constructed from a number and a timestamptz span
 * @sqlfn tbox()
 */
Datum
Number_tstzspan_to_tbox(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_TBOX_P(number_tstzspan_to_tbox(value, basetype, s));
}

PGDLLEXPORT Datum Numspan_timestamptz_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_timestamptz_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Return a temporal box constructed from a number span and a timestamptz
 * @sqlfn tbox()
 */
Datum
Numspan_timestamptz_to_tbox(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_TBOX_P(numspan_timestamptz_to_tbox(span, t));
}

PGDLLEXPORT Datum Numspan_tstzspan_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_tstzspan_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Return a temporal box constructed from a number span and a
 * timestamptz span
 * @sqlfn tbox()
 */
Datum
Numspan_tstzspan_to_tbox(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Span *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_TBOX_P(numspan_tstzspan_to_tbox(s, p));
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Number_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_to_tbox);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a number converted to a temporal box
 * @sqlfn tbox()
 */
Datum
Number_to_tbox(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_TBOX_P(number_to_tbox(value, basetype));
}

PGDLLEXPORT Datum Numeric_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numeric_to_tbox);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a numeric converted to a temporal box
 * @sqlfn tbox()
 */
Datum
Numeric_to_tbox(PG_FUNCTION_ARGS)
{
  Datum num = PG_GETARG_DATUM(0);
  Datum d = call_function1(numeric_float8, num);
  PG_RETURN_TBOX_P(number_to_tbox(d, T_FLOAT8));
}

PGDLLEXPORT Datum Timestamptz_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_to_tbox);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Conver a timestamptz to a temporal box
 * @sqlfn tbox()
 */
Datum
Timestamptz_to_tbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PG_RETURN_TBOX_P(timestamptz_to_tbox(t));
}

PGDLLEXPORT Datum Set_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_to_tbox);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a set converted to a temporal box
 * @sqlfn tbox()
 */
Datum
Set_to_tbox(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  TBox *result = set_to_tbox(s);
  PG_FREE_IF_COPY_P(s, 0);
  PG_RETURN_TBOX_P(result);
}

PGDLLEXPORT Datum Span_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_to_tbox);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a span converted to a temporal box
 * @sqlfn tbox()
 */
Datum
Span_to_tbox(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  TBox *result = span_to_tbox(s);
  PG_RETURN_TBOX_P(result);
}

/**
 * @brief Peek into a span set datum to find the bounding box
 * @note If the datum needs to be detoasted, extract only the header and not
 * the full object
 */
void
spanset_tbox_slice(Datum d, TBox *box)
{
  SpanSet *ss = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) d))
    ss = (SpanSet *) PG_DETOAST_DATUM_SLICE(d, 0, time_max_header_size());
  else
    ss = (SpanSet *) d;
  if (numspan_type(ss->span.spantype))
    numspan_set_tbox(&ss->span, box);
  else
    tstzspan_set_tbox(&ss->span, box);
  PG_FREE_IF_COPY_P(ss, DatumGetPointer(d));
  return;
}

PGDLLEXPORT Datum Spanset_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_to_tbox);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a span set converted to a temporal box
 * @sqlfn tbox()
 */
Datum
Spanset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  TBox *result = palloc(sizeof(TBox));
  spanset_tbox_slice(d, result);
  PG_RETURN_TBOX_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_to_intspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_to_intspan);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a temporal box converted to an integer span
 * @sqlfn floatspan()
 */
Datum
Tbox_to_intspan(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_SPAN_P(tbox_to_intspan(box));
}

PGDLLEXPORT Datum Tbox_to_floatspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_to_floatspan);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a temporal box converted to a float span
 * @sqlfn floatspan()
 */
Datum
Tbox_to_floatspan(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_SPAN_P(tbox_to_floatspan(box));
}

PGDLLEXPORT Datum Tbox_to_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_to_tstzspan);
/**
 * @ingroup mobilitydb_box_conversion
 * @brief Return a temporal box converted to a timestamptz span
 * @sqlfn period()
 */
Datum
Tbox_to_tstzspan(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_SPAN_P(tbox_to_tstzspan(box));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_hasx(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_hasx);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a temporal box has value dimension
 * @sqlfn hasX()
 */
Datum
Tbox_hasx(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hasx(box));
}

PGDLLEXPORT Datum Tbox_hast(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_hast);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a temporal box has time dimension
 * @sqlfn hasT()
 */
Datum
Tbox_hast(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hast(box));
}

PGDLLEXPORT Datum Tbox_xmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_xmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum X value of a temporal box, if any
 * @sqlfn Xmin()
 */
Datum
Tbox_xmin(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  double result;
  if (! tbox_xmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Tbox_xmin_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_xmin_inc);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if the minimum X value of a temporal box is inclusive,
 * if any
 * @sqlfn Xmin_inc()
 */
Datum
Tbox_xmin_inc(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  bool result;
  if (! tbox_xmin_inc(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Tbox_xmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_xmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum X value of a temporal box as a double, if any
 * @sqlfn Xmax()
 */
Datum
Tbox_xmax(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  double result;
  if (! tbox_xmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Tbox_xmax_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_xmax_inc);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if the maximum X value of a temporal box is inclusive,
 * if any
 * @sqlfn Xmin_inc()
 */
Datum
Tbox_xmax_inc(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  bool result;
  if (! tbox_xmax_inc(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Tbox_tmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum timestamp of a temporal box, if any
 * @sqlfn Tmin()
 */
Datum
Tbox_tmin(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  TimestampTz result;
  if (! tbox_tmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Tbox_tmin_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tmin_inc);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if the minimum timestamp value of a temporal box is
 * inclusive, if any
 * @sqlfn Tmin_inc()
 */
Datum
Tbox_tmin_inc(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  bool result;
  if (! tbox_tmin_inc(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Tbox_tmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum timestamp of a temporal box, if any
 * @sqlfn Tmax()
 */
Datum
Tbox_tmax(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  TimestampTz result;
  if (! tbox_tmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Tbox_tmax_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tmax_inc);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if the maximum timestamp value of a temporal box is
 * inclusive, if any
 * @sqlfn Tmin_inc()
 */
Datum
Tbox_tmax_inc(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  bool result;
  if (! tbox_tmax_inc(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_shift_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_shift_value);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the value span shifted by a value
 * @sqlfn shiftValue()
 */
Datum
Tbox_shift_value(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_TBOX_P(tbox_shift_scale_value(box, shift, 0, basetype, true,
    false));
}

PGDLLEXPORT Datum Tbox_shift_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_shift_time);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the time span shifted by a value
 * @sqlfn shiftTime()
 */
Datum
Tbox_shift_time(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_TBOX_P(tbox_shift_scale_time(box, shift, NULL));
}

PGDLLEXPORT Datum Tbox_scale_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_scale_value);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the value span scaled by a value
 * @sqlfn scaleValue()
 */
Datum
Tbox_scale_value(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Datum width = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_TBOX_P(tbox_shift_scale_value(box, 0, width, basetype, false,
    true));
}

PGDLLEXPORT Datum Tbox_scale_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_scale_time);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the time span scaled by an interval
 * @sqlfn scaleTime()
 */
Datum
Tbox_scale_time(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_TBOX_P(tbox_shift_scale_time(box, NULL, duration));
}

PGDLLEXPORT Datum Tbox_shift_scale_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_shift_scale_value);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the value span shifted and scaled by two
 * values
 * @sqlfn scaleValue()
 */
Datum
Tbox_shift_scale_value(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Datum width = PG_GETARG_DATUM(2);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_TBOX_P(tbox_shift_scale_value(box, shift, width, basetype, true,
    true));
}

PGDLLEXPORT Datum Tbox_shift_scale_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_shift_scale_time);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the time span shifted and scaled by two
 * intervals
 * @sqlfn shiftScaleTime()
 */
Datum
Tbox_shift_scale_time(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  PG_RETURN_TBOX_P(tbox_shift_scale_time(box, shift, duration));
}

PGDLLEXPORT Datum Tbox_expand_int(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_expand_int);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the value span expanded by an integer
 * @sqlfn expandValue()
 */
Datum
Tbox_expand_int(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  int i = PG_GETARG_INT32(1);
  PG_RETURN_TBOX_P(tbox_expand_int(box, i));
}

PGDLLEXPORT Datum Tbox_expand_float(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_expand_float);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the value span expanded by a double
 * @sqlfn expandValue()
 */
Datum
Tbox_expand_float(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_TBOX_P(tbox_expand_float(box, d));
}

PGDLLEXPORT Datum Tbox_expand_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_expand_time);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the the time span expanded by an interval
 * @sqlfn expandTime()
 */
Datum
Tbox_expand_time(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_TBOX_P(tbox_expand_time(box, interval));
}

PGDLLEXPORT Datum Tbox_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_round);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box with the precision of the value span set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Tbox_round(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  int maxdd = PG_GETARG_INT32(1);
  PG_RETURN_TBOX_P(tbox_round(box, maxdd));
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PGDLLEXPORT Datum Contains_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first temporal box contains the second one
 * @sqlfn tbox_contains()
 */
Datum
Contains_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contains_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Contained_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first temporal box is contained in the second one
 * @sqlfn tbox_contained()
 */
Datum
Contained_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contained_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Overlaps_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if two temporal boxes overlap
 * @sqlfn tbox_overlaps()
 */
Datum
Overlaps_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overlaps_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Same_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if two temporal boxes are equal on the common dimensions
 * @sqlfn tbox_same()
 */
Datum
Same_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(same_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Adjacent_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if two temporal boxes are adjacent
 * @sqlfn tbox_adjacent()
 */
Datum
Adjacent_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(adjacent_tbox_tbox(box1, box2));
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

PGDLLEXPORT Datum Left_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is to the left of the second
 * one
 * @sqlfn tbox_left()
 */
Datum
Left_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(left_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Overleft_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend to the right of
 * the second one
 * @sqlfn tbox_overleft()
 */
Datum
Overleft_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overleft_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Right_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is to the right of the second
 * one
 * @sqlfn tbox_right()
 */
Datum
Right_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(right_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Overright_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend to the left of
 * the second one
 * @sqlfn tbox_overright()
 */
Datum
Overright_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overright_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Before_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is before the second one
 * @sqlfn tbox_before()
 */
Datum
Before_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(before_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Overbefore_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is not after the second one
 * @sqlfn tbox_overbefore()
 */
Datum
Overbefore_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overbefore_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum After_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is after the second one
 * @sqlfn tbox_after()
 */
Datum
After_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(after_tbox_tbox(box1, box2));
}

PGDLLEXPORT Datum Overafter_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is not before the second one
 * @sqlfn tbox_overafter()
 */
Datum
Overafter_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overafter_tbox_tbox(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

PGDLLEXPORT Datum Union_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_tbox_tbox);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the union of two temporal boxes
 * @sqlfn union()
 * @sqlop @p +
 */
Datum
Union_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_TBOX_P(union_tbox_tbox(box1, box2, true));
}

PGDLLEXPORT Datum Intersection_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_tbox_tbox);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the intersection of two temporal boxes
 * @sqlfn intersection()
 * @sqlop @p *
 */
Datum
Intersection_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  TBox *result = intersection_tbox_tbox(box1, box2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TBOX_P(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_extent_transfn);
/**
 * @brief Transition function for extent aggregation for boxes
 */
Datum
Tbox_extent_transfn(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  /* Can't do anything with null inputs */
  if (! box1 && ! box2)
    PG_RETURN_NULL();
  TBox *result = palloc(sizeof(TBox));
  /* One of the boxes is null, return the other one */
  if (! box1)
  {
    memcpy(result, box2, sizeof(TBox));
    PG_RETURN_TBOX_P(result);
  }
  if (! box2)
  {
    memcpy(result, box1, sizeof(TBox));
    PG_RETURN_TBOX_P(result);
  }

  /* Both boxes are not null */
  memcpy(result, box1, sizeof(TBox));
  tbox_expand(box2, result);
  PG_RETURN_TBOX_P(result);
}

PGDLLEXPORT Datum Tbox_extent_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_extent_combinefn);
/**
 * @brief Combine function for extent aggregation for temporal boxes
 */
Datum
Tbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);
  if (!box1 && !box2)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_TBOX_P(box1);
  if (!box1 && box2)
    PG_RETURN_TBOX_P(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality_tbox(box1, box2);
  TBox *result = tbox_cp(box1);
  tbox_expand(box2, result);
  PG_RETURN_TBOX_P(result);
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_cmp);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal box
 * is less than, equal to, or greater than the second one
 * @sqlfn tbox_cmp()
 */
Datum
Tbox_cmp(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_INT32(tbox_cmp(box1, box2));
}

PGDLLEXPORT Datum Tbox_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_lt);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is less than the second one
 * @sqlfn tbox_lt()
 * @sqlop @p <
 */
Datum
Tbox_lt(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_lt(box1, box2));
}

PGDLLEXPORT Datum Tbox_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_le);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is less than or equal to
 * the second one
 * @sqlfn tbox_le()
 * @sqlop @p <=
 */
Datum
Tbox_le(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_le(box1, box2));
}

PGDLLEXPORT Datum Tbox_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_ge);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is greater than or equal to
 * the second one
 * @sqlfn tbox_ge()
 * @sqlop @p >=
 */
Datum
Tbox_ge(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_ge(box1, box2));
}

PGDLLEXPORT Datum Tbox_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_gt);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is greater than the second one
 * @sqlfn tbox_gt()
 * @sqlop @p >
 */
Datum
Tbox_gt(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_gt(box1, box2));
}

PGDLLEXPORT Datum Tbox_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_eq);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if two temporal boxes are equal
 * @sqlfn tbox_eq()
 * @sqlop @p =
 */
Datum
Tbox_eq(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_eq(box1, box2));
}

PGDLLEXPORT Datum Tbox_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_ne);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if two temporal boxes are different
 * @sqlfn tbox_ne()
 * @sqlop @p <>
 */
Datum
Tbox_ne(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_ne(box1, box2));
}

/*****************************************************************************/

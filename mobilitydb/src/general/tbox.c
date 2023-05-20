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
 * @brief Functions for temporal bounding boxes.
 */

#include "general/tbox.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/temporal.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_out.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_general/tnumber_mathfuncs.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_in);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Input function for temporal boxes.
 *
 * Examples of input:
 * @code
 * TBOX((1.0, 2.0), (1.0, 2.0))   -- Both X and T dimensions
 * TBOX((1.0, ), (1.0, ))      -- Only X dimension
 * TBOX((, 2.0), (, 2.0))      -- Only T dimension
 * @endcode
 * where the commas are optional
 * @sqlfunc tbox_in()
 */
Datum
Tbox_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(tbox_in(input));
}

PGDLLEXPORT Datum Tbox_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_out);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Output function for temporal boxes.
 * @sqlfunc tbox_out()
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
 * @brief Receive function for TBox
 * @sqlfunc tbox_recv()
 */
Datum
Tbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  TBox *result = tbox_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tbox_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_send);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Send function for TBox
 * @sqlfunc tbox_send()
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
 * @brief Output function for temporal boxes.
 * @sqlfunc asText()
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

/*****************************************************************************/

PGDLLEXPORT Datum Number_timestamp_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_timestamp_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the integer and the timestamp to a temporal box
 * @sqlfunc tbox()
 */
Datum
Number_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  TBox *result = number_timestamp_to_tbox(d, basetype, t);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Number_period_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_period_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief  Transform the integer and the period to a temporal box
 * @sqlfunc tbox()
 */
Datum
Number_period_to_tbox(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *p = PG_GETARG_SPAN_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  TBox *result = number_period_to_tbox(d, basetype, p);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Span_timestamp_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_timestamp_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the span and the timestamp to a temporal box
 * @sqlfunc tbox()
 */
Datum
Span_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBox *result = span_timestamp_to_tbox(span, t);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Span_period_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_period_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the span and the period to a temporal box
 * @sqlfunc tbox()
 */
Datum
Span_period_to_tbox(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  Span *p = PG_GETARG_SPAN_P(1);
  TBox *result = span_period_to_tbox(span, p);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PGDLLEXPORT Datum Number_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the number to a temporal box
 * @sqlfunc tbox()
 */
Datum
Number_to_tbox(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  TBox *result = palloc(sizeof(TBox));
  number_set_tbox(d, basetype, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Numeric_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numeric_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the numeric to a temporal box
 * @sqlfunc tbox()
 */
Datum
Numeric_to_tbox(PG_FUNCTION_ARGS)
{
  Datum num = PG_GETARG_DATUM(0);
  Datum d = call_function1(numeric_float8, num);
  TBox *result = palloc(sizeof(TBox));
  number_set_tbox(d, T_FLOAT8, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Timestamp_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamp_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the timestamp to a temporal box
 * @sqlfunc tbox()
 */
Datum
Timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TBox *result = palloc(sizeof(TBox));
  timestamp_set_tbox(t, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Set_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the set to a temporal box
 * @sqlfunc tbox()
 */
Datum
Set_to_tbox(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  TBox *result = palloc(sizeof(TBox));
  if (numset_type(s->settype))
    numset_set_tbox(s, result);
  else
    timestampset_set_tbox(s, result);
  PG_FREE_IF_COPY_P(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Span_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the span to a temporal box
 * @sqlfunc tbox()
 */
Datum
Span_to_tbox(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  TBox *result = palloc(sizeof(TBox));
  if (tnumber_spantype(s->spantype))
    numspan_set_tbox(s, result);
  else
    period_set_tbox(s, result);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Peak into a period set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
spanset_tbox_slice(Datum ssdatum, TBox *box)
{
  SpanSet *ss = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) ssdatum))
    ss = (SpanSet *) PG_DETOAST_DATUM_SLICE(ssdatum, 0,
      time_max_header_size());
  else
    ss = (SpanSet *) ssdatum;
  if (numspan_type(ss->span.spantype))
    numspan_set_tbox(&ss->span, box);
  else
    period_set_tbox(&ss->span, box);
  PG_FREE_IF_COPY_P(ss, DatumGetPointer(ssdatum));
  return;
}

PGDLLEXPORT Datum Spanset_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the span set to a temporal box
 * @sqlfunc tbox()
 */
Datum
Spanset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum ssdatum = PG_GETARG_DATUM(0);
  TBox *result = palloc(sizeof(TBox));
  spanset_tbox_slice(ssdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_to_floatspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_to_floatspan);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast a temporal box as a float span
 * @sqlfunc floatspan()
 */
Datum
Tbox_to_floatspan(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Span *result = tbox_to_floatspan(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tbox_to_period(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_to_period);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast a temporal box as a period
 * @sqlfunc period()
 */
Datum
Tbox_to_period(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Span *result = tbox_to_period(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_hasx(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_hasx);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a temporal box has value dimension
 * @sqlfunc hasX()
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
 * @sqlfunc hasT()
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
 * @brief Return the minimum X value of a temporal box
 * @sqlfunc Xmin()
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

PGDLLEXPORT Datum Tbox_xmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_xmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum X value of a temporal box
 * @sqlfunc Xmax()
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

PGDLLEXPORT Datum Tbox_tmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum timestamp of a temporal box
 * @sqlfunc Tmin()
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

PGDLLEXPORT Datum Tbox_tmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum timestamp of a temporal box
 * @sqlfunc Tmax()
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

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_expand_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_expand_value);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box expanded in the value dimension by a double
 * @sqlfunc expandValue()
 */
Datum
Tbox_expand_value(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(tbox_expand_value(box, d));
}

PGDLLEXPORT Datum Tbox_expand_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_expand_time);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box expanded in the time dimension by an interval
 * @sqlfunc expandTime()
 */
Datum
Tbox_expand_time(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_POINTER(tbox_expand_time(box, interval));
}

/**
 * @brief Set the precision of the value dimension of the temporal box to
 * the number of decimal places.
 */
static TBox *
tbox_round(const TBox *box, Datum size)
{
  ensure_has_X_tbox(box);
  TBox *result = tbox_copy(box);
  result->span.lower = datum_round_float(box->span.lower, size);
  result->span.upper = datum_round_float(box->span.upper, size);
  return result;
}

PGDLLEXPORT Datum Tbox_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_round);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Set the precision of the value dimension of the temporal box to the
 * number of decimal places
 * @sqlfunc round()
 */
Datum
Tbox_round(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(tbox_round(box, size));
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PGDLLEXPORT Datum Contains_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first temporal box contains the second one
 * @sqlfunc tbox_contains()
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
 * @sqlfunc tbox_contained()
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
 * @brief Return true if the temporal boxes overlap
 * @sqlfunc tbox_overlaps()
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
 * @brief Return true if the temporal boxes are equal on the common dimensions
 * @sqlfunc tbox_same()
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
 * @brief Return true if the temporal boxes are adjacent
 * @sqlfunc tbox_adjacent()
 */
Datum
Adjacent_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(adjacent_tbox_tbox(box1, box2));
}

/*****************************************************************************
 * Relative position operators
 *****************************************************************************/

PGDLLEXPORT Datum Left_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is strictly to the left of the second one
 * @sqlfunc tbox_left()
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
 * @brief Return true if the first temporal box does not extend to the right of the second one
 * @sqlfunc tbox_overleft()
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
 * @brief Return true if the first temporal box is strictly to the right of the second one
 * @sqlfunc tbox_right()
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
 * @brief Return true if the first temporal box does not extend to the left of the second one
 * @sqlfunc tbox_overright()
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
 * @brief Return true if the first temporal box is strictly before the second one
 * @sqlfunc tbox_before()
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
 * @brief Return true if the first temporal box does not extend after the second one
 * @sqlfunc tbox_overbefore()
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
 * @brief Return true if the first temporal box is strictly after the second one
 * @sqlfunc tbox_after()
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
 * @brief Return true if the first temporal box does not extend before the second one
 * @sqlfunc tbox_overafter()
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
 * @brief Return the union of the temporal boxes
 * @sqlfunc union()
 * @sqlop @p +
 */
Datum
Union_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  TBox *result = union_tbox_tbox(box1, box2);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intersection_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_tbox_tbox);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the intersection of the temporal boxes
 * @sqlfunc intersection()
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
  PG_RETURN_POINTER(result);
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
    PG_RETURN_POINTER(result);
  }
  if (! box2)
  {
    memcpy(result, box1, sizeof(TBox));
    PG_RETURN_POINTER(result);
  }

  /* Both boxes are not null */
  memcpy(result, box1, sizeof(TBox));
  tbox_expand(box2, result);
  PG_RETURN_POINTER(result);
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
    PG_RETURN_POINTER(box1);
  if (!box1 && box2)
    PG_RETURN_POINTER(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality_tbox(box1, box2);
  TBox *result = tbox_copy(box1);
  tbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_cmp);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal box
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 * @sqlfunc tbox_cmp()
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
 * @sqlfunc tbox_lt()
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
 * @sqlfunc tbox_le()
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
 * @sqlfunc tbox_ge()
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
 * @sqlfunc tbox_gt()
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
 * @brief Return true if the temporal boxes are equal
 * @sqlfunc tbox_eq()
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
 * @brief Return true if the temporal boxes are different
 * @sqlfunc tbox_ne()
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

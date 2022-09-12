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
#include "general/temporal_out.h"
#include "general/temporal_util.h"
#include "general/time_ops.h"
#include "general/tnumber_mathfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_util.h"
#include "pg_general/tnumber_mathfuncs.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_in);
/**
 * @ingroup mobilitydb_box_in_out
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
PGDLLEXPORT Datum
Tbox_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(tbox_in(input));
}

PG_FUNCTION_INFO_V1(Tbox_out);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Output function for temporal boxes.
 * @sqlfunc tbox_out()
 */
PGDLLEXPORT Datum
Tbox_out(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_CSTRING(tbox_out(box, OUT_DEFAULT_DECIMAL_DIGITS));
}

PG_FUNCTION_INFO_V1(Tbox_recv);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Receive function for TBOX
 * @sqlfunc tbox_recv()
 */
PGDLLEXPORT Datum
Tbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  TBOX *result = tbox_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_send);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Send function for TBOX
 * @sqlfunc tbox_send()
 */
PGDLLEXPORT Datum
Tbox_send(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
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

PG_FUNCTION_INFO_V1(Tbox_as_text);
/**
 * @ingroup mobilitydb_box_in_out
 * @brief Output function for temporal boxes.
 * @sqlfunc asText()
 */
PGDLLEXPORT Datum
Tbox_as_text(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
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

PG_FUNCTION_INFO_V1(Int_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the integer to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Int_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  TBOX *result = palloc(sizeof(TBOX));
  int_set_tbox(i, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the float to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Float_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  TBOX *result = palloc(sizeof(TBOX));
  float_set_tbox(d, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Numeric_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the numeric to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Numeric_to_tbox(PG_FUNCTION_ARGS)
{
  Datum num = PG_GETARG_DATUM(0);
  double d = DatumGetFloat8(call_function1(numeric_float8, num));
  TBOX *result = palloc(sizeof(TBOX));
  float_set_tbox(d, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the span to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Span_to_tbox(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  TBOX *result = palloc(sizeof(TBOX));
  span_set_tbox(span, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestamp_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the timestamp to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TBOX *result = palloc(sizeof(TBOX));
  timestamp_set_tbox(t, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the period to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Period_to_tbox(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TBOX *result = palloc(sizeof(TBOX));
  period_set_tbox(p, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Int_timestamp_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the integer and the timestamp to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Int_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = int_timestamp_to_tbox(i, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_timestamp_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the float and the timestamp to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Float_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = float_timestamp_to_tbox(d, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Int_period_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief  Transform the integer and the period to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Int_period_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TBOX *result = int_period_to_tbox(i, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_period_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the float and the period to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Float_period_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TBOX *result = float_period_to_tbox(d, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_timestamp_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the span and the timestamp to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Span_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = span_timestamp_to_tbox(span, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_period_to_tbox);
/**
 * @ingroup mobilitydb_box_constructor
 * @brief Transform the span and the period to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Span_period_to_tbox(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TBOX *result = span_period_to_tbox(span, p);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * @brief Peak into a timestamp set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
timestampset_tbox_slice(Datum tsdatum, TBOX *box)
{
  TimestampSet *ts = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) tsdatum))
    ts = (TimestampSet *) PG_DETOAST_DATUM_SLICE(tsdatum, 0,
      time_max_header_size());
  else
    ts = (TimestampSet *) tsdatum;
  timestampset_set_tbox(ts, box);
  PG_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the period set to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Timestampset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  TBOX *result = palloc(sizeof(TBOX));
  timestampset_tbox_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Peak into a period set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
periodset_tbox_slice(Datum psdatum, TBOX *box)
{
  PeriodSet *ps = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) psdatum))
    ps = (PeriodSet *) PG_DETOAST_DATUM_SLICE(psdatum, 0,
      time_max_header_size());
  else
    ps = (PeriodSet *) psdatum;
  periodset_set_tbox(ps, box);
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PG_FUNCTION_INFO_V1(Periodset_to_tbox);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Transform the period set to a temporal box
 * @sqlfunc tbox()
 */
PGDLLEXPORT Datum
Periodset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  TBOX *result = palloc(sizeof(TBOX));
  periodset_tbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_to_floatspan);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast a temporal box as a float span
 * @sqlfunc floatspan()
 */
PGDLLEXPORT Datum
Tbox_to_floatspan(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Span *result = tbox_to_floatspan(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_to_period);
/**
 * @ingroup mobilitydb_box_cast
 * @brief Cast a temporal box as a period
 * @sqlfunc period()
 */
PGDLLEXPORT Datum
Tbox_to_period(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Period *result = tbox_to_period(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_hasx);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a temporal box has value dimension
 * @sqlfunc hasX()
 */
PGDLLEXPORT Datum
Tbox_hasx(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hasx(box));
}

PG_FUNCTION_INFO_V1(Tbox_hast);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return true if a temporal box has time dimension
 * @sqlfunc hasT()
 */
PGDLLEXPORT Datum
Tbox_hast(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hast(box));
}

PG_FUNCTION_INFO_V1(Tbox_xmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum X value of a temporal box
 * @sqlfunc Xmin()
 */
PGDLLEXPORT Datum
Tbox_xmin(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  double result;
  if (! tbox_xmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tbox_xmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum X value of a temporal box
 * @sqlfunc Xmax()
 */
PGDLLEXPORT Datum
Tbox_xmax(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  double result;
  if (! tbox_xmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tbox_tmin);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the minimum timestamp of a temporal box
 * @sqlfunc Tmin()
 */
PGDLLEXPORT Datum
Tbox_tmin(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  TimestampTz result;
  if (! tbox_tmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Tbox_tmax);
/**
 * @ingroup mobilitydb_box_accessor
 * @brief Return the maximum timestamp of a temporal box
 * @sqlfunc Tmax()
 */
PGDLLEXPORT Datum
Tbox_tmax(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  TimestampTz result;
  if (! tbox_tmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_expand_value);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box expanded in the value dimension by a double
 * @sqlfunc expandValue()
 */
PGDLLEXPORT Datum
Tbox_expand_value(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(tbox_expand_value(box, d));
}

PG_FUNCTION_INFO_V1(Tbox_expand_temporal);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Return a temporal box expanded in the time dimension by an interval
 * @sqlfunc expandTemporal()
 */
PGDLLEXPORT Datum
Tbox_expand_temporal(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_POINTER(tbox_expand_temporal(box, interval));
}

/**
 * @brief Set the precision of the value dimension of the temporal box to
 * the number of decimal places.
 */
static TBOX *
tbox_round(const TBOX *box, int size)
{
  ensure_has_X_tbox(box);
  TBOX *result = tbox_copy(box);
  result->span.lower = datum_round_float(box->span.lower, size);
  result->span.upper = datum_round_float(box->span.upper, size);
  return result;
}

PG_FUNCTION_INFO_V1(Tbox_round);
/**
 * @ingroup mobilitydb_box_transf
 * @brief Set the precision of the value dimension of the temporal box to the
 * number of decimal places
 * @sqlfunc round()
 */
PGDLLEXPORT Datum
Tbox_round(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  int size = PG_GETARG_INT32(1);
  PG_RETURN_POINTER(tbox_round(box, size));
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first temporal box contains the second one
 * @sqlfunc tbox_contains()
 */
PGDLLEXPORT Datum
Contains_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contains_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Contained_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the first temporal box is contained by the second one
 * @sqlfunc tbox_contained()
 */
PGDLLEXPORT Datum
Contained_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contained_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overlaps_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the temporal boxes overlap
 * @sqlfunc tbox_overlaps()
 */
PGDLLEXPORT Datum
Overlaps_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overlaps_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Same_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the temporal boxes are equal on the common dimensions
 * @sqlfunc tbox_same()
 */
PGDLLEXPORT Datum
Same_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(same_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Adjacent_tbox_tbox);
/**
 * @ingroup mobilitydb_box_topo
 * @brief Return true if the temporal boxes are adjacent
 * @sqlfunc tbox_adjacent()
 */
PGDLLEXPORT Datum
Adjacent_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(adjacent_tbox_tbox(box1, box2));
}

/*****************************************************************************
 * Relative position operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Left_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is strictly to the left of the second one
 * @sqlfunc tbox_left()
 */
PGDLLEXPORT Datum
Left_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(left_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overleft_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend to the right of the second one
 * @sqlfunc tbox_overleft()
 */
PGDLLEXPORT Datum
Overleft_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overleft_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Right_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is strictly to the right of the second one
 * @sqlfunc tbox_right()
 */
PGDLLEXPORT Datum
Right_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(right_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overright_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend to the left of the second one
 * @sqlfunc tbox_overright()
 */
PGDLLEXPORT Datum
Overright_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overright_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Before_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is strictly before the second one
 * @sqlfunc tbox_before()
 */
PGDLLEXPORT Datum
Before_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(before_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overbefore_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend after the second one
 * @sqlfunc tbox_overbefore()
 */
PGDLLEXPORT Datum
Overbefore_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overbefore_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(After_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box is strictly after the second one
 * @sqlfunc tbox_after()
 */
PGDLLEXPORT Datum
After_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(after_tbox_tbox(box1, box2));
}

PG_FUNCTION_INFO_V1(Overafter_tbox_tbox);
/**
 * @ingroup mobilitydb_box_pos
 * @brief Return true if the first temporal box does not extend before the second one
 * @sqlfunc tbox_overafter()
 */
PGDLLEXPORT Datum
Overafter_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overafter_tbox_tbox(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_tbox_tbox);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the union of the temporal boxes
 * @sqlfunc union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  TBOX *result = union_tbox_tbox(box1, box2);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_tbox_tbox);
/**
 * @ingroup mobilitydb_box_set
 * @brief Return the intersection of the temporal boxes
 * @sqlfunc intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  TBOX *result = intersection_tbox_tbox(box1, box2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_extent_transfn);
/**
 * @brief Transition function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Tbox_extent_transfn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  /* Can't do anything with null inputs */
  if (! box1 && ! box2)
    PG_RETURN_NULL();
  TBOX *result = palloc(sizeof(TBOX));
  /* One of the boxes is null, return the other one */
  if (! box1)
  {
    memcpy(result, box2, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }
  if (! box2)
  {
    memcpy(result, box1, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }

  /* Both boxes are not null */
  memcpy(result, box1, sizeof(TBOX));
  tbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_extent_combinefn);
/**
 * @brief Combine function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Tbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);
  if (!box1 && !box2)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (!box1 && box2)
    PG_RETURN_POINTER(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality_tbox(box1, box2);
  TBOX *result = tbox_copy(box1);
  tbox_expand(box2, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_cmp);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal box
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 * @sqlfunc tbox_cmp()
 */
PGDLLEXPORT Datum
Tbox_cmp(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_INT32(tbox_cmp(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_lt);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is less than the second one
 * @sqlfunc tbox_lt()
 * @sqlop @p <
 */
PGDLLEXPORT Datum
Tbox_lt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_lt(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_le);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is less than or equal to
 * the second one
 * @sqlfunc tbox_le()
 * @sqlop @p <=
 */
PGDLLEXPORT Datum
Tbox_le(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_le(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_ge);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is greater than or equal to
 * the second one
 * @sqlfunc tbox_ge()
 * @sqlop @p >=
 */
PGDLLEXPORT Datum
Tbox_ge(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_ge(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_gt);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the first temporal box is greater than the second one
 * @sqlfunc tbox_gt()
 * @sqlop @p >
 */
PGDLLEXPORT Datum
Tbox_gt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_gt(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_eq);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the temporal boxes are equal
 * @sqlfunc tbox_eq()
 * @sqlop @p =
 */
PGDLLEXPORT Datum
Tbox_eq(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_eq(box1, box2));
}

PG_FUNCTION_INFO_V1(Tbox_ne);
/**
 * @ingroup mobilitydb_box_comp
 * @brief Return true if the temporal boxes are different
 * @sqlfunc tbox_ne()
 * @sqlop @p <>
 */
PGDLLEXPORT Datum
Tbox_ne(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_ne(box1, box2));
}

/*****************************************************************************/

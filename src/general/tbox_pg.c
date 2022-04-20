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
 * @file tbox.c
 * @brief Functions for temporal bounding boxes.
 */

#include "general/tbox.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/builtins.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/rangetypes_ext.h"
#include "general/temporal.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_in);
/**
 * Input function for temporal boxes.
 *
 * Examples of input:
 * @code
 * TBOX((1.0, 2.0), (1.0, 2.0))   -- Both X and T dimensions
 * TBOX((1.0, ), (1.0, ))      -- Only X dimension
 * TBOX((, 2.0), (, 2.0))      -- Only T dimension
 * @endcode
 * where the commas are optional
 */
PGDLLEXPORT Datum
Tbox_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  TBOX *result = tbox_parse(&input);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_out);
/**
 * Output function for temporal boxes.
 */
PGDLLEXPORT Datum
Tbox_out(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  char *result = tbox_to_string(box);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Tbox_send);
/**
 * Send function for TBOX
 */
PGDLLEXPORT Datum
Tbox_send(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  tbox_write(box, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Tbox_recv);
/**
 * Receive function for TBOX
 */
PGDLLEXPORT Datum
Tbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(tbox_read(buf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_constructor);
/**
 * Construct a temporal box value from the arguments
 */
PGDLLEXPORT Datum
Tbox_constructor(PG_FUNCTION_ARGS)
{
  double xmin = 0, xmax = 0; /* keep compiler quiet */
  TimestampTz tmin = 0, tmax = 0;
  bool hast = false;

  assert (PG_NARGS() == 2 || PG_NARGS() == 4);
  if (PG_NARGS() == 2)
  {
    xmin = PG_GETARG_FLOAT8(0);
    xmax = PG_GETARG_FLOAT8(1);
  }
  else if (PG_NARGS() == 4)
  {
    xmin = PG_GETARG_FLOAT8(0);
    tmin = PG_GETARG_TIMESTAMPTZ(1);
    xmax = PG_GETARG_FLOAT8(2);
    tmax = PG_GETARG_TIMESTAMPTZ(3);
    hast = true;
  }

  TBOX *result = tbox_make(true, hast, xmin, xmax, tmin, tmax);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_constructor_t);
/**
 * Construct a temporal box value from the timestamps
 */
PGDLLEXPORT Datum
Tbox_constructor_t(PG_FUNCTION_ARGS)
{
  TimestampTz tmin = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz tmax = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = tbox_make(false, true, 0.0, 0.0, tmin, tmax);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Casting
 * The functions set the argument box to 0
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Int_to_tbox);
/**
 * Transform the integer to a temporal box
 */
PGDLLEXPORT Datum
Int_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  int_tbox(i, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_to_tbox);
/**
 * Transform the float to a temporal box
 */
PGDLLEXPORT Datum
Float_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  float_tbox(d, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Numeric_to_tbox);
/**
 * Transform the numeric to a temporal box
 */
PGDLLEXPORT Datum
Numeric_to_tbox(PG_FUNCTION_ARGS)
{
  Datum num = PG_GETARG_DATUM(0);
  double d = DatumGetFloat8(call_function1(numeric_float8, num));
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  float_tbox(d, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Range_to_tbox);
/**
 * Transform the range to a temporal box
 */
PGDLLEXPORT Datum
Range_to_tbox(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  /* Return null on empty or unbounded range */
  char flags = range_get_flags(range);
  if (flags & (RANGE_EMPTY | RANGE_LB_INF | RANGE_UB_INF))
    PG_RETURN_NULL();
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  range_tbox(range, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestamp_to_tbox);
/**
 * Transform the timestamp to a temporal box
 */
PGDLLEXPORT Datum
Timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  timestamp_tbox(t, result);
  PG_RETURN_POINTER(result);
}

/**
 * Peak into a timestamp set datum to find the bounding box. If the datum needs
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
  timestampset_tbox(ts, box);
  PG_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_to_tbox);
/**
 * Transform the period set to a temporal box
 */
PGDLLEXPORT Datum
Timestampset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  timestampset_tbox_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_tbox);
/**
 * Transform the period to a temporal box
 */
PGDLLEXPORT Datum
Period_to_tbox(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  period_tbox(p, result);
  PG_RETURN_POINTER(result);
}

/**
 * Peak into a period set datum to find the bounding box. If the datum needs
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
  periodset_tbox(ps, box);
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PG_FUNCTION_INFO_V1(Periodset_to_tbox);
/**
 * Transform the period set to a temporal box
 */
PGDLLEXPORT Datum
Periodset_to_tbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  TBOX *result = (TBOX *) palloc(sizeof(TBOX));
  periodset_tbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Int_timestamp_to_tbox);
/**
 * Transform the integer and the timestamp to a temporal box
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
 * Transform the float and the timestamp to a temporal box
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
 *  Transform the integer and the period to a temporal box
 */
PGDLLEXPORT Datum
Int_period_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  TBOX *result = int_period_to_tbox(i, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Float_period_to_tbox);
/**
 * Transform the float and the period to a temporal box
 */
PGDLLEXPORT Datum
Float_period_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  TBOX *result = float_period_to_tbox(d, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Range_timestamp_to_tbox);
/**
 * Transform the range and the timestamp to a temporal box
 */
PGDLLEXPORT Datum
Range_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = range_timestamp_to_tbox(range, t);
  PG_FREE_IF_COPY(range, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Range_period_to_tbox);
/**
 * Transform the range and the period to a temporal box
 */
PGDLLEXPORT Datum
Range_period_to_tbox(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  Period *p = PG_GETARG_PERIOD_P(1);
  TBOX *result = range_period_to_tbox(range, p);
  if (! result)
    PG_RETURN_NULL();
  PG_FREE_IF_COPY(range, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_to_floatrange);
/**
 * Cast the temporal box value as a float range value
 */
PGDLLEXPORT Datum
Tbox_to_floatrange(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  RangeType *result = tbox_to_floatrange(box);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tbox_to_period);
/**
 * Cast the temporal box value as a period value
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
 * Return true if the temporal box has X dimension
 */
PGDLLEXPORT Datum
Tbox_hasx(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hasx(box));
}

PG_FUNCTION_INFO_V1(Tbox_hast);
/**
 * Return true if the temporal box has T dimension
 */
PGDLLEXPORT Datum
Tbox_hast(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  PG_RETURN_BOOL(tbox_hast(box));
}

PG_FUNCTION_INFO_V1(Tbox_xmin);
/**
 * Return the minimum X value of the temporal box value
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
 * Return the maximum X value of the temporal box value
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
 * Return the minimum timestamp of the temporal box value
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
 * Return the maximum timestamp of the temporal box value
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
 * Expand the value dimension of the temporal box with the double value
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
 * Expand the time dimension of the temporal box with the interval value
 */
PGDLLEXPORT Datum
Tbox_expand_temporal(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Datum interval = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(tbox_expand_temporal(box, interval));
}


PG_FUNCTION_INFO_V1(Tbox_round);
/**
 * Set the precision of the value dimension of the temporal box to the number
 * of decimal places
 */
PGDLLEXPORT Datum
Tbox_round(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Datum size = PG_GETARG_DATUM(1);
  TBOX *result = tbox_round(box, size);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_tbox_tbox);
/**
 * Return true if the first temporal box contains the second one
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
 * Return true if the first temporal box is contained by the second one
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
 * Return true if the temporal boxes overlap
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
 * Return true if the temporal boxes are equal on the common dimensions
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
 * Return true if the temporal boxes are adjacent
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
 * Return true if the first temporal box is strictly to the left of the second one
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
 * Return true if the first temporal box does not extend to the right of the second one
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
 * Return true if the first temporal box is strictly to the right of the second one
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
 * Return true if the first temporal box does not extend to the left of the second one
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
 * Return true if the first temporal box is strictly before the second one
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
 * Return true if the first temporal box does not extend after the second one
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
 * Return true if the first temporal box is strictly after the second one
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
 * Return true if the first temporal box does not extend before the second one
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
 * Return the union of the temporal boxes
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
 * Return the intersection of the temporal boxes
 */
PGDLLEXPORT Datum
Intersection_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  TBOX *result = palloc(sizeof(TBOX));
  if (! inter_tbox_tbox(box1, box2, result))
  {
    pfree(result);
    PG_RETURN_NULL();
  }
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_extent_transfn);
/**
 * Transition function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Tbox_extent_transfn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  /* Can't do anything with null inputs */
  if (!box1 && !box2)
    PG_RETURN_NULL();
  TBOX *result = (TBOX *) palloc0(sizeof(TBOX));
  /* One of the boxes is null, return the other one */
  if (!box1)
  {
    memcpy(result, box2, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }
  if (!box2)
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
 * Combine function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
Tbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  if (!box2 && !box1)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (box2 && !box1)
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
 * Return -1, 0, or 1 depending on whether the first temporal box value
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
Tbox_cmp(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int cmp = tbox_cmp(box1, box2);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(Tbox_lt);
/**
 * Return true if the first temporal box value is less than the second one
 */
PGDLLEXPORT Datum
Tbox_lt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int cmp = tbox_cmp(box1, box2);
  PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(Tbox_le);
/**
 * Return true if the first temporal box value is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Tbox_le(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int cmp = tbox_cmp(box1, box2);
  PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(Tbox_ge);
/**
 * Return true if the first temporal box value is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Tbox_ge(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int cmp = tbox_cmp(box1, box2);
  PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(Tbox_gt);
/**
 * Return true if the first temporal box value is greater than the second one
 */
PGDLLEXPORT Datum
Tbox_gt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int cmp = tbox_cmp(box1, box2);
  PG_RETURN_BOOL(cmp > 0);
}

PG_FUNCTION_INFO_V1(Tbox_eq);
/**
 * Return true if the two temporal boxes are equal
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
 * Return true if the two temporal boxes are different
 */
PGDLLEXPORT Datum
Tbox_ne(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(! tbox_eq(box1, box2));
}

/*****************************************************************************/

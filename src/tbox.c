/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * Functions for temporal bounding boxes.
 */

#include "tbox.h"

#include <assert.h>
#include <utils/builtins.h>

#include "tempcache.h"
#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "rangetypes_ext.h"
#include "temporal.h"
#include "temporal_parser.h"
#include "temporal_util.h"
#include "tnumber_mathfuncs.h"

/** Buffer size for input and output of TBOX values */
#define MAXTBOXLEN    128

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * Constructs a newly allocated temporal box
 */
TBOX *
tbox_make(bool hasx, bool hast, double xmin, double xmax,
  TimestampTz tmin, TimestampTz tmax)
{
  TBOX *result = palloc0(sizeof(TBOX));
  MOBDB_FLAGS_SET_X(result->flags, hasx);
  MOBDB_FLAGS_SET_T(result->flags, hast);
  if (hasx)
  {
    if (xmin > xmax)
    {
      double tmp = xmin;
      xmin = xmax;
      xmax = tmp;
    }
    result->xmin = xmin;
    result->xmax = xmax;
  }
  if (hast)
  {
    if (tmin > tmax)
    {
      TimestampTz ttmp = tmin;
      tmin = tmax;
      tmax = ttmp;
    }
    result->tmin = tmin;
    result->tmax = tmax;
  }
  return result;
}

/**
 * Returns a copy of the temporal box value
 */
TBOX *
tbox_copy(const TBOX *box)
{
  TBOX *result = palloc0(sizeof(TBOX));
  memcpy(result, box, sizeof(TBOX));
  return result;
}

/**
 * Expand the first temporal box value with the second one
 */
void
tbox_expand(TBOX *box1, const TBOX *box2)
{
  box1->xmin = Min(box1->xmin, box2->xmin);
  box1->xmax = Max(box1->xmax, box2->xmax);
  box1->tmin = Min(box1->tmin, box2->tmin);
  box1->tmax = Max(box1->tmax, box2->tmax);
}

/**
 * Shift and/or scale the time span of the temporal box by the interval
 */
void
tbox_shift_tscale(TBOX *box, const Interval *start, const Interval *duration)
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

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * Ensure that the temporal box has X values
 */
void
ensure_has_X_tbox(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_X(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have value dimension")));
}

/**
 * Ensure that the temporal box has T values
 */
void
ensure_has_T_tbox(const TBOX *box)
{
  if (! MOBDB_FLAGS_GET_T(box->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The box must have time dimension")));
}

/**
 * Ensure that the temporal boxes have the same dimensionality
 */
void
ensure_same_dimensionality_tbox(const TBOX *box1, const TBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
    MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The boxes must be of the same dimensionality")));
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_in);
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
tbox_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  TBOX *result = tbox_parse(&input);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the string representation of the temporal box
 */
static char *
tbox_to_string(const TBOX *box)
{
  static size_t size = MAXTBOXLEN + 1;
  char *result = (char *) palloc(size);
  char *xmin = NULL, *xmax = NULL, *tmin = NULL, *tmax = NULL;
  bool hasx = MOBDB_FLAGS_GET_X(box->flags);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  assert(hasx || hast);
  if (hasx)
  {
    xmin = call_output(FLOAT8OID, Float8GetDatum(box->xmin));
    xmax = call_output(FLOAT8OID, Float8GetDatum(box->xmax));
  }
  if (hast)
  {
    tmin = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmin));
    tmax = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmax));
  }
  if (hasx)
  {
    if (hast)
      snprintf(result, size, "TBOX((%s,%s),(%s,%s))", xmin, tmin,
        xmax, tmax);
    else
      snprintf(result, size, "TBOX((%s,),(%s,))", xmin, xmax);
  }
  else
    /* Missing X dimension */
    snprintf(result, size, "TBOX((,%s),(,%s))", tmin, tmax);
  if (hasx)
  {
    pfree(xmin); pfree(xmax);
  }
  if (hast)
  {
    pfree(tmin); pfree(tmax);
  }
  return result;
}

PG_FUNCTION_INFO_V1(tbox_out);
/**
 * Output function for temporal boxes.
 */
PGDLLEXPORT Datum
tbox_out(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  char *result = tbox_to_string(box);
  PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_constructor);
/**
 * Construct a temporal box value from the arguments
 */
PGDLLEXPORT Datum
tbox_constructor(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(tbox_constructor_t);
/**
 * Construct a temporal box value from the timestamps
 */
PGDLLEXPORT Datum
tbox_constructor_t(PG_FUNCTION_ARGS)
{
  TimestampTz tmin = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz tmax = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = tbox_make(false, true, 0.0, 0.0, tmin, tmax);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Casting
 * The internal functions assume that the argument box is set to 0 before
 * with palloc0
 *****************************************************************************/

/**
 * Transform the value to a temporal box (internal function only)
 */
void
number_to_box(TBOX *box, Datum value, Oid basetypid)
{
  ensure_tnumber_base_type(basetypid);
  if (basetypid == INT4OID)
    box->xmin = box->xmax = (double)(DatumGetInt32(value));
  else /* basetypid == FLOAT8OID */
    box->xmin = box->xmax = DatumGetFloat8(value);
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
}

/**
 * Transform the integer to a temporal box (internal function)
 */
void
int_to_tbox_internal(TBOX *box, int i)
{
  box->xmin = box->xmax = (double) i;
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
}

PG_FUNCTION_INFO_V1(int_to_tbox);
/**
 * Transform the integer to a temporal box
 */
PGDLLEXPORT Datum
int_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  TBOX *result = palloc0(sizeof(TBOX));
  int_to_tbox_internal(result, i);
  PG_RETURN_POINTER(result);
}

/**
 * Transform the float to a temporal box (internal function)
 */
void
float_to_tbox_internal(TBOX *box, double d)
{
  box->xmin = box->xmax = d;
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
}

PG_FUNCTION_INFO_V1(float_to_tbox);
/**
 * Transform the float to a temporal box
 */
PGDLLEXPORT Datum
float_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  TBOX *result = palloc0(sizeof(TBOX));
  float_to_tbox_internal(result, d);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(numeric_to_tbox);
/**
 * Transform the numeric to a temporal box
 */
PGDLLEXPORT Datum
numeric_to_tbox(PG_FUNCTION_ARGS)
{
  Datum num = PG_GETARG_DATUM(0);
  double d = DatumGetFloat8(call_function1(numeric_float8, num));
  TBOX *result = palloc0(sizeof(TBOX));
  float_to_tbox_internal(result, d);
  PG_RETURN_POINTER(result);
}

/**
 * Transform the range to a temporal box (internal function)
 */
void
range_to_tbox_internal(TBOX *box, const RangeType *range)
{
  ensure_tnumber_range_type(range->rangetypid);
  range_bounds(range, &box->xmin, &box->xmax);
  MOBDB_FLAGS_SET_X(box->flags, true);
  MOBDB_FLAGS_SET_T(box->flags, false);
}

PG_FUNCTION_INFO_V1(range_to_tbox);
/**
 * Transform the range to a temporal box
 */
PGDLLEXPORT Datum
range_to_tbox(PG_FUNCTION_ARGS)
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(0);
#else
  RangeType *range = PG_GETARG_RANGE_P(0);
#endif
  /* Return null on empty range */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY)
    PG_RETURN_NULL();
  TBOX *result = palloc0(sizeof(TBOX));
  range_to_tbox_internal(result, range);
  PG_RETURN_POINTER(result);
}

/**
 * Transform the timestamp to a temporal box (internal function)
 */
void
timestamp_to_tbox_internal(TBOX *box, TimestampTz t)
{
  box->tmin = box->tmax = t;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(timestamp_to_tbox);
/**
 * Transform the timestamp to a temporal box
 */
PGDLLEXPORT Datum
timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TBOX *result = palloc0(sizeof(TBOX));
  timestamp_to_tbox_internal(result, t);
  PG_RETURN_POINTER(result);
}

/**
 * Transform the period set to a temporal box (internal function)
 */
void
timestampset_to_tbox_internal(TBOX *box, const TimestampSet *ts)
{
  const Period *p = timestampset_bbox_ptr(ts);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(timestampset_to_tbox);
/**
 * Transform the period set to a temporal box
 */
PGDLLEXPORT Datum
timestampset_to_tbox(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TBOX *result = palloc0(sizeof(TBOX));
  timestampset_to_tbox_internal(result, ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Transform the period to a temporal box (internal function)
 */
void
period_to_tbox_internal(TBOX *box, const Period *p)
{
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(period_to_tbox);
/**
 * Transform the period to a temporal box
 */
PGDLLEXPORT Datum
period_to_tbox(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TBOX *result = palloc0(sizeof(TBOX));
  period_to_tbox_internal(result, p);
  PG_RETURN_POINTER(result);
}

/**
 * Transform the period set to a temporal box (internal function)
 */
void
periodset_to_tbox_internal(TBOX *box, const PeriodSet *ps)
{
  const Period *p = periodset_bbox_ptr(ps);
  box->tmin = p->lower;
  box->tmax = p->upper;
  MOBDB_FLAGS_SET_X(box->flags, false);
  MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(periodset_to_tbox);
/**
 * Transform the period set to a temporal box
 */
PGDLLEXPORT Datum
periodset_to_tbox(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TBOX *result = palloc0(sizeof(TBOX));
  periodset_to_tbox_internal(result, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(int_timestamp_to_tbox);
/**
 * Transform the integer and the timestamp to a temporal box
 */
PGDLLEXPORT Datum
int_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = tbox_make(true, true, (double) i, (double) i, t, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(float_timestamp_to_tbox);
/**
 * Transform the float and the timestamp to a temporal box
 */
PGDLLEXPORT Datum
float_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TBOX *result = tbox_make(true, true, d, d, t, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(int_period_to_tbox);
/**
 *  Transform the integer and the period to a temporal box
 */
PGDLLEXPORT Datum
int_period_to_tbox(PG_FUNCTION_ARGS)
{
  int i = PG_GETARG_INT32(0);
  Period *p = PG_GETARG_PERIOD(1);
  TBOX *result = tbox_make(true, true, (double) i, (double)i, p->lower,
    p->upper);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(float_period_to_tbox);
/**
 * Transform the float and the period to a temporal box
 */
PGDLLEXPORT Datum
float_period_to_tbox(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  Period *p = PG_GETARG_PERIOD(1);
  TBOX *result = tbox_make(true, true, d, d, p->lower, p->upper);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(range_timestamp_to_tbox);
/**
 * Transform the range and the timestamp to a temporal box
 */
PGDLLEXPORT Datum
range_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(0);
#else
  RangeType *range = PG_GETARG_RANGE_P(0);
#endif
  /* Return null on empty range */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY)
    PG_RETURN_NULL();
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double xmin, xmax;
  range_bounds(range, &xmin, &xmax);
  TBOX *result = tbox_make(true, true, xmin, xmax, t, t);
  PG_FREE_IF_COPY(range, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(range_period_to_tbox);
/**
 * Transform the range and the period to a temporal box
 */
PGDLLEXPORT Datum
range_period_to_tbox(PG_FUNCTION_ARGS)
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(0);
#else
  RangeType *range = PG_GETARG_RANGE_P(0);
#endif
  /* Return null on empty range */
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY)
    PG_RETURN_NULL();
  Period *p = PG_GETARG_PERIOD(1);
  ensure_tnumber_range_type(range->rangetypid);
  double xmin, xmax;
  range_bounds(range, &xmin, &xmax);
  TBOX *result = tbox_make(true, true, xmin, xmax, p->lower, p->upper);
  PG_FREE_IF_COPY(range, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_to_floatrange);
/**
 * Cast the temporal box value as a float range value
 */
PGDLLEXPORT Datum
tbox_to_floatrange(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  if (!MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  RangeType *result = range_make(Float8GetDatum(box->xmin),
    Float8GetDatum(box->xmax), true, true, FLOAT8OID);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbox_to_period);
/**
 * Cast the temporal box value as a period value
 */
PGDLLEXPORT Datum
tbox_to_period(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  if (!MOBDB_FLAGS_GET_T(box->flags))
    PG_RETURN_NULL();
  Period *result = period_make(box->tmin, box->tmax, true, true);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_hasx);
/**
 * Returns true if the temporal box has X dimension
 */
PGDLLEXPORT Datum
tbox_hasx(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  bool result = MOBDB_FLAGS_GET_X(box->flags);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tbox_hast);
/**
 * Returns true if the temporal box has T dimension
 */
PGDLLEXPORT Datum
tbox_hast(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  bool result = MOBDB_FLAGS_GET_T(box->flags);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tbox_xmin);
/**
 * Returns the minimum X value of the temporal box value
 */
PGDLLEXPORT Datum
tbox_xmin(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  if (!MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->xmin);
}

PG_FUNCTION_INFO_V1(tbox_xmax);
/**
 * Returns the maximum X value of the temporal box value
 */
PGDLLEXPORT Datum
tbox_xmax(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  if (!MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(box->xmax);
}

PG_FUNCTION_INFO_V1(tbox_tmin);
/**
 * Returns the minimum timestamp of the temporal box value
 */
PGDLLEXPORT Datum
tbox_tmin(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  if (!MOBDB_FLAGS_GET_T(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(box->tmin);
}

PG_FUNCTION_INFO_V1(tbox_tmax);
/**
 * Returns the maximum timestamp of the temporal box value
 */
PGDLLEXPORT Datum
tbox_tmax(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  if (!MOBDB_FLAGS_GET_T(box->flags))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(box->tmax);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * Expand the value dimension of the temporal box with the double value
 * (internal function)
 */
static TBOX *
tbox_expand_value_internal(const TBOX *box, const double d)
{
  ensure_has_X_tbox(box);
  TBOX *result = tbox_copy(box);
  result->xmin = box->xmin - d;
  result->xmax = box->xmax + d;
  return result;
}

PG_FUNCTION_INFO_V1(tbox_expand_value);
/**
 * Expand the value dimension of the temporal box with the double value
 */
PGDLLEXPORT Datum
tbox_expand_value(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  PG_RETURN_POINTER(tbox_expand_value_internal(box, d));
}

/**
 * Expand the time dimension of the temporal box with the interval value
 * (internal function)
 */
static TBOX *
tbox_expand_temporal_internal(const TBOX *box, const Datum interval)
{
  ensure_has_T_tbox(box);
  TBOX *result = tbox_copy(box);
  result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval,
    TimestampTzGetDatum(box->tmin), interval));
  result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval,
    TimestampTzGetDatum(box->tmax), interval));
  return result;
}

PG_FUNCTION_INFO_V1(tbox_expand_temporal);
/**
 * Expand the time dimension of the temporal box with the interval value
 */
PGDLLEXPORT Datum
tbox_expand_temporal(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Datum interval = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(tbox_expand_temporal_internal(box, interval));
}

PG_FUNCTION_INFO_V1(tbox_set_precision);
/**
 * Set the precision of the value dimension of the temporal box to the number
 * of decimal places
 */
PGDLLEXPORT Datum
tbox_set_precision(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Datum size = PG_GETARG_DATUM(1);
  ensure_has_X_tbox(box);
  TBOX *result = tbox_copy(box);
  result->xmin = DatumGetFloat8(datum_round(Float8GetDatum(box->xmin), size));
  result->xmax = DatumGetFloat8(datum_round(Float8GetDatum(box->xmax), size));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

/**
 * Set the ouput variables with the values of the flags of the boxes.
 *
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hast Boolean variables
 */
static void
tbox_tbox_flags(const TBOX *box1, const TBOX *box2, bool *hasx, bool *hast)
{
  *hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  *hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  return;
}

/**
 * Set the ouput variables with the values of the flags of the boxes.
 *
 * @param[in] box1,box2 Input boxes
 * @param[out] hasx,hast Boolean variables
 */
static void
topo_tbox_tbox_init(const TBOX *box1, const TBOX *box2, bool *hasx, bool *hast)
{
  ensure_common_dimension(box1->flags, box2->flags);
  *hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  *hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  return;
}

/**
 * Returns true if the first temporal box contains the second one
 * (internal function)
 */
bool
contains_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  if (hasx && (box2->xmin < box1->xmin || box2->xmax > box1->xmax))
    return false;
  if (hast && (box2->tmin < box1->tmin || box2->tmax > box1->tmax))
    return false;
  return true;
}

PG_FUNCTION_INFO_V1(contains_tbox_tbox);
/**
 * Returns true if the first temporal box contains the second one
 */
PGDLLEXPORT Datum
contains_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contains_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box is contained by the second one
 * (internal function)
 */
bool
contained_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  return contains_tbox_tbox_internal(box2, box1);
}

PG_FUNCTION_INFO_V1(contained_tbox_tbox);
/**
 * Returns true if the first temporal box is contained by the second one
 */
PGDLLEXPORT Datum
contained_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(contained_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the temporal boxes overlap
 * (internal function)
 */
bool
overlaps_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  if (hasx && (box1->xmax < box2->xmin || box1->xmin > box2->xmax))
    return false;
  if (hast && (box1->tmax < box2->tmin || box1->tmin > box2->tmax))
    return false;
  return true;
}

PG_FUNCTION_INFO_V1(overlaps_tbox_tbox);
/**
 * Returns true if the temporal boxes overlap
 */
PGDLLEXPORT Datum
overlaps_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overlaps_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the temporal boxes are equal on the common dimensions
 * (internal function)
 */
bool
same_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  if (hasx && (box1->xmin != box2->xmin || box1->xmax != box2->xmax))
    return false;
  if (hast && (box1->tmin != box2->tmin || box1->tmax != box2->tmax))
    return false;
  return true;
}

PG_FUNCTION_INFO_V1(same_tbox_tbox);
/**
 * Returns true if the temporal boxes are equal on the common dimensions
 */
PGDLLEXPORT Datum
same_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(same_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the temporal boxes are adjacent
 * (internal function)
 */
bool
adjacent_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  topo_tbox_tbox_init(box1, box2, &hasx, &hast);
  TBOX *inter = tbox_intersection_internal(box1, box2);
  if (inter == NULL)
    return false;
  /* Boxes are adjacent if they share n dimensions and their intersection is
   * at most of n-1 dimensions */
  bool result;
  if (!hasx && hast)
    result = inter->tmin == inter->tmax;
  else if (hasx && !hast)
    result = inter->xmin == inter->xmax;
  else
    result = inter->xmin == inter->xmax || inter->tmin == inter->tmax;
  pfree(inter);
  return result;
}

PG_FUNCTION_INFO_V1(adjacent_tbox_tbox);
/**
 * Returns true if the temporal boxes are adjacent
 */
PGDLLEXPORT Datum
adjacent_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(adjacent_tbox_tbox_internal(box1, box2));
}

/*****************************************************************************
 * Relative position operators
 *****************************************************************************/

/**
 * Returns true if the first temporal box is strictly to the left of the second one
 * (internal function)
 */
bool
left_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmax < box2->xmin);
}

PG_FUNCTION_INFO_V1(left_tbox_tbox);
/**
 * Returns true if the first temporal box is strictly to the left of the second one
 */
PGDLLEXPORT Datum
left_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(left_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend to the right of the second one
 * (internal function)
 */
bool
overleft_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmax <= box2->xmax);
}

PG_FUNCTION_INFO_V1(overleft_tbox_tbox);
/**
 * Returns true if the first temporal box does not extend to the right of the second one
 */
PGDLLEXPORT Datum
overleft_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overleft_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box is strictly to the right of the second one
 * (internal function)
 */
bool
right_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmin > box2->xmax);
}

PG_FUNCTION_INFO_V1(right_tbox_tbox);
/**
 * Returns true if the first temporal box is strictly to the right of the second one
 */
PGDLLEXPORT Datum
right_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(right_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend to the left of the second one
 * (internal function)
 */
bool
overright_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_X_tbox(box1);
  ensure_has_X_tbox(box2);
  return (box1->xmin >= box2->xmin);
}

PG_FUNCTION_INFO_V1(overright_tbox_tbox);
/**
 * Returns true if the first temporal box does not extend to the left of the second one
 */
PGDLLEXPORT Datum
overright_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overright_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box is strictly before the second one
 * (internal function)
 */
bool
before_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmax < box2->tmin);
}

PG_FUNCTION_INFO_V1(before_tbox_tbox);
/**
 * Returns true if the first temporal box is strictly before the second one
 */
PGDLLEXPORT Datum
before_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(before_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend after the second one
 * (internal function)
 */
bool
overbefore_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmax <= box2->tmax);
}

PG_FUNCTION_INFO_V1(overbefore_tbox_tbox);
/**
 * Returns true if the first temporal box does not extend after the second one
 */
PGDLLEXPORT Datum
overbefore_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overbefore_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box is strictly after the second one
 * (internal function)
 */
bool
after_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmin > box2->tmax);
}

PG_FUNCTION_INFO_V1(after_tbox_tbox);
/**
 * Returns true if the first temporal box is strictly after the second one
 */
PGDLLEXPORT Datum
after_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(after_tbox_tbox_internal(box1, box2));
}

/**
 * Returns true if the first temporal box does not extend before the second one
 * (internal function)
 */
bool
overafter_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_has_T_tbox(box1);
  ensure_has_T_tbox(box2);
  return (box1->tmin >= box2->tmin);
}

PG_FUNCTION_INFO_V1(overafter_tbox_tbox);
/**
 * Returns true if the first temporal box does not extend before the second one
 */
PGDLLEXPORT Datum
overafter_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(overafter_tbox_tbox_internal(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/**
 * Returns the union of the temporal boxes
 * (internal function)
 */
TBOX *
tbox_union_internal(const TBOX *box1, const TBOX *box2)
{
  ensure_same_dimensionality_tbox(box1, box2);
  /* The union of boxes that do not intersect cannot be represented by a box */
  if (! overlaps_tbox_tbox_internal(box1, box2))
    elog(ERROR, "Result of box union would not be contiguous");

  bool hasx = MOBDB_FLAGS_GET_X(box1->flags);
  bool hast = MOBDB_FLAGS_GET_T(box1->flags);
  double xmin = 0, xmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  if (hasx)
  {
    xmin = Min(box1->xmin, box2->xmin);
    xmax = Max(box1->xmax, box2->xmax);
  }
  if (hast)
  {
    tmin = Min(box1->tmin, box2->tmin);
    tmax = Max(box1->tmax, box2->tmax);
  }
  TBOX *result = tbox_make(hasx, hast, xmin, xmax, tmin, tmax);
  return(result);
}

PG_FUNCTION_INFO_V1(tbox_union);
/**
 * Returns the union of the temporal boxes
 */
PGDLLEXPORT Datum
tbox_union(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  TBOX *result = tbox_union_internal(box1, box2);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the intersection of the temporal boxes
 * (internal function)
 */
TBOX *
tbox_intersection_internal(const TBOX *box1, const TBOX *box2)
{
  bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
  bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
  /* If there is no common dimension */
  if ((! hasx && ! hast) ||
    /* If they do no intersect in one common dimension */
    (hasx && (box1->xmin > box2->xmax || box2->xmin > box1->xmax)) ||
    (hast && (box1->tmin > box2->tmax || box2->tmin > box1->tmax)))
    return(NULL);

  double xmin = 0, xmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  if (hasx)
  {
    xmin = Max(box1->xmin, box2->xmin);
    xmax = Min(box1->xmax, box2->xmax);
  }
  if (hast)
  {
    tmin = Max(box1->tmin, box2->tmin);
    tmax = Min(box1->tmax, box2->tmax);
  }
  TBOX *result = tbox_make(hasx, hast, xmin, xmax, tmin, tmax);
  return(result);
}

PG_FUNCTION_INFO_V1(tbox_intersection);
/**
 * Returns the intersection of the temporal boxes
 */
PGDLLEXPORT Datum
tbox_intersection(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  TBOX *result = tbox_intersection_internal(box1, box2);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_extent_transfn);
/**
 * Transition function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
tbox_extent_transfn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  /* Can't do anything with null inputs */
  if (!box1 && !box2)
    PG_RETURN_NULL();
  TBOX *result = palloc0(sizeof(TBOX));
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
  tbox_expand(result, box2);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbox_extent_combinefn);
/**
 * Combine function for extent aggregation for boxes
 */
PGDLLEXPORT Datum
tbox_extent_combinefn(PG_FUNCTION_ARGS)
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
  tbox_expand(result, box2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/**
 * Returns -1, 0, or 1 depending on whether the first temporal box value is
 * less than, equal to, or greater than the second one (internal function).
 * The time dimension is compared first and then the value dimension.
 *
 * @note Function used for B-tree comparison
 */
int
tbox_cmp_internal(const TBOX *box1, const TBOX *box2)
{
  bool hasx, hast;
  tbox_tbox_flags(box1, box2, &hasx, &hast);
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
    /* Compare the box maxima */
    if (box1->xmax < box2->xmax)
      return -1;
    if (box1->xmax > box2->xmax)
      return 1;
  }
  /* Finally compare the flags */
  if (box1->flags < box2->flags)
    return -1;
  if (box1->flags > box2->flags)
    return 1;
  /* The two boxes are equal */
  return 0;
}

PG_FUNCTION_INFO_V1(tbox_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first temporal box value
 * is less than, equal, or greater than the second one
 *
 * @note Function used for B-tree comparison
 */
PGDLLEXPORT Datum
tbox_cmp(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int  cmp = tbox_cmp_internal(box1, box2);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(tbox_lt);
/**
 * Returns true if the first temporal box value is less than the second one
 */
PGDLLEXPORT Datum
tbox_lt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int  cmp = tbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(tbox_le);
/**
 * Returns true if the first temporal box value is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
tbox_le(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int  cmp = tbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(tbox_ge);
/**
 * Returns true if the first temporal box value is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
tbox_ge(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int  cmp = tbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(tbox_gt);
/**
 * Returns true if the first temporal box value is greater than the second one
 */
PGDLLEXPORT Datum
tbox_gt(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  int  cmp = tbox_cmp_internal(box1, box2);
  PG_RETURN_BOOL(cmp > 0);
}

/**
 * Returns true if the two temporal boxes are equal
 * (internal function)
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
tbox_eq_internal(const TBOX *box1, const TBOX *box2)
{
  if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
    MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
      return false;
  if (box1->xmin != box2->xmin || box1->tmin != box2->tmin ||
    box1->xmax != box2->xmax || box1->tmax != box2->tmax)
    return false;
  /* The two boxes are equal */
  return true;
}

PG_FUNCTION_INFO_V1(tbox_eq);
/**
 * Returns true if the two temporal boxes are equal
 */
PGDLLEXPORT Datum
tbox_eq(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(tbox_eq_internal(box1, box2));
}

PG_FUNCTION_INFO_V1(tbox_ne);
/**
 * Returns true if the two temporal boxes are different
 */
PGDLLEXPORT Datum
tbox_ne(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  PG_RETURN_BOOL(! tbox_eq_internal(box1, box2));
}

/*****************************************************************************/


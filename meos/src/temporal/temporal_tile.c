/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Bin and tile functions for temporal types
 * @note The time bin functions are inspired from TimescaleDB
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "temporal/temporal_tile.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/date.h>
#include <utils/datetime.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

/*****************************************************************************
 * Bin functions for the various span base types
 *****************************************************************************/

/**
 * @ingroup meos_setspan_bin
 * @brief Return the initial value of the bin that contains an integer
 * @param[in] value Input value
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @errval INT_MAX
 */
int
int_get_bin(int value, int vsize, int vorigin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_positive(vsize))
    return INT_MAX;

  if (vorigin != 0)
  {
    /*
     * We need to ensure that the value is in span AFTER the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    if ((vorigin > 0 && value < PG_INT32_MIN + vorigin) ||
        (vorigin < 0 && value > PG_INT32_MAX + vorigin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT_MAX;
    }
    value -= vorigin;
  }
  int result = (value / vsize) * vsize;
  if (value < 0 && value % vsize)
  {
    /*
     * We need to subtract another size if remainder < 0 this only happens
     * if value is negative to begin with and there is a remainder
     * after division. Need to subtract another size since division
     * truncates toward 0 in C99.
     */
    if (result < PG_INT32_MIN + vsize)
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT_MAX;
    }
    else
      result -= vsize;
  }
  result += vorigin;
  return result;
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the initial value of the bin that contains an integer
 * @param[in] value Input value
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @errval INT64_MAX
 */
int64
bigint_get_bin(int64 value, int64 vsize, int64 vorigin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_positive_datum(vsize, T_INT8))
    return INT64_MAX;

  if (vorigin != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    if ((vorigin > 0 && value < PG_INT64_MIN + vorigin) ||
        (vorigin < 0 && value > PG_INT64_MAX + vorigin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT64_MAX;
    }
    value -= vorigin;
  }
  int64 result = (value / vsize) * vsize;
  if (value < 0 && value % vsize)
  {
    /*
     * We need to subtract another size if remainder < 0 this only happens
     * if value is negative to begin with and there is a remainder
     * after division. Need to subtract another size since division
     * truncates toward 0 in C99.
     */
    if (result < PG_INT64_MIN + vsize)
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT64_MAX;
    }
    else
      result -= vsize;
  }
  result += vorigin;
  return result;
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the initial value of the bin that contains a float
 * @param[in] value Input value
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @errval DBL_MAX
 */
double
float_get_bin(double value, double vsize, double vorigin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_nan(value) || ! ensure_not_nan(vsize) ||
      ! ensure_not_nan(vorigin))
    return DBL_MAX;
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8))
    return DBL_MAX;

  if (vorigin != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum double value (-1 * DBL_MAX) and when
     * negative that it is less than the maximum double value (DBL_MAX)
     */
    if ((vorigin > 0 && value < -1 * DBL_MAX + vorigin) ||
        (vorigin < 0 && value > DBL_MAX + vorigin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return DBL_MAX;
    }
    value -= vorigin;
  }
  /* Notice that by using the floor function above we remove the need to
   * add the additional if needed for the integer case to take into account
   * that integer division truncates toward 0 in C99 */
  double result = floor(value / vsize) * vsize;
  result += vorigin;
  /* An infinite value or size has no bin */
  if (! ensure_not_nan_result(result))
    return DBL_MAX;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the interval in the same representation as Postgres timestamps
 */
inline int64
interval_units(const Interval *interval)
{
  return interval->time + (interval->day * USECS_PER_DAY);
}

/**
 * @brief To bin by day we get the year and month of a date and convert that to
 * the nth month since origin
 * @details This allows us to treat month bining similar to int bining. During
 * this process we ignore the day component and therefore only support bining by
 * full months.
 */
static DateADT
date_bin_start(DateADT d, int32 ndays, DateADT origin)
{
  /* In PostgreSQL DateADT is defined as a typedef of int32 */
  return (DateADT) int_get_bin((int) d, (int) ndays, (int) origin);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the initial date of the bin that contains a date
 * @param[in] d Input date
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @errval DATEVAL_NOEND
 */
DateADT
date_get_bin(DateADT d, const Interval *duration, DateADT torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, DATEVAL_NOEND);
  if (! ensure_valid_day_duration(duration))
    return DATEVAL_NOEND;

  if (DATE_NOT_FINITE(d))
    return d;

  int32 ndays = interval_units(duration) / USECS_PER_DAY;
  return date_bin_start(d, ndays, torigin);
}

/**
 * @brief Return the initial timestamp of the bin that contains a timestamptz
 * (internal function)
 * @param[in] t Input timestamp
 * @param[in] size Size of the time bins in PostgreSQL time units
 * @param[in] origin Origin of the bins
 * @errval DT_NOEND
 */
TimestampTz
timestamptz_bin_start(TimestampTz t, int64 size, TimestampTz origin)
{
  if (TIMESTAMP_NOT_FINITE(t))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of span");
    return DT_NOEND;
  }
  if (origin != 0)
  {
    /*
     * We need to ensure that the timestamp is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * time is at least the minimum time value (DT_NOBEGIN) and when negative,
     * that it is less than the maximum time value (DT_NOEND)
     */
    if ((origin > 0 && t < DT_NOBEGIN + origin) ||
        (origin < 0 && t > DT_NOEND + origin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of span");
      return DT_NOEND;
    }

    t -= origin;
  }
  TimestampTz result = (t / size) * size;
  if (t < 0 && t % size)
  {
    /*
     * We need to subtract another size if remainder < 0 this only happens
     * if timestamp is negative to begin with and there is a remainder
     * after division. Need to subtract another size since division
     * truncates toward 0 in C99.
     */
    if (result < DT_NOBEGIN + size)
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of span");
      return DT_NOEND;
    }
    else
      result -= size;
  }
  result += origin;
  return result;
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the initial timestamp of the bin that contains a timestamptz
 * @param[in] t Input timestamp
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 */
TimestampTz
timestamptz_get_bin(TimestampTz t, const Interval *duration,
  TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, DT_NOEND);
  if (! ensure_positive_duration(duration))
    return DT_NOEND;
  int64 size = interval_units(duration);
  return timestamptz_bin_start(t, size, torigin);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bin that contains an integer
 * @details The bin is the half-open integer span starting at the value
 * #int_get_bin returns
 * @param[in] value Input value
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @csqlfn #Value_bin()
 */
Span *
intspan_get_bin(int value, int vsize, int vorigin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_positive(vsize))
    return NULL;
  int lower = int_get_bin(value, vsize, vorigin);
  /* The error value of #int_get_bin is among the starts of a bin whose upper
   * bound exceeds the integer range */
  if (lower > PG_INT32_MAX - vsize)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
    return NULL;
  }
  return span_make(Int32GetDatum(lower), Int32GetDatum(lower + vsize), true,
    false, T_INT4);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bin that contains a big integer
 * @details The bin is the half-open big integer span starting at the value
 * #bigint_get_bin returns
 * @param[in] value Input value
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @csqlfn #Value_bin()
 */
Span *
bigintspan_get_bin(int64 value, int64 vsize, int64 vorigin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8))
    return NULL;
  int64 lower = bigint_get_bin(value, vsize, vorigin);
  /* The error value of #bigint_get_bin is among the starts of a bin whose
   * upper bound exceeds the big integer range */
  if (lower > PG_INT64_MAX - vsize)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
    return NULL;
  }
  return span_make(Int64GetDatum(lower), Int64GetDatum(lower + vsize), true,
    false, T_INT8);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bin that contains a float
 * @details The bin is the half-open float span starting at the value
 * #float_get_bin returns
 * @param[in] value Input value
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @csqlfn #Value_bin()
 */
Span *
floatspan_get_bin(double value, double vsize, double vorigin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_nan(value) || ! ensure_not_nan(vsize) ||
      ! ensure_not_nan(vorigin))
    return NULL;
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8))
    return NULL;
  double lower = float_get_bin(value, vsize, vorigin);
  if (lower == DBL_MAX)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
    return NULL;
  }
  return span_make(Float8GetDatum(lower), Float8GetDatum(lower + vsize), true,
    false, T_FLOAT8);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bin that contains a date
 * @details The bin is the half-open date span starting at the date
 * #date_get_bin returns
 * @param[in] d Input date
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @csqlfn #Date_bin()
 */
Span *
datespan_get_bin(DateADT d, const Interval *duration, DateADT torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_valid_day_duration(duration))
    return NULL;
  DateADT lower = date_get_bin(d, duration, torigin);
  int32 ndays = interval_units(duration) / USECS_PER_DAY;
  DateADT upper = add_date_int(lower, ndays);
  return span_make(DateADTGetDatum(lower), DateADTGetDatum(upper), true,
    false, T_DATE);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bin that contains a timestamptz
 * @details The bin is the half-open timestamptz span starting at the
 * timestamp #timestamptz_get_bin returns
 * @param[in] t Input timestamp
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @csqlfn #Timestamptz_bin()
 */
Span *
tstzspan_get_bin(TimestampTz t, const Interval *duration, TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;
  TimestampTz lower = timestamptz_get_bin(t, duration, torigin);
  /* A finite timestamp has a finite bin, so the end of time is the error
   * value of #timestamptz_get_bin */
  if (lower == DT_NOEND)
    return NULL;
  TimestampTz upper = lower + interval_units(duration);
  return span_make(TimestampTzGetDatum(lower), TimestampTzGetDatum(upper),
    true, false, T_TIMESTAMPTZ);
}

/*****************************************************************************
 * Bin functions for spans
 *****************************************************************************/

/**
 * @brief Return the initial value of the bin that contains a number value
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[in] type Data type of the arguments
 * @pre When called for dates, this function assumes that the duration interval
 * which the calling function translated into the argument `size` does NOT
 * have a month component
 */
Datum
datum_bin(Datum value, Datum size, Datum origin, MeosType type)
{
  /* This function is called directly by the MobilityDB API */
  if (! ensure_positive_datum(size, type))
    return 0;

  assert(span_basetype(type));
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(int_get_bin(DatumGetInt32(value),
        DatumGetInt32(size), DatumGetInt32(origin)));
    case T_INT8:
      return Int64GetDatum(bigint_get_bin(DatumGetInt64(value),
        DatumGetInt64(size), DatumGetInt64(origin)));
    case T_FLOAT8:
      return Float8GetDatum(float_get_bin(DatumGetFloat8(value),
        DatumGetFloat8(size), DatumGetFloat8(origin)));
    case T_DATE:
      return DateADTGetDatum(date_bin_start(DatumGetDateADT(value),
          DatumGetInt32(size), DatumGetDateADT(origin)));
    case T_TIMESTAMPTZ:
      return TimestampTzGetDatum(timestamptz_bin_start(
        DatumGetTimestampTz(value), DatumGetInt64(size),
        DatumGetTimestampTz(origin)));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Unknown bin function for type: %s", meostype_name(type));
      return 0;
  }
}

/**
 * @brief Get the bins of a span
 * @details The bins are half-open and the last one holds the greatest value
 * of the span. A grid that does not contain the upper border of the span
 * leaves out the bin starting at its greatest value when the span extends
 * below it, since that bin holds nothing of the span but its border, as
 * #tile_dim_count states for a box
 * @param[in] s Span to tile
 * @param[in] size Size of the bins
 * @param[in] origin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * span
 * @param[out] start_bin,end_bin Values of the start and end bins
 * @return Number of bins
 * @pre When called for dates, this function assumes that the duration interval
 * which the calling function translated into the argument `size` does NOT
 * have a month component
 */
int
span_num_bins(const Span *s, Datum size, Datum origin, bool border_inc,
  Datum *start_bin, Datum *end_bin)
{
  assert(s); assert(start_bin); assert(end_bin);

  *start_bin = datum_bin(s->lower, size, origin, s->basetype);
  /* The last bin is the one holding the greatest value of the span: the bin
   * starting at an exclusive upper bound holds no value of the span */
  *end_bin = datum_bin(s->upper, size, origin, s->basetype);
  if (s->upper_inc || ! datum_eq(*end_bin, s->upper, s->basetype))
    *end_bin = datum_add(*end_bin, size, s->basetype);
  if (! border_inc)
  {
    /* The greatest value of the span: the upper bound of a discrete span in
     * canonical form is exclusive, so its greatest value is the one before */
    Datum greatest = s->upper;
    bool attained = s->upper_inc;
    if (span_canon_basetype(s->basetype))
    {
      greatest = datum_sub(s->upper, (s->basetype == T_INT8) ?
        Int64GetDatum(1) : Int32GetDatum(1), s->basetype);
      attained = true;
    }
    if (attained && datum_gt(greatest, s->lower, s->basetype) &&
        datum_eq(datum_bin(greatest, size, origin, s->basetype), greatest,
          s->basetype))
      *end_bin = (s->basetype == T_TIMESTAMPTZ) ?
        TimestampTzGetDatum(DatumGetTimestampTz(*end_bin) -
          DatumGetInt64(size)) :
        datum_sub(*end_bin, size, s->basetype);
  }
  switch (s->basetype)
  {
    case T_INT4:
      return (DatumGetInt32(*end_bin) - DatumGetInt32(*start_bin)) /
        DatumGetInt32(size);
    case T_INT8:
      return (DatumGetInt64(*end_bin) - DatumGetInt64(*start_bin)) /
        DatumGetInt64(size);
    case T_FLOAT8:
      return (int) floor((DatumGetFloat8(*end_bin) -
        DatumGetFloat8(*start_bin)) / DatumGetFloat8(size));
    case T_DATE:
      return (DatumGetDateADT(*end_bin) - DatumGetDateADT(*start_bin)) /
          DatumGetInt32(size);
    case T_TIMESTAMPTZ:
      return (DatumGetInt64(*end_bin) - DatumGetInt64(*start_bin)) /
        DatumGetInt64(size);
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Unknown number of bins function for type: %s",
        meostype_name(s->basetype));
      return 0;
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_setspan_bin
 * @brief Return the bins from a span
 * @param[in] s ISpan to split
 * @param[in] vsize Bin size
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
Span *
span_bins(const Span *s, Datum vsize, Datum vorigin, bool border_inc,
  int *count)
{
  assert(s); assert(count);
  /* The out parameter is defined even when a later check fails */
  *count = 0;
  assert(numspan_type(s->spantype) || timespan_type(s->spantype));
  /* The size of the time bins is an interval, a date span among them */
  if (timespan_type(s->spantype) ?
        ! ensure_positive_duration(DatumGetIntervalP(vsize)) :
        ! ensure_positive_datum(vsize, s->basetype))
    return NULL;

  /* Convert an interval into time units */
  Datum size1;
  if (timespan_type(s->spantype))
  {
    if (s->spantype == T_DATESPAN)
      size1 = Int32GetDatum((int32) (interval_units(DatumGetIntervalP(vsize)) /
        USECS_PER_DAY));
    else
      size1 = Int64GetDatum(interval_units(DatumGetIntervalP(vsize)));
  }
  else
    size1 = vsize;
  /* Get the span bounds of the state */
  Datum start_bin, end_bin;
  int nbins = span_num_bins(s, size1, vorigin, border_inc, &start_bin,
    &end_bin);
  Span *bins = palloc0(sizeof(Span) * nbins);
  /* Iterate for each bin */
  Datum lower = start_bin;
  for (int i = 0; i < nbins; i++)
  {
    Datum upper = datum_add(lower, size1, s->basetype);
    span_set(lower, upper, true, false, s->basetype, s->spantype, &bins[i]);
    lower = upper;
  }
  *count = nbins;
  return bins;
}

/**
 * @ingroup meos_internal_setspan_bin
 * @brief Return the bins from a span set
 * @param[in] ss Span set to split
 * @param[in] vsize Bin size
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
Span *
spanset_bins(const SpanSet *ss, Datum vsize, Datum vorigin, bool border_inc,
  int *count)
{
  assert(ss); assert(count);
  /* The out parameter is defined even when a later check fails */
  *count = 0;
  assert(numspan_type(ss->spantype) || timespan_type(ss->spantype));
  /* The size of the time bins is an interval, a date span among them */
  if (timespan_type(ss->spantype) ?
        ! ensure_positive_duration(DatumGetIntervalP(vsize)) :
        ! ensure_positive_datum(vsize, ss->basetype))
    return NULL;

  /* Convert an interval into time units */
  Datum size1;
  if (timespan_type(ss->spantype))
  {
    if (ss->spantype == T_DATESPAN)
      size1 = Int32GetDatum((int32) (interval_units(DatumGetIntervalP(vsize)) /
        USECS_PER_DAY));
    else
      size1 = Int64GetDatum(interval_units(DatumGetIntervalP(vsize)));
  }
  else
    size1 = vsize;
  /* Get the span bounds of the state */
  Datum start_bin, end_bin;
  int nbins = span_num_bins(&ss->span, size1, vorigin, border_inc, &start_bin,
    &end_bin);
  Span *bins = palloc0(sizeof(Span) * nbins);
  /* Set the span of the state */
  /* Iterate for each bin */
  Datum lower = start_bin;
  int count1 = 0;
  for (int i = 0; i < nbins; i++)
  {
    Datum upper = datum_add(lower, size1, ss->basetype);
    Span span;
    span_set(lower, upper, true, false, ss->basetype, ss->spantype, &span);
    /* Restrict the spanset to the span */
    SpanSet *atspan = intersection_spanset_span(ss, &span);
    if (atspan)
    {
      memcpy(&bins[count1++], &atspan->span, sizeof(Span));
      pfree(atspan);
    }
    lower = upper;
  }
  *count = count1;
  return bins;
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of an integer span
 * @param[in] s Input span to split
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Span_bins()
 */
Span *
intspan_bins(const Span *s, int vsize, int vorigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, NULL);
  return span_bins(s, Int32GetDatum(vsize), Int32GetDatum(vorigin), border_inc,
    count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a big integer span
 * @param[in] s Input span to split
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Span_bins()
 */
Span *
bigintspan_bins(const Span *s, int64 vsize, int64 vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, NULL);
  return span_bins(s, Int64GetDatum(vsize), Int64GetDatum(vorigin), border_inc,
    count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a float span
 * @param[in] s Input span to split
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Span_bins()
 */
Span *
floatspan_bins(const Span *s, double vsize, double vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin))
    return NULL;
  return span_bins(s, Float8GetDatum(vsize), Float8GetDatum(vorigin),
    border_inc, count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a date span
 * @param[in] s Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Span_bins()
 */
Span *
datespan_bins(const Span *s, const Interval *duration, DateADT torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return span_bins(s, PointerGetDatum(duration), DateADTGetDatum(torigin),
    border_inc, count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a timestamptz span
 * @param[in] s Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Span_bins()
 */
Span *
tstzspan_bins(const Span *s, const Interval *duration, TimestampTz torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return span_bins(s, PointerGetDatum(duration), TimestampTzGetDatum(torigin),
    border_inc, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of an integer span set
 * @param[in] ss SpanSet number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Spanset_bins()
 */
Span *
intspanset_bins(const SpanSet *ss, int vsize, int vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return spanset_bins(ss, Int32GetDatum(vsize), Int32GetDatum(vorigin),
    border_inc, count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a big integer span set
 * @param[in] ss SpanSet number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Spanset_bins()
 */
Span *
bigintspanset_bins(const SpanSet *ss, int64 vsize, int64 vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return spanset_bins(ss, Int64GetDatum(vsize), Int64GetDatum(vorigin),
    border_inc, count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a float span set
 * @param[in] ss SpanSet number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Spanset_bins()
 */
Span *
floatspanset_bins(const SpanSet *ss, double vsize, double vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin))
    return NULL;
  return spanset_bins(ss, Float8GetDatum(vsize), Float8GetDatum(vorigin),
    border_inc, count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a date span set
 * @param[in] ss Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Spanset_bins()
 */
Span *
datespanset_bins(const SpanSet *ss, const Interval *duration,
  DateADT torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return spanset_bins(ss, PointerGetDatum(duration), DateADTGetDatum(torigin),
    border_inc, count);
}

/**
 * @ingroup meos_setspan_bin
 * @brief Return the bins of a timestamptz span set
 * @param[in] ss Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Spanset_bins()
 */
Span *
tstzspanset_bins(const SpanSet *ss, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return spanset_bins(ss, PointerGetDatum(duration),
    TimestampTzGetDatum(torigin), border_inc, count);
}

/*****************************************************************************
 * Bins functions for temporal types
 *****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time bins of a temporal value
 * @param[in] temp Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Temporal_time_bins()
 */
Span *
temporal_time_bins(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;

  /* Set bounding box */
  Span bounds;
  temporal_set_tstzspan(temp, &bounds);
  /* Compute the spans and create the resulting array */
  int nbins;
  Span *bins = span_bins(&bounds, PointerGetDatum(duration), 
    TimestampTzGetDatum(torigin), border_inc, &nbins);
  Span *result = palloc(sizeof(Span) * nbins);
  int count1 = 0;
  for (int i = 0; i < nbins; i++)
  {
    /* Restrict the temporal value to the timestamptz span and compute
     * its bounding span */
    Temporal *atspan = temporal_restrict_tstzspan(temp, &bins[i], REST_AT);
    if (atspan == NULL)
      continue;
    /* Copy the span to the result */
    memcpy(&result[count1++], &bins[i], sizeof(Span));
    pfree(atspan);
  }
  pfree(bins);
  *count = count1;
  return result;
}

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return the bins of a temporal number
 * @param[in] temp Input span to split
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
Span *
tnumber_value_bins(const Temporal *temp, Datum vsize, Datum vorigin,
  bool border_inc, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  /* Set bounding box */
  Span bounds;
  tnumber_set_span(temp, &bounds);
  /* Compute the spans and create the resulting array */
  int nbins;
  Span *bins = span_bins(&bounds, vsize, vorigin, border_inc, &nbins);
  if (! bins)
    return NULL;
  Span *result = palloc(sizeof(Span) * nbins);
  int count1 = 0;
  for (int i = 0; i < nbins; i++)
  {
    /* Restrict the temporal value to the timestamptz span and compute
     * its bounding span */
    Temporal *atspan = tnumber_restrict_span(temp, &bins[i], REST_AT);
    if (atspan == NULL)
      continue;
    /* Copy the span to the result */
    memcpy(&result[count1++], &bins[i], sizeof(Span));
    pfree(atspan);
  }
  pfree(bins);
  *count = count1;
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of an integer span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_bins()
 */
Span *
tint_value_bins(const Temporal *temp, int vsize, int vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_bins(temp, Int32GetDatum(vsize),
    Int32GetDatum(vorigin), border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a big integer span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_bins()
 */
Span *
tbigint_value_bins(const Temporal *temp, int64 vsize, int64 vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_bins(temp, Int64GetDatum(vsize),
    Int64GetDatum(vorigin), border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a float span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_bins()
 */
Span *
tfloat_value_bins(const Temporal *temp, double vsize, double vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin))
    return NULL;
  return tnumber_value_bins(temp, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), border_inc, count);
}

/*****************************************************************************
 * TBox tile functions
 *****************************************************************************/

/**
 * @brief Create the initial state for tiling operations
 * @param[in] temp Temporal number @nullable
 * @param[in] box Bounds of the multidimensional grid
 * @param[in] vsize Value size of the tiles, may be 0 for time tiles
 * @param[in] duration Interval defining the time size of the tile, may be
 * @p NULL for value tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
  * @param[in] border_inc True when the grid contains the upper border of the
 * extent
*/
TboxGridState *
tbox_tile_state_make(const Temporal *temp, const TBox *box, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin, bool border_inc)
{
  assert(box);
  /* A box without value span is tiled by time alone */
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  assert(duration || (hasx && datum_gt(vsize, 0, box->span.basetype)));

  /* Create the state, use palloc0 to initialize missing dimensions */
  TboxGridState *state = palloc0(sizeof(TboxGridState));
  /* Fill in state */
  state->i = 1;
  state->ntiles = 1;
  Datum start_bin, end_bin;
  /* Set the value dimension of the state box*/
  if (hasx && datum_gt(vsize, 0, box->span.basetype))
  {
    /* The given vsize is greater than 0 */
    state->valuegrid = true;
    state->vsize = vsize;
    state->max_coords[0] = span_num_bins(&box->span, vsize, vorigin,
      border_inc, &start_bin, &end_bin) - 1;
    state->ntiles *= (state->max_coords[0] + 1);
    span_set(start_bin, end_bin, true, false, box->span.basetype,
      box->span.spantype, &state->box.span);
  }
  else if (hasx)
  {
    /* A value size of 0 leaves the value dimension undivided: the vsize of
     * the state stays 0 and every tile keeps the value span of the box */
    state->max_coords[0] = 0;
    span_set(box->span.lower, box->span.upper, box->span.lower_inc,
      box->span.upper_inc, box->span.basetype, box->span.spantype,
      &state->box.span);
  }
  MEOS_FLAGS_SET_X(state->box.flags, hasx);
  /* Set the time dimension of the state box */
  if (duration)
  {
    state->tunits = interval_units(duration);
    state->max_coords[1] = span_num_bins(&box->period,
      Int64GetDatum(state->tunits), TimestampTzGetDatum(torigin),
      border_inc, &start_bin, &end_bin) - 1;
    state->ntiles *= (state->max_coords[1] + 1);
    span_set(start_bin, end_bin, true, false, T_TIMESTAMPTZ, T_TSTZSPAN,
      &state->box.period);
    MEOS_FLAGS_SET_T(state->box.flags, true);
  }
  /* Set the temporal value */
  state->temp = temp;
  /* Set the state start values */
  state->value = state->box.span.lower;
  state->t = DatumGetTimestampTz(state->box.period.lower);
  return state;
}

/**
 * @brief Generate a tile from the a multidimensional grid
 * @param[in] value Start value of the tile to output
 * @param[in] t Start timestamp of the tile to output
 * @param[in] vsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 * @param[in] basetype Type of the value
 * @param[in] spantype Span type of the value
 * @param[out] box Output box
 */
void
tbox_tile_state_set(Datum value, TimestampTz t, Datum vsize, int64 tunits,
  MeosType basetype, MeosType spantype, TBox *box)
{
  assert(box);

  /* Initialize the box for missing dimensions */
  memset(box, 0, sizeof(TBox));
  /* Set value span */
  if (numspan_type(spantype))
  {
    Datum xmin = value;
    Datum xmax = datum_add(value, vsize, basetype);
    span_set(xmin, xmax, true, false, basetype, spantype, &box->span);
    MEOS_FLAGS_SET_X(box->flags, true);
  }
  /* Set value span */
  if (tunits)
  {
    Datum tmin = TimestampTzGetDatum(t);
    Datum tmax = TimestampTzGetDatum(t + tunits);
    MEOS_FLAGS_SET_T(box->flags, true);
    span_set(tmin, tmax, true, false, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  }
  return;
}

/**
 * @brief Get the current tile of the multidimensional grid
 * @param[in] state State to increment
 * @param[out] box Current tile
 */
bool
tbox_tile_state_get(TboxGridState *state, TBox *box)
{
  if (! state || state->done)
    return false;
  /* Get the box of the current tile */
  tbox_tile_state_set(state->value, state->t, state->vsize, state->tunits,
    state->box.span.basetype,
    state->valuegrid ? state->box.span.spantype : T_UNKNOWN, box);
  /* A grid that does not divide the value dimension keeps the value span of
   * the box, with its bounds, in every tile */
  if (! state->valuegrid && MEOS_FLAGS_GET_X(state->box.flags))
  {
    box->span = state->box.span;
    MEOS_FLAGS_SET_X(box->flags, true);
  }
  return true;
}

/**
 * @brief Increment the current state to the next tile of the multidimensional
 * grid
 * @param[in] state State to increment
 */
void
tbox_tile_state_next(TboxGridState *state)
{
  assert(state);
  assert(MEOS_FLAGS_GET_X(state->box.flags) ||
    MEOS_FLAGS_GET_T(state->box.flags));

  /* state is non-NULL by the assert above; only the done check matters. */
  if (state->done)
    return;
  /* Move to the next tile */
  state->i++;
  /* Advance the current values for the available dimensions */
  if (MEOS_FLAGS_GET_X(state->box.flags))
  {
    /* X dimension */
    state->value = datum_add(state->value, state->vsize,
      state->box.span.basetype);
    state->coords[0]++;
    if (state->coords[0] > state->max_coords[0])
    {
      if (MEOS_FLAGS_GET_T(state->box.flags))
      {
        /* X and T dimensions */
        state->value = state->box.span.lower;
        state->coords[0] = 0;
        state->t += state->tunits;
        state->coords[1]++;
        if (state->coords[1] > state->max_coords[1])
        {
          state->done = true;
          return;
        }
      }
      else
      {
        /* Only X dimension */
        state->done = true;
        return;
      }
    }
  }
  else
  {
    /* Only T dimension */
    if (MEOS_FLAGS_GET_T(state->box.flags))
    {
      state->t += state->tunits;
      state->coords[1]++;
      if (state->coords[1] > state->max_coords[1])
      {
        state->done = true;
        return;
      }
    }
  }
  return;
}

/*****************************************************************************
 * Multidimensional tile functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] basetype Type of the value
 * @param[in] spantype Type of the value span, a time span type for a tile
 * without value dimension
 */
TBox *
tbox_value_time_tile(Datum value, TimestampTz t, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  MeosType basetype, MeosType spantype)
{
  /* Ensure the validity of the arguments */
  if (duration && ! ensure_positive_duration(duration))
    return NULL;

  /* Determine whether there is a value tile */
  bool valuetile = datum_gt(vsize, (Datum) 0, basetype);
  assert(valuetile || duration);
  /* Initialize to 0 missing arguments */
  Datum value_bin = (Datum) 0;
  int64 tunits = 0;
  TimestampTz time_bin = 0;
  /* Determine the tile */
  if (valuetile)
    value_bin = datum_bin(value, vsize, vorigin, basetype);
  if (duration)
  {
    tunits = interval_units(duration);
    time_bin = timestamptz_get_bin(t, duration, torigin);
  }
  TBox *result = palloc(sizeof(TBox));
  tbox_tile_state_set(value_bin, time_bin, vsize, tunits, basetype, spantype,
    result);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return a tile in the value and time grid of a temporal box
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_get_value_time_tile()
 */
TBox *
tbox_get_value_time_tile(double value, TimestampTz t, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_not_nan(value) || ! ensure_not_nan(vsize) ||
      ! ensure_not_nan(vorigin))
    return NULL;
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_positive_duration(duration))
    return NULL;
  return tbox_value_time_tile(Float8GetDatum(value), t, Float8GetDatum(vsize),
    duration, Float8GetDatum(vorigin), torigin, T_FLOAT8, T_FLOATSPAN);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return a tile in the value grid of a temporal box
 * @param[in] value Value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @csqlfn #Tbox_get_value_tile()
 */
TBox *
tbox_get_value_tile(double value, double vsize, double vorigin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_nan(value) || ! ensure_not_nan(vsize) ||
      ! ensure_not_nan(vorigin))
    return NULL;
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8))
    return NULL;
  return tbox_value_time_tile(Float8GetDatum(value), 0, Float8GetDatum(vsize),
    NULL, Float8GetDatum(vorigin), 0, T_FLOAT8, T_FLOATSPAN);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return a tile in the time grid of a temporal box
 * @param[in] t Timestamp
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_get_time_tile()
 */
TBox *
tbox_get_time_tile(TimestampTz t, const Interval *duration,
  TimestampTz torigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;
  /* A time span type states that the tile has no value dimension */
  return tbox_value_time_tile(Float8GetDatum(0.0), t, Float8GetDatum(0.0),
    duration, Float8GetDatum(0.0), torigin, T_TIMESTAMPTZ, T_TSTZSPAN);
}

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Generate a multidimensional grid for temporal numbers
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the tiles on the time
 * dimension @nullable
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tbox_value_time_tiles(const TBox *box, Datum vsize, const Interval *duration,
  Datum vorigin, TimestampTz torigin, bool border_inc, int *count)
{
  assert(box); assert(count);
  assert(! MEOS_FLAGS_GET_X(box->flags) ||
    not_negative_datum(vsize, box->span.basetype));
  assert(! duration || positive_duration(duration));

  TboxGridState *state = tbox_tile_state_make(NULL, box, vsize, duration,
    vorigin, torigin, border_inc);
  int count1 = state->ntiles;

  /* Compute the tiles */
  TBox *result = palloc0(sizeof(TBox) * count1);
  for (int i = 0; i < count1; i++)
  {
    tbox_tile_state_get(state, &result[i]);
    tbox_tile_state_next(state);
  }
  *count = count1;
  pfree(state);
  return result;
}

/**
 * @brief Ensure the validity of a temporal box and the grid laid over it
 * @details Each caller tests the sizes of the dimensions it takes, so a value
 * size of 0 is a value dimension the caller does not take. A value dimension
 * needs a box with a value span of the given type, as #tintbox_shift_scale
 * needs, and a time dimension needs a box with a period and a positive
 * duration
 * @param[in] box Temporal box
 * @param[in] vsize Value size of the tiles, 0 when the value dimension is not
 * used for tiling
 * @param[in] duration Interval defining the size of the bins @nullable
 * @param[in] spantype Span type of the value dimension
 */
static bool
ensure_valid_tbox_tiles(const TBox *box, double vsize,
  const Interval *duration, MeosType spantype)
{
  if (vsize > 0 && (! ensure_has_X(T_TBOX, box->flags) ||
      ! ensure_span_isof_type(&box->span, spantype)))
    return false;
  if (duration && (! ensure_has_T(T_TBOX, box->flags) ||
      ! ensure_positive_duration(duration)))
    return false;
  return true;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tintbox_value_time_tiles(const TBox *box, int vsize, const Interval *duration,
  int vorigin, TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int32GetDatum(vsize), T_INT4) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, duration, T_INTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal big integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tbigintbox_value_time_tiles(const TBox *box, int64 vsize,
  const Interval *duration, int64 vorigin, TimestampTz torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, duration, T_BIGINTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int64GetDatum(vsize), duration,
    Int64GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tfloatbox_value_time_tiles(const TBox *box, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin))
    return NULL;
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_valid_tbox_tiles(box, vsize, duration, T_FLOATSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tintbox_value_tiles(const TBox *box, int vsize, int vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_positive_datum(Int32GetDatum(vsize), T_INT4) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, NULL, T_INTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int32GetDatum(vsize), NULL,
    Int32GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal big integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tbigintbox_value_tiles(const TBox *box, int64 vsize, int64 vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8) ||
      ! ensure_valid_tbox_tiles(box, (double) vsize, NULL, T_BIGINTSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Int64GetDatum(vsize), NULL,
    Int64GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the value tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_tiles()
 */
TBox *
tfloatbox_value_tiles(const TBox *box, double vsize, double vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin))
    return NULL;
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_valid_tbox_tiles(box, vsize, NULL, T_FLOATSPAN))
    return NULL;
  return tbox_value_time_tiles(box, Float8GetDatum(vsize), NULL,
    Float8GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_time_tiles()
 */
TBox *
tbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_valid_tbox_tiles(box, 0.0, duration, box->span.spantype))
    return NULL;
  /* The value size of the tiles is zero in the base type of the box */
  Datum vzero = (box->span.basetype == T_FLOAT8) ? Float8GetDatum(0.0) :
    (box->span.basetype == T_INT8) ? Int64GetDatum(0) : Int32GetDatum(0);
  return tbox_value_time_tiles(box, vzero, duration, vzero, torigin,
    border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tintbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  return tbox_time_tiles(box, duration, torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal big integer box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tbigintbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  return tbox_time_tiles(box, duration, torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time tiles of a temporal float box
 * @param[in] box Input box to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 */
TBox *
tfloatbox_time_tiles(const TBox *box, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  return tbox_time_tiles(box, duration, torigin, border_inc, count);
}

/*****************************************************************************
 * TBoxes functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Set the state with a temporal number and a value and possibly time
 * grid for splitting or obtaining a set of temporal boxes
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension, may be zero if the value
 * dimension is not used for splitting
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] ntiles Number of tiles
 * @note The function can be used for obtaining value boxes, time boxes, and
 * value and time boxes
 */
TboxGridState *
tnumber_value_time_tile_init(const Temporal *temp, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  bool border_inc, int *ntiles)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(ntiles, NULL);
  *ntiles = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  /* A grid without a time dimension needs a positive value size */
  MeosType basetype = temptype_basetype(temp->temptype);
  if (! ensure_not_negative_datum(vsize, basetype) ||
      (! duration && ! ensure_positive_datum(vsize, basetype)) ||
      (duration && ! ensure_positive_duration(duration)))
    return NULL;

  /* Set bounding box */
  TBox bounds;
  tnumber_set_tbox(temp, &bounds);
  /* Create function state */
  TboxGridState *state = tbox_tile_state_make(temp, &bounds, vsize, duration,
    vorigin, torigin, border_inc);
  *ntiles = state->ntiles;
  return state;
}

/**
 * @brief Set the state with a temporal number and a value and possibly time
 * grid for splitting it
 * @details A split states the value bin of each fragment, so it lays a value
 * grid, which the state of #tnumber_value_time_tile_init lays for a positive
 * `vsize`, as #tgeo_space_time_split_init requires its spatial grid
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension, which must be positive
 * @param[in] duration Size of the time dimension as an interval @nullable
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] ntiles Number of tiles
 */
TboxGridState *
tnumber_value_time_split_init(const Temporal *temp, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  bool border_inc, int *ntiles)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(ntiles, NULL);
  *ntiles = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);
  if (! ensure_positive_datum(vsize, temptype_basetype(temp->temptype)))
    return NULL;
  return tnumber_value_time_tile_init(temp, vsize, duration, vorigin,
    torigin, border_inc, ntiles);
}

/**
 * @brief Set the state with a temporal box and a value and possibly time
 * grid for obtaining a set of tiles
 * @details The value size and origin are read against the base type of the
 * box, as #tbox_expand_value reads a value: an integer box takes a size and an
 * origin of its own base type, and a float box also takes integer ones, which
 * it reads as floats
 * @param[in] box Temporal box
 * @param[in] vsize Size of the value dimension, ignored when @p basetype is
 * `T_UNKNOWN`
 * @param[in] duration Size of the time dimension as an interval @nullable
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] basetype Base type of the value size and origin, `T_UNKNOWN`
 * when the value dimension is not used for tiling
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] ntiles Number of tiles
 * @note The function can be used for obtaining value tiles, time tiles, and
 * value and time tiles
 */
TboxGridState *
tbox_value_time_tile_init(const TBox *box, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  MeosType basetype, bool border_inc, int *ntiles)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(ntiles, NULL);
  *ntiles = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL);
  /* A box tiled by time alone takes a zero value size */
  Datum size = Float8GetDatum(0.0), origin = Float8GetDatum(0.0);
  if (basetype != T_UNKNOWN)
  {
    if (! ensure_has_X(T_TBOX, box->flags))
      return NULL;
    MeosType boxtype = box->span.basetype;
    if (boxtype == T_FLOAT8 && basetype != T_FLOAT8)
    {
      size = Float8GetDatum(datum_double(vsize, basetype));
      origin = Float8GetDatum(datum_double(vorigin, basetype));
    }
    else if (basetype == boxtype)
    {
      size = vsize;
      origin = vorigin;
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The value size and origin of the tiles of a box of %s values cannot "
        "be of type %s", meostype_name(boxtype), meostype_name(basetype));
      return NULL;
    }
    if (! ensure_positive_datum(size, boxtype))
      return NULL;
  }
  if (duration && (! ensure_has_T(T_TBOX, box->flags) ||
      ! ensure_positive_duration(duration)))
    return NULL;

  /* Create function state */
  TboxGridState *state = tbox_tile_state_make(NULL, box, size, duration,
    origin, torigin, border_inc);
  *ntiles = state->ntiles;
  return state;
}

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and possibly a time grid
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension
 * @param[in] duration Size of the time dimension as an interval @nullable
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @note The check for parameter validity is done in function
 * #tnumber_value_time_tile_init to be shared for both MEOS and MobilityDB
 * @csqlfn #Tnumber_value_boxes(), #Tnumber_time_boxes()
 */
TBox *
tnumber_value_time_boxes(const Temporal *temp, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  bool border_inc, int *count)
{
  assert(temp); assert(count);
  /* The out parameter is defined even when a later check fails */
  *count = 0;

  /* Initialize state */
  int ntiles;
  TboxGridState *state = tnumber_value_time_tile_init(temp, vsize, duration,
    vorigin, torigin, border_inc, &ntiles);
  if (! state)
    return NULL;

  TBox *result = palloc(sizeof(TBox) * ntiles);
  /* We need to loop since atTbox may be NULL */
  int i = 0;
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      pfree(state);
      break;
    }

    /* Get current tile (if any) and advance state
     * It is necessary to test if we found a tile since the previous tile
     * may be the last one set in the associated bit matrix */
    TBox box;
    if (! tbox_tile_state_get(state, &box))
    {
      pfree(state);
      break;
    }
    tbox_tile_state_next(state);

    /* Restrict the temporal number to the box and compute its bounding box */
    Temporal *attbox = tnumber_at_tbox((Temporal *) state->temp, &box);
    if (attbox == NULL)
      continue;
    tnumber_set_tbox(attbox, &box);
    pfree(attbox);

    /* Copy the box to the result */
    memcpy(&result[i++], &box, sizeof(TBox));
  }
  *count = i;
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and a time grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_time_boxes()
 */
TBox *
tint_value_time_boxes(const Temporal *temp, int vsize,
  const Interval *duration, int vorigin, TimestampTz torigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int32GetDatum(vsize), T_INT4) ||
      ! ensure_positive_duration(duration))
    return NULL;
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal big integer split with
 * respect to a value and a time grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_time_boxes()
 */
TBox *
tbigint_value_time_boxes(const Temporal *temp, int64 vsize,
  const Interval *duration, int64 vorigin, TimestampTz torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8) ||
      ! ensure_positive_duration(duration))
    return NULL;
  return tnumber_value_time_boxes(temp, Int64GetDatum(vsize), duration,
    Int64GetDatum(vorigin), torigin, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and a time grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles, which must be positive
 * @param[in] duration Interval defining the size of the bins, which must be
 * positive
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_time_boxes()
 */
TBox *
tfloat_value_time_boxes(const Temporal *temp, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin))
    return NULL;
  if (! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_positive_duration(duration))
    return NULL;
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, border_inc, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes()
 */
TBox *
tint_value_boxes(const Temporal *temp, int vsize, int vorigin, bool border_inc,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), NULL,
    Int32GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a time grid
 * @param[in] temp Temporal value
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes()
 */
TBox *
tint_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return tnumber_value_time_boxes(temp, Int32GetDatum(0), duration,
    Int32GetDatum(0), torigin, border_inc, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal big integer split with
 * respect to a value grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes()
 */
TBox *
tbigint_value_boxes(const Temporal *temp, int64 vsize, int64 vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_time_boxes(temp, Int64GetDatum(vsize), NULL,
    Int64GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal big integer split with
 * respect to a time grid
 * @param[in] temp Temporal value
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes()
 */
TBox *
tbigint_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return tnumber_value_time_boxes(temp, Int64GetDatum(0), duration,
    Int64GetDatum(0), torigin, border_inc, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value grid
 * @param[in] temp Temporal value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes()
 */
TBox *
tfloat_value_boxes(const Temporal *temp, double vsize, double vorigin,
  bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin))
    return NULL;
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), NULL,
    Float8GetDatum(vorigin), 0, border_inc, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a time grid
 * @param[in] temp Temporal value
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes()
 */
TBox *
tfloat_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  return tnumber_value_time_boxes(temp, Float8GetDatum(0.0), duration,
    Float8GetDatum(0.0), torigin, border_inc, count);
}

/*****************************************************************************
 * Time split functions for temporal values
 *****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] inst Temporal value
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] count Number of values in the output array
 */
static TInstant **
tinstant_time_split(const TInstant *inst, int64 tunits, TimestampTz torigin,
  TimestampTz **bins, int *count)
{
  assert(inst);
  TInstant **result = palloc(sizeof(TInstant *));
  TimestampTz *times = palloc(sizeof(TimestampTz));
  result[0] = tinstant_copy(inst);
  times[0] = timestamptz_bin_start(inst->t, tunits, torigin);
  *bins = times;
  *count = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start Start timestamp of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tdiscseq_time_split(const TSequence *seq, TimestampTz start, int64 tunits,
  int count, TimestampTz **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0,       /* counter for instants of temporal value */
      ninsts = 0,  /* counter for instants of next split */
      nfrags = 0;  /* counter for resulting fragments */
  TimestampTz lower = start;
  TimestampTz upper = start + tunits;
  /* The instants past the last of the bins are in none of them */
  TimestampTz end = start + tunits * count;
  while (i < seq->count && lower < end)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (lower <= inst->t && inst->t < upper)
    {
      instants[ninsts++] = (TInstant *) inst;
      i++;
    }
    else
    {
      if (ninsts > 0)
      {
        times[nfrags] = lower;
        result[nfrags++] = tsequence_make(instants, ninsts, true, true,
          DISCRETE, NORMALIZE_NO);
        ninsts = 0;
      }
      lower = upper;
      upper += tunits;
    }
  }
  if (ninsts > 0)
  {
    times[nfrags] = lower;
    result[nfrags++] = tsequence_make(instants, ninsts, true, true, DISCRETE,
      NORMALIZE_NO);
  }
  pfree(instants);
  *bins = times;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] result Output array of fragments of the temporal value
 * @param[out] times Output array of bin lower bounds
 * @note This function is called for each sequence of a temporal sequence set
 */
static int
tcontseq_time_split_iter(const TSequence *seq, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TSequence **result,
  TimestampTz *times)
{
  assert(seq); assert(result); assert(times);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  TimestampTz lower = start;
  TimestampTz upper = lower + tunits;
  /* This loop is needed for filtering unnecesary time bins that are in the
   * time gaps between sequences composing a sequence set.
   * The upper bound for the bin is exclusive => the test below is >= */
  while (lower < end &&
    (DatumGetTimestampTz(seq->period.lower) >= upper ||
     lower > DatumGetTimestampTz(seq->period.upper) ||
     (lower == DatumGetTimestampTz(seq->period.upper) &&
        ! seq->period.upper_inc)))
  {
    lower = upper;
    upper += tunits;
  }

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  int i = 0,      /* counter for instants of temporal value */
      ninsts = 0, /* counter for instants of next split */
      nfree = 0,  /* counter for instants to free */
      nfrags = 0; /* counter for resulting fragments */
  bool lower_inc1;
  while (i < seq->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    /* If the instant is in the bin, or is the end value of the bin on a
     * linear segment continuing past it: the bins are half-open, so the last
     * instant on the upper bound of a bin starts the next one */
    if ((lower <= inst->t && inst->t < upper) ||
      (inst->t == upper && interp == LINEAR && i < seq->count - 1))
    {
      instants[ninsts++] = (TInstant *) inst;
      i++;
    }
    else
    {
      assert(ninsts > 0); assert(i > 0);
      /* Compute the value at the end of the bin on the segment of the
       * sequence that crosses it, as the restriction to the bin does: the
       * last instant of the bin may be the value computed at its lower bound */
      if (instants[ninsts - 1]->t < upper)
      {
        if (interp == LINEAR)
          tofree[nfree] = tsegment_at_timestamptz(TSEQUENCE_INST_N(seq, i - 1),
            inst, interp, upper);
        else
        {
          /* The last two values of sequences with step interpolation and
           * exclusive upper bound must be equal */
          Datum value = tinstant_value_p(instants[ninsts - 1]);
          tofree[nfree] = tinstant_make(value, seq->temptype, upper);
        }
        instants[ninsts++] = tofree[nfree++];
      }

      /* Compute the fragment */
      lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
      times[nfrags] = lower;
      result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
         (ninsts > 1) ? false : true, interp, NORMALIZE);

      /* Set up for the next bin */
      ninsts = 0;
      lower = upper;
      upper += tunits;
      /* The second condition is needed for filtering unnecesary time bins
       * that are in the gaps between sequences composing a sequence set */
      if (lower >= end || ! contains_span_timestamptz(&seq->period, lower))
        break;
      /* The end value of the previous bin is the start of the new bin */
      if (lower < inst->t)
        instants[ninsts++] = (TInstant *) TSEQUENCE_INST_N(result[nfrags - 1],
          result[nfrags - 1]->count - 1);
     }
  }
  if (ninsts > 0)
  {
    lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
    times[nfrags] = lower;
    result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
      seq->period.upper_inc, interp, NORMALIZE);
  }
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  return nfrags;
}

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tcontseq_time_split(const TSequence *seq, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  *newcount = tcontseq_time_split_iter(seq, start, end, tunits, count, result,
    times);
  *bins = times;
  return result;
}

/**
 * @brief Split a temporal value into an array of disjoint fragments
 * @param[in] ss Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequenceSet **
tsequenceset_time_split(const TSequenceSet *ss, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TimestampTz **bins,
  int *newcount)
{
  assert(ss); assert(bins); assert(newcount);

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    TSequence **sequences = tcontseq_time_split(TSEQUENCESET_SEQ_N(ss, 0),
      start, end, tunits, count, bins, newcount);
    TSequenceSet **result = palloc(sizeof(TSequenceSet *) * *newcount);
    for (int i = 0; i < *newcount; i++)
      result[i] = tsequence_to_tsequenceset_free(sequences[i]);
    pfree(sequences);
    return result;
  }

  /* General case */
  /* Sequences obtained by spliting one composing sequence */
  TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Start timestamp of bins obtained by spliting one composing sequence */
  TimestampTz *times = palloc(sizeof(TimestampTz) * (ss->count + count));
  /* Sequences composing the currently constructed bin of the sequence set */
  TSequence **fragments = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Sequences for the bins of the sequence set */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  /* Variable used to adjust the start timestamp passed to the
   * tcontseq_time_split1 function in the loop */
  TimestampTz lower = start;
  int nfrags = 0, /* Number of accumulated fragments of the current time bin */
      nbucks = 0; /* Number of time bins already processed */
  for (int i = 0; i < ss->count; i++)
  {
    TimestampTz upper = lower + tunits;
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Output the accumulated fragments of the current time bin (if any)
     * if the current sequence starts on the next time bin */
    if (nfrags > 0 && DatumGetTimestampTz(seq->period.lower) >= upper)
    {
      result[nbucks++] = tsequenceset_make(fragments, nfrags, NORMALIZE);
      for (int j = 0; j < nfrags; j++)
        pfree(fragments[j]);
      nfrags = 0;
      lower += tunits;
    }
    /* Number of time bins of the current sequence */
    int l = tcontseq_time_split_iter(seq, lower, end, tunits, count,
      sequences, &times[nbucks]);
    /* If the current sequence has produced more than two time bins */
    if (l > 1)
    {
      /* Assemble the accumulated fragments of the first time bin (if any)  */
      if (nfrags == 0)
        result[nbucks++] = tsequence_to_tsequenceset_free(sequences[0]);
      else
      {
        fragments[nfrags++] = sequences[0];
        result[nbucks++] = tsequenceset_make(fragments, nfrags, NORMALIZE);
        for (int j = 0; j < nfrags; j++)
          pfree(fragments[j]);
        nfrags = 0;
      }
      for (int j = 1; j < l - 1; j++)
        result[nbucks++] = tsequence_to_tsequenceset_free(sequences[j]);
    }
    /* Save the last fragment in case it is necessary to assemble with the
     * first one of the next sequence */
    fragments[nfrags++] = sequences[l - 1];
    lower = times[nbucks];
  }
  /* Process the accumulated fragments of the last time bin */
  if (nfrags > 0)
  {
    result[nbucks++] = tsequenceset_make(fragments, nfrags, NORMALIZE);
    for (int j = 0; j < nfrags; j++)
      pfree(fragments[j]);
  }
  pfree(sequences); pfree(fragments);
  *bins = times;
  *newcount = nbucks;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the fragments of a temporal value split according to
 * time bins
 * @param[in] temp Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
Temporal **
temporal_time_split_intl(const Temporal *temp, TimestampTz start,
  TimestampTz end, int64 tunits, TimestampTz torigin, int count,
  TimestampTz **bins, int *newcount)
{
  assert(temp); assert(bins); assert(newcount); assert(start < end);
  assert(count > 0);

  /* Split the temporal value */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal **) tinstant_time_split((const TInstant *) temp,
        tunits, torigin, bins, newcount);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tdiscseq_time_split((const TSequence *) temp,
          start, tunits, count, bins, newcount) :
        (Temporal **) tcontseq_time_split((const TSequence *) temp,
          start, end, tunits, count, bins, newcount);
    default: /* TSEQUENCESET */
      return (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
        start, end, tunits, count, bins, newcount);
  }
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal value split according to
 * time bins
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_time_split()
 */
Temporal **
temporal_time_split(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, bool border_inc, TimestampTz **bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_duration(duration))
    return NULL;

  Span s;
  temporal_set_tstzspan(temp, &s);
  Datum start_bin, end_bin;
  int64 tunits = interval_units(duration);
  int nbins = span_num_bins(&s, Int64GetDatum(tunits),
    TimestampTzGetDatum(torigin), border_inc, &start_bin, &end_bin);
  return temporal_time_split_intl(temp, DatumGetTimestampTz(start_bin),
    DatumGetTimestampTz(end_bin), tunits, torigin, nbins, bins, count);
}

/*****************************************************************************
 * Value split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Get the bin number in the bin space that contains the value
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[in] type Type of the arguments
 */
static int
bin_position(Datum value, Datum size, Datum origin, MeosType type)
{
  assert(tnumber_basetype(type));
  if (type == T_INT4)
    return (DatumGetInt32(value) - DatumGetInt32(origin)) /
      DatumGetInt32(size);
  else /* type == T_FLOAT8 */
    return (int) floor( (DatumGetFloat8(value) - DatumGetFloat8(origin)) /
      DatumGetFloat8(size) );
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] inst Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TInstant **
tnumberinst_value_split(const TInstant *inst, Datum start_bin, Datum size,
  Datum **bins, int *newcount)
{
  assert(inst); assert(bins); assert(newcount);

  Datum value = tinstant_value_p(inst);
  MeosType basetype = temptype_basetype(inst->temptype);
  TInstant **result = palloc(sizeof(TInstant *));
  Datum *values = palloc(sizeof(Datum));
  result[0] = tinstant_copy(inst);
  values[0] = datum_bin(value, size, start_bin, basetype);
  *bins = values;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TSequence **
tnumberseq_disc_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  MeosType basetype = temptype_basetype(seq->temptype);
  TSequence **result;
  Datum *values, value, bin_value;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    values = palloc(sizeof(Datum));
    result[0] = tsequence_copy(seq);
    value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *ninsts = palloc0(sizeof(int) * count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    value = tinstant_value_p(inst);
    bin_value = datum_bin(value, size, start_bin, basetype);
    int bin_no = bin_position(bin_value, size, start_bin, basetype);
    int inst_no = ninsts[bin_no]++;
    instants[bin_no * seq->count + inst_no] = inst;
  }
  /* Assemble the result for each value bin */
  result = palloc(sizeof(TSequence *) * count);
  values = palloc(sizeof(Datum) * count);
  int nfrags = 0;
  bin_value = start_bin;
  for (int i = 0; i < count; i++)
  {
    if (ninsts[i] > 0)
    {
      result[nfrags] = tsequence_make((TInstant **) &instants[i * seq->count],
        ninsts[i], true, true, DISCRETE, NORMALIZE_NO);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(instants);
  pfree(ninsts);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_step_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);

  MeosType basetype = temptype_basetype(seq->temptype);
  Datum value, bin_value;
  int bin_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_value_p(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
    TInstant *bounds[2];
    bounds[0] = (TInstant *) inst1;
    tofree[nfree++] = bounds[1] = tinstant_make(value, seq->temptype,
      inst2->t);
    result[bin_no * numcols + seq_no] = tsequence_make(bounds, 2,
      lower_inc1, false, STEP, NORMALIZE);
    inst1 = inst2;
  }
  /* Last value if upper inclusive */
  if (seq->period.upper_inc)
  {
    inst1 = TSEQUENCE_INST_N(seq, seq->count - 1);
    value = tinstant_value_p(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tinstant_as_tsequence(inst1, STEP);
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_linear_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  MeosType basetype = temptype_basetype(seq->temptype);
  MeosType spantype = basetype_spantype(basetype);
  Datum value1, bin_value1;
  int bin_no1, seq_no;
  Span segspan;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    bin_value1 = datum_bin(value1, size, start_bin, basetype);
    bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
    seq_no = nseqs[bin_no1]++;
    result[bin_no1 * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_value_p(inst1);
  bin_value1 = datum_bin(value1, size, start_bin, basetype);
  bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
  /* For each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    Datum bin_value2 = datum_bin(value2, size, start_bin, basetype);
    int bin_no2 = bin_position(bin_value2, size, start_bin, basetype);

    /* Set variables depending on whether the segment is constant, increasing,
     * or decreasing */
    Datum min_value, max_value;
    int first_bin, last_bin, first, last;
    bool lower_inc1, upper_inc1; /* Lower/upper bound inclusion of the segment */
    bool lower_inc_def, upper_inc_def; /* Default lower/upper bound inclusion */
    int cmp = datum_cmp(value1, value2, basetype);
    if (cmp <= 0)
    {
      /* Both for constant and increasing segments */
      min_value = value1;
      max_value = value2;
      first_bin = bin_no1;
      last_bin = bin_no2;
      first = 0;
      last = 1;
      lower_inc_def = true;
      upper_inc_def = false;
      lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
      upper_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
    }
    else
    {
      min_value = value2;
      max_value = value1;
      first_bin = bin_no2;
      last_bin = bin_no1;
      first = 1;
      last = 0;
      lower_inc_def = false;
      upper_inc_def = true;
      lower_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
      upper_inc1 = (i == 1) ? seq->period.lower_inc : true;
    }

    /* Split the segment into bins */
    span_set(min_value, max_value, lower_inc1, (cmp != 0) ? upper_inc1 : true,
      basetype, spantype, &segspan);
    TInstant *bounds[2];
    bounds[first] = (cmp <= 0) ? (TInstant *) inst1 : (TInstant *) inst2;
    Datum bin_lower = (cmp <= 0) ? bin_value1 : bin_value2;
    Datum bin_upper = datum_add(bin_lower, size, basetype);
    for (int j = first_bin; j <= last_bin; j++)
    {
      /* Choose between interpolate or take one of the segment ends */
      if (datum_lt(min_value, bin_upper, basetype) &&
          datum_lt(bin_upper, max_value, basetype))
      {
        TimestampTz t1, t2;
        /* We are sure there is a unique turning point */
        tsegment_intersection_value(value1, value2, bin_upper, inst1->temptype,
          inst1->t, inst2->t, &t1, &t2);
        /* To reduce the roundoff errors we take the value projected to the
         * timestamp instead of the bound value */
        Datum projvalue1 = tsegment_value_at_timestamptz(value1, value2,
          inst1->temptype, inst1->t, inst2->t, t1);
        tofree[nfree++] = bounds[last] = 
          tinstant_make(projvalue1, seq->temptype, t1);
      }
      else
        bounds[last] = (cmp <= 0) ? (TInstant *) inst2 : (TInstant *) inst1;
      /* Determine the bounds of the resulting sequence */
      if (j == first_bin || j == last_bin)
      {
        Span binspan;
        span_set(bin_lower, bin_upper, true, false, basetype, spantype,
          &binspan);
        Span inter;
        bool found = inter_span_span(&segspan, &binspan, &inter);
        if (found)
        {
          /* Do nothing for constant segments */
          if (cmp < 0)
          {
            lower_inc1 = inter.lower_inc;
            upper_inc1 = inter.upper_inc;
          }
          else if (cmp > 0)
          {
            lower_inc1 = inter.upper_inc;
            upper_inc1 = inter.lower_inc;
          }
        }
        else
        {
          lower_inc1 = upper_inc1 = false;
        }
      }
      else
      {
        /* Sequence bounds are the bin bounds */
        lower_inc1 = lower_inc_def;
        upper_inc1 = upper_inc_def;
      }
      /* If last bin contains a single instant */
      int nfrags = (bounds[0]->t == bounds[1]->t) ? 1 : 2;
      /* We cannot add to last bin if last instant has exclusive bound */
      if (nfrags == 1 && ! upper_inc1)
        break;
      seq_no = nseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make(bounds, nfrags,
        (nfrags > 1) ? lower_inc1 : true, (nfrags > 1) ? upper_inc1 : true,
        LINEAR, NORMALIZE_NO);
      bounds[first] = bounds[last];
      bin_lower = bin_upper;
      bin_upper = datum_add(bin_upper, size, basetype);
    }
    inst1 = inst2;
    value1 = value2;
    bin_value1 = bin_value2;
    bin_no1 = bin_no2;
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing the fragments
 * @param[out] newcount Number of elements in output arrays
 */
static TSequenceSet **
tnumberseq_cont_value_split(const TSequence *seq, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(interp != DISCRETE);
  MeosType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TSequenceSet **result = palloc(sizeof(TSequenceSet *));
    Datum *values = palloc(sizeof(Datum));
    result[0] = tsequence_as_tsequenceset(seq);
    Datum value = tinstant_value_p(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  if (interp == LINEAR)
    tnumberseq_linear_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  else
    tnumberseq_step_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make(&sequences[seq->count * i],
        nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
    /* Clean up */
    for (int j = 0; j < nseqs[i]; j++)
      pfree(sequences[seq->count * i + j]);
  }
  pfree(sequences);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] ss Temporal value
 * @param[in] start_bin Start value of the first bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Array of start values of the bins containing the
 * fragments
 * @param[out] newcount Number of values in the output arrays
 */
static TSequenceSet **
tnumberseqset_value_split(const TSequenceSet *ss, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(ss); assert(bins); assert(newcount);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_cont_value_split(TSEQUENCESET_SEQ_N(ss, 0), start_bin,
      size, count, bins, newcount);

  /* General case */
  MeosType basetype = temptype_basetype(ss->temptype);
  TSequence **binseqs = palloc(sizeof(TSequence *) * ss->totalcount * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss->flags);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (linear)
      tnumberseq_linear_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
    else
      tnumberseq_step_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
  }
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make(&binseqs[i * ss->totalcount],
        nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
    /* Clean up */
    for (int j = 0; j < nseqs[i]; j++)
      pfree(binseqs[i * ss->totalcount + j]);
  }
  pfree(binseqs);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_tile
 * @brief Split a temporal number into an array of fragments according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Origin of the value bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of start values of the bins containing the fragments
 * @param[out] count Number of values in the output arrays
 */
Temporal **
tnumber_value_split(const Temporal *temp, Datum vsize, Datum vorigin,
  bool border_inc, Datum **bins, int *count)
{
  assert(temp); assert(bins); assert(count);
  assert(tnumber_type(temp->temptype));

  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  Datum start_bin, end_bin, end_bin_border;
  int nbins = span_num_bins(&s, vsize, vorigin, true, &start_bin, &end_bin);

  /* Split the temporal value over every bin, since the kernels place each
   * piece by the position of its bin */
  Temporal **result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = (Temporal **) tnumberinst_value_split((const TInstant *) temp,
        start_bin, vsize, bins, count);
      break;
    case TSEQUENCE:
      result = MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tnumberseq_disc_value_split((const TSequence *) temp,
          start_bin, vsize, nbins, bins, count) :
        (Temporal **) tnumberseq_cont_value_split((const TSequence *) temp,
          start_bin, vsize, nbins, bins, count);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal **) tnumberseqset_value_split(
        (const TSequenceSet *) temp, start_bin, vsize, nbins, bins, count);
  }

  /* A grid not containing the upper border of the values leaves out the bin
   * starting at it, the last one, which holds nothing but that border */
  if (! border_inc && span_num_bins(&s, vsize, vorigin, false, &start_bin,
        &end_bin_border) < nbins && *count > 0 &&
      datum_eq((*bins)[*count - 1], end_bin_border,
        temptype_basetype(temp->temptype)))
  {
    pfree(result[*count - 1]);
    (*count)--;
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a temporal value split according to a base value and possibly
 * a temporal grid
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 */
Temporal **
tnumber_value_time_split(const Temporal *temp, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  bool border_inc, Datum **value_bins, TimestampTz **time_bins, int *count)
{
  assert(temp); assert(count); assert(tnumber_type(temp->temptype));
  MeosType basetype = temptype_basetype(temp->temptype);
  assert(positive_datum(vsize, basetype)); assert(positive_duration(duration));

  Datum start_bin, end_bin, start_time_bin, end_time_bin;
  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  int value_count = span_num_bins(&s, vsize, vorigin, border_inc, &start_bin,
    &end_bin);
  /* Compute the time bounds */
  temporal_set_tstzspan(temp, &s);
  int64 tunits = interval_units(duration);
  int time_count = span_num_bins(&s, Int64GetDatum(tunits),
    TimestampTzGetDatum(torigin), border_inc, &start_time_bin, &end_time_bin);
  TimestampTz start_time = DatumGetTimestampTz(start_time_bin);
  TimestampTz end_time = DatumGetTimestampTz(end_time_bin);
  /* Total number of tiles */
  int ntiles = value_count * time_count;

  /* Split the temporal value */
  Datum *v_bins = NULL;
  TimestampTz *t_bins = NULL;
  Temporal **fragments;
  v_bins = palloc(sizeof(Datum) * ntiles);
  t_bins = palloc(sizeof(TimestampTz) * ntiles);
  fragments = palloc(sizeof(Temporal *) * ntiles);
  int nfrags = 0;
  Datum lower_value = start_bin;
  MeosType spantype = basetype_spantype(basetype);
  while (datum_lt(lower_value, end_bin, basetype))
  {
    Datum upper_value = datum_add(lower_value, vsize, basetype);
    span_set(lower_value, upper_value, true, false, basetype, spantype, &s);
    Temporal *atspan = tnumber_restrict_span(temp, &s, REST_AT);
    if (atspan != NULL)
    {
      int num_time_splits;
      TimestampTz *times;
      Temporal **time_splits = temporal_time_split_intl(atspan, start_time,
        end_time, tunits, torigin, time_count, &times, &num_time_splits);
      for (int i = 0; i < num_time_splits; i++)
      {
        v_bins[i + nfrags] = lower_value;
        t_bins[i + nfrags] = times[i];
        fragments[i + nfrags] = time_splits[i];
      }
      nfrags += num_time_splits;
      pfree(time_splits);
      pfree(times);
      pfree(atspan);
    }
    lower_value = upper_value;
  }
  *count = nfrags;
  if (value_bins)
    *value_bins = v_bins;
  else
    pfree(v_bins);
  if (time_bins)
    *time_bins = t_bins;
  else
    pfree(t_bins);
  return fragments;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of bins @nullable
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tint_value_split(const Temporal *temp, int vsize, int vorigin, bool border_inc,
  int **bins,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(bins, NULL);
  if (! ensure_positive(vsize))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Int32GetDatum(vsize),
    Int32GetDatum(vorigin), border_inc, &datum_bins, count);
  /* Transform the datum bins into integer bins and return */
  int *values = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal big integer split according to
 * value bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tbigint_value_split(const Temporal *temp, int64 vsize, int64 vorigin,
  bool border_inc, int64 **bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(bins, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Int64GetDatum(vsize),
    Int64GetDatum(vorigin), border_inc, &datum_bins, count);
  /* Transform the datum bins into big integer bins and return */
  int64 *values = palloc(sizeof(int64) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt64(datum_bins[i]);
  *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal float split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] bins Array of bins @nullable
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tfloat_value_split(const Temporal *temp, double vsize, double vorigin,
  bool border_inc, double **bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(bins, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin) ||
      ! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), border_inc, &datum_bins, count);
  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tint_value_time_split(const Temporal *temp, int vsize, const Interval *duration,
  int vorigin, TimestampTz torigin, bool border_inc, int **value_bins,
  TimestampTz **time_bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive(vsize) || ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Int32GetDatum(vsize),
    duration, Int32GetDatum(vorigin), torigin, border_inc, &datum_bins,
    time_bins,
    count);

  /* Transform the datum bins into integer bins and return */
  int *values = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal big integer split according to
 * value and time bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tbigint_value_time_split(const Temporal *temp, int64 vsize,
  const Interval *duration, int64 vorigin, TimestampTz torigin,
  bool border_inc, int64 **value_bins, TimestampTz **time_bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_positive_datum(Int64GetDatum(vsize), T_INT8) ||
      ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Int64GetDatum(vsize),
    duration, Int64GetDatum(vorigin), torigin, border_inc, &datum_bins,
    time_bins,
    count);

  /* Transform the datum bins into big integer bins and return */
  int64 *values = palloc(sizeof(int64) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt64(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Value origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] value_bins Array of value bins @nullable
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tfloat_value_time_split(const Temporal *temp, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin,
  bool border_inc, double **value_bins, TimestampTz **time_bins, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  if (! ensure_not_nan(vsize) || ! ensure_not_nan(vorigin) ||
      ! ensure_positive_datum(Float8GetDatum(vsize), T_FLOAT8) ||
      ! ensure_positive_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Float8GetDatum(vsize),
    duration, Float8GetDatum(vorigin), torigin, border_inc, &datum_bins,
    time_bins,
    count);

  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/*****************************************************************************/

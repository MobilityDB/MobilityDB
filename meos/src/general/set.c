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
 * @brief General functions for set values composed of an ordered list of
 * distinct values.
 */

#include "general/set.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the n-th value of an ordered set
 * @pre The argument @p index is less than or equal to the number of values
 * in the ordered set
 */
Datum
orderedset_val_n(const OrderedSet *os, int index)
{
  return os->elems[index];
}

/**
 * @brief Return the location of a value in an ordered set using binary search.
 *
 * If the value is found, the index of the value is returned
 * in the output parameter. Otherwise, return a number encoding whether it
 * is before, between two values, or after the ordered set.
 * For example, given an ordered set composed of 3 values and a parameter
 * value, the result of the function is as follows:
 * @code
 *            0       1        2
 *            |       |        |
 * 1)    d^                            => loc = 0
 * 2)        d^                        => loc = 0
 * 3)            d^                    => loc = 1
 * 4)                    d^            => loc = 2
 * 5)                            d^    => loc = 3
 * @endcode
 *
 * @param[in] os Ordered set
 * @param[in] d Value
 * @param[out] loc Location
 * @result Return true if the value is contained in the set
 */
bool
orderedset_find_value(const OrderedSet *os, Datum d, int *loc)
{
  int first = 0;
  int last = os->count - 1;
  int middle = 0; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    Datum d1 = orderedset_val_n(os, middle);
    int cmp = datum_cmp(d, d1, os->span.basetype);
    if (cmp == 0)
    {
      *loc = middle;
      return true;
    }
    if (cmp < 0)
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (middle == os->count)
    middle++;
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Input/output functions in string format
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an ordered set from its Well-Known Text (WKT) representation.
 */
OrderedSet *
orderedset_in(const char *str, mobdbType ostype)
{
  return orderedset_parse(&str, ostype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an ordered set from its Well-Known Text (WKT) representation.
 */
OrderedSet *
intset_in(const char *str)
{
  return orderedset_parse(&str, T_INT4);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an ordered set from its Well-Known Text (WKT) representation.
 */
OrderedSet *
bigintset_in(const char *str)
{
  return orderedset_parse(&str, T_INT8);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an ordered set from its Well-Known Text (WKT) representation.
 */
OrderedSet *
floatset_in(const char *str)
{
  return orderedset_parse(&str, T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an ordered set from its Well-Known Text (WKT) representation.
 */
TimestampSet *
timestampset_in(const char *str)
{
  return orderedset_parse(&str, T_TIMESTAMPSET);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of an ordered set.
 */
char *
orderedset_out(const OrderedSet *os, int maxdd)
{
  char **strings = palloc(sizeof(char *) * os->count);
  size_t outlen = 0;
  for (int i = 0; i < os->count; i++)
  {
    Datum d = orderedset_val_n(os, i);
    strings[i] = basetype_output(d, os->span.basetype, maxdd);
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, os->count, outlen, "", '{', '}');
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct an ordered set from an array of values.
 *
 * For example, the memory structure of an ordered set with 3 values is as
 * follows
 * @code
 * -------------------------------------------------------------
 * ( OrderedSet )_X | ( bbox )_X | Value_0 | Value_1 | Value_2 |
 * -------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, and bbox is the
 * bounding box which is a span.
 *
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] basetype Base type
 * @sqlfunc intset(), bigintset(), floatset(), timestampset()
 * @pymeosfunc TimestampSet()
 */
OrderedSet *
orderedset_make(const Datum *values, int count, mobdbType basetype)
{
  /* Test the validity of the values */
  for (int i = 0; i < count - 1; i++)
  {
    if (datum_ge(values[i], values[i + 1], basetype))
      elog(ERROR, "Invalid value for ordered set");
  }
  /* Notice that the first value is already declared in the struct */
  size_t memsize = double_pad(sizeof(OrderedSet)) +
    sizeof(Datum) * (count - 1);
  /* Create the OrderedSet */
  OrderedSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = count;

  /* Compute the bounding period */
  span_set(values[0], values[count - 1], true, true, basetype, &result->span);
  /* Copy the value array */
  for (int i = 0; i < count; i++)
    result->elems[i] = values[i];
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct an ordered set from the array of values and free the
 * array after the creation.
 *
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[in] basetype Base type
 */
OrderedSet *
orderedset_make_free(Datum *values, int count, mobdbType basetype)
{
  if (count == 0)
  {
    pfree(values);
    return NULL;
  }
  OrderedSet *result = orderedset_make(values, count, basetype);
  pfree(values);
  return result;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Return a copy of an ordered set.
 */
TimestampSet *
orderedset_copy(const TimestampSet *ts)
{
  TimestampSet *result = palloc(VARSIZE(ts));
  memcpy(result, ts, VARSIZE(ts));
  return result;
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_cast
 * @brief Cast a value as an ordered set
 * @sqlop @p ::
 */
OrderedSet *
value_to_orderedset(Datum d, mobdbType basetype)
{
  OrderedSet *result = orderedset_make(&d, 1, basetype);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as an ordered set
 * @sqlop @p ::
 */
OrderedSet *
int_to_intset(int i)
{
  Datum v = Int32GetDatum(i);
  OrderedSet *result = orderedset_make(&v, 1, T_INT4);
  return result;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as an ordered set
 * @sqlop @p ::
 */
OrderedSet *
bigint_to_bigintset(int64 i)
{
  Datum v = Int64GetDatum(i);
  OrderedSet *result = orderedset_make(&v, 1, T_INT8);
  return result;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as an ordered set
 * @sqlop @p ::
 */
OrderedSet *
float_to_floatset(double d)
{
  Datum v = Float8GetDatum(d);
  OrderedSet *result = orderedset_make(&v, 1, T_FLOAT8);
  return result;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as an ordered set
 * @sqlop @p ::
 */
TimestampSet *
timestamp_to_timestampset(TimestampTz t)
{
  Datum v = TimestampTzGetDatum(t);
  TimestampSet *result = orderedset_make(&v, 1, T_TIMESTAMPTZ);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the timespan of an ordered set.
 * @sqlfunc timespan()
 * @pymeosfunc timespan()
 */
Interval *
timestampset_timespan(const TimestampSet *ts)
{
  TimestampTz start = DatumGetTimestampTz(orderedset_val_n(ts, 0));
  TimestampTz end = DatumGetTimestampTz(orderedset_val_n(ts, ts->count - 1));
  Interval *result = pg_timestamp_mi(end, start);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding span of an ordered set.
 * @sqlfunc span()
 * @sqlop @p ::
 * @pymeosfunc span()
 */
Span *
intset_to_intspan(const OrderedSet *os)
{
  Span *result = palloc(sizeof(Span));
  memcpy(result, &os->span, sizeof(Span));
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding span of an ordered set.
 * @sqlfunc span()
 * @sqlop @p ::
 * @pymeosfunc span()
 */
Span *
bigintset_to_bigintspan(const OrderedSet *os)
{
  Span *result = palloc(sizeof(Span));
  memcpy(result, &os->span, sizeof(Span));
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding span of an ordered set.
 * @sqlfunc span()
 * @sqlop @p ::
 * @pymeosfunc span()
 */
Span *
floatset_to_floatspan(const OrderedSet *os)
{
  Span *result = palloc(sizeof(Span));
  memcpy(result, &os->span, sizeof(Span));
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding period of an ordered set.
 * @sqlfunc period()
 * @sqlop @p ::
 * @pymeosfunc period()
 */
Period *
timestampset_to_period(const TimestampSet *ts)
{
  Period *result = palloc(sizeof(Period));
  memcpy(result, &ts->span, sizeof(Period));
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the timespan of an ordered set.
 * @sqlfunc timespan()
 * @pymeosfunc timespan()
 */
Interval *
timestampset_to_timespan(const TimestampSet *ts)
{
  TimestampTz start = DatumGetTimestampTz(orderedset_val_n(ts, 0));
  TimestampTz end = DatumGetTimestampTz(orderedset_val_n(ts, ts->count - 1));
  Interval *result = pg_timestamp_mi(end, start);
  return result;
}


/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the size in bytes of an ordered set.
 * @sqlfunc memSize()
 */
int
orderedset_mem_size(const OrderedSet *os)
{
  return (int) VARSIZE(DatumGetPointer(os));
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of values of an ordered set.
 * @sqlfunc numTimestamps()
 * @pymeosfunc numTimestamps()
 */
int
orderedset_num_values(const OrderedSet *os)
{
  return os->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of an ordered set.
 * @sqlfunc startTimestamp()
 * @pymeosfunc startTimestamp()
 */
Datum
orderedset_start_value(const OrderedSet *os)
{
  Datum result = orderedset_val_n(os, 0);
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of an ordered set.
 * @sqlfunc endTimestamp()
 * @pymeosfunc endTimestamp()
 */
Datum
orderedset_end_value(const OrderedSet *os)
{
  Datum result = orderedset_val_n(os, os->count - 1);
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of an ordered set.
 *
 * @param[in] os Ordered set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN(), timestampN()
 * @pymeosfunc timestampN()
 */
bool
orderedset_value_n(const OrderedSet *os, int n, Datum *result)
{
  if (n < 1 || n > os->count)
    return false;
  *result = orderedset_val_n(os, n - 1);
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of an ordered set.
 * @sqlfunc values(), timestamps()
 * @pymeosfunc timestamps()
 */
Datum *
orderedset_values(const OrderedSet *os)
{
  Datum *result = palloc(sizeof(Datum) * os->count);
  for (int i = 0; i < os->count; i++)
    result[i] = orderedset_val_n(os, i);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of values of an integer set.
 * @sqlfunc numValues()
 * @pymeosfunc numValues()
 */
int
intset_num_values(const OrderedSet *os)
{
  return os->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of values of a big integer set.
 * @sqlfunc numValues()
 * @pymeosfunc numValues()
 */
int
bigintset_num_values(const OrderedSet *os)
{
  return os->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of values of a float set.
 * @sqlfunc numValues()
 * @pymeosfunc numValues()
 */
int
floatset_num_values(const OrderedSet *os)
{
  return os->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of values of an ordered set.
 * @sqlfunc numTimestamps()
 * @pymeosfunc numTimestamps()
 */
int
timestampset_num_timestamps(const TimestampSet *ts)
{
  return ts->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of an integer set.
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
int
intset_start_value(const OrderedSet *os)
{
  int result = DatumGetInt32(orderedset_val_n(os, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of a big integer set.
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
int64
bigintset_start_value(const OrderedSet *os)
{
  int64 result = DatumGetInt64(orderedset_val_n(os, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of a float set.
 * @sqlfunc startValue()
 * @pymeosfunc startValue()
 */
double
floatset_start_value(const OrderedSet *os)
{
  double result = DatumGetFloat8(orderedset_val_n(os, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start value of an ordered set.
 * @sqlfunc startTimestamp()
 * @pymeosfunc startTimestamp()
 */
TimestampTz
timestampset_start_timestamp(const TimestampSet *ts)
{
  TimestampTz result = DatumGetTimestampTz(orderedset_val_n(ts, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of an integer set.
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
int
intset_end_value(const OrderedSet *os)
{
  int result = DatumGetInt32(orderedset_val_n(os, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of a big integer set.
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
int64
bigintset_end_value(const OrderedSet *os)
{
  int64 result = DatumGetInt64(orderedset_val_n(os, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of a float set.
 * @sqlfunc endValue()
 * @pymeosfunc endValue()
 */
double
floatset_end_value(const OrderedSet *os)
{
  double result = DatumGetFloat8(orderedset_val_n(os, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end value of an ordered set.
 * @sqlfunc endTimestamp()
 * @pymeosfunc endTimestamp()
 */
TimestampTz
timestampset_end_timestamp(const TimestampSet *ts)
{
  TimestampTz result = DatumGetTimestampTz(orderedset_val_n(ts, ts->count - 1));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of an integer set.
 *
 * @param[in] os Integer set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN()
 * @pymeosfunc valueN()
 */
bool
intset_value_n(const OrderedSet *os, int n, int *result)
{
  if (n < 1 || n > os->count)
    return false;
  *result = DatumGetInt32(orderedset_val_n(os, n - 1));
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of a big integer set.
 *
 * @param[in] os Integer set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN()
 * @pymeosfunc valueN()
 */
bool
bigintset_value_n(const OrderedSet *os, int n, int64 *result)
{
  if (n < 1 || n > os->count)
    return false;
  *result = DatumGetInt64(orderedset_val_n(os, n - 1));
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of a float set.
 *
 * @param[in] os Float set
 * @param[in] n Number
 * @param[out] result Value
 * @result Return true if the value is found
 * @note It is assumed that n is 1-based
 * @sqlfunc valueN()
 * @pymeosfunc valueN()
 */
bool
floatset_value_n(const OrderedSet *os, int n, double *result)
{
  if (n < 1 || n > os->count)
    return false;
  *result = DatumGetFloat8(orderedset_val_n(os, n - 1));
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th value of an ordered set.
 *
 * @param[in] ts Timestamp set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the timestamp is found
 * @note It is assumed that n is 1-based
 * @sqlfunc timestampN()
 * @pymeosfunc timestampN()
 */
bool
timestampset_timestamp_n(const TimestampSet *ts, int n, TimestampTz *result)
{
  if (n < 1 || n > ts->count)
    return false;
  *result = DatumGetTimestampTz(orderedset_val_n(ts, n - 1));
  return true;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of an integer set.
 * @sqlfunc values()
 * @pymeosfunc values()
 */
int *
intset_values(const OrderedSet *os)
{
  int *result = palloc(sizeof(int) * os->count);
  for (int i = 0; i < os->count; i++)
    result[i] = DatumGetInt32(orderedset_val_n(os, i));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of a big integer set.
 * @sqlfunc values()
 * @pymeosfunc values()
 */
int64 *
bigintset_values(const OrderedSet *os)
{
  int64 *result = palloc(sizeof(int64) * os->count);
  for (int i = 0; i < os->count; i++)
    result[i] = DatumGetInt64(orderedset_val_n(os, i));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of values of a float set.
 * @sqlfunc values()
 * @pymeosfunc values()
 */
double *
floatset_values(const OrderedSet *os)
{
  double *result = palloc(sizeof(double) * os->count);
  for (int i = 0; i < os->count; i++)
    result[i] = DatumGetFloat8(orderedset_val_n(os, i));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the array of timestamps of an ordered set.
 * @sqlfunc timestamps()
 * @pymeosfunc timestamps()
 */
TimestampTz *
timestampset_timestamps(const TimestampSet *ts)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * ts->count);
  for (int i = 0; i < ts->count; i++)
    result[i] = DatumGetTimestampTz(orderedset_val_n(ts, i));
  return result;
}


/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a timestamp set shifted and/or scaled by the intervals
 * @sqlfunc shift(), tscale(), shiftTscale()
 * @pymeosfunc shift()
 */
TimestampSet *
timestampset_shift_tscale(const TimestampSet *ts, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  TimestampSet *result = orderedset_copy(ts);

  /* Shift and/or scale the bounding period */
  period_shift_tscale(shift, duration, &result->span);

  /* Set the first instant */
  result->elems[0] = result->span.lower;
  if (ts->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    TimestampTz delta;
    if (shift != NULL)
      delta = result->span.lower - ts->span.lower;
    double scale;
    if (duration != NULL)
      scale = (double) (result->span.upper - result->span.lower) /
        (double) (ts->span.upper - ts->span.lower);
    for (int i = 1; i < ts->count - 1; i++)
    {
      if (shift != NULL)
        result->elems[i] += delta;
      if (duration != NULL)
        result->elems[i] = result->span.lower +
          (result->elems[i] - result->span.lower) * scale;
    }
    /* Set the last instant */
    result->elems[ts->count - 1] = result->span.upper;
  }
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
orderedset_eq(const OrderedSet *os1, const OrderedSet *os2)
{
  assert(os1->span.basetype == os2->span.basetype);
  if (os1->count != os2->count)
    return false;
  /* os1 and os2 have the same number of values */
  for (int i = 0; i < os1->count; i++)
  {
    Datum v1 = orderedset_val_n(os1, i);
    Datum v2 = orderedset_val_n(os2, i);
    if (datum_ne(v1, v2, os1->span.basetype))
      return false;
  }
  /* All values of the two ordered sets are equal */
  return true;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is different from the
 * second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p <>
 */
bool
orderedset_ne(const OrderedSet *os1, const OrderedSet *os2)
{
  return ! orderedset_eq(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first ordered set
 * value is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc orderedset_cmp()
 */
int
orderedset_cmp(const OrderedSet *os1, const OrderedSet *os2)
{
  assert(os1->span.basetype == os2->span.basetype);
  int count = Min(os1->count, os2->count);
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    Datum v1 = orderedset_val_n(os1, i);
    Datum v2 = orderedset_val_n(os2, i);
    result = datum_cmp(v1, v2, os1->span.basetype);
    if (result)
      break;
  }
  /* The first count times of the two OrderedSet are equal */
  if (! result)
  {
    if (count < os1->count) /* os1 has more values than os2 */
      result = 1;
    else if (count < os2->count) /* os2 has more values than os1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than the second one
 * @sqlop @p <
 */
bool
orderedset_lt(const OrderedSet *os1, const OrderedSet *os2)
{
  int cmp = orderedset_cmp(os1, os2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than
 * or equal to the second one
 * @sqlop @p <=
 */
bool
orderedset_le(const OrderedSet *os1, const OrderedSet *os2)
{
  int cmp = orderedset_cmp(os1, os2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than
 * or equal to the second one
 * @sqlop @p >=
 */
bool
orderedset_ge(const OrderedSet *os1, const OrderedSet *os2)
{
  int cmp = orderedset_cmp(os1, os2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than the second one
 * @sqlop @p >
 */
bool
orderedset_gt(const OrderedSet *os1, const OrderedSet *os2)
{
  int cmp = orderedset_cmp(os1, os2);
  return cmp > 0;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
intset_eq(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_eq(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
bigintset_eq(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_eq(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
floatset_eq(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_eq(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
timestampset_eq(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return orderedset_eq(ts1, ts2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is different from the
 * second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p <>
 */
bool
intset_ne(const OrderedSet *os1, const OrderedSet *os2)
{
  return ! orderedset_eq(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is different from the
 * second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p <>
 */
bool
bigintset_ne(const OrderedSet *os1, const OrderedSet *os2)
{
  return ! orderedset_eq(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is different from the
 * second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p <>
 */
bool
floatset_ne(const OrderedSet *os1, const OrderedSet *os2)
{
  return ! orderedset_eq(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is different from the
 * second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p <>
 */
bool
timestampset_ne(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return ! orderedset_eq(ts1, ts2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first ordered set
 * value is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc intset_cmp()
 */
int
intset_cmp(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_cmp(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first ordered set
 * value is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc bigintset_cmp()
 */
int
bigintset_cmp(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_cmp(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first ordered set
 * value is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc floatset_cmp()
 */
int
floatset_cmp(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_cmp(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first ordered set
 * value is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc timestampset_cmp()
 */
int
timestampset_cmp(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return orderedset_cmp(ts1, ts2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than the second one
 * @sqlop @p <
 */
bool
intset_lt(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_lt(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than the second one
 * @sqlop @p <
 */
bool
bigintset_lt(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_lt(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than the second one
 * @sqlop @p <
 */
bool
floatset_lt(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_lt(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than the second one
 * @sqlop @p <
 */
bool
timestampset_lt(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return orderedset_lt(ts1, ts2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than or equal to the
 * second one
 * @sqlop @p <=
 */
bool
intset_le(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_le(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than or equal to the
 * second one
 * @sqlop @p <=
 */
bool
bigintset_le(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_le(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than or equal to the
 * the second one
 * @sqlop @p <=
 */
bool
floatset_le(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_le(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is less than or equal to the
 * second one
 * @sqlop @p <=
 */
bool
timestampset_le(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return orderedset_le(ts1, ts2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than or equal to the
 * second one
 * @sqlop @p >=
 */
bool
intset_ge(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_ge(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than or equal to the
 * second one
 * @sqlop @p >=
 */
bool
bigintset_ge(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_ge(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than or equal to the
 * second one
 * @sqlop @p >=
 */
bool
floatset_ge(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_ge(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than or equal to the
 * second one
 * @sqlop @p >=
 */
bool
timestampset_ge(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return orderedset_ge(ts1, ts2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than the second one
 * @sqlop @p >
 */
bool
intset_gt(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_gt(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than the second one
 * @sqlop @p >
 */
bool
bigintset_gt(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_gt(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than the second one
 * @sqlop @p >
 */
bool
floatset_gt(const OrderedSet *os1, const OrderedSet *os2)
{
  return orderedset_gt(os1, os2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first ordered set is greater than the second one
 * @sqlop @p >
 */
bool
timestampset_gt(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return orderedset_gt(ts1, ts2);
}
#endif /* MEOS */

/*****************************************************************************
 * Functions for defining hash index
 * The function reuses PostgreSQL approach for array types for combining the
 * hash of the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the 32-bit hash value of a datum.
 */
uint32
datum_hash(Datum d, mobdbType basetype)
{
  ensure_set_basetype(basetype);
  switch (basetype)
  {
    case T_TIMESTAMPTZ:
      return pg_hashint8(TimestampTzGetDatum(d));
    case T_INT4:
      return DatumGetInt32(hash_bytes_uint32(d));
    case T_INT8:
      return pg_hashint8(Int64GetDatum(d));
    case T_FLOAT8:
      return pg_hashfloat8(Float8GetDatum(d));
    default: /* Error! */
      elog(ERROR, "Unknown base type: %d", basetype);
      break;
  }
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 32-bit hash value of an ordered set.
 * @sqlfunc timestampset_hash()
 */
uint32
orderedset_hash(const OrderedSet *os)
{
  uint32 result = 1;
  for (int i = 0; i < os->count; i++)
  {
    Datum d = orderedset_val_n(os, i);
    uint32 value_hash = datum_hash(d, os->span.basetype);
    result = (result << 5) - result + value_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the 32-bit hash value of a datum.
 */
uint64
datum_hash_extended(Datum d, mobdbType basetype, uint64 seed)
{
  ensure_set_basetype(basetype);
  switch (basetype)
  {
    case T_TIMESTAMPTZ:
      return pg_hashint8extended(TimestampTzGetDatum(d), seed);
    case T_INT4:
      return hash_bytes_uint32_extended(DatumGetInt32(d), seed);
    case T_INT8:
      return pg_hashint8extended(Int64GetDatum(d), seed);
    case T_FLOAT8:
      return pg_hashfloat8extended(Float8GetDatum(d), seed);
    default: /* Error! */
      elog(ERROR, "Unknown base type: %d", basetype);
      break;
  }
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 64-bit hash value of an ordered set using a seed.
 * @sqlfunc timestampset_hash_extended()
 */
uint64
orderedset_hash_extended(const OrderedSet *os, uint64 seed)
{
  uint64 result = 1;
  for (int i = 0; i < os->count; i++)
  {
    Datum d = orderedset_val_n(os, i);
    uint64 value_hash = datum_hash_extended(d, os->span.basetype, seed);
    result = (result << 5) - result + value_hash;
  }
  return result;
}

/*****************************************************************************/

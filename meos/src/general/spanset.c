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
 * @brief General functions for set of disjoint spans.
 */

#include "general/spanset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the location of a value in a span set using binary search.
 *
 * If the value is found, the index of the span is returned in the output
 * parameter. Otherwise, return a number encoding whether the value is before
 * between two spans, or after the span set.
 * For example, given a value composed of 3 spans and a value v, the
 * result of the function is as follows:
 * @code
 *               0          1          2
 *            |-----|    |-----|    |-----|
 * 1)    v^                                        => loc = 0
 * 2)            v^                                => loc = 0
 * 3)                 v^                           => loc = 1
 * 4)                            v^                => loc = 2
 * 5)                                        v^    => loc = 3
 * @endcode
 * @param[in] ss Span set
 * @param[in] v Value
 * @param[out] loc Location
 * @result Return true if the value is contained in the span set
 */
bool
spanset_find_value(const SpanSet *ss, Datum v, int *loc)
{
  int first = 0;
  int last = ss->count - 1;
  int middle = 0; /* make compiler quiet */
  const Span *s = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    s = spanset_sp_n(ss, middle);
    if (contains_span_value(s, v, s->basetype))
    {
      *loc = middle;
      return true;
    }
    if (datum_le(v, s->lower, s->basetype))
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (datum_ge(v, s->upper, s->basetype))
    middle++;
  *loc = middle;
  return false;
}

#if MEOS
bool
periodset_find_timestamp(const SpanSet *ps, TimestampTz t, int *loc)
{
  return spanset_find_value(ps, TimestampTzGetDatum(t), loc);
}
#endif /* MEOS */

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_inout
 * @brief Return a span set from its Well-Known Text (WKT) representation.
 */
SpanSet *
spanset_in(const char *str, meosType spansettype)
{
  return spanset_parse(&str, spansettype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation.
 */
SpanSet *
intspanset_in(const char *str)
{
  return spanset_parse(&str, T_INTSPANSET);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a big integer span from its Well-Known Text (WKT) representation.
 */
SpanSet *
bigintspanset_in(const char *str)
{
  return spanset_parse(&str, T_BIGINTSPANSET);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation.
 */
SpanSet *
floatspanset_in(const char *str)
{
  return spanset_parse(&str, T_FLOATSPANSET);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a period set from its Well-Known Text (WKT) representation.
 */
SpanSet *
periodset_in(const char *str)
{
  return spanset_parse(&str, T_TSTZSPANSET);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
spanset_out(const SpanSet *ss, int maxdd)
{
  char **strings = palloc(sizeof(char *) * ss->count);
  size_t outlen = 0;

  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = spanset_sp_n(ss, i);
    strings[i] = span_out(s, maxdd);
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, ss->count, outlen, "", '{', '}');
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
intspanset_out(const SpanSet *ss)
{
  return spanset_out(ss, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
bigintspanset_out(const SpanSet *ss)
{
  return spanset_out(ss, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
floatspanset_out(const SpanSet *ss, int maxdd)
{
  return spanset_out(ss, maxdd);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
periodset_out(const SpanSet *ss)
{
  return spanset_out(ss, 0);
}
#endif /* MEOS */

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the n-th span of a span set.
 * @pre The argument @p index is less than the number of spans in the span set
 * @note This is the internal function equivalent to `spanset_span_n`.
 * This function does not verify that the index is is in the correct bounds
 */
const Span *
spanset_sp_n(const SpanSet *ss, int index)
{
  return (Span *) &ss->elems[index];
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Construct a span set from an array of disjoint span.
 *
 * For example, the memory structure of a SpanSet with 3 span is as
 * follows
 * @code
 * ---------------------------------------------------------------------------------
 * ( SpanSet )_X | ( bbox )_X | ( Span_0 )_X | ( Span_1 )_X | ( Span_2 )_X |
 * ---------------------------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, and `bbox` is the
 * bounding box which is also a span.
 *
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized
 * @sqlfunc spanset()
 */
SpanSet *
spanset_make(const Span **spans, int count, bool normalize)
{
  /* Test the validity of the spans */
  for (int i = 0; i < count - 1; i++)
  {
    int cmp = datum_cmp(spans[i]->upper, spans[i + 1]->lower,
      spans[i]->basetype);
    if (cmp > 0 ||
      (cmp == 0 && spans[i]->upper_inc && spans[i + 1]->lower_inc))
      elog(ERROR, "Invalid value for span set");
  }

  Span **newspans = (Span **) spans;
  int newcount = count;
  if (normalize && count > 1)
    newspans = spanarr_normalize((Span **) spans, count, SORT_NO, &newcount);
  /* Notice that the first span is already declared in the struct */
  size_t memsize = double_pad(sizeof(SpanSet)) +
    double_pad(sizeof(Span)) * (newcount - 1);
  SpanSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->spansettype = spantype_spansettype(spans[0]->spantype);
  result->spantype = spans[0]->spantype;
  result->basetype = spans[0]->basetype;
  result->count = newcount;

  /* Compute the bounding span */
  span_set(newspans[0]->lower, newspans[newcount - 1]->upper,
    newspans[0]->lower_inc, newspans[newcount - 1]->upper_inc,
    result->basetype, &result->span);
  /* Copy the span array */
  for (int i = 0; i < newcount; i++)
    memcpy(&result->elems[i], newspans[i], sizeof(Span));
  /* Free after normalization */
  if (normalize && count > 1)
    pfree_array((void **) newspans, newcount);
  return result;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Construct a span set from an array of spans and free the array
 * and the spans after the creation.
 *
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * @see spanset_make
 * @sqlfunc spanset()
 */
SpanSet *
spanset_make_free(Span **spans, int count, bool normalize)
{
  if (count == 0)
  {
    pfree(spans);
    return NULL;
  }
  SpanSet *result = spanset_make((const Span **) spans, count, normalize);
  pfree_array((void **) spans, count);
  return result;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Return a copy of a span set.
 */
SpanSet *
spanset_copy(const SpanSet *ps)
{
  SpanSet *result = palloc(VARSIZE(ps));
  memcpy(result, ps, VARSIZE(ps));
  return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_cast
 * @brief Cast an element as a span set
 */
SpanSet *
value_to_spanset(Datum d, meosType basetype)
{
  ensure_span_basetype(basetype);
  Span s;
  Span *s1 = &s;
  span_set(d, d, true, true, basetype, &s);
  SpanSet *result = spanset_make((const Span **) &s1, 1, NORMALIZE_NO);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast an element as a span set
 * @sqlop @p ::
 */
SpanSet *
int_to_intspanset(int i)
{
  return value_to_spanset(i, T_INT4);
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast an element as a span set
 * @sqlop @p ::
 */
SpanSet *
bigint_to_bigintspanset(int i)
{
  return value_to_spanset(i, T_INT8);
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast an element as a span set
 * @sqlop @p ::
 */
SpanSet *
float_to_floaspanset(double d)
{
  return value_to_spanset(d, T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a timestamp as a period set
 * @sqlop @p ::
 */
SpanSet *
timestamp_to_periodset(TimestampTz t)
{
  return value_to_spanset(t, T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a set as a span set.
 * @sqlop @p ::
 */
SpanSet *
set_to_spanset(const Set *s)
{
  Span **spans = palloc(sizeof(Span *) * s->count);
  for (int i = 0; i < s->count; i++)
  {
    Datum d = set_val_n(s, i);
    spans[i] = span_make(d, d, true, true, s->basetype);
  }
  SpanSet *result = spanset_make_free(spans, s->count, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a period as a period set.
 * @sqlop @p ::
 */
SpanSet *
span_to_spanset(const Span *s)
{
  return spanset_make((const Span **) &s, 1, NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_cast
 * @brief Return the bounding span of a span set.
 * @sqlfunc intspan(), floatspan(), period()
 * @sqlop @p ::
 * @pymeosfunc period()
 */
Span *
spanset_to_span(const SpanSet *ss)
{
  Span *result = palloc(sizeof(Span));
  memcpy(result, &ss->span, sizeof(Span));
  return result;
}
#endif /* MEOS */

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Shift a span set by a value.
 * @pre The value is of the same type as the span base type
 * @sqlfunc shift()
 * @pymeosfunc shift()
 */
void
spanset_shift(SpanSet *ss, Datum shift)
{
  for (int i = 0; i < ss->count; i++)
  {
    Span *s = (Span *) spanset_sp_n(ss, i);
    span_shift(s, shift);
  }
  return;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a period set shifted and/or scaled by the intervals.
 * @sqlfunc shift(), tscale(), shiftTscale()
 * @pymeosfunc shift()
 */
SpanSet *
periodset_shift_tscale(const SpanSet *ps, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  bool instant = (ps->span.lower == ps->span.upper);

  /* Copy the input period set to the output period set */
  SpanSet *result = spanset_copy(ps);
  /* Shift and/or scale the bounding period */
  period_shift_tscale(&result->span, shift, duration);
  /* Shift and/or scale the periods of the period set */
  TimestampTz delta;
  if (shift != NULL)
    delta = result->span.lower - ps->span.lower;
  /* If the periodset is instantaneous we cannot scale */
  double scale;
  if (duration != NULL && ! instant)
    scale = (double) (result->span.upper - result->span.lower) /
      (double) (ps->span.upper - ps->span.lower);
  for (int i = 0; i < ps->count; i++)
  {
    if (shift != NULL)
    {
      result->elems[i].lower += delta;
      result->elems[i].upper += delta;
    }
    if (duration != NULL && ! instant)
    {
      result->elems[i].lower = result->span.lower +
        (result->elems[i].lower - result->span.lower) * scale;
      result->elems[i].upper = result->span.lower +
        (result->elems[i].upper - result->span.lower) * scale;
    }
  }
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the size in bytes of a period set
 * @sqlfunc memorySize()
 */
int
spanset_memory_size(const SpanSet *ss)
{
  return (int) VARSIZE(DatumGetPointer(ss));
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @sqlfunc lower()
 */
int
intspanset_lower(const SpanSet *ss)
{
  return DatumGetInt32(ss->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @sqlfunc lower()
 */
int
bigintspanset_lower(const SpanSet *ss)
{
  return DatumGetInt64(ss->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a float span set
 * @sqlfunc lower()
 */
double
floatspanset_lower(const SpanSet *ss)
{
  return Float8GetDatum(ss->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a period
 * @sqlfunc lower()
 * @pymeosfunc lower()
 */
TimestampTz
periodset_lower(const SpanSet *ps)
{
  return TimestampTzGetDatum(ps->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @sqlfunc upper()
 */
int
intspanset_upper(const SpanSet *ss)
{
  return Int32GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @sqlfunc upper()
 */
int
bigintspanset_upper(const SpanSet *ss)
{
  return Int64GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a float span set
 * @sqlfunc upper()
 */
double
floatspanset_upper(const SpanSet *ss)
{
  return Float8GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a period
 * @sqlfunc upper()
 * @pymeosfunc upper()
 */
TimestampTz
periodset_upper(const SpanSet *ps)
{
  return TimestampTzGetDatum(ps->elems[ps->count - 1].upper);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the lower bound of a span set is inclusive
 * @sqlfunc lower_inc()
 * @pymeosfunc lower_inc()
 */
bool
spanset_lower_inc(const SpanSet *ss)
{
  return ss->elems[0].lower_inc;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the upper bound of a span set is inclusive
 * @sqlfunc upper_inc()
 * @pymeosfunc upper_inc()
 */
bool
spanset_upper_inc(const SpanSet *ss)
{
  return ss->elems[ss->count - 1].upper_inc;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the width of a span set as a double.
 * @sqlfunc width()
 */
double
spanset_width(const SpanSet *ss)
{
  double result = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = spanset_sp_n(ss, i);
    result += span_width(s);
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the timespan of a period set
 * @sqlfunc timespan()
 * @pymeosfunc timespan()
 */
Interval *
periodset_timespan(const SpanSet *ps)
{
  const Span *p1 = spanset_sp_n(ps, 0);
  const Span *p2 = spanset_sp_n(ps, ps->count - 1);
  Interval *result = pg_timestamp_mi(p2->upper, p1->lower);
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the duration of a period set
 * @sqlfunc duration()
 * @pymeosfunc duration()
 */
Interval *
periodset_duration(const SpanSet *ps)
{
  const Span *p = spanset_sp_n(ps, 0);
  Interval *result = pg_timestamp_mi(p->upper, p->lower);
  for (int i = 1; i < ps->count; i++)
  {
    p = spanset_sp_n(ps, i);
    Interval *interval1 = pg_timestamp_mi(p->upper, p->lower);
    Interval *interval2 = pg_interval_pl(result, interval1);
    pfree(result); pfree(interval1);
    result = interval2;
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of timestamps of a period set
 * @sqlfunc numTimestamps()
 * @pymeosfunc numTimestamps()
 */
int
periodset_num_timestamps(const SpanSet *ps)
{
  const Span *p = spanset_sp_n(ps, 0);
  TimestampTz prev = p->lower;
  bool start = false;
  int result = 1;
  TimestampTz d;
  int i = 1;
  while (i < ps->count || !start)
  {
    if (start)
    {
      p = spanset_sp_n(ps, i++);
      d = p->lower;
      start = !start;
    }
    else
    {
      d = p->upper;
      start = !start;
    }
    if (prev != d)
    {
      result++;
      prev = d;
    }
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start timestamp of a period set.
 * @sqlfunc startTimestamp()
 * @pymeosfunc startTimestamp()
 */
TimestampTz
periodset_start_timestamp(const SpanSet *ps)
{
  const Span *p = spanset_sp_n(ps, 0);
  return p->lower;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end timestamp of a period set.
 * @sqlfunc endTimestamp()
 * @pymeosfunc endTimestamp()
 */
TimestampTz
periodset_end_timestamp(const SpanSet *ps)
{
  const Span *p = spanset_sp_n(ps, ps->count - 1);
  return p->upper;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th timestamp of a period set.
 *
 * @param[in] ps Period set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the timestamp is found
 * @note It is assumed that n is 1-based
 * @sqlfunc timestampN()
 * @pymeosfunc timestampN()
 */
bool
periodset_timestamp_n(const SpanSet *ps, int n, TimestampTz *result)
{
  int pernum = 0;
  const Span *p = spanset_sp_n(ps, pernum);
  TimestampTz d = p->lower;
  if (n == 1)
  {
    *result = d;
    return true;
  }

  bool start = false;
  int i = 1;
  TimestampTz prev = d;
  while (i < n)
  {
    if (start)
    {
      pernum++;
      if (pernum == ps->count)
        break;

      p = spanset_sp_n(ps, pernum);
      d = p->lower;
      start = !start;
    }
    else
    {
      d = p->upper;
      start = !start;
    }
    if (prev != d)
    {
      i++;
      prev = d;
    }
  }
  if (i != n)
    return false;
  *result = d;
  return true;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the timestamps of a period set
 * @sqlfunc timestamps()
 * @pymeosfunc timestamps()
 */
TimestampTz *
periodset_timestamps(const SpanSet *ps, int *count)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * 2 * ps->count);
  const Span *p = spanset_sp_n(ps, 0);
  result[0] = p->lower;
  int k = 1;
  if (p->lower != p->upper)
    result[k++] = p->upper;
  for (int i = 1; i < ps->count; i++)
  {
    p = spanset_sp_n(ps, i);
    if (result[k - 1] != (TimestampTz) p->lower)
      result[k++] = p->lower;
    if (result[k - 1] != (TimestampTz) p->upper)
      result[k++] = p->upper;
  }
  *count = k;
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of spans of a span set
 * @sqlfunc numSpans()
 * @pymeosfunc numSpans()
 */
int
spanset_num_spans(const SpanSet *ss)
{
  return ss->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start span of a span set
 * @sqlfunc startSpan()
 * @pymeosfunc startSpan()
 */
Span *
spanset_start_span(const SpanSet *ss)
{
  Span *result = span_copy(spanset_sp_n(ss, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end span of a span set
 * @sqlfunc endSpan()
 * @pymeosfunc endSpan()
 */
Span *
spanset_end_span(const SpanSet *ss)
{
  Span *result = span_copy(spanset_sp_n(ss, ss->count - 1));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th span of a span set
 * @sqlfunc spanN()
 * @pymeosfunc spanN()
 */
Span *
spanset_span_n(const SpanSet *ss, int i)
{
  Span *result = NULL;
  if (i >= 1 && i <= ss->count)
    result = span_copy(spanset_sp_n(ss, i - 1));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the spans of a span set.
 * @post The output parameter @p count is equal to the number of spans of
 * the input span set
 * @sqlfunc spans()
 * @pymeosfunc spans()
 */
const Span **
spanset_spans(const SpanSet *ss, int *count)
{
  const Span **spans = palloc(sizeof(Span *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    spans[i] = spanset_sp_n(ss, i);
  *count = ss->count;
  return spans;
}

/*****************************************************************************
 * B-tree support
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
spanset_eq(const SpanSet *ss1, const SpanSet *ss2)
{
  assert(ss1->spantype == ss2->spantype);
  if (ss1->count != ss2->count)
    return false;
  /* ss1 and ss2 have the same number of SpanSet */
  for (int i = 0; i < ss1->count; i++)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, i);
    if (span_ne(s1, s2))
      return false;
  }
  /* All spans of the two span sets are equal */
  return true;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span set is different from the
 * second one.
 * @sqlop @p <>
 */
bool
spanset_ne(const SpanSet *ss1, const SpanSet *ss2)
{
  return ! spanset_eq(ss1, ss2);
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span set
 * is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc spanset_cmp()
 */
int
spanset_cmp(const SpanSet *ss1, const SpanSet *ss2)
{
  assert(ss1->spantype == ss2->spantype);
  int count1 = ss1->count;
  int count2 = ss2->count;
  int count = count1 < count2 ? count1 : count2;
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, i);
    result = span_cmp(s1, s2);
    if (result)
      break;
  }
  /* The first count spans of the two SpanSet are equal */
  if (! result)
  {
    if (count < count1) /* ss1 has more SpanSet than ss2 */
      result = 1;
    else if (count < count2) /* ss2 has more SpanSet than ss1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span set is less than the second one
 * @sqlop @p <
 */
bool
spanset_lt(const SpanSet *ss1, const SpanSet *ss2)
{
  return spanset_cmp(ss1, ss2) < 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span set is less than or equal to
 * the second one
 * @sqlop @p <=
 */
bool
spanset_le(const SpanSet *ss1, const SpanSet *ss2)
{
  return spanset_cmp(ss1, ss2) <= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span set is greater than or equal to
 * the second one
 * @sqlop @p >=
 */
bool
spanset_ge(const SpanSet *ss1, const SpanSet *ss2)
{
  return spanset_cmp(ss1, ss2) >= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span set is greater than the second one
 * @sqlop @p >
 */
bool
spanset_gt(const SpanSet *ss1, const SpanSet *ss2)
{
  return spanset_cmp(ss1, ss2) > 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 32-bit hash value of a span set.
 * @sqlfunc spanset_hash()
 */
uint32
spanset_hash(const SpanSet *ps)
{
  uint32 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    uint32 per_hash = span_hash(p);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 64-bit hash value of a span set using a seed
 * @sqlfunc spanset_hash_extended()
 */
uint64
spanset_hash_extended(const SpanSet *ps, uint64 seed)
{
  uint64 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    uint64 per_hash = span_hash_extended(p, seed);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/*****************************************************************************/

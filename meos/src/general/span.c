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
 * @brief General functions for spans (a.k.a. ranges) composed of two `Datum`
 * values and two `Boolean` values stating whether the bounds are inclusive.
 */

#include "general/span.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/meos_catalog.h"
#include "general/pg_types.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_parser.h"
#include "general/type_util.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Deconstruct a span
 * @param[in] s Span value
 * @param[out] lower,upper Bounds
 */
void
span_deserialize(const Span *s, SpanBound *lower, SpanBound *upper)
{
  if (lower)
  {
    lower->val = s->lower;
    lower->inclusive = s->lower_inc;
    lower->lower = true;
    lower->spantype = s->spantype;
    lower->basetype = s->basetype;
  }
  if (upper)
  {
    upper->val = s->upper;
    upper->inclusive = s->upper_inc;
    upper->lower = false;
    upper->spantype = s->spantype;
    upper->basetype = s->basetype;
  }
}

/*****************************************************************************/

/**
 * @brief Compare two span boundary points, returning <0, 0, or >0 according to
 * whether the first one is less than, equal to, or greater than the second one.
 *
 * The boundaries can be any combination of upper and lower; so it's useful
 * for a variety of operators.
 *
 * The simple case is when b1 and b2 are both inclusive, in which
 * case the result is just a comparison of the values held in b1 and b2.
 *
 * If a bound is exclusive, then we need to know whether it's a lower bound,
 * in which case we treat the boundary point as "just greater than" the held
 * value; or an upper bound, in which case we treat the boundary point as
 * "just less than" the held value.
 *
 * There is only one case where two boundaries compare equal but are not
 * identical: when both bounds are inclusive and hold the same value,
 * but one is an upper bound and the other a lower bound.
 */
int
span_bound_cmp(const SpanBound *b1, const SpanBound *b2)
{
  /* Compare the values */
  int32 result = datum_cmp2(b1->val, b2->val, b1->basetype, b2->basetype);

  /*
   * If the comparison is not equal and the bounds are both inclusive or
   * both exclusive, we're done. If they compare equal, we still have to
   * consider whether the boundaries are inclusive or exclusive.
  */
  if (result == 0)
  {
    if (! b1->inclusive && ! b2->inclusive)
    {
      /* both are exclusive */
      if (b1->lower == b2->lower)
        return 0;
      else
        return b1->lower ? 1 : -1;
    }
    else if (! b1->inclusive)
      return b1->lower ? 1 : -1;
    else if (! b2->inclusive)
      return b2->lower ? -1 : 1;
  }

  return result;
}

/**
 * @brief Comparison function for sorting span bounds.
 */
int
span_bound_qsort_cmp(const void *a1, const void *a2)
{
  SpanBound *b1 = (SpanBound *) a1;
  SpanBound *b2 = (SpanBound *) a2;
  return span_bound_cmp(b1, b2);
}

/**
 * @brief Compare the lower bounds of two spans, returning <0, 0, or >0 according to
 * whether the first bound is less than, equal to, or greater than the second one.
 *
 * @note The function is equivalent to `span_bound_cmp` but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_lower_cmp(const Span *a, const Span *b)
{
  int result = datum_cmp2(a->lower, b->lower, a->basetype, b->basetype);
  if (result == 0)
  {
    if (a->lower_inc == b->lower_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (a->lower_inc)
      /* first is inclusive and second is exclusive */
      return 1;
    else
      /* first is exclusive and second is inclusive */
      return -1;
  }
  return result;
}

/**
 * @brief Compare the upper bounds of two spans, returning <0, 0, or >0 according to
 * whether the first bound is less than, equal to, or greater than the second one.
 *
 * @note The function is equivalent to `span_bound_cmp` but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_upper_cmp(const Span *a, const Span *b)
{
  int result = datum_cmp2(a->upper, b->upper, a->basetype, b->basetype);
  if (result == 0)
  {
    if (a->upper_inc == b->upper_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (a->upper_inc)
      /* first is inclusive and second is exclusive */
      return 1;
    else
      /* first is exclusive and second is inclusive */
      return -1;
  }
  return result;
}

/**
 * @brief Normalize an array of spans
 *
 * The input spans may overlap and may be non contiguous.
 * The normalized spans are new spans that must be freed.
 *
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the input array
 * @param[in] sort True if the spans should be sorted before normalization
 * @param[out] newcount Number of elements in the output array
 * @pre i
 */
Span *
spanarr_normalize(Span *spans, int count, bool sort, int *newcount)
{
  /* Sort the spans before normalization */
  if (sort)
    spanarr_sort(spans, count);
  int nspans = 0;
  Span *result = palloc(sizeof(Span) * count);
  Span current = spans[0];
  for (int i = 1; i < count; i++)
  {
    Span next = spans[i];
    if (overlaps_span_span(&current, &next) ||
        adjacent_span_span(&current, &next))
      /* Compute the union of the spans */
      span_expand(&next, &current);
    else
    {
      result[nspans++] = current;
      current = next;
    }
  }
  result[nspans++] = current;
  /* Set the output parameter */
  *newcount = nspans;
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_inout
 * @brief Return a span from its Well-Known Text (WKT) representation.
 */
Span *
span_in(const char *str, meosType spantype)
{
  Span *result = palloc(sizeof(Span));
  span_parse(&str, spantype, true, result);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation.
 */
Span *
intspan_in(const char *str)
{
  return span_in(str, T_INTSPAN);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation.
 */
Span *
bigintspan_in(const char *str)
{
  return span_in(str, T_BIGINTSPAN);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation.
 */
Span *
floatspan_in(const char *str)
{
  return span_in(str, T_FLOATSPAN);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a period from its Well-Known Text (WKT) representation.
 */
Span *
period_in(const char *str)
{
  return span_in(str, T_TSTZSPAN);
}
#endif /* MEOS */

/**
 * @brief Remove the quotes from the Well-Known Text (WKT) representation of a span
 */
static char *
unquote(char *str)
{
  /* Save the initial pointer */
  char *result = str;
  char *last = str;
  while (*str != '\0')
  {
    if (*str != '"')
    {
      *last++ = *str;
    }
    str++;
  }
  *last = '\0';
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 */
char *
span_out(const Span *s, int maxdd)
{
  char *lower = unquote(basetype_out(s->lower, s->basetype, maxdd));
  char *upper = unquote(basetype_out(s->upper, s->basetype, maxdd));
  char open = s->lower_inc ? (char) '[' : (char) '(';
  char close = s->upper_inc ? (char) ']' : (char) ')';
  char *result = palloc(strlen(lower) + strlen(upper) + 5);
  sprintf(result, "%c%s, %s%c", open, lower, upper, close);
  pfree(lower); pfree(upper);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 */
char *
floatspan_out(const Span *s, int maxdd)
{
  return span_out(s, maxdd);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 */
char *
intspan_out(const Span *s)
{
  return span_out(s, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 */
char *
bigintspan_out(const Span *s)
{
  return span_out(s, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 */
char *
period_out(const Span *s)
{
  return span_out(s, 0);
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct a span from the bounds.
 */
Span *
span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  meosType basetype)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(lower, upper, lower_inc, upper_inc, basetype, s);
  return s;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_constructor
 * @brief Construct an integer span from the bounds.
 * @sqlfunc intspan()
 */
Span *
intspan_make(int lower, int upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Int32GetDatum(lower), Int32GetDatum(upper), lower_inc, upper_inc,
    T_INT4, s);
  return s;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Construct a big integer span from the bounds.
 * @sqlfunc bigintspan()
 */
Span *
bigintspan_make(int64 lower, int64 upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Int64GetDatum(lower), Int64GetDatum(upper), lower_inc, upper_inc,
    T_INT8, s);
  return s;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Construct a float span from the bounds.
 * @sqlfunc floatspan()
 */
Span *
floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Float8GetDatum(lower), Float8GetDatum(upper), lower_inc, upper_inc,
    T_FLOAT8, s);
  return s;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Construct a timestamp with time zone span from the bounds.
 * @sqlfunc period()
 */
Span *
period_make(TimestampTz lower, TimestampTz upper, bool lower_inc,
  bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(TimestampTzGetDatum(lower), TimestampTzGetDatum(upper), lower_inc,
    upper_inc, T_TIMESTAMPTZ, s);
  return s;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Set a span from the bounds.
 * @see span_make()
 */
void
span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  meosType basetype, Span *s)
{
  /* Canonicalize */
  if (span_canon_basetype(basetype))
  {
    if (basetype == T_INT4)
    {
      if (! lower_inc)
      {
        lower = Int32GetDatum(DatumGetInt32(lower) + 1);
        lower_inc = true;
      }
      if (upper_inc)
      {
        upper = Int32GetDatum(DatumGetInt32(upper) + 1);
        upper_inc = false;
      }
    }
    else /* basetype == T_INT8 */
    {
      if (! lower_inc)
      {
        lower = Int64GetDatum(DatumGetInt64(lower) + 1);
        lower_inc = true;
      }
      if (upper_inc)
      {
        upper = Int64GetDatum(DatumGetInt64(upper) + 1);
        upper_inc = false;
      }
    }
  }

  meosType spantype = basetype_spantype(basetype);
  int cmp = datum_cmp2(lower, upper, basetype, basetype);
  /* error check: if lower bound value is above upper, it's wrong */
  if (cmp > 0)
    elog(ERROR, "Span lower bound must be less than or equal to span upper bound");

  /* error check: if bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && !(lower_inc && upper_inc))
    elog(ERROR, "Span cannot be empty");

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(s, 0, sizeof(Span));
  /* Fill in the span */
  s->lower = lower;
  s->upper = upper;
  s->lower_inc = lower_inc;
  s->upper_inc = upper_inc;
  s->spantype = spantype;
  s->basetype = basetype;
  return;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Return a copy of a span.
 */
Span *
span_copy(const Span *s)
{
  Span *result = palloc(sizeof(Span));
  memcpy((char *) result, (char *) s, sizeof(Span));
  return result;
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Cast a value as a span
 */
void
value_set_span(Datum d, meosType basetype, Span *s)
{
  assert(span_basetype(basetype));
  span_set(d, d, true, true, basetype, s);
  return;
}

/**
 * @ingroup libmeos_internal_setspan_cast
 * @brief Cast a value as a span
 */
Span *
value_to_span(Datum d, meosType basetype)
{
  Span *result = palloc(sizeof(Span));
  value_set_span(d, basetype, result);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as a span
 * @sqlop @p ::
 */
Span *
int_to_intspan(int i)
{
  Span *result = span_make(Int32GetDatum(i), Int32GetDatum(i), true, true,
    T_INT4);
  return result;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as a span
 * @sqlop @p ::
 */
Span *
bigint_to_bigintspan(int i)
{
  Span *result = span_make(Int64GetDatum(i), Int64GetDatum(i), true, true,
    T_INT8);
  return result;
}

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a value as a span
 * @sqlop @p ::
 */
Span *
float_to_floatspan(double d)
{
  Span *result = span_make(Float8GetDatum(d), Float8GetDatum(d), true, true,
    T_FLOAT8);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_cast
 * @brief Cast a timestamp as a period
 * @sqlop @p ::
 */
Span *
timestamp_to_period(TimestampTz t)
{
  Span *result = span_make(TimestampTzGetDatum(t), TimestampTzGetDatum(t),
    true, true, T_TIMESTAMPTZ);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @sqlfunc lower()
 */
int
intspan_lower(const Span *s)
{
  return DatumGetInt32(s->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @sqlfunc lower()
 */
int
bigintspan_lower(const Span *s)
{
  return DatumGetInt64(s->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a float span
 * @sqlfunc lower()
 */
double
floatspan_lower(const Span *s)
{
  return Float8GetDatum(s->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a period
 * @sqlfunc lower()
 * @pymeosfunc lower()
 */
TimestampTz
period_lower(const Span *p)
{
  return TimestampTzGetDatum(p->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @sqlfunc upper()
 */
int
intspan_upper(const Span *s)
{
  return Int32GetDatum(s->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @sqlfunc upper()
 */
int
bigintspan_upper(const Span *s)
{
  return Int64GetDatum(s->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a float span
 * @sqlfunc upper()
 */
double
floatspan_upper(const Span *s)
{
  return Float8GetDatum(s->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a period
 * @sqlfunc upper()
 * @pymeosfunc upper()
 */
TimestampTz
period_upper(const Span *p)
{
  return TimestampTzGetDatum(p->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the lower bound of a span is inclusive
 * @sqlfunc lower_inc()
 * @pymeosfunc lower_inc()
 */
bool
span_lower_inc(const Span *s)
{
  return s->lower_inc;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the upper bound of a span is inclusive
 * @sqlfunc upper_inc()
 * @pymeosfunc upper_inc()
 */
bool
span_upper_inc(const Span *s)
{
  return s->upper_inc;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the width of a span as a double.
 * @sqlfunc width()
 */
double
span_width(const Span *s)
{
  return distance_value_value(s->lower, s->upper, s->basetype, s->basetype);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the duration of a period as an interval.
 * @sqlfunc duration()
 * @pymeosfunc duration()
 */
Interval *
period_duration(const Span *s)
{
  return pg_timestamp_mi(s->upper, s->lower);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Set the second span with the first one transformed to floatspan
 * @note This currently works only for intspan <-> floatspan
 */
void
numspan_set_floatspan(const Span *s1, Span *s2)
{
  memset(s2, 0, sizeof(Span));
  if (s1->basetype == T_INT4)
    intspan_set_floatspan(s1, s2);
  else /* s1->basetype == T_FLOAT8 */
    memcpy(s2, s1, sizeof(Span));
  return;
}

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Set the second span with the first one transformed to intspan
 * @note This currently works only for intspan <-> floatspan
 */
void
floatspan_set_numspan(const Span *s1, Span *s2, meosType basetype)
{
  memset(s2, 0, sizeof(Span));
  if (basetype == T_INT4)
    floatspan_set_intspan(s1, s2);
  else /* basetype == T_FLOAT8 */
    memcpy(s2, s1, sizeof(Span));
  return;
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_setspan_transf
 * @brief Transform an integer span to a float span
 */
Span *
intspan_floatspan(const Span *s)
{
  Span *result = malloc(sizeof(Span));
  intspan_set_floatspan(s, result);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Transform a float span to an integer span
 */
Span *
floatspan_intspan(const Span *s)
{
  Span *result = malloc(sizeof(Span));
  floatspan_set_intspan(s, result);
  return result;
}
#endif /* MEOS */


/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Set the second span with the first one transformed to a float span
 */
void
intspan_set_floatspan(const Span *s1, Span *s2)
{
  memset(s2, 0, sizeof(Span));
  Datum lower = Float8GetDatum((double) DatumGetInt32(s1->lower));
  Datum upper = Float8GetDatum((double) (DatumGetInt32(s1->upper) - 1));
  span_set(lower, upper, true, true, T_FLOAT8, s2);
  return;
}

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Set the second span with the first one transformed to a integer span
 */
void
floatspan_set_intspan(const Span *s1, Span *s2)
{
  memset(s2, 0, sizeof(Span));
  Datum lower = Int32GetDatum((int) DatumGetFloat8(s1->lower));
  Datum upper = Int32GetDatum((int) (DatumGetFloat8(s1->upper)) + 1);
  span_set(lower, upper, true, false, T_INT4, s2);
  return;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Expand the second span with the first one
 */
void
span_expand(const Span *s1, Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp1 = datum_cmp(s2->lower, s1->lower, s1->basetype);
  int cmp2 = datum_cmp(s2->upper, s1->upper, s1->basetype);
  bool lower1 = cmp1 < 0 || (cmp1 == 0 && (s2->lower_inc || ! s1->lower_inc));
  bool upper1 = cmp2 > 0 || (cmp2 == 0 && (s2->upper_inc || ! s1->upper_inc));
  s2->lower = lower1 ? s2->lower : s1->lower;
  s2->lower_inc = lower1 ? s2->lower_inc : s1->lower_inc;
  s2->upper = upper1 ? s2->upper : s1->upper;
  s2->upper_inc = upper1 ? s2->upper_inc : s1->upper_inc;
  return;
}

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Shift a span by a value.
 * @pre The value is of the same type as the span base type
 * @sqlfunc shift()
 * @pymeosfunc shift()
 */
void
span_shift(Span *s, Datum shift)
{
  s->lower = datum_add(s->lower, shift, s->basetype, s->basetype);
  s->upper = datum_add(s->upper, shift, s->basetype, s->basetype);
  return;
}

/**
 * @brief Shift and/or scale period bounds by the intervals.
 * @param[in] shift Interval to shift the bounds
 * @param[in] duration Interval for the duration of the result
 * @param[in,out] lower,upper Bounds of the period
 */
void
lower_upper_shift_tscale(const Interval *shift, const Interval *duration,
  TimestampTz *lower, TimestampTz *upper)
{
  assert(shift != NULL || duration != NULL);
  assert(lower != NULL && upper != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);

  bool instant = (*lower == *upper);
  if (shift != NULL)
  {
    *lower = pg_timestamp_pl_interval(*lower, shift);
    if (instant)
      *upper = *lower;
    else
      *upper = pg_timestamp_pl_interval(*upper, shift);
  }
  if (duration != NULL && ! instant)
    *upper = pg_timestamp_pl_interval(*lower, duration);
  return;
}


/**
 * @brief Shift and/or scale a period by a delta and a scale
 */
void
period_delta_scale(Span *p, TimestampTz origin, TimestampTz delta, double scale)
{
  TimestampTz lower = DatumGetTimestampTz(p->lower);
  TimestampTz upper = DatumGetTimestampTz(p->upper);
  /* The default value when there is not shift is 0 */
  if (delta != 0)
  {
    p->lower = TimestampTzGetDatum(lower + delta);
    p->upper = TimestampTzGetDatum(upper + delta);
  }
  /* Shifted lower and upper */
  lower = DatumGetTimestampTz(p->lower);
  upper = DatumGetTimestampTz(p->upper);
  /* The default value when there is not scale is 1.0 */
  if (scale != 1.0)
  {
    /* The potential shift has been already taken care in the previous if */
    p->lower = TimestampTzGetDatum(
      origin + (TimestampTz) ((lower - origin) * scale));
    if (lower == upper)
      p->upper = p->lower;
    else
      p->upper = TimestampTzGetDatum(
        origin + (TimestampTz) ((upper - origin) * scale));
  }
}

/**
 * @brief Shift and/or scale a period by the intervals.
 * @note Returns the delta and scale of the transformation
 */
void
period_shift_tscale1(Span *p, const Interval *shift, const Interval *duration,
  TimestampTz *delta, double *scale)
{
  TimestampTz lower = DatumGetTimestampTz(p->lower);
  TimestampTz upper = DatumGetTimestampTz(p->upper);
  lower_upper_shift_tscale(shift, duration, &lower, &upper);
  /* Compute delta and scale before overwriting p->lower and p->upper */
  // *delta = 0;   /* Default value when shift == NULL */
  // *scale = 1.0; /* Default value when duration == NULL */
  if (delta != NULL && shift != NULL)
    *delta = lower - DatumGetTimestampTz(p->lower);
  /* If the period is instantaneous we cannot scale */
  if (scale != NULL && duration != NULL && p->lower != p->upper)
    *scale = (double) (upper - lower) /
      (double) (DatumGetTimestampTz(p->upper) - DatumGetTimestampTz(p->lower));
  p->lower = TimestampTzGetDatum(lower);
  p->upper = TimestampTzGetDatum(upper);
  return;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Shift and/or scale a period by the intervals.
 * @sqlfunc shift(), tscale(), shiftTscale()
 * @pymeosfunc shiftTscale()
 */
Span *
period_shift_tscale(const Span *p, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);

  /* Copy the input period to the result */
  Span *result = span_copy(p);
  /* Shift and/or scale the resulting period */
  TimestampTz lower = DatumGetTimestampTz(p->lower);
  TimestampTz upper = DatumGetTimestampTz(p->upper);
  lower_upper_shift_tscale(shift, duration, &lower, &upper);
  result->lower = TimestampTzGetDatum(lower);
  result->upper = TimestampTzGetDatum(upper);
  return result;
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
span_eq(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  if (s1->lower != s2->lower || s1->upper != s2->upper ||
    s1->lower_inc != s2->lower_inc || s1->upper_inc != s2->upper_inc)
    return false;
  return true;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span is different from the second one.
 * @sqlop @p <>
 */
bool
span_ne(const Span *s1, const Span *s2)
{
  return (! span_eq(s1, s2));
}

/* B-tree comparator */

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span is less than,
 * equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc intspan_cmp(), bigintspan_cmp(), floatspan_cmp(), period_cmp()
 * @pymeosfunc _cmp()
 */
int
span_cmp(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp2(s1->lower, s2->lower, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->lower_inc != s2->lower_inc)
    return s1->lower_inc ? -1 : 1;
  cmp = datum_cmp2(s1->upper, s2->upper, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->upper_inc != s2->upper_inc)
    return s1->upper_inc ? 1 : -1;
  return 0;
}

/* Inequality operators using the span_cmp function */

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span is less than the second one.
 * @sqlop @p <
 * @pymeosfunc __lt__()
 */
bool
span_lt(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) < 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span is less than or equal to the
 * second one.
 * @sqlop @p <=
 * @pymeosfunc __le__()
 */
bool
span_le(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) <= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span is greater than or equal to the
 * second one.
 * @sqlop @p >=
 * @pymeosfunc __ge__()
 */
bool
span_ge(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) >= 0;
}

/**
 * @ingroup libmeos_setspan_comp
 * @brief Return true if the first span is greater than the second one.
 * @sqlop @p >
 * @pymeosfunc __gt__()
 */
bool
span_gt(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) > 0;
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 32-bit hash of a span.
 * @sqlfunc intspan_hash(), bigintspan_hash(), floatspan_hash(), period_hash()
 */
uint32
span_hash(const Span *s)
{
  /* Create flags from the lower_inc and upper_inc values */
  char flags = '\0';
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Create type from the spantype and basetype values */
  uint16 type = ((uint16) (s->spantype) << 8) | (uint16) (s->basetype);
  uint32 type_hash = hash_bytes_uint32((int32) type);

  /* Apply the hash function to each bound */
  uint32 lower_hash = datum_hash(s->lower, s->basetype);
  uint32 upper_hash = datum_hash(s->upper, s->basetype);

  /* Merge hashes of flags, type, and bounds */
  uint32 result = hash_bytes_uint32((uint32) flags);
  result ^= type_hash;
  result = (result << 1) | (result >> 31);
  result ^= lower_hash;
  result = (result << 1) | (result >> 31);
  result ^= upper_hash;

  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 64-bit hash of a span using a seed
 * @sqlfunc hash_extended()
 */
uint64
span_hash_extended(const Span *s, uint64 seed)
{
  uint64 result;
  char flags = '\0';
  uint64 type_hash;
  uint64 lower_hash;
  uint64 upper_hash;

  /* Create flags from the lower_inc and upper_inc values */
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Create type from the spantype and basetype values */
  uint16 type = ((uint16) (s->spantype) << 8) | (uint16) (s->basetype);
  type_hash = DatumGetUInt64(hash_uint32_extended(type, seed));

  /* Apply the hash function to each bound */
  lower_hash = pg_hashint8extended(s->lower, seed);
  upper_hash = pg_hashint8extended(s->upper, seed);

  /* Merge hashes of flags and bounds */
  result = DatumGetUInt64(hash_uint32_extended((uint32) flags,
    DatumGetInt64(seed)));
  result ^= type_hash;
  result = ROTATE_HIGH_AND_LOW_32BITS(result);
  result ^= lower_hash;
  result = ROTATE_HIGH_AND_LOW_32BITS(result);
  result ^= upper_hash;

  return result;
}

/******************************************************************************/

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
#include <float.h>
#include <limits.h>
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
 * @brief Ensure that a span value is of a span type
 */
bool
ensure_span_has_type(const Span *s, meosType spantype)
{
  if (s->spantype != spantype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The span value must be of type %s", meostype_name(spantype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the span values have the same type
 */
bool
ensure_same_span_type(const Span *s1, const Span *s2)
{
  if (s1->spantype != s2->spantype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed span types: %s and %s",
      meostype_name(s1->spantype), meostype_name(s2->spantype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a span value has the same base type as the given one
 */
bool
ensure_same_span_basetype(const Span *s, meosType basetype)
{
  if (s->basetype != basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed span and base types: %s and %s",
      meostype_name(s->spantype), meostype_name(basetype));
    return false;
  }
  return true;
}

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
  assert(b1->basetype == b2->basetype);
  /* Compare the values */
  int32 result = datum_cmp(b1->val, b2->val, b1->basetype);

  /*
   * If the comparison is not equal and the bounds are both inclusive or
   * both exclusive, we're done. If they compare equal, we still have to
   * consider whether the boundaries are inclusive or exclusive.
  */
  if (result == 0)
  {
    if (! b1->inclusive && ! b2->inclusive)
    {
      /* both bounds are exclusive */
      if (b1->lower == b2->lower)
        /* both are lower bound */
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
 * @note The function is equivalent to #span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_lower_cmp(const Span *s1, const Span *s2)
{
  assert(s1->basetype == s2->basetype);
  int result = datum_cmp(s1->lower, s2->lower, s1->basetype);
  if (result == 0)
  {
    if (s1->lower_inc == s2->lower_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (s1->lower_inc)
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
 * @note The function is equivalent to #span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_upper_cmp(const Span *s1, const Span *s2)
{
  assert(s1->basetype == s2->basetype);
  int result = datum_cmp(s1->upper, s2->upper, s1->basetype);
  if (result == 0)
  {
    if (s1->upper_inc == s2->upper_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (s1->upper_inc)
      /* first is inclusive and second is exclusive */
      return 1;
    else
      /* first is exclusive and second is inclusive */
      return -1;
  }
  return result;
}

/**
 * @brief Return the inclusive upper bound accounting for canonicalized spans.
 */
Datum
span_canon_upper(const Span *s)
{
  Datum result;
  if (s->basetype == T_INT4)
    result = Int32GetDatum(DatumGetInt32(s->upper) - (int32) 1);
  else if (s->basetype == T_INT8)
    result = Int64GetDatum(DatumGetInt64(s->upper) - (int64) 1);
  else
    result = s->upper;
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
  assert(spans); assert(newcount);
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
  assert(str);
  Span *result = palloc(sizeof(Span));
  if (! span_parse(&str, spantype, true, result))
    return NULL;
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation.
 * @return On error return NULL
 */
Span *
intspan_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return span_in(str, T_INTSPAN);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation.
 * @return On error return NULL
 */
Span *
bigintspan_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return span_in(str, T_BIGINTSPAN);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation.
 * @return On error return NULL
 */
Span *
floatspan_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return span_in(str, T_FLOATSPAN);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a period from its Well-Known Text (WKT) representation.
 * @return On error return NULL
 */
Span *
tstzspan_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
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
  assert(s);
  /* Ensure validity of the arguments */
  if (! ensure_not_negative(maxdd))
    return NULL;

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
 * @return On error return NULL
 */
char *
intspan_out(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_INTSPAN))
    return NULL;
  return span_out(s, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 * @return On error return NULL
 */
char *
bigintspan_out(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_BIGINTSPAN))
    return NULL;
  return span_out(s, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 * @return On error return NULL
 */
char *
floatspan_out(const Span *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_FLOATSPAN))
    return NULL;
  return span_out(s, maxdd);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span.
 * @return On error return NULL
 */
char *
tstzspan_out(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_TSTZSPAN))
    return NULL;
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
 * @sqlfunc tstzspan()
 */
Span *
tstzspan_make(TimestampTz lower, TimestampTz upper, bool lower_inc,
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
  assert(s);
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
  int cmp = datum_cmp(lower, upper, basetype);
  /* error check: if lower bound value is above upper, it's wrong */
  if (cmp > 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Span lower bound must be less than or equal to span upper bound");
    return;
  }

  /* error check: if bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && !(lower_inc && upper_inc))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Span cannot be empty");
    return;
  }

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  Span *result = palloc(sizeof(Span));
  memcpy((char *) result, (char *) s, sizeof(Span));
  return result;
}

/*****************************************************************************
 * Conversion
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Convert a value as a span
 */
void
value_set_span(Datum d, meosType basetype, Span *s)
{
  assert(s);
  assert(span_basetype(basetype));
  span_set(d, d, true, true, basetype, s);
  return;
}

/**
 * @ingroup libmeos_internal_setspan_conversion
 * @brief Convert a value as a span
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
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a value as a span
 * @sqlop @p ::
 */
Span *
int_to_span(int i)
{
  Span *result = span_make(Int32GetDatum(i), Int32GetDatum(i), true, true,
    T_INT4);
  return result;
}

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a value as a span
 * @sqlop @p ::
 */
Span *
bigint_to_span(int i)
{
  Span *result = span_make(Int64GetDatum(i), Int64GetDatum(i), true, true,
    T_INT8);
  return result;
}

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a value as a span
 * @sqlop @p ::
 */
Span *
float_to_span(double d)
{
  Span *result = span_make(Float8GetDatum(d), Float8GetDatum(d), true, true,
    T_FLOAT8);
  return result;
}

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a timestamptz as a span
 * @sqlop @p ::
 */
Span *
timestamptz_to_span(TimestampTz t)
{
  Span *result = span_make(TimestampTzGetDatum(t), TimestampTzGetDatum(t),
    true, true, T_TIMESTAMPTZ);
  return result;
}
#endif /* MEOS */

#if POSTGRESQL_VERSION_NUMBER >= 130000
/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a date to a span
 * @sqlop @p ::
 */
Span *
date_to_tstzspan(DateADT d)
{
  int overflow;
  TimestampTz t = date2timestamptz_opt_overflow(d, &overflow);
  if (overflow != 0)
    return NULL;
  Span *result = span_make(TimestampTzGetDatum(t), TimestampTzGetDatum(t),
    true, true, T_TIMESTAMPTZ);
  return result;
}
#endif 

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @return On error return INT_MAX
 * @sqlfunc lower()
 */
int
intspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_INTSPAN))
    return INT_MAX;
  return DatumGetInt32(s->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @return On error return INT_MAX
 * @sqlfunc lower()
 */
int
bigintspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_BIGINTSPAN))
    return INT_MAX;
  return DatumGetInt64(s->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a float span
 * @return On error return DBL_MAX
 * @sqlfunc lower()
 */
double
floatspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_FLOATSPAN))
    return DBL_MAX;
  return DatumGetFloat8(s->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a period
 * @return On error return DT_NOEND
 * @sqlfunc lower()
 */
TimestampTz
tstzspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_TSTZSPAN))
    return DT_NOEND;
  return TimestampTzGetDatum(s->lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @return On error return INT_MAX
 * @sqlfunc upper()
 */
int
intspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_INTSPAN))
    return INT_MAX;
  return Int32GetDatum(s->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @return On error return INT_MAX
 * @sqlfunc upper()
 */
int
bigintspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_BIGINTSPAN))
    return INT_MAX;
  return Int64GetDatum(s->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a float span
 * @return On error return DBL_MAX
 * @sqlfunc upper()
 */
double
floatspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_FLOATSPAN))
    return DBL_MAX;
  return DatumGetFloat8(s->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a period
 * @return On error return DT_NOEND
 * @sqlfunc upper()
 */
TimestampTz
tstzspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_TSTZSPAN))
    return DT_NOEND;
  return TimestampTzGetDatum(s->upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the lower bound of a span is inclusive
 * @sqlfunc lower_inc()
 */
bool
span_lower_inc(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return false;
  return s->lower_inc;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the upper bound of a span is inclusive
 * @sqlfunc upper_inc()
 */
bool
span_upper_inc(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return false;
  return s->upper_inc;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the width of a span as a double.
 * @return On error return -1.0
 * @sqlfunc width()
 */
double
span_width(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return -1.0;
  return distance_value_value(s->lower, s->upper, s->basetype);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the duration of a period as an interval.
 * @sqlfunc duration()
 */
Interval *
tstzspan_duration(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_TSTZSPAN))
    return NULL;
  return pg_timestamp_mi(s->upper, s->lower);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Set the precision of the float span to the number of decimal places.
 */
void
floatspan_round_int(const Span *s, Datum size, Span *result)
{
  assert(s); assert(result);
  /* Set precision of bounds */
  Datum lower = datum_round_float(s->lower, size);
  Datum upper = datum_round_float(s->upper, size);
  /* Set resulting span */
  span_set(lower, upper, s->lower_inc, s->upper_inc, s->basetype, result);
  return;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Set the precision of the float span to the number of decimal places.
 * @return On error return NULL
 */
Span *
floatspan_round(const Span *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_negative(maxdd) ||
      ! ensure_span_has_type(s, T_FLOATSPAN))
    return NULL;

  Span *result = palloc(sizeof(Span));
  floatspan_round_int(s, Int32GetDatum(maxdd), result);
  return result;
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_setspan_transf
 * @brief Transform an integer span to a float span
 * @return On error return NULL
 */
Span *
intspan_floatspan(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_INTSPAN))
    return NULL;
  Span *result = malloc(sizeof(Span));
  intspan_set_floatspan(s, result);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Transform a float span to an integer span
 * @return On error return NULL
 */
Span *
floatspan_intspan(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_FLOATSPAN))
    return NULL;
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
  assert(s1); assert(s2);
  assert(s1->spantype == T_INTSPAN);
  memset(s2, 0, sizeof(Span));
  Datum lower = Float8GetDatum((double) DatumGetInt32(s1->lower));
  Datum upper = Float8GetDatum((double) (DatumGetInt32(s1->upper) - 1));
  span_set(lower, upper, true, true, T_FLOAT8, s2);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Set the second span with the first one transformed to a integer span
 */
void
floatspan_set_intspan(const Span *s1, Span *s2)
{
  assert(s1); assert(s2);
  assert(s1->spantype == T_FLOATSPAN);
  memset(s2, 0, sizeof(Span));
  Datum lower = Int32GetDatum((int) DatumGetFloat8(s1->lower));
  Datum upper = Int32GetDatum((int) (DatumGetFloat8(s1->upper)) + 1);
  span_set(lower, upper, true, false, T_INT4, s2);
  return;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Expand the second span with the first one
 */
void
span_expand(const Span *s1, Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);

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
 */
void
span_shift(Span *s, Datum shift)
{
  assert(s);
  s->lower = datum_add(s->lower, shift, s->basetype);
  s->upper = datum_add(s->upper, shift, s->basetype);
  return;
}

/**
 * @brief Shift and/or scale the span bounds by the values.
 * @param[in] shift Value to shift the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @param[in] type Type of the values
 * @param[in,out] lower,upper Bounds of the period
 */
void
lower_upper_shift_scale_value(Datum shift, Datum width, meosType type,
  bool hasshift, bool haswidth, Datum *lower, Datum *upper)
{
  assert(hasshift || haswidth);
  assert(lower && upper);
  assert(! haswidth || positive_datum(width, type));

  bool instant = datum_eq(*lower, *upper, type);
  if (hasshift)
  {
    *lower = datum_add(*lower, shift, type);
    if (instant)
      *upper = *lower;
    else
      *upper = datum_add(*upper, shift, type);
  }
  if (haswidth && ! instant)
  {
    /* Integer spans have exclusive upper bound */
    if (span_canon_basetype(type))
      width = datum_add(width, 1, type);
    *upper = datum_add(*lower, width, type);
  }
  return;
}

/**
 * @brief Shift and/or scale period bounds by the intervals.
 * @param[in] shift Interval to shift the bounds
 * @param[in] duration Interval for the duration of the result
 * @param[in,out] lower,upper Bounds of the period
 */
void
lower_upper_shift_scale_time(const Interval *shift, const Interval *duration,
  TimestampTz *lower, TimestampTz *upper)
{
  assert(shift || duration);
  assert(lower && upper);
  assert(! duration || valid_duration(duration));

  bool instant = (*lower == *upper);
  if (shift)
  {
    *lower = pg_timestamp_pl_interval(*lower, shift);
    if (instant)
      *upper = *lower;
    else
      *upper = pg_timestamp_pl_interval(*upper, shift);
  }
  if (duration && ! instant)
    *upper = pg_timestamp_pl_interval(*lower, duration);
  return;
}

/**
 * @brief Shift and/or scale a span by a delta and a scale
 */
void
numspan_delta_scale_iter(Span *s, Datum origin, Datum delta, bool hasdelta,
  double scale)
{
  assert(s);

  Datum lower = s->lower;
  Datum upper = s->upper;
  meosType type = s->basetype;
  /* The default value when shift is not given is 0 */
  if (hasdelta)
  {
    s->lower = datum_add(s->lower, delta, type);
    s->upper = datum_add(s->upper, delta, type);
  }
  /* Shifted lower and upper */
  lower = s->lower;
  upper = s->upper;
  /* The default value when scale is not given is 1.0 */
  if (scale != 1.0)
  {
    /* The potential shift has been already taken care in the previous if */
    s->lower = datum_add(origin, double_datum(
      datum_double(datum_sub(lower, origin, type), type) * scale, type), type);
    if (datum_eq(lower, upper, type))
      s->upper = s->lower;
    else
    {
      /* Integer spans have exclusive upper bound */
      Datum upper1 = span_canon_upper(s);
      s->upper = datum_add(origin,
        double_datum(
          datum_double(datum_sub(upper1, origin, type), type) * scale,
          type), type);
      /* Integer spans have exclusive upper bound */
      if (span_canon_basetype(type))
        s->upper = datum_add(s->upper, 1, type);
    }
  }
  return;
}

/**
 * @brief Shift and/or scale a period by a delta and a scale
 */
void
tstzspan_delta_scale_iter(Span *s, TimestampTz origin, TimestampTz delta,
  double scale)
{
  assert(s);

  TimestampTz lower = DatumGetTimestampTz(s->lower);
  TimestampTz upper = DatumGetTimestampTz(s->upper);
  /* The default value when there is not shift is 0 */
  if (delta != 0)
  {
    s->lower = TimestampTzGetDatum(lower + delta);
    s->upper = TimestampTzGetDatum(upper + delta);
  }
  /* Shifted lower and upper */
  lower = DatumGetTimestampTz(s->lower);
  upper = DatumGetTimestampTz(s->upper);
  /* The default value when there is not scale is 1.0 */
  if (scale != 1.0)
  {
    /* The potential shift has been already taken care in the previous if */
    s->lower = TimestampTzGetDatum(
      origin + (TimestampTz) ((lower - origin) * scale));
    if (lower == upper)
      s->upper = s->lower;
    else
      s->upper = TimestampTzGetDatum(
        origin + (TimestampTz) ((upper - origin) * scale));
  }
  return;
}

/**
 * @brief Shift and/or scale a span by the values.
 * @note Returns the delta and scale of the transformation
 */
void
numspan_shift_scale1(Span *s, Datum shift, Datum width, bool hasshift,
  bool haswidth, Datum *delta, double *scale)
{
  assert(s); assert(delta); assert(scale);
  Datum lower = s->lower;
  Datum upper = s->upper;
  meosType type = s->basetype;
  lower_upper_shift_scale_value(shift, width, type, hasshift, haswidth,
    &lower, &upper);
  /* Compute delta and scale before overwriting s->lower and s->upper */
  *delta = 0;   /* Default value when shift is not given */
  *scale = 1.0; /* Default value when width is not given */
  if (hasshift)
    *delta = datum_sub(lower, s->lower, type);
  /* If the period is instantaneous we cannot scale */
  if (haswidth && ! datum_eq(s->lower, s->upper, type))
  {
    /* Integer spans have exclusive upper bound */
    Datum upper1, upper2;
    if (span_canon_basetype(type))
    {
      upper1 = datum_sub(upper, 1, type);
      upper2 = datum_sub(s->upper, 1, type);
    }
    else
    {
      upper1 = upper;
      upper2 = s->upper;
    }
    *scale = datum_double(datum_sub(upper1, lower, type), type) /
      datum_double(datum_sub(upper2, s->lower, type), type);
  }
  s->lower = lower;
  s->upper = upper;
  return;
}

/**
 * @brief Shift and/or scale a period by the intervals.
 * @note Returns the delta and scale of the transformation
 */
void
tstzspan_shift_scale1(Span *s, const Interval *shift, const Interval *duration,
  TimestampTz *delta, double *scale)
{
  assert(s); assert(delta); assert(scale);
  TimestampTz lower = DatumGetTimestampTz(s->lower);
  TimestampTz upper = DatumGetTimestampTz(s->upper);
  lower_upper_shift_scale_time(shift, duration, &lower, &upper);
  /* Compute delta and scale before overwriting s->lower and s->upper */
  *delta = 0;   /* Default value when shift == NULL */
  *scale = 1.0; /* Default value when duration == NULL */
  if (shift != NULL)
    *delta = lower - DatumGetTimestampTz(s->lower);
  /* If the period is instantaneous we cannot scale */
  if (duration != NULL && s->lower != s->upper)
    *scale = (double) (upper - lower) /
      (double) (DatumGetTimestampTz(s->upper) - DatumGetTimestampTz(s->lower));
  s->lower = TimestampTzGetDatum(lower);
  s->upper = TimestampTzGetDatum(upper);
  return;
}

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Shift and/or scale a number span by the values.
 * @sqlfunc shift(), scale(), shiftScale()
 */
Span *
numspan_shift_scale(const Span *s, Datum shift, Datum width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_one_shift_width(hasshift, haswidth) ||
      (haswidth && ! ensure_positive_datum(width, s->basetype)))
    return NULL;

  /* Copy the input span to the result */
  Span *result = span_copy(s);
  /* Shift and/or scale the resulting span */
  lower_upper_shift_scale_value(shift, width, s->basetype, hasshift, haswidth,
    &result->lower, &result->upper);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_transf
 * @brief Return an integer span shifted and/or scaled by the values
 * @sqlfunc shift(), scale(), shiftScale()
 */
Span *
intspan_shift_scale(const Span *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_has_type(s, T_INTSPAN))
    return NULL;

  return numspan_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a big integer span shifted and/or scaled by the values
 * @sqlfunc shift(), scale(), shiftScale()
 */
Span *
bigintspan_shift_scale(const Span *s, int64 shift, int64 width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_has_type(s, T_BIGINTSPAN))
    return NULL;

  return numspan_shift_scale(s, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a float span shifted and/or scaled by the values
 * @sqlfunc shift(), scale(), shiftScale()
 */
Span *
floatspan_shift_scale(const Span *s, double shift, double width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_has_type(s, T_FLOATSPAN))
    return NULL;

  return numspan_shift_scale(s, Float8GetDatum(shift), Float8GetDatum(width),
    hasshift, haswidth);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_transf
 * @brief Shift and/or scale a period by the intervals.
 * @sqlfunc shift(), scale(), shiftScale()
 */
Span *
tstzspan_shift_scale(const Span *s, const Interval *shift,
  const Interval *duration)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_has_type(s, T_TSTZSPAN) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Copy the input period to the result */
  Span *result = span_copy(s);
  /* Shift and/or scale the resulting period */
  TimestampTz lower = DatumGetTimestampTz(s->lower);
  TimestampTz upper = DatumGetTimestampTz(s->upper);
  lower_upper_shift_scale_time(shift, duration, &lower, &upper);
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
 */
bool
span_eq(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;

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
 * @sqlfunc intspan_cmp(), bigintspan_cmp(), floatspan_cmp(), tstzspan_cmp()
 */
int
span_cmp(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return INT_MAX;

  int cmp = datum_cmp(s1->lower, s2->lower, s1->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->lower_inc != s2->lower_inc)
    return s1->lower_inc ? -1 : 1;
  cmp = datum_cmp(s1->upper, s2->upper, s1->basetype);
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
 * @return On error return INT_MAX
 * @sqlfunc intspan_hash(), bigintspan_hash(), floatspan_hash(), tstzspan_hash()
 */
uint32
span_hash(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return INT_MAX;

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
 * @return On error return INT_MAX
 * @sqlfunc hash_extended()
 */
uint64
span_hash_extended(const Span *s, uint64 seed)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return INT_MAX;

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

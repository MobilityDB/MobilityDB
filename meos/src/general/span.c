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
 * @brief General functions for spans (a.k.a. ranges) composed of two `Datum`
 * values and two `Boolean` values stating whether the bounds are inclusive
 */

#include "general/span.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/temporal.h"
#include "general/tnumber_mathfuncs.h"
#include "general/type_parser.h"
#include "general/type_util.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a span is of a given span type
 */
bool
ensure_span_isof_type(const Span *s, meosType spantype)
{
  if (s->spantype != spantype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The span must be of type %s", meostype_name(spantype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a span is of a given base type
 */
bool
ensure_span_isof_basetype(const Span *s, meosType basetype)
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
 * @brief Ensure that the spans have the same type
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
  return;
}

/**
 * @brief Compare two span boundary points, returning <0, 0, or >0 according to
 * whether the first one is less than, equal to, or greater than the second one
 * @details The boundaries can be any combination of upper and lower; so it is
 * useful for a variety of operators.
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
  assert(b1); assert(b2); assert(b1->basetype == b2->basetype);
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
 * @brief Comparison function for sorting span bounds
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
 * whether the first bound is less than, equal to, or greater than the second one
 * @note The function is equivalent to #span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_lower_cmp(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->basetype == s2->basetype);
  int result = datum_cmp(s1->lower, s2->lower, s1->basetype);
  if (result != 0)
    return result;
  /* The bound values are equal */
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

/**
 * @brief Compare the upper bounds of two spans, returning <0, 0, or >0
 * according to whether the first bound is less than, equal to, or greater than
 * the second one.
 * @note The function is equivalent to #span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_upper_cmp(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->basetype == s2->basetype);
  int result = datum_cmp(s1->upper, s2->upper, s1->basetype);
  if (result != 0)
    return result;
  /* The bound values are equal */
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

/**
 * @brief Return the bound increased by 1 for accounting for canonicalized spans
 */
Datum
span_incr_bound(Datum lower, meosType basetype)
{
  Datum result;
  switch (basetype)
  {
    case T_INT4:
      result = Int32GetDatum(DatumGetInt32(lower) + (int32) 1);
      break;
    case T_INT8:
      result = Int64GetDatum(DatumGetInt64(lower) + (int64) 1);
      break;
    case T_DATE:
      result = DateADTGetDatum(DatumGetDateADT(lower) + 1);
      break;
    default:
      result = lower;
  }
  return result;
}

/**
 * @brief Return the bound decreased by 1 for accounting for canonicalized spans
 */
Datum
span_decr_bound(Datum lower, meosType basetype)
{
  Datum result;
  switch (basetype)
  {
    case T_INT4:
      result = Int32GetDatum(DatumGetInt32(lower) - (int32) 1);
      break;
    case T_INT8:
      result = Int64GetDatum(DatumGetInt64(lower) - (int64) 1);
      break;
    case T_DATE:
      result = DateADTGetDatum(DatumGetDateADT(lower) - 1);
      break;
    default:
      result = lower;
  }
  return result;
}

/**
 * @brief Normalize an array of spans
 * @details The input spans may overlap and may be non contiguous.
 * The normalized spans are new spans that must be freed.
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the input array
 * @param[in] ordered True if the spans are ordered
 * @param[out] newcount Number of elements in the output array
 * @pre @p count is greater than 0
 */
Span *
spanarr_normalize(Span *spans, int count, bool ordered, int *newcount)
{
  assert(spans); assert(count > 0); assert(newcount);
  /* Sort the spans if they are not ordered */
  if (! ordered)
    spanarr_sort(spans, count);
  int nspans = 0;
  Span *result = palloc(sizeof(Span) * count);
  Span *current = &spans[0];
  for (int i = 1; i < count; i++)
  {
    Span *next = &spans[i];
    if (ovadj_span_span(current, next))
      /* Compute the union of the spans */
      span_expand(next, current);
    else
    {
      result[nspans++] = *current;
      current = next;
    }
  }
  result[nspans++] = *current;
  /* Set the output parameter */
  *newcount = nspans;
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_inout
 * @brief Return a span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @param[in] spantype Span type
 */
Span *
span_in(const char *str, meosType spantype)
{
  assert(str);
  Span result;
  if (! span_parse(&str, spantype, true, &result))
    return NULL;
  return span_cp(&result);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
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
 * @ingroup meos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
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
 * @ingroup meos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
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
 * @ingroup meos_setspan_inout
 * @brief Return a date span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
 */
Span *
datespan_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return span_in(str, T_DATESPAN);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a timestamptz span from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
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
 * @brief Remove the quotes from the Well-Known Text (WKT) representation of a
 * span
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
      *last++ = *str;
    str++;
  }
  *last = '\0';
  return result;
}

/**
 * @ingroup meos_internal_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @param[in] s Span
 * @param[in] maxdd Maximum number of decimal digits
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
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @return On error return @p NULL
 * @param[in] s Span
 * @csqlfn #Span_out()
 */
char *
intspan_out(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_INTSPAN))
    return NULL;
  return span_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @param[in] s Span
 * @return On error return @p NULL
 * @csqlfn #Span_out()
 */
char *
bigintspan_out(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_BIGINTSPAN))
    return NULL;
  return span_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @param[in] s Span
 * @param[in] maxdd Maximum number of decimal digits
 * @return On error return @p NULL
  * @csqlfn #Span_out()
*/
char *
floatspan_out(const Span *s, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_FLOATSPAN))
    return NULL;
  return span_out(s, maxdd);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @param[in] s Span
 * @return On error return @p NULL
 * @csqlfn #Span_out()
 */
char *
datespan_out(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_DATESPAN))
    return NULL;
  return span_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @param[in] s Span
 * @return On error return @p NULL
 * @csqlfn #Span_out()
 */
char *
tstzspan_out(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return span_out(s, 0);
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @param[in] basetype Type of the bounds
 */
Span *
span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  meosType basetype)
{
  Span *s = palloc(sizeof(Span));
  meosType spantype = basetype_spantype(basetype);
  span_set(lower, upper, lower_inc, upper_inc, basetype, spantype, s);
  return s;
}

#if MEOS
/**
 * @ingroup meos_setspan_constructor
 * @brief Return an integer span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
intspan_make(int lower, int upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Int32GetDatum(lower), Int32GetDatum(upper), lower_inc, upper_inc,
    T_INT4, T_INTSPAN, s);
  return s;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a big integer span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
bigintspan_make(int64 lower, int64 upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Int64GetDatum(lower), Int64GetDatum(upper), lower_inc, upper_inc,
    T_INT8, T_BIGINTSPAN, s);
  return s;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a float span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Float8GetDatum(lower), Float8GetDatum(upper), lower_inc, upper_inc,
    T_FLOAT8, T_FLOATSPAN, s);
  return s;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a date span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
datespan_make(DateADT lower, DateADT upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(DateADTGetDatum(lower), DateADTGetDatum(upper), lower_inc,
    upper_inc, T_DATE, T_DATESPAN, s);
  return s;
}
/**
 * @ingroup meos_setspan_constructor
 * @brief Return a timestamptz span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
tstzspan_make(TimestampTz lower, TimestampTz upper, bool lower_inc,
  bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(TimestampTzGetDatum(lower), TimestampTzGetDatum(upper), lower_inc,
    upper_inc, T_TIMESTAMPTZ, T_TSTZSPAN, s);
  return s;
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return the last argument initialized to a span constructed from the
 * other arguments
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @param[in] basetype Base type
 * @param[in] spantype Span type
 * @param[out] s Result span
 * @see #span_make()
 */
void
span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  meosType basetype, meosType spantype, Span *s)
{
  assert(s); assert(basetype_spantype(basetype) == spantype);
  /* Canonicalize */
  if (span_canon_basetype(basetype))
  {
    if (! lower_inc)
    {
      lower = span_incr_bound(lower, basetype);
      lower_inc = true;
    }
    if (upper_inc)
    {
      upper = span_incr_bound(upper, basetype);
      upper_inc = false;
    }
  }

  int cmp = datum_cmp(lower, upper, basetype);
  /* error check: if lower bound value is above upper, it's wrong */
  if (cmp > 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Span lower bound must be less than or equal to span upper bound");
    return;
  }

  /* error check: if bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && ! (lower_inc && upper_inc))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Span cannot be empty");
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
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a copy of a span
 * @param[in] s Span
 */
Span *
span_cp(const Span *s)
{
  assert(s);
  Span *result = palloc(sizeof(Span));
  memcpy((char *) result, (char *) s, sizeof(Span));
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_constructor
 * @brief Return a copy of a span
 * @param[in] s Span
 */
Span *
span_copy(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return span_cp(s);
}
#endif /* MEOS */

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return the last argument initialized to a span constructed from the
 * value
 * @param[in] value Value
 * @param[in] basetype Type of the value
 * @param[out] s Result span
*/
void
value_set_span(Datum value, meosType basetype, Span *s)
{
  assert(s); assert(span_basetype(basetype));
  meosType spantype = basetype_spantype(basetype);
  span_set(value, value, true, true, basetype, spantype, s);
  return;
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a value converted to a span
 * @param[in] value Value
 * @param[in] basetype Type of the value
 */
Span *
value_to_span(Datum value, meosType basetype)
{
  Span *result = palloc(sizeof(Span));
  value_set_span(value, basetype, result);
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return an integer converted to a span
 * @param[in] i Value
 * @csqlfn #Value_to_span()
 */
Span *
int_to_span(int i)
{
  Span *result = palloc(sizeof(Span));
  /* Account for canonicalized spans */
  span_set(Int32GetDatum(i), Int32GetDatum(i + 1), true, false, T_INT4,
    T_INTSPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a big integer converted to a span
 * @param[in] i Value
 * @csqlfn #Value_to_span()
 */
Span *
bigint_to_span(int i)
{
  Span *result = palloc(sizeof(Span));
  /* Account for canonicalized spans */
  span_set(Int64GetDatum(i), Int64GetDatum(i + 1), true, false, T_INT8,
    T_BIGINTSPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a float converted to a span
 * @param[in] d Value
 * @csqlfn #Value_to_span()
 */
Span *
float_to_span(double d)
{
  Span *result = palloc(sizeof(Span));
  span_set(Float8GetDatum(d), Float8GetDatum(d), true, true, T_FLOAT8,
    T_FLOATSPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a date converted to a span
 * @param[in] d Value
 * @csqlfn #Value_to_span()
 */
Span *
date_to_span(DateADT d)
{
  Span *result = palloc(sizeof(Span));
  /* Account for canonicalized spans */
  span_set(DateADTGetDatum(d), DateADTGetDatum(d + 1), true, false, T_DATE,
    T_DATESPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a timestamptz converted to a span
 * @param[in] t Value
 * @csqlfn #Value_to_span()
 */
Span *
timestamptz_to_span(TimestampTz t)
{
  Span *result = palloc(sizeof(Span));
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return the last argument initialized to the bounding span of a set
 * @param[in] s Set
 * @param[in] sp Span
 */
void
set_set_span(const Set *s, Span *sp)
{
  assert(s); assert(sp);
  meosType spantype = basetype_spantype(s->basetype);
  span_set(SET_VAL_N(s, MINIDX), SET_VAL_N(s, s->MAXIDX), true, true,
    s->basetype, spantype, sp);
  return;
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return the bounding span of a set
 * @param[in] s Set
 */
Span *
set_span(const Set *s)
{
  assert(s); assert(set_spantype(s->settype));
  Span *result = palloc(sizeof(Span));
  set_set_span(s, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a set converted to a span
 * @param[in] s Set
 * @csqlfn #Set_to_span()
 */
Span *
set_to_span(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_spantype(s->settype))
    return NULL;
  return set_span(s);
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return the second span initialized with the first one transformed to
 * a float span
 * @param[in] s1,s2 Spans
 */
void
intspan_set_floatspan(const Span *s1, Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == T_INTSPAN);
  Datum lower = Float8GetDatum((double) DatumGetInt32(s1->lower));
  Datum upper = Float8GetDatum((double) (DatumGetInt32(s1->upper) - 1));
  span_set(lower, upper, true, true, T_FLOAT8, T_FLOATSPAN, s2);
  return;
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return an integer span converted to a float span
 * @param[in] s Span
 */
Span *
intspan_floatspan(const Span *s)
{
  assert(s); assert(s->spantype == T_INTSPAN);
  Span *result = palloc(sizeof(Span));
  intspan_set_floatspan(s, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return an integer span converted to a float span
 * @param[in] s Span
 * @return On error return @p NULL
 */
Span *
intspan_to_floatspan(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_INTSPAN))
    return NULL;
  return intspan_floatspan(s);
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return the second span initialized with the first one transformed to
 * an integer span
 * @param[in] s1,s2 Spans
 */
void
floatspan_set_intspan(const Span *s1, Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == T_FLOATSPAN);
  Datum lower = Int32GetDatum((int) DatumGetFloat8(s1->lower));
  Datum upper = Int32GetDatum((int) (DatumGetFloat8(s1->upper)));
  span_set(lower, upper, s1->lower_inc, s1->upper_inc, T_INT4, T_INTSPAN, s2);
  return;
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a float span converted to an integer span
 * @param[in] s Span
 * @return On error return @p NULL
 */
Span *
floatspan_intspan(const Span *s)
{
  assert(s); assert(s->spantype == T_FLOATSPAN);
  Span *result = palloc(sizeof(Span));
  floatspan_set_intspan(s, result);
  return result;
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a float span converted to an integer span
 * @param[in] s Span
 * @return On error return @p NULL
 */
Span *
floatspan_to_intspan(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_FLOATSPAN))
    return NULL;
  return floatspan_intspan(s);
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return the second span initialized with the first one transformed to
 * a timetstamptz span
 * @param[in] s1,s2 Spans
 */
void
datespan_set_tstzspan(const Span *s1, Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == T_DATESPAN);
  Datum lower =
    TimestampTzGetDatum(date_to_timestamptz(DatumGetDateADT(s1->lower)));
  Datum upper =
    TimestampTzGetDatum(date_to_timestamptz(DatumGetDateADT(s1->upper)));
  /* Date spans are always canonicalized */
  span_set(lower, upper, true, false, T_TIMESTAMPTZ, T_TSTZSPAN, s2);
  return;
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a date span converted to a timestamptz span
 * @param[in] s Span
 */
Span *
datespan_tstzspan(const Span *s)
{
  assert(s); assert(s->spantype == T_DATESPAN);
  Span *result = palloc(sizeof(Span));
  datespan_set_tstzspan(s, result);
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a date span converted to a timestamptz span
 * @param[in] s Span
 * @return On error return @p NULL
 */
Span *
datespan_to_tstzspan(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_DATESPAN))
    return NULL;
  return datespan_tstzspan(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return the last span initialized with the first one transformed to a
 * date span
 * @param[in] s1,s2 Spans
 */
void
tstzspan_set_datespan(const Span *s1, Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == T_TSTZSPAN);
  DateADT lower = timestamptz_to_date(DatumGetTimestampTz(s1->lower));
  DateADT upper = timestamptz_to_date(DatumGetTimestampTz(s1->upper));
  bool lower_inc = s1->lower_inc;
  bool upper_inc = s1->upper_inc;
  /* Both bounds are set to true when the resulting dates are equal, e.g.,
   * (2001-10-18 19:46:00, 2001-10-18 19:50:00) -> [2001-10-18, 2001-10-18] */
  if (lower == upper)
  {
    lower_inc = upper_inc = true;
  }
  /* Canonicalization takes place in the following function */
  span_set(DateADTGetDatum(lower), DateADTGetDatum(upper), lower_inc,
    upper_inc, T_DATE, T_DATESPAN, s2);
  return;
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a timestamptz span converted to a date span
 * @param[in] s Span
 */
Span *
tstzspan_datespan(const Span *s)
{
  assert(s); assert(s->spantype == T_TSTZSPAN);
  Span *result = palloc(sizeof(Span));
  tstzspan_set_datespan(s, result);
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a timestamptz span converted to a date span
 * @param[in] s Span
 * @return On error return @p NULL
 */
Span *
tstzspan_to_datespan(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return tstzspan_datespan(s);
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @return On error return @p INT_MAX
 * @param[in] s Span
 * @csqlfn #Span_lower()
 */
int
intspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_INTSPAN))
    return INT_MAX;
  return DatumGetInt32(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @param[in] s Span
 * @return On error return LONG_MAX
 * @csqlfn #Span_lower()
 */
int64
bigintspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_BIGINTSPAN))
    return LONG_MAX;
  return DatumGetInt64(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a float span
 * @param[in] s Span
 * @return On error return @p DBL_MAX
 * @csqlfn #Span_lower()
 */
double
floatspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_FLOATSPAN))
    return DBL_MAX;
  return DatumGetFloat8(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a date span
 * @param[in] s Span
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Span_lower()
 */
DateADT
datespan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_DATESPAN))
    return DATEVAL_NOEND;
  return DateADTGetDatum(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a timestamptz span
 * @param[in] s Span
 * @return On error return DT_NOEND
 * @csqlfn #Span_lower()
 */
TimestampTz
tstzspan_lower(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return DT_NOEND;
  return TimestampTzGetDatum(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @param[in] s Span
 * @return On error return @p INT_MAX
 * @csqlfn #Span_upper()
 */
int
intspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_INTSPAN))
    return INT_MAX;
  return Int32GetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @param[in] s Span
 * @return On error return LONG_MAX
 * @csqlfn #Span_upper()
 */
int64
bigintspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_BIGINTSPAN))
    return LONG_MAX;
  return Int64GetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a float span
 * @param[in] s Span
 * @return On error return @p DBL_MAX
 * @csqlfn #Span_upper()
 */
double
floatspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_FLOATSPAN))
    return DBL_MAX;
  return DatumGetFloat8(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a date span
 * @param[in] s Span
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Span_upper()
 */
DateADT
datespan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_DATESPAN))
    return DATEVAL_NOEND;
  return DateADTGetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a timestamptz span
 * @param[in] s Span
 * @return On error return DT_NOEND
 * @csqlfn #Span_upper()
 */
TimestampTz
tstzspan_upper(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return DT_NOEND;
  return TimestampTzGetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return true if the lower bound of a span is inclusive
 * @param[in] s Span
 * @csqlfn #Span_lower_inc()
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
 * @ingroup meos_setspan_accessor
 * @brief Return true if the upper bound of a span is inclusive
 * @param[in] s Span
 * @csqlfn #Span_lower_inc()
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
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the width of a span
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Numspan_width()
 */
Datum
numspan_width(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return (Datum) -1;
  return distance_value_value(s->upper, s->lower, s->basetype);
}

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of an integer span
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Numspan_width()
 */
int
intspan_width(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_INTSPAN))
    return -1;
  return Int32GetDatum(numspan_width(s));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of a big integer span
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Numspan_width()
 */
int64
bigintspan_width(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_BIGINTSPAN))
    return -1;
  return Int64GetDatum(numspan_width(s));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of a float span
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Numspan_width()
 */
double
floatspan_width(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_FLOATSPAN))
    return -1.0;
  return DatumGetFloat8(numspan_width(s));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the duration of a date span as an interval
 * @param[in] s Span
 * @csqlfn #Datespan_duration()
 */
Interval *
datespan_duration(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_DATESPAN))
    return NULL;
  Interval *result = palloc0(sizeof(Interval));
  result->day = DateADTGetDatum(s->upper) - DateADTGetDatum(s->lower);
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the duration of a timestamptz span as an interval
 * @param[in] s Span
 * @csqlfn #Tstzspan_duration()
 */
Interval *
tstzspan_duration(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return minus_timestamptz_timestamptz(s->upper, s->lower);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return the second span expanded with the first one
 * @param[in] s1,s2 Spans
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

/*****************************************************************************/

/**
 * @brief Shift and/or scale the span bounds by two values
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] basetype Type of the values
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @param[in,out] lower,upper Bounds of the period
 */
void
lower_upper_shift_scale_value(Datum shift, Datum width, meosType basetype,
  bool hasshift, bool haswidth, Datum *lower, Datum *upper)
{
  assert(hasshift || haswidth); assert(lower); assert(upper);
  assert(! haswidth || positive_datum(width, basetype));

  bool instant = datum_eq(*lower, *upper, basetype);
  if (hasshift)
  {
    *lower = datum_add(*lower, shift, basetype);
    if (instant)
      *upper = *lower;
    else
      *upper = datum_add(*upper, shift, basetype);
  }
  if (haswidth && ! instant)
  {
    /* Integer and date spans have exclusive upper bound */
    if (span_canon_basetype(basetype))
      width = datum_add(width, 1, basetype);
    *upper = datum_add(*lower, width, basetype);
  }
  return;
}

/**
 * @brief Shift and/or scale period bounds by two intervals
 * @param[in] shift Interval to shift the bounds, may be NULL
 * @param[in] duration Interval for the duration of the result, may be NULL
 * @param[in,out] lower,upper Bounds of the period
 */
void
lower_upper_shift_scale_time(const Interval *shift, const Interval *duration,
  TimestampTz *lower, TimestampTz *upper)
{
  assert(shift || duration); assert(lower); assert(upper);
  assert(! duration || valid_duration(duration));

  bool instant = (*lower == *upper);
  if (shift)
  {
    *lower = add_timestamptz_interval(*lower, shift);
    if (instant)
      *upper = *lower;
    else
      *upper = add_timestamptz_interval(*upper, shift);
  }
  if (duration && ! instant)
    *upper = add_timestamptz_interval(*lower, duration);
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
      Datum upper1 = span_decr_bound(s->upper, s->basetype);
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
 * @brief Shift and/or scale a timestamptz span by a delta and a scale
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
 * @brief Return a number span shifted and/or scaled by two values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @param[out] delta,scale Delta and scale of the transformation
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
 * @brief Return a timestamptz span shifted and/or scaled by two intervals
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
 * @ingroup meos_internal_setspan_transf
 * @brief Return a number span shifted and/or scaled by two values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
numspan_shift_scale(const Span *s, Datum shift, Datum width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_one_true(hasshift, haswidth) ||
      (haswidth && ! ensure_positive_datum(width, s->basetype)))
    return NULL;

  /* Copy the input span to the result */
  Span *result = span_cp(s);
  /* Shift and/or scale the resulting span */
  lower_upper_shift_scale_value(shift, width, s->basetype, hasshift, haswidth,
    &result->lower, &result->upper);
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_transf
 * @brief Return an integer span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
intspan_shift_scale(const Span *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_INTSPAN))
    return NULL;
  return numspan_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a big integer span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
bigintspan_shift_scale(const Span *s, int64 shift, int64 width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_BIGINTSPAN))
    return NULL;
  return numspan_shift_scale(s, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
floatspan_shift_scale(const Span *s, double shift, double width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_FLOATSPAN))
    return NULL;
  return numspan_shift_scale(s, Float8GetDatum(shift), Float8GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a date span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
datespan_shift_scale(const Span *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_DATESPAN))
    return NULL;
  return numspan_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_transf
 * @brief Return a timestamptz span shifted and/or scaled by two intervals
 * @param[in] s Span
 * @param[in] shift Interval to shift the bounds, may be NULL
 * @param[in] duration Duation of the result, may be NULL
 * @csqlfn #Tstzspan_shift(), #Tstzspan_scale(), #Tstzspan_shift_scale()
 */
Span *
tstzspan_shift_scale(const Span *s, const Interval *shift,
  const Interval *duration)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Copy the input period to the result */
  Span *result = span_cp(s);
  /* Shift and/or scale the resulting period */
  TimestampTz lower = DatumGetTimestampTz(s->lower);
  TimestampTz upper = DatumGetTimestampTz(s->upper);
  lower_upper_shift_scale_time(shift, duration, &lower, &upper);
  result->lower = TimestampTzGetDatum(lower);
  result->upper = TimestampTzGetDatum(upper);
  return result;
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_comp
 * @brief Return true if the two spans are equal
 * @param[in] s1,s2 Sets
 */
bool
span_eq_int(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  if (s1->lower != s2->lower || s1->upper != s2->upper ||
    s1->lower_inc != s2->lower_inc || s1->upper_inc != s2->upper_inc)
    return false;
  return true;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the two spans are equal
 * @note The ifunction #span_cmp() is not used to increase efficiency
 * @param[in] s1,s2 Sets
 * @csqlfn #Span_eq()
 */
bool
span_eq(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;
  return span_eq_int(s1, s2);
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span is different from the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Span_ne()
 */
bool
span_ne(const Span *s1, const Span *s2)
{
  return (! span_eq(s1, s2));
}

/**
 * @ingroup meos_internal_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span is less than,
 * equal, or greater than the second one
 * @param[in] s1,s2 Sets
 */
int
span_cmp_int(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
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

/**
 * @ingroup meos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span is less than,
 * equal, or greater than the second one
 * @param[in] s1,s2 Sets
 * @note Function used for B-tree comparison
 * @csqlfn #Span_cmp()
 */
int
span_cmp(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return INT_MAX;
  return span_cmp_int(s1, s2);
}

/* Inequality operators using the span_cmp function */

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span is less than the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Span_lt()
 */
bool
span_lt(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) < 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span is less than or equal to the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Span_le()
 */
bool
span_le(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) <= 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span is greater than or equal to the second
 * one
 * @param[in] s1,s2 Sets
 * @csqlfn #Span_gt()
 */
bool
span_ge(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) >= 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span is greater than the second one
 * @param[in] s1,s2 Sets
 * @csqlfn #Span_ge()
 */
bool
span_gt(const Span *s1, const Span *s2)
{
  return span_cmp(s1, s2) > 0;
}

/*****************************************************************************
 * Functions for defining hash indexes
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the 32-bit hash of a span
 * @param[in] s Span
 * @return On error return @p INT_MAX
 * @csqlfn #Span_hash()
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
 * @ingroup meos_setspan_accessor
 * @brief Return the 64-bit hash of a span using a seed
 * @param[in] s Span
 * @param[in] seed Seed
 * @return On error return @p INT_MAX
 * @csqlfn #Span_hash_extended()
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

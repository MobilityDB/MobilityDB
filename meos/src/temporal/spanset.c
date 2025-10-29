/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief General functions for set of disjoint spans
 */

#include "temporal/spanset.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/timestamp.h"
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/type_parser.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a span set is of a given span set type
 */
bool
ensure_spanset_isof_type(const SpanSet *ss, meosType spansettype)
{
  if (ss->spansettype == spansettype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "The span set must be of type %s", meostype_name(spansettype));
  return false;
}

/**
 * @brief Ensure that the span set values have the same type
 */
bool
ensure_same_spanset_type(const SpanSet *ss1, const SpanSet *ss2)
{
  if (ss1->spansettype == ss2->spansettype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Operation on mixed span set types: %s, %s",
    meostype_name(ss1->spansettype), meostype_name(ss2->spansettype));
  return false;
}

/**
 * @brief Ensure that a span set and a span value have the same span type
 */
bool
ensure_same_spanset_span_type(const SpanSet *ss, const Span *s)
{
  if (ss->spantype == s->spantype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Operation on mixed span set and span types: %s, %s",
    meostype_name(ss->spansettype), meostype_name(s->spantype));
  return false;
}

/**
 * @brief Ensure that a span set and a span value have the same span type
 */
bool
ensure_valid_spanset_span(const SpanSet *ss, const Span *s)
{
  VALIDATE_NOT_NULL(ss, false); VALIDATE_NOT_NULL(s, false);
  if (! ensure_same_spanset_span_type(ss, s))
    return false;
  return true;
}

/**
 * @brief Ensure that two span sets are of the same span type
 */
bool
ensure_valid_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  VALIDATE_NOT_NULL(ss1, false); VALIDATE_NOT_NULL(ss2, false);
  if (! ensure_same_spanset_type(ss1, ss2))
    return false;
  return true;
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Return the location of a value in a span set using binary search
 * @details If the value is found, the index of the span is returned in the
 * output parameter. Otherwise, return a number encoding whether the value is
 * before, between two spans, or after the span set.
 *
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
 * @return Return true if the value is contained in the span set
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
    s = SPANSET_SP_N(ss, middle);
    if (contains_span_value(s, v))
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

#if DEBUG_BUILD
/**
 * @brief Return the n-th span of a span set
 * @pre The argument @p index is less than the number of spans in the span set
 * @note This is the internal function equivalent to #spanset_span_n.
 * This function does not verify that the index is is in the correct bounds
 */
const Span *
SPANSET_SP_N(const SpanSet *ss, int index)
{
  assert(ss); assert(index < ss->count);
  return &ss->elems[index];
}
#endif /* DEBUG_BUILD */

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_inout
 * @brief Return a span set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @param[in] spansettype Span set type
 */
SpanSet *
spanset_in(const char *str, meosType spansettype)
{
  assert(str);
  return spanset_parse(&str, spansettype);
}

/**
 * @ingroup meos_internal_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set
 * @param[in] ss Span set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Spanset_out()
 */
char *
spanset_out(const SpanSet *ss, int maxdd)
{
  assert(ss);
  /* Ensure the validity of the arguments */
  if (! ensure_not_negative(maxdd))
    return NULL;

  char **strings = palloc(sizeof(char *) * ss->count);
  size_t outlen = 0;
  for (int i = 0; i < ss->count; i++)
  {
    strings[i] = span_out(SPANSET_SP_N(ss, i), maxdd);
    outlen += strlen(strings[i]) + 1;
  }
  return stringarr_to_string(strings, ss->count, outlen, "", '{', '}',
    QUOTES_NO, SPACES);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a span set from an array of disjoint spans enabling the
 * data structure to expand
 * @details For example, the memory structure of a SpanSet with 3 span is as
 * follows
 * @code
 * ---------------------------------------------------------------------------------
 * ( SpanSet )_X | ( bbox )_X | ( Span_0 )_X | ( Span_1 )_X | ( Span_2 )_X |
 * ---------------------------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, and `bbox` is the
 * bounding box which is also a span.
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized
 * @param[in] order True when the input spans should should be ordered
 */
SpanSet *
spanset_make_exp(Span *spans, int count, int maxcount, bool normalize,
  bool order)
{
  assert(spans); assert(count > 0); assert(count <= maxcount);
  /* Test the validity of the spans */
  if (! order)
  {
    for (int i = 0; i < count - 1; i++)
    {
      int cmp = datum_cmp(spans[i].upper, spans[i + 1].lower, spans[i].basetype);
      if (cmp > 0 ||
        (cmp == 0 && spans[i].upper_inc && spans[i + 1].lower_inc))
      {
        char *str1 = span_out(&spans[i], OUT_MAX_DIGITS);
        char *str2 = span_out(&spans[i + 1], OUT_MAX_DIGITS);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "The spans composing a span set must be increasing: %s, %s", str1, str2);
#if MEOS
        pfree(str1); pfree(str2);
        return NULL;
#endif
      }
    }
  }

  /* Sort the values and remove duplicates */
  Span *newspans = spans;
  int newcount = count;
  if (normalize && count > 1)
    /* Sort the values and remove duplicates */
    newspans = spanarr_normalize(spans, count, order, &newcount);

  /* The first element span is already declared in the struct */
  size_t memsize = DOUBLE_PAD(sizeof(SpanSet)) +
    DOUBLE_PAD(sizeof(Span)) * (maxcount - 1);
  SpanSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->spansettype = spantype_spansettype(spans[0].spantype);
  result->spantype = spans[0].spantype;
  result->basetype = spans[0].basetype;
  result->count = newcount;
  result->maxcount = maxcount;

  /* Compute the bounding span */
  span_set(newspans[0].lower, newspans[newcount - 1].upper,
    newspans[0].lower_inc, newspans[newcount - 1].upper_inc,
    result->basetype, result->spantype, &result->span);
  /* Copy the span array */
  for (int i = 0; i < newcount; i++)
    result->elems[i] = newspans[i];
  /* Free after normalization */
  if (normalize && count > 1)
    pfree(newspans);
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_constructor
 * @brief Return a span set from an array of disjoint spans
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @csqlfn #Spanset_constructor()
 */
SpanSet *
spanset_make(Span *spans, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(spans, NULL);
  if (! ensure_positive(count))
    return NULL;
  return spanset_make_exp(spans, count, count, true, true);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a span set from an array of spans and free the input array
 * of spans after the creation
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * @param[in] order True if the input spans should be ordered
 * @see #spanset_make
 */
SpanSet *
spanset_make_free(Span *spans, int count, bool normalize, bool order)
{
  assert(spans); assert(count >= 0);
  SpanSet *result = NULL;
  if (count > 0)
    result = spanset_make_exp(spans, count, count, normalize, order);
  pfree(spans);
  return result;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a copy of a span set
 * @param[in] ss Span set
 */
SpanSet *
spanset_copy(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  SpanSet *result = palloc(VARSIZE(ss));
  memcpy(result, ss, VARSIZE(ss));
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Convert a value into a span set
 * @param[in] value Value
 * @param[in] basetype Type of the value
 */
SpanSet *
value_spanset(Datum value, meosType basetype)
{
  assert(span_basetype(basetype));
  meosType spantype = basetype_spantype(basetype);
  Span s;
  span_set(value, value, true, true, basetype, spantype, &s);
  return spanset_make_exp(&s, 1, 1, NORMALIZE_NO, ORDER_NO);
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Convert a set into a span set
 * @param[in] s Set
 * @csqlfn #Set_to_spanset()
 */
SpanSet *
set_spanset(const Set *s)
{
  assert(s); assert(set_spantype(s->settype));
  Span *spans = palloc(sizeof(Span) * s->count);
  meosType spantype = basetype_spantype(s->basetype);
  for (int i = 0; i < s->count; i++)
  {
    Datum value = SET_VAL_N(s, i);
    span_set(value, value, true, true, s->basetype, spantype, &spans[i]);
  }
  return spanset_make_free(spans, s->count, NORMALIZE, ORDER_NO);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a set into a span set
 * @param[in] s Set
 * @csqlfn #Set_to_spanset()
 */
SpanSet *
set_to_spanset(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL);
  if (! ensure_set_spantype(s->settype))
    return NULL;
  return set_spanset(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a span into a span set
 * @param[in] s Span
 * @csqlfn #Spanset_to_span()
 */
SpanSet *
span_to_spanset(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL);
  return spanset_make_exp((Span *) s, 1, 1, NORMALIZE_NO, ORDER_NO);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert an integer span set into a float span set
 * @param[in] ss Span set
 * @csqlfn #Intspanset_to_floatspanset()
 */
SpanSet *
intspanset_to_floatspanset(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    intspan_set_floatspan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a float span set into an integer span set
 * @param[in] ss Span set
 * @csqlfn #Floatspanset_to_intspanset()
 */
SpanSet *
floatspanset_to_intspanset(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    floatspan_set_intspan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a date span set into a timestamptz span set
 * @param[in] ss Span set
 * @csqlfn #Datespanset_to_tstzspanset()
 */
SpanSet *
datespanset_to_tstzspanset(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    datespan_set_tstzspan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a timestamptz span set into a date span set
 * @param[in] ss Span set
 * @csqlfn #Tstzspanset_to_datespanset()
 */
SpanSet *
tstzspanset_to_datespanset(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    tstzspan_set_datespan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDER);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the size in bytes of a span set
 * @param[in] ss Span set
 * @return On error return -1
 * @csqlfn #Spanset_mem_size()
 */
int
spanset_mem_size(const SpanSet *ss)
{
  VALIDATE_NOT_NULL(ss, -1);
  return (int) VARSIZE(ss);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the bounding span of a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_to_span()
 */
Span *
spanset_span(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  return span_copy(&ss->span);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the lower bound a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_lower()
 */
Datum
spanset_lower(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  assert(ss);
  return ss->elems[0].lower;
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the lower bound a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_upper()
 */
Datum
spanset_upper(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  assert(ss);
  return ss->elems[ss->count - 1].upper;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return true if the lower bound of a span set is inclusive
 * @param[in] ss Span set
 * @csqlfn #Spanset_lower_inc()
 */
bool
spanset_lower_inc(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  return ss->elems[0].lower_inc;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return true if the upper bound of a span set is inclusive
 * @param[in] ss Span set
 * @csqlfn #Spanset_upper_inc()
 */
bool
spanset_upper_inc(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  return ss->elems[ss->count - 1].upper_inc;
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the width of a span set
 * @param[in] ss Span set
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return -1
 * @csqlfn #Numspanset_width()
 */
Datum
numspanset_width(const SpanSet *ss, bool boundspan)
{
  assert(ss);
  if (boundspan)
  {
    Datum lower = (SPANSET_SP_N(ss, 0))->lower;
    Datum upper = (SPANSET_SP_N(ss, ss->count - 1))->upper;
    return distance_value_value(lower, upper, ss->basetype);
  }

  Datum result = (Datum) 0;
  for (int i = 0; i < ss->count; i++)
    result = datum_add(result, numspan_width(SPANSET_SP_N(ss, i)),
      ss->basetype);
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the duration of a date span set
 * @param[in] ss Span set
 * @param[in] boundspan True when the potential time gaps are ignored
 * @csqlfn #Datespanset_duration()
 */
Interval *
datespanset_duration(const SpanSet *ss, bool boundspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);

  Interval *result = palloc0(sizeof(Interval));
  int ndays;
  if (boundspan)
  {
    ndays = minus_date_date(ss->span.upper, ss->span.lower);
    result->day = ndays;
    return result;
  }

  ndays = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = SPANSET_SP_N(ss, i);
    ndays += (int32) (s->upper - s->lower);
  }
  result->day = ndays;
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the duration of a timestamptz span set
 * @param[in] ss Span set
 * @param[in] boundspan True when the potential time gaps are ignored
 * @csqlfn #Tstzspanset_duration()
 */
Interval *
tstzspanset_duration(const SpanSet *ss, bool boundspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);

  if (boundspan)
    return minus_timestamptz_timestamptz(ss->span.upper, ss->span.lower);

  const Span *s = SPANSET_SP_N(ss, 0);
  Interval *result = minus_timestamptz_timestamptz(s->upper, s->lower);
  for (int i = 1; i < ss->count; i++)
  {
    s = SPANSET_SP_N(ss, i);
    Interval *interv1 = minus_timestamptz_timestamptz(s->upper, s->lower);
    Interval *interv2 = add_interval_interval(result, interv1);
    pfree(result); pfree(interv1);
    result = interv2;
  }
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the number of dates of a span set
 * @param[in] ss Span set
 * @return On error return -1
 * @csqlfn #Datespanset_num_dates()
 */
int
datespanset_num_dates(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, -1);
  /* Date span sets are always canonicalized */
  return ss->count * 2;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start date of a span set
 * @param[in] ss Span set
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Datespanset_start_date()
 */
DateADT
datespanset_start_date(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, DATEVAL_NOEND);
  const Span *span = SPANSET_SP_N(ss, 0);
  return DatumGetDateADT(span->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end date of a span set
 * @param[in] ss Span set
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Datespanset_end_date()
 */
DateADT
datespanset_end_date(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, DATEVAL_NOEND);
  const Span *span = SPANSET_SP_N(ss, ss->count - 1);
  return DatumGetDateADT(span->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument the n-th date of a date span set
 * @param[in] ss Span set
 * @param[in] n Number (1-based)
 * @param[out] result Date
 * @return Return true if the date is found
 * @csqlfn #Datespanset_date_n()
 */
bool
datespanset_date_n(const SpanSet *ss, int n, DateADT *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > ss->count * 2)
    return false;

  /* Date span sets are always canonicalized */
  int i = n / 2; /* 1-based */
  int j = (i * 2  < n) ? i : i - 1;
  const Span *s = SPANSET_SP_N(ss, j);
  *result = (i * 2  < n) ?
    DatumGetDateADT(s->lower) : DatumGetDateADT(s->upper);
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the set of dates of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 * @csqlfn #Datespanset_dates()
 */
Set *
datespanset_dates(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);

  Datum *dates = palloc(sizeof(Datum) * 2 * ss->count);
  int ndates = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = SPANSET_SP_N(ss, i);
    dates[ndates++] = s->lower;
    dates[ndates++] = s->upper;
  }
  return set_make_free(dates, ndates, T_DATE, ORDER_NO);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the number of timestamps of a span set
 * @param[in] ss Span set
 * @return On error return -1
 * @csqlfn #Tstzspanset_num_timestamps()
 */
int
tstzspanset_num_timestamps(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, -1);

  const Span *s = SPANSET_SP_N(ss, 0);
  Datum prev = s->lower;
  bool start = false;
  int result = 1;
  Datum value;
  int i = 1;
  while (i < ss->count || !start)
  {
    if (start)
    {
      s = SPANSET_SP_N(ss, i++);
      value = s->lower;
      start = !start;
    }
    else
    {
      value = s->upper;
      start = !start;
    }
    if (prev != value)
    {
      result++;
      prev = value;
    }
  }
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start timestamptz of a span set
 * @param[in] ss Span set
 * @return On error return DT_NOEND
 * @csqlfn #Tstzspanset_start_timestamptz()
 */
TimestampTz
tstzspanset_start_timestamptz(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, DT_NOEND);
  const Span *span = SPANSET_SP_N(ss, 0);
  return DatumGetTimestampTz(span->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end timestamptz of a span set
 * @param[in] ss Span set
 * @return On error return DT_NOEND
 * @csqlfn #Tstzspanset_end_timestamptz()
 */
TimestampTz
tstzspanset_end_timestamptz(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, DT_NOEND);
  const Span *span = SPANSET_SP_N(ss, ss->count - 1);
  return DatumGetTimestampTz(span->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument the n-th timestamptz of a span set
 * @param[in] ss Span set
 * @param[in] n Number (1-based)
 * @param[out] result Timestamptz
 * @return Return true if the timestamptz is found
 * @csqlfn #Tstzspanset_timestamptz_n()
 */
bool
tstzspanset_timestamptz_n(const SpanSet *ss, int n, TimestampTz *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false); VALIDATE_NOT_NULL(result, false);

  int i = 0;
  const Span *s = SPANSET_SP_N(ss, i);
  Datum value = s->lower;
  if (n == 1)
  {
    *result = DatumGetTimestampTz(value);
    return true;
  }

  bool start = false;
  int j = 1;
  Datum prev = value;
  while (j < n)
  {
    if (start)
    {
      i++;
      if (i == ss->count)
        break;

      s = SPANSET_SP_N(ss, i);
      value = s->lower;
      start = !start;
    }
    else
    {
      value = s->upper;
      start = !start;
    }
    if (prev != value)
    {
      j++;
      prev = value;
    }
  }
  if (j != n)
    return false;
  *result = DatumGetTimestampTz(value);
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the set of timestamps of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 * @csqlfn #Tstzspanset_timestamps()
 */
Set *
tstzspanset_timestamps(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);

  Datum *times = palloc(sizeof(Datum) * 2 * ss->count);
  const Span *s = SPANSET_SP_N(ss, 0);
  times[0] = s->lower;
  int ntimes = 1;
  if (s->lower != s->upper)
    times[ntimes++] = s->upper;
  for (int i = 1; i < ss->count; i++)
  {
    s = SPANSET_SP_N(ss, i);
    /* Notice that we are comparing the Datum corresponding to timestamptz */
    if (times[ntimes - 1] != s->lower)
      times[ntimes++] = s->lower;
    if (times[ntimes - 1] != s->upper)
      times[ntimes++] = s->upper;
  }
  return set_make_free(times, ntimes, T_TIMESTAMPTZ, ORDER_NO);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the number of spans of a span set
 * @param[in] ss Span set
 * @return On error return -1
 * @csqlfn #Spanset_num_spans()
 */
int
spanset_num_spans(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, -1);
  return ss->count;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy to the the start span of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 * @csqlfn #Spanset_start_span()
 */
Span *
spanset_start_span(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  return span_copy(SPANSET_SP_N(ss, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the end span of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 * @csqlfn #Spanset_end_span()
 */
Span *
spanset_end_span(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  return span_copy(SPANSET_SP_N(ss, ss->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the n-th span of a span set
 * @param[in] ss Span set
 * @param[in] n Index
 * @csqlfn #Spanset_span_n()
 */
Span *
spanset_span_n(const SpanSet *ss, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  if (n >= 1 && n <= ss->count)
    return span_copy(SPANSET_SP_N(ss, n - 1));
  return NULL;
}

#if MEOS
/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return an array of pointers to the spans of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 */
const Span **
spanset_sps(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  const Span **spans = palloc(sizeof(Span *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    spans[i] = SPANSET_SP_N(ss, i);
  return spans;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a C array with copies of the spans of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 */
Span **
spanset_spanarr(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);
  Span **spans = palloc(sizeof(Span *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    spans[i] = span_copy(SPANSET_SP_N(ss, i));
  return spans;
}
#endif /* MEOS */

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span set with the precision of the spans set to a
 * number of decimal places
 * @param[in] ss Span set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Floatspanset_round()
 */
SpanSet *
floatspanset_round(const SpanSet *ss, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    floatspan_round_set(SPANSET_SP_N(ss, i), maxdd, &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDER_NO);
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a copy of a span set without any extra storage space
 * @param[in] ss Span set
 */
SpanSet *
spanset_compact(const SpanSet *ss)
{
  assert(ss);
  /* Create the final value reusing the array of spans in the span set */
  return spanset_make_exp((Span *) &ss->elems, ss->count, ss->count,
    NORMALIZE, ORDER);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span set rounded down to the nearest integer
 * @param[in] ss Span set
 * @csqlfn #Floatspanset_floor()
 */
SpanSet *
floatspanset_floor(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);

  Span *spans = palloc(sizeof(Span) * ss->count);
  memcpy(spans, ss->elems, sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    floatspan_floor_ceil_iter(&spans[i], &datum_floor);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDER_NO);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span set rounded up to the nearest integer
 * @param[in] ss Span set
 * @csqlfn #Floatspanset_ceil()
 */
SpanSet *
floatspanset_ceil(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);

  Span *spans = palloc(sizeof(Span) * ss->count);
  memcpy(spans, ss->elems, sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    floatspan_floor_ceil_iter(&spans[i], &datum_ceil);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDER_NO);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span set with the values converted to degrees
 * @param[in] ss Span set
 * @param[in] normalize True when the result must be normalized
 * @csqlfn #Floatspanset_degrees()
 */
SpanSet *
floatspanset_degrees(const SpanSet *ss, bool normalize)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);

  SpanSet *result = spanset_copy(ss);
  for (int i = 0; i < ss->count; i++)
  {
    Span *s = &(result->elems[i]);
    s->lower = datum_degrees(s->lower, normalize);
    s->upper = datum_degrees(s->upper, normalize);
  }
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span set with the values converted to radians
 * @param[in] ss Span set
 * @csqlfn #Floatspanset_radians()
 */
SpanSet *
floatspanset_radians(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);

  SpanSet *result = spanset_copy(ss);
  for (int i = 0; i < ss->count; i++)
  {
    Span *s = &(result->elems[i]);
    s->lower = datum_radians(s->lower);
    s->upper = datum_radians(s->upper);
  }
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_setspan_transf
 * @brief Return a number set shifted and/or scaled by two intervals
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
numspanset_shift_scale(const SpanSet *ss, Datum shift, Datum width,
  bool hasshift, bool haswidth)
{
  assert(ss); assert(numspan_type(ss->spantype));
  /* Ensure the validity of the arguments */
  if (! ensure_one_true(hasshift, haswidth) ||
      (haswidth && ! ensure_positive_datum(haswidth, ss->basetype)))
    return NULL;

  /* Copy the input span set to the output span set */
  SpanSet *copy = spanset_copy(ss);

  /* Shift and/or scale the bounding span */
  Datum delta;
  double scale;
  numspan_shift_scale_iter(&copy->span, shift, width, hasshift, haswidth,
    &delta, &scale);
  Datum origin = copy->span.lower;

  /* Shift and/or scale the span set */
  for (int i = 0; i < ss->count; i++)
    numspan_delta_scale_iter(&copy->elems[i], origin, delta, hasshift,
      scale);

  /* Normalization is required after scaling */
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    memcpy(&spans[i], &copy->elems[i], sizeof(Span));
  SpanSet *result = spanset_make_exp(spans, ss->count, ss->count, true, true);
  pfree(copy); pfree(spans);
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a timestamptz span set shifted and/or scaled by two intervals
 * @param[in] ss Span set
 * @param[in] shift Interval to shift the span set, may be NULL
 * @param[in] duration Interval for the duration of the result, may be NULL
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
tstzspanset_shift_scale(const SpanSet *ss, const Interval *shift,
  const Interval *duration)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  if (! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_positive_duration(duration)))
    return NULL;

  /* Copy the input span set to the output span set */
  SpanSet *result = spanset_copy(ss);

  /* Shift and/or scale the bounding period */
  TimestampTz delta;
  double scale;
  tstzspan_shift_scale1(&result->span, shift, duration, &delta, &scale);
  TimestampTz origin = DatumGetTimestampTz(result->span.lower);
  /* Shift and/or scale the span set */
  for (int i = 0; i < ss->count; i++)
    tstzspan_delta_scale_iter(&result->elems[i], origin, delta, scale);
  return result;
}

/*****************************************************************************
 * Spans function
 *****************************************************************************/

/**
 * @ingroup meos_setspan_bbox_split
 * @brief Return the array of spans of a spanset
 * @param[in] ss Span set
 * @return On error return @p NULL
 * @csqlfn #Spanset_spans()
 */
Span *
spanset_spans(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL);

  Span *result = palloc(sizeof(Span) * ss->count);
  /* Output the composing spans */
  for (int i = 0; i < ss->count; i++)
    memcpy(&result[i], SPANSET_SP_N(ss, i), sizeof(Span));
  return result;
}

/**
 * @brief Return -1, 0, or 1 depending on whether the size of the first
 * span is less than, equal to, or greater than the second one
 * @param[in] s1,s2 Spans
 */
static int
span_cmp_size(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  int result;
  if (numspan_type(s1->spantype))
  {
    Datum d1 = distance_value_value(s1->upper, s1->lower, s1->basetype);
    Datum d2 = distance_value_value(s2->upper, s2->lower, s2->basetype);
    result = datum_cmp(d1, d2, s1->basetype);
  }
  else /* timespan_type(s1->spantype) */
  {
    Interval *dur1 = (s1->spantype == T_DATESPAN) ?
      datespan_duration(s1) : tstzspan_duration(s1);
    Interval *dur2 = (s2->spantype == T_DATESPAN) ?
      datespan_duration(s2) : tstzspan_duration(s2);
    result = pg_interval_cmp(dur1, dur2);
    pfree(dur1); pfree(dur2);
  }
  return result;
}

/**
 * @brief Sort function for comparying spans based on their size
 */
static void
spanarr_sort_size(Span *spans, int count)
{
  qsort(spans, (size_t) count, sizeof(Span),
    (qsort_comparator) &span_cmp_size);
  return;
}

/**
 * @ingroup meos_setspan_bbox_split
 * @brief Return an array of N spans from the composing spans of a spanset
 * @param[in] ss Span set
 * @param[in] span_count Number of spans
 * @param[out] count Number of elements in the output array
 * @return If the number of spans of the spanset is <= `span_count`, the result
 * contains one span per composing span. On error return @p NULL
 * @return On error return @p NULL
 * @csqlfn #Spanset_split_n_spans()
 */
Span *
spanset_split_n_spans(const SpanSet *ss, int span_count, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_positive(span_count))
    return NULL;

  /* Output the composing spans */
  if (ss->count <= span_count)
  {
    *count = ss->count;
    return spanset_spans(ss);
  }

  /* Merge consecutive sequences having the smallest gap */
  Span *result = palloc(sizeof(Span) * span_count);
  SpanSet *minus = minus_span_spanset(&ss->span, ss);
  Span *holes = palloc(sizeof(Span) * minus->count);
  for (int i = 0; i < minus->count; i++)
    memcpy(&holes[i], SPANSET_SP_N(minus, i), sizeof(Span));
  /* Sort the holes in increasing size */
  spanarr_sort_size(holes, minus->count);
  /* Number of holes in the original spanset that will be filled */
  int nfills = minus->count - span_count + 1;
  /* Sort the holes to fill the original spanset in increasing value/time */
  spanarr_sort(holes, nfills);
  SpanSet *tofill = spanset_make_exp(holes, nfills, nfills, NORMALIZE_NO,
    ORDER_NO);
  /* Resulting spanset with the holes filed */
  SpanSet *res = union_spanset_spanset(ss, tofill);
  assert(res->count == span_count);
  /* Construct the resulting array of spans */
  for (int i = 0; i < res->count; i++)
    memcpy(&result[i], SPANSET_SP_N(res, i), sizeof(Span));
  /* Clean-up and return */
  pfree(minus); pfree(holes); pfree(tofill); pfree(res);
  *count = span_count;
  return result;
}

/**
 * @ingroup meos_setspan_bbox_split
 * @brief Return an array of N spans from a spanset obtained by merging
 * consecutive composing spans 
 * @param[in] ss SpanSet
 * @param[in] elems_per_span Number of spans merged into an ouput span
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 * @csqlfn #Spanset_split_each_n_spans()
 */
Span *
spanset_split_each_n_spans(const SpanSet *ss, int elems_per_span, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL); VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_positive(elems_per_span))
    return NULL;

  int nspans = ceil((double) ss->count / (double) elems_per_span);
  Span *result = palloc(sizeof(Span) * nspans);
  int k = 0;
  for (int i = 0; i < ss->count; ++i)
  {
    if (i % elems_per_span == 0)
      result[k++] = *SPANSET_SP_N(ss, i);
    else
    {
      Span span = *SPANSET_SP_N(ss, i);
      span_expand(&span, &result[k - 1]);
    }
  }
  assert(k == nspans);
  *count = k;
  return result;
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the two span sets are equal
 * @param[in] ss1,ss2 Span sets
 * @note The function #spanset_cmp() is not used to increase efficiency
 * @csqlfn #Spanset_eq()
 */
bool
spanset_eq(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_spanset_spanset(ss1, ss2))
    return false;

  if (ss1->count != ss2->count)
    return false;
  /* ss1 and ss2 have the same number of spans */
  for (int i = 0; i < ss1->count; i++)
    if (! span_eq(SPANSET_SP_N(ss1, i), SPANSET_SP_N(ss2, i)))
      return false;
  /* All spans of the two span sets are equal */
  return true;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span set is different from the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Spanset_ne()
 */
inline bool
spanset_ne(const SpanSet *ss1, const SpanSet *ss2)
{
  return ! spanset_eq(ss1, ss2);
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span set
 * is less than, equal to, or greater than the second one
 * @param[in] ss1,ss2 Span sets
 * @return On error return @p INT_MAX
 * @note Function used for B-tree comparison
 * @csqlfn #Spanset_cmp()
 */
int
spanset_cmp(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_spanset_spanset(ss1, ss2))
    return INT_MAX;

  int count1 = ss1->count;
  int count2 = ss2->count;
  int count = count1 < count2 ? count1 : count2;
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    result = span_cmp(SPANSET_SP_N(ss1, i), SPANSET_SP_N(ss2, i));
    if (result)
      break;
  }
  /* The first count spans of the two span sets are equal */
  if (! result)
  {
    if (count < count1) /* ss1 has more spans than ss2 */
      result = 1;
    else if (count < count2) /* ss2 has more spans than ss1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span set is less than the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Spanset_lt()
 */
inline bool
spanset_lt(const SpanSet *ss1, const SpanSet *ss2)
{
  return spanset_cmp(ss1, ss2) < 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span set is less than or equal to
 * the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Spanset_le()
 */
inline bool
spanset_le(const SpanSet *ss1, const SpanSet *ss2)
{
  return spanset_cmp(ss1, ss2) <= 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span set is greater than or equal to
 * the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Spanset_ge()
 */
inline bool
spanset_ge(const SpanSet *ss1, const SpanSet *ss2)
{
  return spanset_cmp(ss1, ss2) >= 0;
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span set is greater than the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Spanset_gt()
 */
inline bool
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
 * @ingroup meos_setspan_accessor
 * @brief Return the 32-bit hash value of a span set
 * @param[in] ss Span set
 * @return On error return @p INT_MAX
 * @csqlfn #Spanset_hash()
 */
uint32
spanset_hash(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, INT_MAX);
  uint32 result = 1;
  for (int i = 0; i < ss->count; i++)
  {
    uint32 sp_hash = span_hash(SPANSET_SP_N(ss, i));
    result = (result << 5) - result + sp_hash;
  }
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the 64-bit hash value of a span set using a seed
 * @param[in] ss Span set
 * @param[in] seed Seed
 * @return On error return @p INT_MAX
 * @csqlfn #Spanset_hash_extended()
 */
uint64
spanset_hash_extended(const SpanSet *ss, uint64 seed)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, LONG_MAX);
  uint64 result = 1;
  for (int i = 0; i < ss->count; i++)
  {
    uint64 sp_hash = span_hash_extended(SPANSET_SP_N(ss, i), seed);
    result = (result << 5) - result + sp_hash;
  }
  return result;
}

/*****************************************************************************/

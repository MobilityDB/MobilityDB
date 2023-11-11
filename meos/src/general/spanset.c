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
 * @brief General functions for set of disjoint spans.
 */

#include "general/spanset.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "general/type_parser.h"
#include "general/type_util.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Ensure that a spanset value is of a span type
 */
bool
ensure_spanset_has_type(const SpanSet *ss, meosType spansettype)
{
  if (ss->spansettype != spansettype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The span set value must be of type %s", meostype_name(spansettype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the span set values have the same type
 */
bool
ensure_same_spanset_type(const SpanSet *ss1, const SpanSet *ss2)
{
  if (ss1->spansettype != ss2->spansettype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed span set types");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a span set and a span value have the same span type
 */
bool
ensure_same_spanset_span_type(const SpanSet *ss, const Span *s)
{
  if (ss->spantype != s->spantype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed span set and span types");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a span set value has the same base type as the given one
 */
bool
ensure_same_spanset_basetype(const SpanSet *ss, meosType basetype)
{
  if (ss->basetype != basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed span set and base types");
    return false;
  }
  return true;
}

/**
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
tstzspanset_find_timestamp(const SpanSet *ss, TimestampTz t, int *loc)
{
  return spanset_find_value(ss, TimestampTzGetDatum(t), loc);
}
#endif /* MEOS */

/**
 * @brief Return the n-th span of a span set.
 * @pre The argument @p index is less than the number of spans in the span set
 * @note This is the internal function equivalent to `spanset_span_n`.
 * This function does not verify that the index is is in the correct bounds
 */
const Span *
spanset_sp_n(const SpanSet *ss, int index)
{
  assert(ss);
  return &ss->elems[index];
}

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
  assert(str);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return spanset_parse(&str, T_INTSPANSET);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a big integer span from its Well-Known Text (WKT) representation.
 */
SpanSet *
bigintspanset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return spanset_parse(&str, T_BIGINTSPANSET);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation.
 */
SpanSet *
floatspanset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return spanset_parse(&str, T_FLOATSPANSET);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return a period set from its Well-Known Text (WKT) representation.
 */
SpanSet *
tstzspanset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return spanset_parse(&str, T_TSTZSPANSET);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
spanset_out(const SpanSet *ss, int maxdd)
{
  assert(ss);
  /* Ensure validity of the arguments */
  if (! ensure_not_negative(maxdd))
    return NULL;

  char **strings = palloc(sizeof(char *) * ss->count);
  size_t outlen = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = spanset_sp_n(ss, i);
    strings[i] = span_out(s, maxdd);
    outlen += strlen(strings[i]) + 1;
  }
  return stringarr_to_string(strings, ss->count, outlen, "", '{', '}',
    QUOTES_NO, SPACES);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
intspanset_out(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_INTSPANSET))
    return NULL;
  return spanset_out(ss, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
bigintspanset_out(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_BIGINTSPANSET))
    return NULL;
  return spanset_out(ss, 0);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
floatspanset_out(const SpanSet *ss, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_negative(maxdd) ||
      ! ensure_spanset_has_type(ss, T_FLOATSPANSET))
    return NULL;
  return spanset_out(ss, maxdd);
}

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
tstzspanset_out(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return NULL;
  return spanset_out(ss, 0);
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct a span set from an array of disjoint spans enabling the
 * data structure to expand.
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
 * @param[in] maxcount Maximum number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized
 * @param[in] ordered True for verifying that the input spans are ordered
 * @sqlfunc spanset()
 */
SpanSet *
spanset_make_exp(Span *spans, int count, int maxcount, bool normalize,
  bool ordered)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) spans) || ! ensure_positive(count) ||
      ! ensure_less_equal(count, maxcount))
    return NULL;
  /* Test the validity of the spans */
  if (ordered)
  {
    for (int i = 0; i < count - 1; i++)
    {
      int cmp = datum_cmp(spans[i].upper, spans[i + 1].lower, spans[i].basetype);
      if (cmp > 0 ||
        (cmp == 0 && spans[i].upper_inc && spans[i + 1].lower_inc))
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Invalid value for span set");
        return NULL;
      }
    }
  }

  /* Sort the values and remove duplicates */
  Span *newspans = spans;
  int newcount = count;
  if (normalize && count > 1)
    /* Sort the values and remove duplicates */
    newspans = spanarr_normalize(spans, count, true, &newcount);

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
    result->basetype, &result->span);
  /* Copy the span array */
  for (int i = 0; i < newcount; i++)
    result->elems[i] = newspans[i];
  /* Free after normalization */
  if (normalize && count > 1)
    pfree(newspans);
  return result;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Construct a span set from an array of disjoint spans.
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized
 * @sqlfunc spanset()
 */
SpanSet *
spanset_make(Span *spans, int count, bool normalize)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) spans) || ! ensure_positive(count))
    return NULL;
  return spanset_make_exp(spans, count, count, normalize, true);
}

/**
 * @ingroup libmeos_internal_setspan_constructor
 * @brief Construct a span set from an array of spans and free the input array
 * of spans after the creation.
 *
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * @see spanset_make
 */
SpanSet *
spanset_make_free(Span *spans, int count, bool normalize)
{
  assert(spans);
  assert(count >= 0);
  SpanSet *result = spanset_make(spans, count, normalize);
  pfree(spans);
  return result;
}

/**
 * @ingroup libmeos_setspan_constructor
 * @brief Return a copy of a span set.
 */
SpanSet *
spanset_copy(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  SpanSet *result = palloc(VARSIZE(ss));
  memcpy(result, ss, VARSIZE(ss));
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_conversion
 * @brief Convert a value as a span set
 */
SpanSet *
value_to_spanset(Datum d, meosType basetype)
{
  assert(span_basetype(basetype));
  Span s;
  span_set(d, d, true, true, basetype, &s);
  SpanSet *result = spanset_make(&s, 1, NORMALIZE_NO);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert an integer as a span set
 * @sqlop @p ::
 */
SpanSet *
int_to_spanset(int i)
{
  return value_to_spanset(i, T_INT4);
}

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a big integer as a span set
 * @sqlop @p ::
 */
SpanSet *
bigint_to_spanset(int i)
{
  return value_to_spanset(i, T_INT8);
}

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a float as a span set
 * @sqlop @p ::
 */
SpanSet *
float_to_spanset(double d)
{
  return value_to_spanset(d, T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a timestamp as a period set
 * @sqlop @p ::
 */
SpanSet *
timestamptz_to_spanset(TimestampTz t)
{
  return value_to_spanset(t, T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a set as a span set.
 * @sqlop @p ::
 */
SpanSet *
set_to_spanset(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_spantype(s->settype))
    return NULL;

  Span *spans = palloc(sizeof(Span) * s->count);
  for (int i = 0; i < s->count; i++)
  {
    Datum d = SET_VAL_N(s, i);
    span_set(d, d, true, true, s->basetype, &spans[i]);
  }
  return spanset_make_free(spans, s->count, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_setspan_conversion
 * @brief Convert a period as a period set.
 * @sqlop @p ::
 */
SpanSet *
span_to_spanset(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return spanset_make((Span *) s, 1, NORMALIZE_NO);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Transition function for set aggregate of values
 */
SpanSet *
spanset_compact(SpanSet *ss)
{
  assert(ss);
  /* Create the final value reusing the array of spans in the span set */
  SpanSet *result = spanset_make_exp((Span *) &ss->elems, ss->count,
    ss->count, NORMALIZE, ORDERED_NO);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Transform an integer span set to a float span set
 */
SpanSet *
intspanset_floatspanset(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_INTSPANSET))
    return NULL;
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    intspan_set_floatspan(&ss->elems[i], &spans[i]);
  SpanSet *result = spanset_make_free(spans, ss->count, NORMALIZE);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Transform a float span set to an integer span set
 */
SpanSet *
floatspanset_intspanset(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_FLOATSPANSET))
    return NULL;
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    floatspan_set_intspan(&ss->elems[i], &spans[i]);
  SpanSet *result = spanset_make_free(spans, ss->count, NORMALIZE);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Shift a span set by a value.
 * @pre The value is of the same type as the span base type
 * @sqlfunc shift()
 */
void
spanset_shift(SpanSet *ss, Datum shift)
{
  assert(ss);
  for (int i = 0; i < ss->count; i++)
  {
    Span *s = (Span *) spanset_sp_n(ss, i);
    span_shift(s, shift);
  }
  return;
}

/**
 * @ingroup libmeos_internal_setspan_transf
 * @brief Return a number set shifted and/or scaled by the intervals.
 * @sqlfunc shift(), scale(), shiftScale()
 */
SpanSet *
numspanset_shift_scale(const SpanSet *ss, Datum shift, Datum width,
  bool hasshift, bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_numspan_type(ss->spantype) ||
      ! ensure_one_shift_width(hasshift, haswidth) ||
      (haswidth && ! ensure_positive_datum(haswidth, ss->basetype)))
    return NULL;

  /* Copy the input span set to the output span set */
  SpanSet *result = spanset_copy(ss);

  /* Shift and/or scale the bounding span */
  Datum delta;
  double scale;
  numspan_shift_scale1(&result->span, shift, width, hasshift, haswidth,
    &delta, &scale);
  Datum origin = result->span.lower;

  /* Shift and/or scale the periodset */
  for (int i = 0; i < ss->count; i++)
    numspan_delta_scale_iter(&result->elems[i], origin, delta, hasshift,
      scale);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_transf
 * @brief Return an integer span shifted and/or scaled by the values
 * @sqlfunc shift(), scale(), shiftScale()
 */
SpanSet *
intspanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_INTSPANSET))
    return NULL;

  return numspanset_shift_scale(ss, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a big integer span shifted and/or scaled by the values
 * @sqlfunc shift(), scale(), shiftScale()
 */
SpanSet *
bigintspanset_shift_scale(const SpanSet *ss, int64 shift, int64 width,
  bool hasshift, bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_BIGINTSPANSET))
    return NULL;

  return numspanset_shift_scale(ss, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a float span shifted and/or scaled by the values
 * @sqlfunc shift(), scale(), shiftScale()
 */
SpanSet *
floatspanset_shift_scale(const SpanSet *ss, double shift, double width,
  bool hasshift, bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_FLOATSPANSET))
    return NULL;

  return numspanset_shift_scale(ss, Float8GetDatum(shift),
    Float8GetDatum(width), hasshift, haswidth);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_transf
 * @brief Return a timestamptz span set shifted and/or scaled by the intervals.
 * @sqlfunc shift(), scale(), shiftScale()
 */
SpanSet *
tstzspanset_shift_scale(const SpanSet *ss, const Interval *shift,
  const Interval *duration)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Copy the input period set to the output period set */
  SpanSet *result = spanset_copy(ss);

  /* Shift and/or scale the bounding period */
  TimestampTz delta;
  double scale;
  tstzspan_shift_scale1(&result->span, shift, duration, &delta, &scale);
  TimestampTz origin = DatumGetTimestampTz(result->span.lower);
  /* Shift and/or scale the periodset */
  for (int i = 0; i < ss->count; i++)
    tstzspan_delta_scale_iter(&result->elems[i], origin, delta, scale);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Set the precision of the float span set to the number of decimal places.
 */
SpanSet *
floatspanset_round(const SpanSet *ss, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_negative(maxdd) ||
      ! ensure_spanset_has_type(ss, T_FLOATSPANSET))
    return NULL;

  Span *spans = palloc(sizeof(Span) * ss->count);
  Datum size = Int32GetDatum(maxdd);
  for (int i = 0; i < ss->count; i++)
  {
    const Span *span = spanset_sp_n(ss, i);
    floatspan_round_int(span, size, &spans[i]);
  }
  SpanSet *result = spanset_make_free(spans, ss->count, NORMALIZE);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_internal_setspan_accessor
 * @brief Return the size in bytes of a span set
 * @sqlfunc memSize()
 */
int
spanset_mem_size(const SpanSet *ss)
{
  assert(ss);
  return (int) VARSIZE(DatumGetPointer(ss));
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the bounding span of a span set.
 * @sqlfunc span()
 */
Span *
spanset_span(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  Span *result = palloc(sizeof(Span));
  memcpy(result, &ss->span, sizeof(Span));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @return On error return INT_MAX
 * @sqlfunc lower()
 */
int
intspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_INTSPANSET))
    return INT_MAX;
  return DatumGetInt32(ss->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @return On error return INT_MAX
 * @sqlfunc lower()
 */
int
bigintspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_BIGINTSPANSET))
    return INT_MAX;
  return DatumGetInt64(ss->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a float span set
 * @return On error return DBL_MAX
 * @sqlfunc lower()
 */
double
floatspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_FLOATSPANSET))
    return DBL_MAX;
  return Float8GetDatum(ss->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the lower bound of a period set
 * @return On error return DT_NOEND
 * @sqlfunc lower()
 */
TimestampTz
tstzspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return DT_NOEND;
  return TimestampTzGetDatum(ss->elems[0].lower);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @return On error return INT_MAX
 * @sqlfunc upper()
 */
int
intspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_INTSPANSET))
    return INT_MAX;
  return Int32GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @return On error return INT_MAX
 * @sqlfunc upper()
 */
int
bigintspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_BIGINTSPANSET))
    return INT_MAX;
  return Int64GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a float span set
 * @return On error return DBL_MAX
 * @sqlfunc upper()
 */
double
floatspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_FLOATSPANSET))
    return DBL_MAX;
  return Float8GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the upper bound of a period
 * @return On error return DT_NOEND
 * @sqlfunc upper()
 */
TimestampTz
tstzspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return DT_NOEND;
  return TimestampTzGetDatum(ss->elems[ss->count - 1].upper);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the lower bound of a span set is inclusive
 * @sqlfunc lower_inc()
 */
bool
spanset_lower_inc(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return false;
  return ss->elems[0].lower_inc;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return true if the upper bound of a span set is inclusive
 * @sqlfunc upper_inc()
 */
bool
spanset_upper_inc(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return false;
  return ss->elems[ss->count - 1].upper_inc;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the width of a span set as a double.
 * @return On error return -1.0
 * @sqlfunc width()
 */
double
spanset_width(const SpanSet *ss, bool boundspan)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return -1.0;

  if (boundspan)
  {
    Datum lower = spanset_sp_n(ss, 0)->lower;
    Datum upper = spanset_sp_n(ss, ss->count - 1)->upper;
    return distance_value_value(lower, upper, ss->basetype);
  }

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
 * @brief Return the duration of a timestamptz span set
 * @sqlfunc duration()
 */
Interval *
tstzspanset_duration(const SpanSet *ss, bool boundspan)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return NULL;

  if (boundspan)
    return pg_timestamp_mi(ss->span.upper, ss->span.lower);

  const Span *p = spanset_sp_n(ss, 0);
  Interval *result = pg_timestamp_mi(p->upper, p->lower);
  for (int i = 1; i < ss->count; i++)
  {
    p = spanset_sp_n(ss, i);
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
 * @return On error return -1
 * @sqlfunc numTimestamps()
 */
int
tstzspanset_num_timestamps(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return -1;

  const Span *p = spanset_sp_n(ss, 0);
  TimestampTz prev = p->lower;
  bool start = false;
  int result = 1;
  TimestampTz d;
  int i = 1;
  while (i < ss->count || !start)
  {
    if (start)
    {
      p = spanset_sp_n(ss, i++);
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
 * @return On error return DT_NOEND
 * @sqlfunc startTimestamp()
 */
TimestampTz
tstzspanset_start_timestamp(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return DT_NOEND;

  const Span *p = spanset_sp_n(ss, 0);
  return p->lower;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end timestamp of a period set.
 * @return On error return DT_NOEND
 * @sqlfunc endTimestamp()
 */
TimestampTz
tstzspanset_end_timestamp(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return DT_NOEND;

  const Span *p = spanset_sp_n(ss, ss->count - 1);
  return p->upper;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Compute the n-th timestamp of a period set
 * @param[in] ss Period set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the timestamp is found
 * @note It is assumed that n is 1-based
 * @sqlfunc timestampN()
 */
bool
tstzspanset_timestamp_n(const SpanSet *ss, int n, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) result) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return false;

  int pernum = 0;
  const Span *p = spanset_sp_n(ss, pernum);
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
      if (pernum == ss->count)
        break;

      p = spanset_sp_n(ss, pernum);
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
 * @brief Return the array of timestamps of a period set
 * @return On error return NULL
 * @sqlfunc timestamps()
 */
TimestampTz *
tstzspanset_timestamps(const SpanSet *ss, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) count) ||
      ! ensure_spanset_has_type(ss, T_TSTZSPANSET))
    return NULL;

  TimestampTz *result = palloc(sizeof(TimestampTz) * 2 * ss->count);
  const Span *p = spanset_sp_n(ss, 0);
  result[0] = p->lower;
  int ntimes = 1;
  if (p->lower != p->upper)
    result[ntimes++] = p->upper;
  for (int i = 1; i < ss->count; i++)
  {
    p = spanset_sp_n(ss, i);
    if (result[ntimes - 1] != DatumGetTimestampTz(p->lower))
      result[ntimes++] = p->lower;
    if (result[ntimes - 1] != DatumGetTimestampTz(p->upper))
      result[ntimes++] = p->upper;
  }
  *count = ntimes;
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the number of spans of a span set
 * @return On error return -1
 * @sqlfunc numSpans()
 */
int
spanset_num_spans(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return -1;
  return ss->count;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the start span of a span set
 * @return On error return NULL
 * @sqlfunc startSpan()
 */
Span *
spanset_start_span(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  Span *result = span_copy(spanset_sp_n(ss, 0));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the end span of a span set
 * @return On error return NULL
 * @sqlfunc endSpan()
 */
Span *
spanset_end_span(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  Span *result = span_copy(spanset_sp_n(ss, ss->count - 1));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the n-th span of a span set
 * @sqlfunc spanN()
 */
Span *
spanset_span_n(const SpanSet *ss, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  Span *result = NULL;
  if (i >= 1 && i <= ss->count)
    result = span_copy(spanset_sp_n(ss, i - 1));
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the spans of a span set.
 * @return On error return NULL
 * @sqlfunc spans()
 */
const Span **
spanset_spans(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  const Span **spans = palloc(sizeof(Span *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    spans[i] = spanset_sp_n(ss, i);
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
 */
bool
spanset_eq(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;

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
 * @return On error return INT_MAX
 * @note Function used for B-tree comparison
 * @sqlfunc spanset_cmp()
 */
int
spanset_cmp(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return INT_MAX;

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
 * @return On error return INT_MAX
 * @sqlfunc spanset_hash()
 */
uint32
spanset_hash(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return INT_MAX;

  uint32 result = 1;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *p = spanset_sp_n(ss, i);
    uint32 per_hash = span_hash(p);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_accessor
 * @brief Return the 64-bit hash value of a span set using a seed
 * @return On error return INT_MAX
 * @sqlfunc spanset_hash_extended()
 */
uint64
spanset_hash_extended(const SpanSet *ss, uint64 seed)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return INT_MAX;

  uint64 result = 1;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *p = spanset_sp_n(ss, i);
    uint64 per_hash = span_hash_extended(p, seed);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/*****************************************************************************/

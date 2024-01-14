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
 * @brief General functions for set of disjoint spans
 */

#include "general/spanset.h"

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
#include "general/span.h"
#include "general/temporal.h"
#include "general/type_parser.h"
#include "general/type_out.h"
#include "general/type_util.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a span set is of a given span set type
 */
bool
ensure_spanset_isof_type(const SpanSet *ss, meosType spansettype)
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
 * @brief Ensure that a span set is of a given base type
 */
bool
ensure_spanset_isof_basetype(const SpanSet *ss, meosType basetype)
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

#if MEOS
bool
tstzspanset_find_timestamptz(const SpanSet *ss, TimestampTz t, int *loc)
{
  return spanset_find_value(ss, TimestampTzGetDatum(t), loc);
}
#endif /* MEOS */

#ifdef DEBUG_BUILD
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

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
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
 * @ingroup meos_setspan_inout
 * @brief Return a big integer span from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
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
 * @ingroup meos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
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
 * @ingroup meos_setspan_inout
 * @brief Return a date set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
 */
SpanSet *
datespanset_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return spanset_parse(&str, T_DATESPANSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a timestamptz set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
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
  /* Ensure validity of the arguments */
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

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
intspanset_out(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_INTSPANSET))
    return NULL;
  return spanset_out(ss, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
bigintspanset_out(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_BIGINTSPANSET))
    return NULL;
  return spanset_out(ss, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set
 * @param[in] ss Span set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Spanset_out()
 */
char *
floatspanset_out(const SpanSet *ss, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_negative(maxdd) ||
      ! ensure_spanset_isof_type(ss, T_FLOATSPANSET))
    return NULL;
  return spanset_out(ss, maxdd);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
datespanset_out(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return NULL;
  return spanset_out(ss, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
tstzspanset_out(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  return spanset_out(ss, 0);
}
#endif /* MEOS */

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
 * @param[in] ordered True when the input spans are ordered
 */
SpanSet *
spanset_make_exp(Span *spans, int count, int maxcount, bool normalize,
  bool ordered)
{
  assert(spans); assert(count > 0); assert(count <= maxcount);
  /* Test the validity of the spans */
  if (ordered)
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
        pfree(str1); pfree(str2);
        return NULL;
      }
    }
  }

  /* Sort the values and remove duplicates */
  Span *newspans = spans;
  int newcount = count;
  if (normalize && count > 1)
    /* Sort the values and remove duplicates */
    newspans = spanarr_normalize(spans, count, ordered, &newcount);

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
 * @param[in] normalize True if the resulting value should be normalized
 * @param[in] ordered True if the input spans are ordered
 * @csqlfn #Spanset_constructor()
 */
SpanSet *
spanset_make(Span *spans, int count, bool normalize, bool ordered)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) spans) || ! ensure_positive(count))
    return NULL;
  return spanset_make_exp(spans, count, count, normalize, ordered);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a span set from an array of spans and free the input array
 * of spans after the creation
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * @param[in] ordered True if the input spans are ordered
 * @see #spanset_make
 */
SpanSet *
spanset_make_free(Span *spans, int count, bool normalize, bool ordered)
{
  assert(spans); assert(count >= 0);
  SpanSet *result = NULL;
  if (count > 0)
    result = spanset_make_exp(spans, count, count, normalize, ordered);
  pfree(spans);
  return result;
}

/**
 * @ingroup meos_internal_setspan_constructor
 * @brief Return a copy of a span set
 * @param[in] ss Span set
 */
SpanSet *
spanset_cp(const SpanSet *ss)
{
  assert(ss);
  SpanSet *result = palloc(VARSIZE(ss));
  memcpy(result, ss, VARSIZE(ss));
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_constructor
 * @brief Return a copy of a span set
 * @param[in] ss Span set
 */
SpanSet *
spanset_copy(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return spanset_cp(ss);
}
#endif /* MEOS */

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a value converted to a span set
 * @param[in] value Value
 * @param[in] basetype Type of the value
 */
SpanSet *
value_to_spanset(Datum value, meosType basetype)
{
  assert(span_basetype(basetype));
  meosType spantype = basetype_spantype(basetype);
  Span s;
  span_set(value, value, true, true, basetype, spantype, &s);
  return spanset_make_exp(&s, 1, 1, NORMALIZE_NO, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return an integer converted to a span set
 * @param[in] i Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
int_to_spanset(int i)
{
  return value_to_spanset(i, T_INT4);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a big integer converted to a span set
 * @param[in] i Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
bigint_to_spanset(int i)
{
  return value_to_spanset(i, T_INT8);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a float converted to a span set
 * @param[in] d Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
float_to_spanset(double d)
{
  return value_to_spanset(d, T_FLOAT8);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a date converted to a span set
 * @param[in] d Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
date_to_spanset(DateADT d)
{
  return value_to_spanset(d, T_DATE);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_conversion
 * @brief Return a timestamptz converted to a span set
 * @param[in] t Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
timestamptz_to_spanset(TimestampTz t)
{
  return value_to_spanset(t, T_TIMESTAMPTZ);
}

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a set converted to a span set
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
  return spanset_make_free(spans, s->count, NORMALIZE, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a set converted to a span set
 * @param[in] s Set
 * @csqlfn #Set_to_spanset()
 */
SpanSet *
set_to_spanset(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_spantype(s->settype))
    return NULL;
  return set_spanset(s);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief return a span converted to a span set
 * @param[in] s Span
 * @csqlfn #Span_to_spanset()
 */
SpanSet *
span_spanset(const Span *s)
{
  assert(s);
  return spanset_make_exp((Span *) s, 1, 1, NORMALIZE_NO, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a span converted to a span set
 * @param[in] s Span
 * @csqlfn #Spanset_to_span()
 */
SpanSet *
span_to_spanset(const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s))
    return NULL;
  return span_spanset(s);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return an integer span set converted to a float span set
 * @param[in] ss Span set
 * @csqlfn #Intspanset_to_floatspanset()
 */
SpanSet *
intspanset_floatspanset(const SpanSet *ss)
{
  assert(ss);
  /* Ensure validity of the arguments */
  if (! ensure_spanset_isof_type(ss, T_INTSPANSET))
    return NULL;
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    intspan_set_floatspan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return an integer span set converted to a float span set
 * @param[in] ss Span set
 * @csqlfn #Intspanset_to_floatspanset()
 */
SpanSet *
intspanset_to_floatspanset(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return intspanset_floatspanset(ss);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a float span set converted to an integer span set
 * @param[in] ss Span set
 * @csqlfn #Floatspanset_to_intspanset()
 */
SpanSet *
floatspanset_intspanset(const SpanSet *ss)
{
  assert(ss);
  /* Ensure validity of the arguments */
  if (! ensure_spanset_isof_type(ss, T_FLOATSPANSET))
    return NULL;
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    floatspan_set_intspan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a float span set converted to an integer span set
 * @param[in] ss Span set
 * @csqlfn #Floatspanset_to_intspanset()
 */
SpanSet *
floatspanset_to_intspanset(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return floatspanset_intspanset(ss);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a date span set converted to a timestamptz span set
 * @param[in] ss Span set
 * @csqlfn #Datespanset_to_tstzspanset()
 */
SpanSet *
datespanset_tstzspanset(const SpanSet *ss)
{
  assert(ss);
  /* Ensure validity of the arguments */
  if (! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return NULL;
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    datespan_set_tstzspan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDERED);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a date span set converted to a timestamptz span set
 * @param[in] ss Span set
 * @csqlfn #Datespanset_to_tstzspanset()
 */
SpanSet *
datespanset_to_tstzspanset(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return datespanset_tstzspanset(ss);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_conversion
 * @brief Return a timestamptz span set converted to a date span set
 * @param[in] ss Span set
 * @csqlfn #Tstzspanset_to_datespanset()
 */
SpanSet *
tstzspanset_datespanset(const SpanSet *ss)
{
  assert(ss);
  /* Ensure validity of the arguments */
  if (! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
    tstzspan_set_datespan(SPANSET_SP_N(ss, i), &spans[i]);
  return spanset_make_free(spans, ss->count, NORMALIZE, ORDERED_NO);
}

#if MEOS
/**
 * @ingroup meos_setspan_conversion
 * @brief Return a timestamptz span set converted to a date span set
 * @param[in] ss Span set
 * @csqlfn #Tstzspanset_to_datespanset()
 */
SpanSet *
tstzspanset_to_datespanset(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return tstzspanset_datespanset(ss);
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the size in bytes of a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_mem_size()
 */
int
spanset_mem_size(const SpanSet *ss)
{
  assert(ss);
  return (int) VARSIZE(DatumGetPointer(ss));
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return span_cp(&ss->span);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return (Datum) 0;
  return ss->elems[0].lower;
}

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @param[in] ss Span set
 * @return On error return @p INT_MAX
 * @csqlfn #Spanset_lower()
 */
int
intspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_INTSPANSET))
    return INT_MAX;
  return DatumGetInt32(ss->elems[0].lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @param[in] ss Span set
 * @return On error return LONG_MAX
 * @csqlfn #Spanset_lower()
 */
int64
bigintspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_BIGINTSPANSET))
    return LONG_MAX;
  return DatumGetInt64(ss->elems[0].lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a float span set
 * @param[in] ss Span set
 * @return On error return @p DBL_MAX
 * @csqlfn #Spanset_lower()
 */
double
floatspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_FLOATSPANSET))
    return DBL_MAX;
  return Float8GetDatum(ss->elems[0].lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a timestamptz span set
 * @param[in] ss Span set
 * @return On error return DT_NOEND
 * @csqlfn #Spanset_lower()
 */
TimestampTz
tstzspanset_lower(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_timespanset_type(ss->spansettype))
    return DT_NOEND;
  return TimestampTzGetDatum(ss->elems[0].lower);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return the lower bound a span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_upper()
 */
Datum
spanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return (Datum) 0;
  return ss->elems[ss->count - 1].upper;
}

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @param[in] ss Span set
 * @return On error return @p INT_MAX
 * @csqlfn #Spanset_upper()
 */
int
intspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_INTSPANSET))
    return INT_MAX;
  return Int32GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @param[in] ss Span set
 * @return On error return LONG_MAX
 * @csqlfn #Spanset_upper()
 */
int64
bigintspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_BIGINTSPANSET))
    return LONG_MAX;
  return Int64GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a float span set
 * @param[in] ss Span set
 * @return On error return @p DBL_MAX
 * @csqlfn #Spanset_upper()
 */
double
floatspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_FLOATSPANSET))
    return DBL_MAX;
  return Float8GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a timestamptz span set
 * @param[in] ss Span set
 * @return On error return DT_NOEND
 * @csqlfn #Spanset_upper()
 */
TimestampTz
tstzspanset_upper(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_timespanset_type(ss->spansettype))
    return DT_NOEND;
  return TimestampTzGetDatum(ss->elems[ss->count - 1].upper);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_accessor
 * @brief Return true if the lower bound of a span set is inclusive
 * @param[in] ss Span set
 * @csqlfn #Spanset_lower_inc()
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
 * @ingroup meos_setspan_accessor
 * @brief Return true if the upper bound of a span set is inclusive
 * @param[in] ss Span set
 * @csqlfn #Spanset_upper_inc()
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

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of an integer span set
 * @param[in] ss Span
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return -1
 * @csqlfn #Numspanset_width(()
 */
int
intspanset_width(const SpanSet *ss, bool boundspan)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_INTSPANSET))
    return -1;
  return Int32GetDatum(numspanset_width(ss, boundspan));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of an integer span set
 * @param[in] ss Span
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return -1
 * @csqlfn #Numspanset_width(()
 */
int64
bigintspanset_width(const SpanSet *ss, bool boundspan)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_BIGINTSPANSET))
    return -1;
  return Int64GetDatum(numspanset_width(ss, boundspan));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of a float span set
 * @param[in] ss Span
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return -1
 * @csqlfn #Numspanset_width(()
 */
double
floatspanset_width(const SpanSet *ss, bool boundspan)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_FLOATSPANSET))
    return -1.0;
  return DatumGetFloat8(numspanset_width(ss, boundspan));
}
#endif /* MEOS */

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return NULL;

  if (boundspan)
    return minus_date_date(ss->span.upper, ss->span.lower);

  int nodays = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = SPANSET_SP_N(ss, i);
    nodays += (int32) (s->upper - s->lower);
  }
  Interval *result = palloc0(sizeof(Interval));
  result->day = nodays;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return -1;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return DATEVAL_NOEND;
  const Span *s = SPANSET_SP_N(ss, 0);
  return DatumGetDateADT(s->lower);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_timespanset_type(ss->spansettype))
    return DATEVAL_NOEND;
  const Span *s = SPANSET_SP_N(ss, ss->count - 1);
  return DatumGetDateADT(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized to the n-th date of a span set
 * @param[in] ss Span set
 * @param[in] n Number
 * @param[out] result Date
 * @result Return true if the date is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Datespanset_date_n()
 */
bool
datespanset_date_n(const SpanSet *ss, int n, DateADT *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) result) ||
      ! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return false;
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
 * @brief Return the array of dates of a span set
 * @param[in] ss Span set
 * @param[out] count Number of values of the resulting array
 * @return On error return @p NULL
 * @csqlfn #Datespanset_dates()
 */
DateADT *
datespanset_dates(const SpanSet *ss, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) count) ||
      ! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return NULL;

  DateADT *result = palloc(sizeof(DateADT) * 2 * ss->count);
  int ndates = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = SPANSET_SP_N(ss, i);
    result[ndates++] = DatumGetDateADT(s->lower);
    result[ndates++] = DatumGetDateADT(s->upper);
  }
  *count = ndates;
  return result;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_timespanset_type(ss->spansettype))
    return -1;

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_timespanset_type(ss->spansettype))
    return DT_NOEND;

  const Span *s = SPANSET_SP_N(ss, 0);
  return DatumGetTimestampTz(s->lower);
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_timespanset_type(ss->spansettype))
    return DT_NOEND;

  const Span *s = SPANSET_SP_N(ss, ss->count - 1);
  return DatumGetTimestampTz(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the last argument initialized with the n-th timestamptz of a
 * span set
 * @param[in] ss Span set
 * @param[in] n Number
 * @param[out] result Timestamptz
 * @result Return true if the timestamptz is found
 * @note It is assumed that n is 1-based
 * @csqlfn #Tstzspanset_timestamptz_n()
 */
bool
tstzspanset_timestamptz_n(const SpanSet *ss, int n, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) result) ||
      ! ensure_timespanset_type(ss->spansettype))
    return false;

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
 * @brief Return the array of timestamps of a span set
 * @param[in] ss Span set
 * @param[out] count Number of values of the resulting array
 * @return On error return @p NULL
 * @csqlfn #Tstzspanset_timestamps()
 */
TimestampTz *
tstzspanset_timestamps(const SpanSet *ss, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) count) ||
      ! ensure_timespanset_type(ss->spansettype))
    return NULL;

  TimestampTz *result = palloc(sizeof(TimestampTz) * 2 * ss->count);
  const Span *s = SPANSET_SP_N(ss, 0);
  result[0] = s->lower;
  int ntimes = 1;
  if (s->lower != s->upper)
    result[ntimes++] = s->upper;
  for (int i = 1; i < ss->count; i++)
  {
    s = SPANSET_SP_N(ss, i);
    if (result[ntimes - 1] != DatumGetTimestampTz(s->lower))
      result[ntimes++] = s->lower;
    if (result[ntimes - 1] != DatumGetTimestampTz(s->upper))
      result[ntimes++] = s->upper;
  }
  *count = ntimes;
  return result;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return -1;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return span_cp(SPANSET_SP_N(ss, 0));
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;
  return span_cp(SPANSET_SP_N(ss, ss->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the n-th span of a span set
 * @param[in] ss Span set
 * @param[in] i Index
 * @csqlfn #Spanset_span_n()
 */
Span *
spanset_span_n(const SpanSet *ss, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  if (i >= 1 && i <= ss->count)
    return span_cp(SPANSET_SP_N(ss, i - 1));
  return NULL;
}

/**
 * @ingroup meos_internal_setspan_accessor
 * @brief Return an array of pointers to the spans of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 */
const Span **
spanset_sps(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  const Span **spans = palloc(sizeof(Span *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    spans[i] = SPANSET_SP_N(ss, i);
  return spans;
}

#if MEOS
/**
 * @ingroup meos_setspan_accessor
 * @brief Return an array copies of the spans of a span set
 * @param[in] ss Span set
 * @return On error return @p NULL
 * @csqlfn #Spanset_spans()
 */
Span **
spanset_spans(const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return NULL;

  Span **spans = palloc(sizeof(Span *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    spans[i] = span_cp(SPANSET_SP_N(ss, i));
  return spans;
}
#endif /* MEOS */

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

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
    NORMALIZE, ORDERED);
}
#endif /* MEOS */

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
  /* Ensure validity of the arguments */
  if (! ensure_one_true(hasshift, haswidth) ||
      (haswidth && ! ensure_positive_datum(haswidth, ss->basetype)))
    return NULL;

  /* Copy the input span set to the output span set */
  SpanSet *result = spanset_cp(ss);

  /* Shift and/or scale the bounding span */
  Datum delta;
  double scale;
  numspan_shift_scale1(&result->span, shift, width, hasshift, haswidth,
    &delta, &scale);
  Datum origin = result->span.lower;

  /* Shift and/or scale the span set */
  for (int i = 0; i < ss->count; i++)
    numspan_delta_scale_iter(&result->elems[i], origin, delta, hasshift,
      scale);
  return result;
}

#if MEOS
/**
 * @ingroup meos_setspan_transf
 * @brief Return an integer span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
intspanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_INTSPANSET))
    return NULL;
  return numspanset_shift_scale(ss, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a big integer span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
bigintspanset_shift_scale(const SpanSet *ss, int64 shift, int64 width,
  bool hasshift, bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_BIGINTSPANSET))
    return NULL;
  return numspanset_shift_scale(ss, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
floatspanset_shift_scale(const SpanSet *ss, double shift, double width,
  bool hasshift, bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_FLOATSPANSET))
    return NULL;
  return numspanset_shift_scale(ss, Float8GetDatum(shift),
    Float8GetDatum(width), hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a date span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
datespanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_DATESPANSET))
    return NULL;
  return numspanset_shift_scale(ss, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}
#endif /* MEOS */

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_type(ss, T_TSTZSPANSET) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Copy the input span set to the output span set */
  SpanSet *result = spanset_cp(ss);

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
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_comp
 * @brief Return true if the two span sets are equal
 * @param[in] ss1,ss2 Span sets
 */
bool
spanset_eq_int(const SpanSet *ss1, const SpanSet *ss2)
{
  assert(ss1); assert(ss2); assert(ss1->spansettype == ss2->spansettype);
  if (ss1->count != ss2->count)
    return false;
  /* ss1 and ss2 have the same number of spans */
  for (int i = 0; i < ss1->count; i++)
    if (! span_eq_int(SPANSET_SP_N(ss1, i), SPANSET_SP_N(ss2, i)))
      return false;
  /* All spans of the two span sets are equal */
  return true;
}

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;
  return spanset_eq_int(ss1, ss2);
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span set is different from the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Spanset_ne()
 */
bool
spanset_ne(const SpanSet *ss1, const SpanSet *ss2)
{
  return ! spanset_eq(ss1, ss2);
}

/**
 * @ingroup meos_internal_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span set
 * is less than, equal, or greater than the second one
 * @param[in] ss1,ss2 Span sets
 */
int
spanset_cmp_int(const SpanSet *ss1, const SpanSet *ss2)
{
  assert(ss1); assert(ss2); assert(ss1->spansettype == ss2->spansettype);

  int count1 = ss1->count;
  int count2 = ss2->count;
  int count = count1 < count2 ? count1 : count2;
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    result = span_cmp_int(SPANSET_SP_N(ss1, i), SPANSET_SP_N(ss2, i));
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
 * @brief Return -1, 0, or 1 depending on whether the first span set
 * is less than, equal, or greater than the second one
 * @param[in] ss1,ss2 Span sets
 * @return On error return @p INT_MAX
 * @note Function used for B-tree comparison
 * @csqlfn #Spanset_cmp()
 */
int
spanset_cmp(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return INT_MAX;
  return spanset_cmp_int(ss1, ss2);
}

/**
 * @ingroup meos_setspan_comp
 * @brief Return true if the first span set is less than the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Spanset_lt()
 */
bool
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
bool
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
bool
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
 * @ingroup meos_setspan_accessor
 * @brief Return the 32-bit hash value of a span set
 * @param[in] ss Span set
 * @return On error return @p INT_MAX
 * @csqlfn #Spanset_hash()
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss))
    return INT_MAX;

  uint64 result = 1;
  for (int i = 0; i < ss->count; i++)
  {
    uint64 sp_hash = span_hash_extended(SPANSET_SP_N(ss, i), seed);
    result = (result << 5) - result + sp_hash;
  }
  return result;
}

/*****************************************************************************/

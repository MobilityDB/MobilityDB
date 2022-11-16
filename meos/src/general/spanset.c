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
#include "general/pg_call.h"
#include "general/span.h"
#include "general/temporal_util.h"
#include "general/time_ops.h"
#include "general/temporal_parser.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a span set from its Well-Known Text (WKT) representation.
 */
SpanSet *
spanset_in(const char *str, mobdbType spansettype)
{
  return spanset_parse(&str, spansettype);
}

#if MEOS
/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return an integer span from its Well-Known Text (WKT) representation.
 */
Span *
intspanset_in(const char *str)
{
  return spanset_parse(&str, T_INTSPANSET);
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a float span from its Well-Known Text (WKT) representation.
 */
Span *
floatspanset_in(const char *str)
{
  return spanset_parse(&str, T_FLOATSPANSET);
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a period set from its Well-Known Text (WKT) representation.
 */
Period *
periodset_in(const char *str)
{
  return spanset_parse(&str, T_PERIODSET);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
spanset_out(const SpanSet *ss, Datum maxdd)
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
 * @ingroup libmeos_spantime_in_out
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
floatspanset_out(const SpanSet *ss, int maxdd)
{
  return spanset_out(s, Int32GetDatum(maxdd));
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
intspanset_out(const SpanSet *ss)
{
  return spanset_out(s, Int32GetDatum(0));
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return the Well-Known Text (WKT) representation of a span set.
 */
char *
periodset_out(const SpanSet *ss)
{
  return spanset_out(s, Int32GetDatum(0));
}
#endif /* MEOS */

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_accessor
 * @brief Return the n-th span of a span set.
 * @pre The argument @s index is less than the number of spans in the span set
 * @note This is the internal function equivalent to `spanset_span_n`.
 * This function does not verify that the index is is in the correct bounds
 */
const Span *
spanset_sp_n(const SpanSet *ss, int index)
{
  return (Span *) &ss->elems[index];
}

/**
 * @brief Get the span type of a span set.
 */
mobdbType
spanset_spantype(const SpanSet *ss)
{
  return ss->elems[0].spantype;
}

/**
 * @brief Get the span type of a span set.
 */
mobdbType
spanset_basetype(const SpanSet *ss)
{
  return ss->elems[0].basetype;
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup libmeos_spantime_constructor
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
    newspans = spanarr_normalize((Span **) spans, count, SORT_NO,
      &newcount);
  /* Notice that the first span is already declared in the struct */
  size_t memsize = double_pad(sizeof(SpanSet)) +
    double_pad(sizeof(Span)) * (newcount - 1);
  SpanSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  mobdbType spantype = spans[0]->spantype;
  mobdbType basetype = spans[0]->basetype;
  result->spansettype = spantype_spansettype(spantype);
  result->count = newcount;

  /* Compute the bounding span */
  span_set(newspans[0]->lower, newspans[newcount - 1]->upper,
    newspans[0]->lower_inc, newspans[newcount - 1]->upper_inc, basetype,
    &result->span);
  /* Copy the span array */
  for (int i = 0; i < newcount; i++)
    memcpy(&result->elems[i], newspans[i], sizeof(Span));
  /* Free after normalization */
  if (normalize && count > 1)
    pfree_array((void **) newspans, newcount);
  return result;
}

/**
 * @ingroup libmeos_spantime_constructor
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
 * @ingroup libmeos_spantime_constructor
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
 * @ingroup libmeos_spantime_cast
 * @brief Cast a period as a period set.
 * @sqlop @p ::
 */
SpanSet *
span_to_spanset(const Span *s)
{
  return spanset_make((const Span **) &s, 1, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_temporal_cast
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

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the size in bytes of a period set
 * @sqlfunc memSize()
 */
int
spanset_mem_size(const SpanSet *ss)
{
  return (int) VARSIZE(DatumGetPointer(ss));
}

/**
 * @ingroup libmeos_spantime_accessor
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
 * @ingroup libmeos_spantime_accessor
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
 * @ingroup libmeos_spantime_accessor
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
 * @ingroup libmeos_spantime_accessor
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
 * @ingroup libmeos_spantime_accessor
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

/*****************************************************************************/

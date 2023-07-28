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
 * @brief Aggregate functions for span types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/spanset.h"
#include "general/type_util.h"

/*****************************************************************************
 * Aggregate functions for span set types
 *****************************************************************************/

/**
 * @brief Append a span to an unordered span set.
 * @param[in,out] ss Span set
 * @param[in] span Span to append
 * @param[in] expand True when using expandable structures
 */
static SpanSet *
spanset_append_span(SpanSet *ss, const Span *span, bool expand)
{
  assert(ss->spantype == span->spantype);

  /* Account for expandable structures */
  if (expand && ss->count < ss->maxcount)
  {
    /* There is enough space to add the new span */
    ss->elems[ss->count++] = *span;
    /* Expand the bounding box and return */
    span_expand(span, &ss->span);
    return ss;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  for (int i = 0; i < ss->count; i++)
    spans[i] = *spanset_sp_n(ss, i);
  spans[ss->count] = *span;
  int maxcount = ss->maxcount * 2;
#ifdef DEBUG_BUILD
  printf(" spanset -> %d\n", maxcount);
#endif /* DEBUG_BUILD */

  SpanSet *result = spanset_make_exp(spans, ss->count + 1, maxcount,
    NORMALIZE_NO, ORDERED_NO);
  pfree(spans);
  // pfree(ss);
  return result;
}

/**
 * @brief Append a span set to an unordered span set.
 * @param[in,out] ss1 Span set
 * @param[in] ss2 Span set to append
 * @param[in] expand True when using expandable structures
 */
static SpanSet *
spanset_append_spanset(SpanSet *ss1, const SpanSet *ss2, bool expand)
{
  assert(ss1->spantype == ss2->spantype);

  /* Account for expandable structures */
  if (expand && ss1->count + ss2->count <= ss1->maxcount)
  {
    for (int i = 0; i < ss2->count; i++)
    {
      /* There is enough space to add the new span set */
      ss1->elems[ss1->count++] = ss2->elems[i];
      /* Expand the bounding box and return */
      span_expand(&ss2->elems[i], &ss1->span);
    }
    return ss1;
  }

  /* This is the first time we use an expandable structure or there is no more
   * free space */
  int count = ss1->count + ss2->count;
  Span *spans = palloc(sizeof(Span) * count);
  for (int i = 0; i < ss1->count; i++)
    spans[i] = *spanset_sp_n(ss1, i);
  for (int i = 0; i < ss2->count; i++)
    spans[i + ss1->count] = *spanset_sp_n(ss2, i);
  int maxcount = ss1->maxcount * 2;
  while (maxcount < count)
    maxcount *= 2;
#ifdef DEBUG_BUILD
  printf(" spanset -> %d\n", maxcount);
#endif /* DEBUG_BUILD */

  SpanSet *result = spanset_make_exp(spans, count, maxcount, NORMALIZE_NO,
    ORDERED_NO);
  pfree(spans);
  // pfree(ss);
  return result;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span set aggregate union
 */
SpanSet *
span_union_transfn(SpanSet *state, const Span *span)
{
  /* Null set: create a new span set with the input span */
  if (! state)
    /* Arbitrary initialization to 64 elements */
    return spanset_make_exp((Span *) span, 1, 64, NORMALIZE_NO, ORDERED_NO);

  return spanset_append_span(state, span, true);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span set aggregate union
 */
SpanSet *
spanset_union_transfn(SpanSet *state, const SpanSet *ss)
{
  /* Null set: create a new span set with the input span set */
  if (! state)
  {
    int count = ((ss->count / 64) + 1) * 64;
    /* Arbitrary initialization to next multiple of 64 elements */
    return spanset_make_exp((Span *) &ss->elems, ss->count, count,
      NORMALIZE_NO, ORDERED_NO);
  }

  return spanset_append_spanset(state, ss, true);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for set aggregate of values
 */
SpanSet *
spanset_union_finalfn(SpanSet *state)
{
  if (! state)
    return NULL;

  /* Create the final value reusing the array of spans in the state */
  SpanSet *result = spanset_make_exp((Span *) &state->elems, state->count,
    state->count, NORMALIZE, ORDERED_NO);
  return result;
}

/*****************************************************************************/

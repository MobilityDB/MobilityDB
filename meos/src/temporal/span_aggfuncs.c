/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Aggregate functions for span types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/spanset.h"

/*****************************************************************************
 * Extent
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_agg
 * @brief Transition function for span extent aggregate of values
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] value Value to aggregate
 * @param[in] basetype Type of the value
 */
Span *
spanbase_extent_transfn(Span *state, Datum value, MeosType basetype)
{
  /* Null span: return the span of the base value */
  if (! state)
    return span_make(value, value, true, true, basetype);

  Span s1;
  span_set(value, value, true, true, state->basetype, state->spantype, &s1);
  span_expand(&s1, state);
  return state;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of sets
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] s Set to aggregate, may be `NULL`
 * @csqlfn #Set_extent_transfn()
 */
Span *
set_extent_transfn(Span *state, const Set *s)
{
  /* Can't do anything with null inputs */
  if (! state && ! s)
    return NULL;
  /* Null period and non-null set: return the bbox of the timestamp set */
  if (! state)
    return set_span(s);
  /* Non-null period and null set: return the period */
  if (! s)
    return state;

  /* Ensure the validity of the arguments */
  if (! ensure_set_spantype(s->settype) ||
      ! ensure_span_isof_basetype(state, s->basetype))
    return NULL;

  Span s1;
  set_set_span(s, &s1);
  span_expand(&s1, state);
  return state;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of spans
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] s Span to aggregate, may be `NULL`
 * @note The function is also the combine function of the aggregate, the
 * span to aggregate being the state of another partial aggregation
 * @csqlfn #Span_extent_transfn(), #Span_extent_combinefn()
 */
Span *
span_extent_transfn(Span *state, const Span *s)
{
  /* Can't do anything with null inputs */
  if (! state && ! s)
    return NULL;
  /* Null span and non-null span, return the span */
  if (! state)
    return span_copy(s);
  /* Non-null span and null span, return the span */
  if (! s)
    return state;

  /* Ensure the validity of the arguments */
  if (! ensure_same_span_type(state, s))
    return NULL;

  span_expand(s, state);
  return state;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span extent aggregate of span sets
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] ss Span set to aggregate, may be `NULL`
 * @csqlfn #Spanset_extent_transfn()
 */
Span *
spanset_extent_transfn(Span *state, const SpanSet *ss)
{
  /* Can't do anything with null inputs */
  if (! state && ! ss)
    return NULL;
  /* Null  and non-null span set, return the bbox of the span set */
  if (! state)
    return span_copy(&ss->span);
  /* Non-null span and null temporal, return the span */
  if (! ss)
    return state;

  /* Ensure the validity of the arguments */
  if (! ensure_same_spanset_span_type(ss, state))
    return NULL;

  span_expand(&ss->span, state);
  return state;
}

/*****************************************************************************
 * Aggregate functions for span set types
 *****************************************************************************/

/**
 * @brief Append a span to an unordered span set
 * @param[in,out] ss Span set
 * @param[in] span Span to append
 * @param[in] expand True when using expandable structures
 */
static SpanSet *
spanset_append_span(SpanSet *ss, const Span *span, bool expand)
{
  assert(ss); assert(span);
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
    spans[i] = *SPANSET_SP_N(ss, i);
  spans[ss->count] = *span;
  int maxcount = ss->maxcount * 2;
#ifdef DEBUG_EXPAND
  meos_error(WARNING, " Spanset -> %d\n", maxcount);
#endif /* DEBUG_EXPAND */

  SpanSet *result = spanset_make_exp(spans, ss->count + 1, maxcount,
    NORMALIZE_NO, ORDER);
  pfree(spans); pfree(ss);
  return result;
}

/**
 * @brief Append a span set to an unordered span set
 * @param[in,out] ss1 Span set
 * @param[in] ss2 Span set to append
 * @param[in] expand True when using expandable structures
 */
static SpanSet *
spanset_append_spanset(SpanSet *ss1, const SpanSet *ss2, bool expand)
{
  assert(ss1); assert(ss2);
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
    spans[i] = *SPANSET_SP_N(ss1, i);
  for (int i = 0; i < ss2->count; i++)
    spans[i + ss1->count] = *SPANSET_SP_N(ss2, i);
  int maxcount = ss1->maxcount * 2;
  while (maxcount < count)
    maxcount *= 2;
#ifdef DEBUG_EXPAND
  meos_error(WARNING, " Spanset -> %d\n", maxcount);
#endif /* DEBUG_EXPAND */

  SpanSet *result = spanset_make_exp(spans, count, maxcount, NORMALIZE_NO,
    ORDER);
  pfree(spans); pfree(ss1);
  return result;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span set aggregate union
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] s Span to aggregate
 * @return When the state variable has space for adding the new span, the 
 * function returns the current state variable. Otherwise, a NEW state 
 * variable is returned and the input state is freed.
 * @note Always use the function to overwrite the existing state as in: 
 * @code
 * state = span_union_transfn(state, span);
 * @endcode
 * @csqlfn #Span_union_transfn()
 * @csqlaggfn #spanUnionTransition()
 */
SpanSet *
span_union_transfn(SpanSet *state, const Span *s)
{
  /* Null span: return current state */
  if (! s)
    return state;
  /* Null state: create a new span set with the input span */
  if (! state)
    /* Arbitrary initialization to 64 elements */
    return spanset_make_exp((Span *) s, 1, 64, NORMALIZE_NO, ORDER);

  /* Ensure the validity of the arguments */
  if (! ensure_same_span_type(&state->elems[0], s))
    return NULL;
  return spanset_append_span(state, s, true);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Transition function for span set aggregate union
 * @param[in,out] state Current aggregate state, may be `NULL`
 * @param[in] ss Span set to aggregate
 * @return When the state variable has space for adding the new span set, the 
 * function returns the current state variable. Otherwise, a NEW state 
 * variable is returned and the input state is freed.
 * @note Always use the function to overwrite the existing state as in: 
 * @code
 * state = spanset_union_transfn(state, spanset);
 * @endcode
 * @csqlfn #Spanset_union_transfn()
 * @csqlaggfn #spansetUnionTransition()
 */
SpanSet *
spanset_union_transfn(SpanSet *state, const SpanSet *ss)
{
  /* Null span set: return current state */
  if (! ss)
    return state;
  /* Null state: create a new span set with the input span set */
  if (! state)
  {
    int count = ((ss->count / 64) + 1) * 64;
    /* Arbitrary initialization to next multiple of 64 elements */
    return spanset_make_exp((Span *) &ss->elems, ss->count, count,
      NORMALIZE_NO, ORDER);
  }

  /* Ensure the validity of the arguments */
  if (! ensure_same_span_type(&state->elems[0], &ss->elems[0]))
    return NULL;
  return spanset_append_spanset(state, ss, true);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Final function for span and span set union aggregate
 * @param[in] state Current aggregate state, may be `NULL`
 * @csqlfn #Span_union_finalfn()
 * @csqlaggfn #spanUnionFinal(), #spansetUnionFinal()
 */
SpanSet *
spanset_union_finalfn(SpanSet *state)
{
  if (! state)
    return NULL;
  SpanSet *result = spanset_compact(state);
  pfree(state);
  return result;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Combine function for span and span set union aggregates
 * @param[in,out] state1 Current aggregate state
 * @param[in] state2 Aggregate state to combine with the first one
 * @return The state holding the spans of both, which is the first state when
 * it has space for the spans of the second one and a NEW state otherwise, the
 * first state being freed; the second state is left to the caller
 * @csqlfn #Spanset_union_combinefn()
 */
SpanSet *
spanset_union_combinefn(SpanSet *state1, const SpanSet *state2)
{
  if (! state2)
    return state1;
  if (! state1)
    return spanset_copy(state2);
  return spanset_union_transfn(state1, state2);
}

/**
 * @ingroup meos_setspan_agg
 * @brief Return the bytes of the state of a span or span set union aggregate,
 * read back by #spansetstate_deserialize
 * @details The state holds its spans in the order they arrived, while a span
 * set read from bytes is normalized from increasing spans, so the bytes are
 * the extended Well-Known Binary of the state ordered and normalized, which
 * keeps the union it holds
 * @param[in] state State
 * @param[out] size_out Size of the result in bytes
 * @csqlfn #Spansetstate_serialize()
 */
uint8_t *
spansetstate_serialize(const SpanSet *state, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(state, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  SpanSet *ss = spanset_union_finalfn(spanset_copy(state));
  uint8_t *result = spanset_as_wkb(ss, WKB_EXTENDED, size_out);
  pfree(ss);
  return result;
}

/**
 * @ingroup meos_setspan_agg
 * @brief Return the state of a span or span set union aggregate written by
 * #spansetstate_serialize
 * @param[in] bytes Bytes
 * @param[in] size Size of the bytes
 * @csqlfn #Spansetstate_deserialize()
 */
SpanSet *
spansetstate_deserialize(const uint8_t *bytes, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(bytes, NULL);
  return spanset_from_wkb(bytes, size);
}

/*****************************************************************************/

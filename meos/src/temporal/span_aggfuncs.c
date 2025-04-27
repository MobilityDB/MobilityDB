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
 * @param[in,out] state Current aggregate state
 * @param[in] value Value to aggregate
 * @param[in] basetype Type of the value
 */
Span *
spanbase_extent_transfn(Span *state, Datum value, meosType basetype)
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
 * @param[in,out] state Current aggregate state
 * @param[in] s Set to aggregate
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
 * @param[in,out] state Current aggregate state
 * @param[in] s Span to aggregate
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
 * @param[in,out] state Current aggregate state
 * @param[in] ss Span set to aggregate
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

/*****************************************************************************/

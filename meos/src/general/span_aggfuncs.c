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
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/temporal_aggfuncs.h"
#include "general/temporal_tile.h"
#include "general/type_util.h"

/*****************************************************************************
 * Extent
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_agg
 * @brief Transition function for span extent aggregate of values
 */
Span *
spanbase_extent_transfn(Span *s, Datum d, meosType basetype)
{
  assert(span_basetype(basetype));

  /* Null span: return the span of the base value */
  if (! s)
    return span_make(d, d, true, true, basetype);

  Span s1;
  span_set(d, d, true, true, basetype, &s1);
  span_expand(&s1, s);
  return s;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span extent aggregate of integers
 */
Span *
int_extent_transfn(Span *s, int i)
{
  return spanbase_extent_transfn(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span extent aggregate of big integers
 */
Span *
bigint_extent_transfn(Span *s, int64 i)
{
  return spanbase_extent_transfn(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span extent aggregate of floats
 */
Span *
float_extent_transfn(Span *s, double d)
{
  return spanbase_extent_transfn(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span extent aggregate of timestamps
 */
Span *
timestamp_extent_transfn(Span *s, TimestampTz t)
{
  return spanbase_extent_transfn(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span extent aggregate of sets
 */
Span *
set_extent_transfn(Span *span, const Set *set)
{
  /* Can't do anything with null inputs */
  if (! span && ! set)
    return NULL;
  /* Null period and non-null timestamp set, return the bbox of the timestamp set */
  if (! span)
    return set_span(set);
  /* Non-null period and null timestamp set, return the period */
  if (! set)
    return span;

  Span s;
  set_set_span(set, &s);
  span_expand(&s, span);
  return span;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span extent aggregate of spans
 */
Span *
span_extent_transfn(Span *s1, const Span *s2)
{
  /* Can't do anything with null inputs */
  if (! s1 && ! s2)
    return NULL;
  /* Null span and non-null span, return the span */
  if (! s1)
    return span_copy(s2);
  /* Non-null span and null span, return the span */
  if (! s2)
    return s1;

  span_expand(s2, s1);
  return s1;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for span extent aggregate of span sets
 */
Span *
spanset_extent_transfn(Span *s, const SpanSet *ss)
{
  /* Can't do anything with null inputs */
  if (! s && ! ss)
    return NULL;
  /* Null  and non-null span set, return the bbox of the span set */
  if (! s)
    return span_copy(&ss->span);
  /* Non-null span and null temporal, return the span */
  if (! ss)
    return s;

  span_expand(&ss->span, s);
  return s;
}

/*****************************************************************************/

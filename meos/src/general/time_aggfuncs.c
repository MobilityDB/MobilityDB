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
 * @brief Aggregate functions for time types.
 */

#include "general/time_aggfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/spanset.h"
#include "general/temporal_aggfuncs.h"
#include "general/temporal_tile.h"
#include "general/type_util.h"

/*****************************************************************************
 * Aggregate functions for time types
 *****************************************************************************/

/**
 * @brief Return the sum of the two arguments
 */
Datum
datum_sum_int32(Datum l, Datum r)
{
  return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
}

/*****************************************************************************
 * tcount
 *****************************************************************************/

/**
 * @brief Transform a timestamp value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
timestamp_transform_tcount(TimestampTz t)
{
  TInstant **result = palloc(sizeof(TInstant *));
  result[0] = tinstant_make(Int32GetDatum(1), T_TINT, t);
  return result;
}

/**
 * @brief Transform a timestamp set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
timestampset_transform_tcount(const Set *ts)
{
  TInstant **result = palloc(sizeof(TInstant *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(ts, i));
    result[i] = tinstant_make(Int32GetDatum(1), T_TINT, t);
  }
  return result;
}

/**
 * @brief Transform a period value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence *
period_transform_tcount(const Span *p)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  TInstant *instants[2];
  TimestampTz t = p->lower;
  instants[0] = tinstant_make(datum_one, T_TINT, t);
  if (p->lower == p->upper)
  {
    result = tsequence_make((const TInstant **) instants, 1, p->lower_inc,
      p->upper_inc, STEP, NORMALIZE_NO);
  }
  else
  {
    t = p->upper;
    instants[1] = tinstant_make(datum_one, T_TINT, t);
    result = tsequence_make((const TInstant **) instants, 2,
      p->lower_inc, p->upper_inc, STEP, NORMALIZE_NO);
    pfree(instants[1]);
  }
  pfree(instants[0]);
  return result;
}

/**
 * @brief Transform a period set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence **
periodset_transform_tcount(const SpanSet *ps)
{
  TSequence **result = palloc(sizeof(TSequence *) * ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    result[i] = period_transform_tcount(p);
  }
  return result;
}

void
ensure_same_timetype_skiplist(SkipList *state, uint8 subtype)
{
  Temporal *head = (Temporal *) skiplist_headval(state);
  if (head->subtype != subtype)
    elog(ERROR, "Cannot aggregate temporal values of different duration");
  return;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of timestamps
 */
SkipList *
timestamp_tcount_transfn(SkipList *state, TimestampTz t)
{
  TInstant **instants = timestamp_transform_tcount(t);
  if (! state)
  {
    state = skiplist_make((void **) instants, 1);
  }
  else
  {
    ensure_same_timetype_skiplist(state, TINSTANT);
    skiplist_splice(state, (void **) instants, 1, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree_array((void **) instants, 1);
  return state;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of timestamp sets
 */
SkipList *
timestampset_tcount_transfn(SkipList *state, const Set *ts)
{
  TInstant **instants = timestampset_transform_tcount(ts);
  if (! state)
  {
    state = skiplist_make((void **) instants, ts->count);
  }
  else
  {
    ensure_same_timetype_skiplist(state, TINSTANT);
    skiplist_splice(state, (void **) instants, ts->count, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree_array((void **) instants, ts->count);
  return state;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of periods
 */
SkipList *
period_tcount_transfn(SkipList *state, const Span *p)
{
  TSequence *seq = period_transform_tcount(p);
  if (! state)
  {
    state = skiplist_make((void **) &seq, 1);
  }
  else
  {
    ensure_same_timetype_skiplist(state, TSEQUENCE);
    skiplist_splice(state, (void **) &seq, 1, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree(seq);
  return state;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of period sets
 */
SkipList *
periodset_tcount_transfn(SkipList *state, const SpanSet *ps)
{
  TSequence **sequences = periodset_transform_tcount(ps);
  int start = 0;
  if (! state)
  {
    state = skiplist_make((void **) &sequences[0], 1);
    start++;
  }
  else
    ensure_same_timetype_skiplist(state, TSEQUENCE);
  for (int i = start; i < ps->count; i++)
  {
    skiplist_splice(state, (void **) &sequences[i], 1, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree_array((void **) sequences, ps->count);
  return state;
}

/*****************************************************************************/

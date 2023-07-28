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

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_general/skiplist.h"

/*****************************************************************************
 * Aggregate transition functions for time types
 *****************************************************************************/

PGDLLEXPORT Datum Timestamp_tcount_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamp_tcount_transfn);
/**
 * @brief Transition function for temporal count aggregate of timestamps
 */
Datum
Timestamp_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  store_fcinfo(fcinfo);
  state = timestamp_tcount_transfn(state, t);
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Timestampset_tcount_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestampset_tcount_transfn);
/**
 * @brief Transition function for temporal count aggregate of timestamp sets
 */
Datum
Timestampset_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Set *ts = PG_GETARG_SET_P(1);
  store_fcinfo(fcinfo);
  state = timestampset_tcount_transfn(state, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Period_tcount_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Period_tcount_transfn);
/**
 * @brief Transition function for temporal count aggregate of periods
 */
Datum
Period_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Span *p = PG_GETARG_SPAN_P(1);
  store_fcinfo(fcinfo);
  state = period_tcount_transfn(state, p);
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Periodset_tcount_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_tcount_transfn);
/**
 * @brief Transition function for temporal count aggregate of period sets
 */
Datum
Periodset_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  SpanSet *ps = PG_GETARG_SPANSET_P(1);
  store_fcinfo(fcinfo);
  state = periodset_tcount_transfn(state, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************/

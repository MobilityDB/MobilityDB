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
 * @brief Operators for time types.
 */

#include "general/time_ops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/periodset.h"
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal_util.h"

/*****************************************************************************/
/* contains? */

PG_FUNCTION_INFO_V1(Contains_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period contains a timestamp
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(contains_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Contains_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period contains a timestamp set
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = contains_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period set contains a timestamp
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = contains_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period set contains a timestamp set
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = contains_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* contained? */

PG_FUNCTION_INFO_V1(Contained_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp is contained by a period
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contained_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Contained_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp is contained by a period set
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = contained_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp set is contained by a period
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = contained_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp set is contained by a period set
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = contained_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* overlaps? */

PG_FUNCTION_INFO_V1(Overlaps_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp set and a period overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overlaps_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp set and a period set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overlaps_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period and a timestamp set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overlaps_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period set and a timestamp set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overlaps_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* adjacent to (but not overlapping)? */

PG_FUNCTION_INFO_V1(Adjacent_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp and a period are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(adjacent_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Adjacent_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp and a period set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = adjacent_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp set and a period are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = adjacent_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp set and a period set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = adjacent_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period and a timestamp are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(adjacent_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Adjacent_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period and a timestamp set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = adjacent_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period set and a timestamp are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = adjacent_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a period set and a timestamp set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = adjacent_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly before of? */

PG_FUNCTION_INFO_V1(Before_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is strictly before a period
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(before_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Before_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is strictly before a period set
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = before_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is strictly before a period
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = before_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is strictly before a period set
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = before_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is strictly before a timestamp
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(before_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Before_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is strictly before a timestamp set
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = before_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is strictly before a timestamp
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = before_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is strictly before a timestamp set
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Before_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = before_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly after of? */

PG_FUNCTION_INFO_V1(After_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is strictly after a period
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(after_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(After_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is strictly after a period set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = after_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is strictly after a period
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = after_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is strictly after a period set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = after_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is strictly after a timestamp
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(after_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(After_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is strictly after a timestamp set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = after_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is strictly after a timestamp
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = after_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is strictly after a timestamp set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
After_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = after_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to right of? */

PG_FUNCTION_INFO_V1(Overbefore_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is not after a period
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overbefore_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Overbefore_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is not after a period set
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overbefore_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is not after a period
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overbefore_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is not after a period set
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overbefore_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is not after a timestamp
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(overbefore_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Overbefore_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is not after a timestamp set
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overbefore_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is not after a timestamp
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overbefore_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is not after a timestamp set
 * @sqlfunc time_overbefore()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overbefore_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overbefore_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to left of? */

PG_FUNCTION_INFO_V1(Overafter_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is not before a period
 * @sqlfunc time_overafter()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Overafter_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overafter_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Overafter_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is not before a period set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overafter_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overafter_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is not before a period
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overafter_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overafter_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is not before a period set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overafter_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overafter_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is not before a timestamp
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overafter_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(overafter_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Overafter_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period is not before a timestamp set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overafter_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overafter_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is not before a timestamp
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overafter_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overafter_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a period set is not before a timestamp set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overafter_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overafter_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a timestamp and a period
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PeriodSet *result = union_timestamp_period(t, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a timestamp and a period set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = union_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a timestamp set and a period
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PeriodSet *result = union_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a timestamp set and a period set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = union_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a period and a timestamp
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = union_period_timestamp(p, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a period and a timestamp set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = union_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a period set and a timestamp
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = union_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a period set and a timestamp set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = union_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a timestamp and a period
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampTz result;
  bool found = intersection_timestamp_period(t, p, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a timestamp and a period set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampTz result;
  bool found = intersection_timestamp_periodset(t, ps, &result);
  PG_FREE_IF_COPY(ps, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a timestamp set and a period.
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampSet *result = intersection_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a timestamp set and a period set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampSet *result = intersection_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a period and a timestamp
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz result;
  bool found = intersection_period_timestamp(p, t, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a period and a timestamp set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = intersection_period_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a period set and a timestamp
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz result;
  bool found = intersection_periodset_timestamp(ps, t, &result);
  PG_FREE_IF_COPY(ps, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a period set and a timestamp set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = intersection_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a timestamp and a period
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampTz result;
  bool found = minus_timestamp_period(t, p, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Minus_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a timestamp and a period set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampTz result;
  bool found = minus_timestamp_periodset(t, ps, &result);
  PG_FREE_IF_COPY(ps, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a timestamp set and period
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampSet *result = minus_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a timestamp set and a period set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampSet *result = minus_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a period and a timestamp
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = minus_period_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a period and a timestamp set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = minus_period_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a period set and a timestamp
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = minus_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a period set and a timestamp set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = minus_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

PG_FUNCTION_INFO_V1(Distance_timestamp_timestamp);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between the timestamps
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  double result = distance_timestamp_timestamp(t1, t2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestamp_timestampset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp and a timestamp set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Datum ts = PG_GETARG_DATUM(1);
  Period p;
  orderedset_span_slice(ts, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestamp_period);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp and a period
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  double result = distance_period_timestamp(p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestamp_periodset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp and a period set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Datum ps = PG_GETARG_DATUM(1);
  Period p;
  spanset_span_slice(ps, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_timestamp);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp set and a timestamp
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  Datum ts = PG_GETARG_DATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Period p;
  orderedset_span_slice(ts, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_timestampset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between the timestamp sets
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  Datum ts1 = PG_GETARG_DATUM(0);
  Datum ts2 = PG_GETARG_DATUM(1);
  Period p1, p2;
  orderedset_span_slice(ts1, &p1);
  orderedset_span_slice(ts2, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_period);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp set and a period
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestampset_period(PG_FUNCTION_ARGS)
{
  Datum ts = PG_GETARG_DATUM(0);
  Period *p = PG_GETARG_SPAN_P(1);
  Period p1;
  orderedset_span_slice(ts, &p1);
  double result = distance_span_span(&p1, p);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_periodset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp set and a period set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_timestampset_periodset(PG_FUNCTION_ARGS)
{
  Datum ts = PG_GETARG_DATUM(0);
  Datum ps = PG_GETARG_DATUM(1);
  Period p1, p2;
  orderedset_span_slice(ts, &p1);
  spanset_span_slice(ps, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_timestamp);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a period and a timestamp
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double result = distance_period_timestamp(p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_timestampset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a period and a timestamp set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Datum ts = PG_GETARG_DATUM(1);
  Period p1;
  orderedset_span_slice(ts, &p1);
  double result = distance_span_span(p, &p1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_period);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between the periods
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  double result = distance_span_span(p1, p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_periodset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a period and a period set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Datum ps = PG_GETARG_DATUM(1);
  Period p1;
  spanset_span_slice(ps, &p1);
  double result = distance_span_span(&p1, p);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_timestamp);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a period set and a timestamp
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_periodset_timestamp(PG_FUNCTION_ARGS)
{
  Datum ps = PG_GETARG_DATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Period p;
  spanset_span_slice(ps, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_timestampset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a period set and a timestamp set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_periodset_timestampset(PG_FUNCTION_ARGS)
{
  Datum ps = PG_GETARG_DATUM(0);
  Datum ts = PG_GETARG_DATUM(1);
  Period p1, p2;
  spanset_span_slice(ps, &p1);
  orderedset_span_slice(ts, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_period);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a period set and a period
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_periodset_period(PG_FUNCTION_ARGS)
{
  Datum ps = PG_GETARG_DATUM(0);
  Period *p = PG_GETARG_SPAN_P(1);
  Period p1;
  spanset_span_slice(ps, &p1);
  double result = distance_span_span(&p1, p);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_periodset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between the period sets
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_periodset_periodset(PG_FUNCTION_ARGS)
{
  Datum ps1 = PG_GETARG_DATUM(0);
  Datum ps2 = PG_GETARG_DATUM(1);
  Period p1, p2;
  spanset_span_slice(ps1, &p1);
  spanset_span_slice(ps2, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

/******************************************************************************/

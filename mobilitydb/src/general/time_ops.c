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

/**
 * @brief Return the size in bytes to read from toast to get the basic information
 * from a variable-length time type: Time struct (i.e., OrderedSet
 * or PeriodSet) and bounding box size
*/
uint32_t
time_max_header_size(void)
{
  return double_pad(Max(sizeof(OrderedSet), sizeof(PeriodSet)));
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

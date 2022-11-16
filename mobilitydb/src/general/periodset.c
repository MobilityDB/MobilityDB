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
 * @brief General functions for set of disjoint periods.
 */

#include "general/periodset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_out.h"
#include "general/temporal_util.h"
#include "general/time_ops.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/temporal_util.h"

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_to_periodset);
/**
 * @ingroup mobilitydb_spantime_cast
 * @brief Cast the timestamp value as a period set
 * @sqlfunc periodset()
 */
PGDLLEXPORT Datum
Timestamp_to_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *result = timestamp_to_periodset(t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_to_periodset);
/**
 * @ingroup mobilitydb_spantime_cast
 * @brief Cast the timestamp set value as a period set
 * @sqlfunc periodset()
 */
PGDLLEXPORT Datum
Timestampset_to_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *result = timestampset_to_periodset(ts);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_timespan);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the timespan of a period set
 * @sqlfunc timespan()
 */
PGDLLEXPORT Datum
Periodset_timespan(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *result = periodset_timespan(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_duration);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the timespan of a period set
 * @sqlfunc duration()
 */
PGDLLEXPORT Datum
Periodset_duration(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *result = periodset_duration(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_num_timestamps);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the number of timestamps of a period set
 * @sqlfunc numTimestamps()
 */
PGDLLEXPORT Datum
Periodset_num_timestamps(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int result = periodset_num_timestamps(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Periodset_start_timestamp);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the start timestamp of a period set
 * @sqlfunc startTimestamp()
 */
PGDLLEXPORT Datum
Periodset_start_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  const Period *p = spanset_sp_n(ps, 0);
  TimestampTz result = p->lower;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Periodset_end_timestamp);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the end timestamp of a period set
 * @sqlfunc endTimestamp()
 */
PGDLLEXPORT Datum
Periodset_end_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  const Period *p = spanset_sp_n(ps, ps->count - 1);
  TimestampTz result = p->upper;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Periodset_timestamp_n);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the n-th timestamp of a period set
 * @sqlfunc timestampN()
 */
PGDLLEXPORT Datum
Periodset_timestamp_n(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = periodset_timestamp_n(ps, n, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Periodset_timestamps);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the timestamps of a period set
 * @sqlfunc timestamps()
 */
PGDLLEXPORT Datum
Periodset_timestamps(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int count;
  TimestampTz *times = periodset_timestamps(ps, &count);
  ArrayType *result = timestamparr_to_array(times, count);
  pfree(times);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_shift);
/**
 * @ingroup mobilitydb_spantime_transf
 * @brief Shift a period set by an interval
 * @sqlfunc shift()
 */
PGDLLEXPORT Datum
Periodset_shift(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  PeriodSet *result = periodset_shift_tscale(ps, shift, NULL);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_tscale);
/**
 * @ingroup mobilitydb_spantime_transf
 * @brief Shift a period set by an interval
 * @sqlfunc tscale()
 */
PGDLLEXPORT Datum
Periodset_tscale(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  PeriodSet *result = periodset_shift_tscale(ps, NULL, duration);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_shift_tscale);
/**
 * @ingroup mobilitydb_spantime_transf
 * @brief Shift a period set by an interval
 * @sqlfunc shiftTscale()
 */
PGDLLEXPORT Datum
Periodset_shift_tscale(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  PeriodSet *result = periodset_shift_tscale(ps, shift, duration);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

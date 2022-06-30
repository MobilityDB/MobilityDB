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
 * @brief General functions for `timestampset` values composed of an ordered
 * list of distinct `timestamptz` values.
 */

#include "general/timestampset.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/time_ops.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_in);
/**
 * Input function for timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  TimestampSet *result = timestampset_in(input);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_out);
/**
 * Output function for timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_out(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  char *result = timestampset_out(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Timestampset_recv);
/**
 * Receive function for timestamp set
 */
PGDLLEXPORT Datum
Timestampset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  TimestampSet *result = timestampset_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_send);
/**
 * Send function for timestamp set
 */
PGDLLEXPORT Datum
Timestampset_send(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(ts);
  uint8_t *wkb = timestampset_as_wkb(ts, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_constructor);
/**
 * Construct a timestamp set from an array of timestamps
 */
PGDLLEXPORT Datum
Timestampset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  TimestampTz *times = timestamparr_extract(array, &count);
  TimestampSet *result = timestampset_make_free(times, count);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_to_timestampset);
/**
 * Cast a timestamp as a timestamp set
 */
PGDLLEXPORT Datum
Timestamp_to_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *result = timestamp_to_timestampset(t);
  PG_RETURN_POINTER(result);
}

/**
 * Peak into a timestamp set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
timestampset_period_slice(Datum tsdatum, Period *p)
{
  TimestampSet *ts = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) tsdatum))
    ts = (TimestampSet *) PG_DETOAST_DATUM_SLICE(tsdatum, 0,
      time_max_header_size());
  else
    ts = (TimestampSet *) tsdatum;
  timestampset_set_period(ts, p);
  PG_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_to_period);
/**
 * Return the bounding period on which a timestamp set is defined
 */
PGDLLEXPORT Datum
Timestampset_to_period(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  Period *result = palloc(sizeof(Period));
  timestampset_period_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_mem_size);
/**
 * Return the size in bytes of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_mem_size(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Datum result = timestampset_mem_size(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Timestampset_timespan);
/**
 * Return the timespan of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_timespan(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *result = timestampset_timespan(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_num_timestamps);
/**
 * Return the number of timestamps of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_num_timestamps(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_INT32(timestampset_num_timestamps(ts));
}

PG_FUNCTION_INFO_V1(Timestampset_start_timestamp);
/**
 * Return the start timestamp of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_start_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz result = timestampset_start_timestamp(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Timestampset_end_timestamp);
/**
 * Return the end timestamp of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_end_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz result = timestampset_end_timestamp(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Timestampset_timestamp_n);
/**
 * Return the n-th timestamp of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_timestamp_n(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = timestampset_timestamp_n(ts, n, &result);
  PG_FREE_IF_COPY(ts, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Timestampset_timestamps);
/**
 * Return the timestamps of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_timestamps(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz *times = timestampset_timestamps(ts);
  ArrayType *result = timestamparr_to_array(times, ts->count);
  pfree(times);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_shift);
/**
 * Shift a timestamp set by an interval
 */
PGDLLEXPORT Datum
Timestampset_shift(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  TimestampSet *result = timestampset_shift_tscale(ts, start, NULL);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_tscale);
/**
 * Scale a timestamp set by an interval
 */
PGDLLEXPORT Datum
Timestampset_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  TimestampSet *result = timestampset_shift_tscale(ts, NULL, duration);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_shift_tscale);
/**
 * Shift and scale a timestamp set by the intervals
 */
PGDLLEXPORT Datum
Timestampset_shift_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  ensure_valid_duration(duration);
  TimestampSet *result = timestampset_shift_tscale(ts, start, duration);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first timestamp set
 * is less than, equal, or greater than the second temporal value
 */
PGDLLEXPORT Datum
Timestampset_cmp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  int cmp = timestampset_cmp(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(Timestampset_eq);
/**
 * Return true if the first timestamp set is equal to the second one
 */
PGDLLEXPORT Datum
Timestampset_eq(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_eq(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_ne);
/**
 * Return true if the first timestamp set is different from the second one
 */
PGDLLEXPORT Datum
Timestampset_ne(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_ne(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_lt);
/**
 * Return true if the first timestamp set is less than the second one
 */
PGDLLEXPORT Datum
Timestampset_lt(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_lt(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_le);
/**
 * Return true if the first timestamp set is less than
 * or equal to the second one
 */
PGDLLEXPORT Datum
Timestampset_le(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_le(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_ge);
/**
 * Return true if the first timestamp set is greater than
 * or equal to the second one
 */
PGDLLEXPORT Datum
Timestampset_ge(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_ge(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_gt);
/**
 * Return true if the first timestamp set is greater than the second one
 */
PGDLLEXPORT Datum
Timestampset_gt(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_gt(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Function for defining hash index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_hash);
/**
 * Return the 32-bit hash value of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_hash(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint32 result = timestampset_hash(ts);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Timestampset_hash_extended);
/**
 * Return the 64-bit hash value of a timestamp set using a seed
 */
PGDLLEXPORT Datum
Timestampset_hash_extended(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = timestampset_hash_extended(ts, seed);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/

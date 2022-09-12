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
#include "general/temporal_util.h"
#include "general/time_ops.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_in);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Return a period set from its string representation
 * @sqlfunc periodset_in()
 */
PGDLLEXPORT Datum
Periodset_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  PeriodSet *result = periodset_in(input);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_out);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Return the string representation of a period set
 * @sqlfunc periodset_out()
 */
PGDLLEXPORT Datum
Periodset_out(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  char *result = periodset_out(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Periodset_recv);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Receive function for period set
 * @sqlfunc periodset_recv()
 */
PGDLLEXPORT Datum
Periodset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PeriodSet *result = periodset_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_send);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Send function for period set
 * @sqlfunc periodset_send()
 */
PGDLLEXPORT Datum
Periodset_send(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(ps);
  uint8_t *wkb = periodset_as_wkb(ps, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_constructor);
/**
 * @ingroup mobilitydb_spantime_constructor
 * @brief Construct a period set from an array of period values
 * @sqlfunc periodset()
 */
PGDLLEXPORT Datum
Periodset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  Period **periods = periodarr_extract(array, &count);
  PeriodSet *result = periodset_make((const Period **) periods, count,
    NORMALIZE);
  pfree(periods);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
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

PG_FUNCTION_INFO_V1(Period_to_periodset);
/**
 * @ingroup mobilitydb_spantime_cast
 * @brief Cast the period value as a period set
 * @sqlfunc periodset()
 */
PGDLLEXPORT Datum
Period_to_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *result = period_to_periodset(p);
  PG_RETURN_POINTER(result);
}

/**
 * Peak into a period set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
periodset_period_slice(Datum psdatum, Period *p)
{
  PeriodSet *ps = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) psdatum))
    ps = (PeriodSet *) PG_DETOAST_DATUM_SLICE(psdatum, 0,
      time_max_header_size());
  else
    ps = (PeriodSet *) psdatum;
  memcpy(p, &ps->period, sizeof(Span));
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

PG_FUNCTION_INFO_V1(Periodset_to_period);
/**
 * @ingroup mobilitydb_spantime_cast
 * @brief Return the bounding period on which a period set is defined
 * @sqlfunc period()
 */
PGDLLEXPORT Datum
Periodset_to_period(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  Period *result = palloc(sizeof(Period));
  periodset_period_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_mem_size);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the size in bytes of a period set
 * @sqlfunc memSize()
 */
PGDLLEXPORT Datum
Periodset_mem_size(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Datum result = periodset_mem_size(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_DATUM(result);
}

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

PG_FUNCTION_INFO_V1(Periodset_num_periods);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the number of periods of a period set
 * @sqlfunc numPeriods()
 */
PGDLLEXPORT Datum
Periodset_num_periods(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int result = periodset_num_periods(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Periodset_start_period);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the start period of a period set
 * @sqlfunc startPeriod()
 */
PGDLLEXPORT Datum
Periodset_start_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *result = periodset_start_period(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_end_period);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the end period of a period set
 * @sqlfunc endPeriod()
 */
PGDLLEXPORT Datum
Periodset_end_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *result = periodset_end_period(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_period_n);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the n-th period of a period set
 * @sqlfunc periodN()
 */
PGDLLEXPORT Datum
Periodset_period_n(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int i = PG_GETARG_INT32(1); /* Assume 1-based */
  Period *result = periodset_period_n(ps, i);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_periods);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the periods of a period set
 * @sqlfunc periods()
 */
PGDLLEXPORT Datum
Periodset_periods(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int count;
  const Period **periods = periodset_periods(ps, &count);
  ArrayType *result = periodarr_to_array(periods, ps->count);
  pfree(periods);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
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
  const Period *p = periodset_per_n(ps, 0);
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
  const Period *p = periodset_per_n(ps, ps->count - 1);
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

/*****************************************************************************
 * B-tree support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_cmp);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return -1, 0, or 1 depending on whether the first period set
 * is less than, equal, or greater than the second one
 * @sqlfunc periodset_cmp()
 */
PGDLLEXPORT Datum
Periodset_cmp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  int cmp = periodset_cmp(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(Periodset_eq);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first period set is equal to the second one
 * @sqlfunc periodset_eq()
 * @sqlop @p =
 */
PGDLLEXPORT Datum
Periodset_eq(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_eq(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_ne);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first period set is different from the second one
 * @sqlfunc periodset_ne()
 * @sqlop @p <>
 */
PGDLLEXPORT Datum
Periodset_ne(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_ne(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/* Comparison operators using the internal B-tree comparator */

PG_FUNCTION_INFO_V1(Periodset_lt);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first period set is less than the second one
 * @sqlfunc periodset_lt()
 * @sqlop @p <
 */
PGDLLEXPORT Datum
Periodset_lt(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_lt(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_le);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first period set is less than or equal to
 * the second one
 * @sqlfunc periodset_le()
 * @sqlop @p <=
 */
PGDLLEXPORT Datum
Periodset_le(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_le(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_ge);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first period set is greater than or equal to
 * the second one
 * @sqlfunc periodset_ge()
 * @sqlop @p >=
 */
PGDLLEXPORT Datum
Periodset_ge(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_ge(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_gt);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first period set is greater than the second one
 * @sqlfunc periodset_gt()
 * @sqlop @p >
 */
PGDLLEXPORT Datum
Periodset_gt(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_gt(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_hash);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the 32-bit hash value of a period set
 * @sqlfunc periodset_hash()
 */
PGDLLEXPORT Datum
Periodset_hash(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  uint32 result = periodset_hash(ps);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Periodset_hash_extended);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the 64-bit hash value of a period set using a seed
 * @sqlfunc periodset_hash_extended()
 */
PGDLLEXPORT Datum
Periodset_hash_extended(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = periodset_hash_extended(ps, seed);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/

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
 * @file period.c
 * @brief General functions for time periods composed of two `TimestampTz`
 * values and two Boolean values stating whether the bounds are inclusive.
 */

#include "general/period.h"

/* PostgreSQL */
#include <assert.h>
#include <access/hash.h>
#include <utils/builtins.h>
/* MobilityDB */
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporal.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"
#include "general/rangetypes_ext.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_in);
/**
 * Input function for periods
 */
PGDLLEXPORT Datum
Period_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Period *result = period_parse(&input, true);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_out);
/**
 * Output function for periods
 */
PGDLLEXPORT Datum
Period_out(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_CSTRING(period_to_string(p));
}

PG_FUNCTION_INFO_V1(Period_send);
/**
 * Send function for periods
 */
PGDLLEXPORT Datum
Period_send(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  period_write(p, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Period_recv);
/**
 * Receive function for periods
 */
PGDLLEXPORT Datum
Period_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(period_read(buf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_constructor2);
/**
 * Construct a period from the two arguments
 */
PGDLLEXPORT Datum
Period_constructor2(PG_FUNCTION_ARGS)
{
  TimestampTz lower = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz upper = PG_GETARG_TIMESTAMPTZ(1);
  Period *period;
  period = period_make(lower, upper, true, false);
  PG_RETURN_PERIOD_P(period);
}


PG_FUNCTION_INFO_V1(Period_constructor4);
/**
 * Construct a period from the four arguments
 */
PGDLLEXPORT Datum
Period_constructor4(PG_FUNCTION_ARGS)
{
  TimestampTz lower = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz upper = PG_GETARG_TIMESTAMPTZ(1);
  bool lower_inc = PG_GETARG_BOOL(2);
  bool upper_inc = PG_GETARG_BOOL(3);
  Period *period;
  period = period_make(lower, upper, lower_inc, upper_inc);
  PG_RETURN_PERIOD_P(period);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_to_period);
/**
 * Cast the timestamp value as a period
 */
PGDLLEXPORT Datum
Timestamp_to_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *result = timestamp_period(t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_tstzrange);
/**
 * Convert the period as a tstzrange value
 */
PGDLLEXPORT Datum
Period_to_tstzrange(PG_FUNCTION_ARGS)
{
  Period *period = PG_GETARG_PERIOD_P(0);
  RangeType *range;
  range = range_make(TimestampTzGetDatum(period->lower),
    TimestampTzGetDatum(period->upper), period->lower_inc,
    period->upper_inc, T_TIMESTAMPTZ);
  PG_RETURN_POINTER(range);
}

PG_FUNCTION_INFO_V1(Tstzrange_to_period);
/**
 * Convert the tstzrange value as a period
 */
PGDLLEXPORT Datum
Tstzrange_to_period(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TypeCacheEntry *typcache;
  char flags = range_get_flags(range);
  RangeBound lower;
  RangeBound upper;
  bool empty;
  Period *period;

  typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  assert(typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  if (flags & RANGE_EMPTY)
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range cannot be empty")));
  if ((flags & RANGE_LB_INF) || (flags & RANGE_UB_INF))
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range bounds cannot be infinite")));

  range_deserialize(typcache, range, &lower, &upper, &empty);
  period = period_make(DatumGetTimestampTz(lower.val),
    DatumGetTimestampTz(upper.val), lower.inclusive, upper.inclusive);
  PG_RETURN_POINTER(period);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* period -> timestamptz functions */

PG_FUNCTION_INFO_V1(Period_lower);
/**
 * Return the lower bound value
 */
PGDLLEXPORT Datum
Period_lower(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_TIMESTAMPTZ(p->lower);
}

PG_FUNCTION_INFO_V1(Period_upper);
/**
 * Return the upper bound value
 */
PGDLLEXPORT Datum
Period_upper(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_TIMESTAMPTZ(p->upper);
}

/* period -> bool functions */

PG_FUNCTION_INFO_V1(Period_lower_inc);
/**
 * Return true if the lower bound value is inclusive
 */
PGDLLEXPORT Datum
Period_lower_inc(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_BOOL(p->lower_inc != 0);
}

PG_FUNCTION_INFO_V1(Period_upper_inc);
/**
 * Return true if the upper bound value is inclusive
 */
PGDLLEXPORT Datum
Period_upper_inc(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_BOOL(p->upper_inc != 0);
}

PG_FUNCTION_INFO_V1(Period_duration);
/**
 * Return the duration of the period
 */
PGDLLEXPORT Datum
Period_duration(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *result = period_duration(p);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_shift);
/**
 * Shift the period value by the interval
 */
PGDLLEXPORT Datum
Period_shift(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Period *result = period_copy(p);
  period_shift_tscale(start, NULL, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_tscale);
/**
 * Shift the period  value by the interval
 */
PGDLLEXPORT Datum
Period_tscale(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Period *result = period_copy(p);
  period_shift_tscale(NULL, duration, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_shift_tscale);
/**
 * Shift the period value by the interval
 */
PGDLLEXPORT Datum
Period_shift_tscale(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Period *result = period_copy(p);
  period_shift_tscale(start, duration, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_eq);
/**
 * Return true if the first period is equal to the second one
 */
PGDLLEXPORT Datum
Period_eq(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_eq(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_ne);
/**
 * Return true if the first period is different from the second one
 */
PGDLLEXPORT Datum
Period_ne(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_ne(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first period
 * is less than, equal, or greater than the second one
 */
PGDLLEXPORT Datum
Period_cmp(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_INT32(period_cmp(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_lt);
/**
 * Return true if the first period is less than the second one
 */
PGDLLEXPORT Datum
Period_lt(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_lt(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_le);
/**
 * Return true if the first period is less than or equal to the second one
 */
PGDLLEXPORT Datum
Period_le(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_le(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_ge);
/**
 * Return true if the first period is greater than or equal to the second one
 */
PGDLLEXPORT Datum
Period_ge(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_ge(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_gt);
/**
 * Return true if the first period is greater than the second one
 */
PGDLLEXPORT Datum
Period_gt(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_gt(p1, p2));
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_hash);
/**
 * Return the 32-bit hash value of a period.
 */
PGDLLEXPORT Datum
Period_hash(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  uint32 result = period_hash(p);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Period_hash_extended);
/**
 * Return the 64-bit hash value of a period obtained with a seed.
 */
PGDLLEXPORT Datum
Period_hash_extended(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Datum seed = PG_GETARG_DATUM(1);
  uint64 result = period_hash_extended(p, seed);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/

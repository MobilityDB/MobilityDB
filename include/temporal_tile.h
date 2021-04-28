/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

#ifndef __TEMPORAL_TILE_H__
#define __TEMPORAL_TILE_H__

#include <postgres.h>
#include <fmgr.h>

#include "temporal.h"

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls generating
 * the bucket list
 */
typedef struct RangeBucketState
{
  bool done;
  Oid valuetypid;
  Temporal *temp; /* NULL when generating bucket list, used for splitting */
  Datum size;
  Datum origin;
  int coordmin;
  int coordmax;
  int coord;
} RangeBucketState;

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls generating
 * the multidimensional grid
 */
typedef struct TboxGridState
{
  bool done;
  double xsize;
  int64 tsize;
  double xorigin;
  int64 torigin;
  int min[2];
  int max[2];
  int coords[2];
} TboxGridState;

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal splits
 */
typedef struct TimeSplitState
{
  bool done;
  int64 tunits;
  TimestampTz *buckets;
  Temporal **splits;
  int i;
  int count;
} TimeSplitState;

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal splits
 */
typedef struct ValueTimeSplitState
{
  bool done;
  Datum *value_buckets;
  TimestampTz *time_buckets;
  Temporal **splits;
  int i;
  int count;
} ValueTimeSplitState;

extern ValueTimeSplitState *value_time_split_state_make(Datum *value_buckets,
  TimestampTz *time_buckets, Temporal **splits, int count);


/*****************************************************************************/

extern Datum timestamptz_bucket(PG_FUNCTION_ARGS);
extern Datum int_bucket(PG_FUNCTION_ARGS);
extern Datum float_bucket(PG_FUNCTION_ARGS);
extern Datum temporal_time_split(PG_FUNCTION_ARGS);
extern Datum tnumber_range_split(PG_FUNCTION_ARGS);
extern Datum tnumber_range_time_split(PG_FUNCTION_ARGS);

extern TimestampTz timestamptz_bucket_internal(TimestampTz timestamp,
  int64 tunits, TimestampTz origin);

extern Temporal **temporal_time_split_internal(Temporal *temp,
  TimestampTz start, TimestampTz end, int64 tunits, int count,
  TimestampTz **buckets, int *newcount);

extern TimeSplitState *time_split_state_new(int64 tunits, TimestampTz *buckets,
  Temporal **splits, int count);
extern void time_split_state_next(TimeSplitState *state);

extern ValueTimeSplitState *value_time_split_state_new(Datum *value_buckets,
  TimestampTz *time_buckets, Temporal **splits, int count);
extern void value_time_split_state_next(ValueTimeSplitState *state);

#endif /* __TEMPORAL_TILE_H__ */

/*****************************************************************************/

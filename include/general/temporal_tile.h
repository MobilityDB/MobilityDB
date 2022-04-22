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

#ifndef __TEMPORAL_TILE_H__
#define __TEMPORAL_TILE_H__

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MobilityDB */
#include "general/temporal.h"

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls generating
 * the bucket list
 */
typedef struct RangeBucketState
{
  bool done;
  int i;
  CachedType basetype;
  Temporal *temp; /* NULL when generating bucket list, used for splitting */
  Datum size;
  Datum origin;
  Datum minvalue;
  Datum maxvalue;
  Datum value;
} RangeBucketState;

/**
 * Struct for storing the state that persists across multiple calls generating
 * the bucket list
 */
typedef struct PeriodBucketState
{
  bool done;
  int i;
  int64 tunits;
  int64 torigin;
  TimestampTz mint;
  TimestampTz maxt;
  TimestampTz t;
} PeriodBucketState;

/**
 * Struct for storing the state that persists across multiple calls generating
 * the multidimensional grid
 */
typedef struct TboxGridState
{
  bool done;
  int i;
  double xsize;
  int64 tunits;
  TBOX box;
  double value;
  TimestampTz t;
} TboxGridState;

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal fragments
 */
typedef struct ValueSplitState
{
  bool done;
  Datum size;
  Datum *buckets;
  Temporal **fragments;
  int i;
  int count;
} ValueSplitState;

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal fragments
 */
typedef struct TimeSplitState
{
  bool done;
  int64 tunits;
  TimestampTz *buckets;
  Temporal **fragments;
  int i;
  int count;
} TimeSplitState;

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal fragments
 */
typedef struct ValueTimeSplitState
{
  bool done;
  Datum *value_buckets;
  TimestampTz *time_buckets;
  Temporal **fragments;
  int i;
  int count;
} ValueTimeSplitState;

/*****************************************************************************/

extern double float_bucket(double value, double size, double origin);
extern TimestampTz timestamptz_bucket(TimestampTz timestamp, int64 tunits,
  TimestampTz torigin);
extern int64 get_interval_units(Interval *interval);

extern Temporal **temporal_time_split(Temporal *temp, TimestampTz start,
  TimestampTz end, int64 tunits, TimestampTz torigin, int count,
  TimestampTz **buckets, int *newcount);

/*****************************************************************************/

#endif /* __TEMPORAL_TILE_H__ */


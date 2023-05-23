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

#ifndef __TEMPORAL_TILE_H__
#define __TEMPORAL_TILE_H__

/* MEOS */
#include "temporal.h"

#define MAXDIMS 4

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls generating
 * the bucket list
 */
typedef struct SpanBucketState
{
  bool done;
  int i;
  meosType basetype;
  Temporal *temp; /* NULL when generating bucket list, used for splitting */
  Datum size;
  Datum origin;
  Datum minvalue;
  Datum maxvalue;
  Datum value;
} SpanBucketState;

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
  TBox box;
  double value;
  TimestampTz t;
} TboxGridState;

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal fragments
 */
typedef struct ValueTimeSplitState
{
  bool done;           /**< True when all tiles have been processed */
  int i;               /**< Number of current tile */
  Datum size;
  int64 tunits;
  Datum *value_buckets;
  TimestampTz *time_buckets;
  Temporal **fragments;
  int count;
} ValueTimeSplitState;

/*****************************************************************************/

extern void span_bucket_set(Datum lower, Datum size, meosType basetype,
  Span *span);
extern Span *span_bucket_get(Datum lower, Datum size, meosType basetype);
extern SpanBucketState *span_bucket_state_make(const Span *s, Datum size,
  Datum origin);
extern void span_bucket_state_next(SpanBucketState *state);

extern void tbox_tile_get(double value, TimestampTz t, double xsize,
  int64 tunits, TBox *box);
extern TboxGridState *tbox_tile_state_make(const TBox *box, double xsize,
  const Interval *duration, double xorigin, TimestampTz torigin);
extern void tbox_tile_state_next(TboxGridState *state);

/*****************************************************************************/

extern int64 interval_units(const Interval *interval);
extern TimestampTz timestamptz_bucket1(TimestampTz timestamp, int64 tunits,
  TimestampTz torigin);
extern Datum datum_bucket(Datum value, Datum size, Datum offset,
  meosType basetype);

extern Temporal **temporal_time_split1(const Temporal *temp, TimestampTz start,
  TimestampTz end, int64 tunits, TimestampTz torigin, int count,
  TimestampTz **buckets, int *newcount);

Temporal **
temporal_value_time_split1(Temporal *temp, Datum size, Interval *duration,
  Datum vorigin, TimestampTz torigin, bool valuesplit, bool timesplit,
  Datum **value_buckets, TimestampTz **time_buckets, int *newcount);

/*****************************************************************************/

#endif /* __TEMPORAL_TILE_H__ */


/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
#include <meos.h>
#include "general/meos_catalog.h"

#define MAXDIMS 4

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls generating
 * the bucket list
 */
typedef struct SpanBucketState
{
  bool done;       /**< True when the state is consumed */
  uint8 basetype;  /**< span basetype */
  char padding[2]; /**< Not used */
  int i;           /**< Current tile number */
  Temporal *temp;  /**< NULL when generating bucket list, used for splitting */
  Datum size;      /**< Size of the values */ 
  Datum origin;    /**< Origin of the values */
  Datum minvalue;  /**< Maximum value */
  Datum maxvalue;  /**< Maximum value */
  Datum value;     /**< Current value */
} SpanBucketState;

/**
 * @brief Struct for storing the state for tiling operations
 */
typedef struct TboxGridState
{
  bool done;       /**< True when the state is consumed */
  char padding[3]; /**< Not used */
  int i;           /**< Current tile number */
  Datum vsize;     /**< Vertical size of the values */
  int64 tunits;    /**< Horizontal size of the time */
  TBox box;        /**< Bounding box */
  Datum value;     /**< Current value */
  TimestampTz t;   /**< Current time */
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

extern void tbox_tile_get(Datum value, TimestampTz t, Datum vsize,
  int64 tunits, meosType basetype, TBox *box);
extern TboxGridState *tbox_tile_state_make(const TBox *box, Datum vsize,
  const Interval *duration, Datum xorigin, TimestampTz torigin);
extern void tbox_tile_state_next(TboxGridState *state);
extern TBox *tbox_tile(Datum value, TimestampTz t, Datum vsize,
  Interval *duration, Datum vorigin, TimestampTz torigin, meosType basetype);
  
/*****************************************************************************/

extern int64 interval_units(const Interval *interval);
extern TimestampTz timestamptz_bucket1(TimestampTz timestamp, int64 tunits,
  TimestampTz torigin);
extern Datum datum_bucket(Datum value, Datum size, Datum offset,
  meosType basetype);

extern Temporal **temporal_time_split(Temporal *temp, Interval *duration,
  TimestampTz torigin, TimestampTz **time_buckets, int *count);
extern Temporal **tnumber_value_time_split(Temporal *temp, Datum size,
  Interval *duration, Datum vorigin, TimestampTz torigin,
  Datum **value_buckets, TimestampTz **time_buckets, int *count);

/*****************************************************************************/

#endif /* __TEMPORAL_TILE_H__ */


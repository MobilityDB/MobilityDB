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

#ifndef __PG_TEMPORAL_TILE_H__
#define __PG_TEMPORAL_TILE_H__

/* MobilityDB */
#include "general/temporal.h"

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls generating
 * the bucket list
 */
typedef struct SpanBucketState
{
  bool done;
  int i;
  mobdbType basetype;
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
  TBOX box;
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

extern int64 get_interval_units(Interval *interval);

/*****************************************************************************/

#endif /* __PG_TEMPORAL_TILE_H__ */


/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
#include "temporal/meos_catalog.h"

#define MAXDIMS 4

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls generating
 * the bin list
 */
typedef struct SpanBinState
{
  bool done;            /**< True when the state is consumed */
  uint8 basetype;       /**< span basetype */
  int i;                /**< Current tile number */
  Datum size;           /**< Size of the values */
  Datum origin;         /**< Origin of the values */
  Span span;            /**< Bounding span */
  const void *to_split; /**< NULL when generating bins, used for splitting
                         * span sets or temporal values*/
  Datum value;          /**< Current value */
  int nbins;            /**< Total number of bins */
} SpanBinState;

/**
 * @brief Struct for storing the state for tiling operations
 */
typedef struct TboxGridState
{
  bool done;            /**< True when the state is consumed */
  int i;                /**< Current tile number */
  Datum vsize;          /**< Value size */
  int64 tunits;         /**< Time size */
  TBox box;             /**< Bounding box */
  const Temporal *temp; /**< Optional temporal number to be split */
  Datum value;          /**< Current value */
  TimestampTz t;        /**< Current time */
  int ntiles;           /**< Total number of tiles */
  int max_coords[2];    /**< Maximum coordinates of the tiles */
  int coords[2];        /**< Coordinates of the current tile */
} TboxGridState;

/*****************************************************************************/

extern int span_num_bins(const Span *s, Datum size, Datum origin, 
  Datum *start_bin, Datum *end_bin);

extern Span *span_bins(const Span *s, Datum size, Datum origin, int *count);
extern Span *spanset_bins(const SpanSet *ss, Datum size, Datum origin, int *count);

extern SpanBinState *temporal_time_bin_init(const Temporal *temp,
  const Interval *duration, TimestampTz torigin, int *nbins);

extern TboxGridState *tbox_tile_state_make(const Temporal *temp,
  const TBox *box, Datum vsize, const Interval *duration, Datum xorigin,
  TimestampTz torigin);
extern void tbox_tile_state_next(TboxGridState *state);
extern void tbox_tile_state_set(Datum value, TimestampTz t, Datum vsize,
  int64 tunits, meosType basetype, meosType spantype, TBox *box);

/*****************************************************************************/

extern int64 interval_units(const Interval *interval);
extern TimestampTz timestamptz_bin_start(TimestampTz timestamp, int64 tunits,
  TimestampTz torigin);
extern Datum datum_bin(Datum value, Datum size, Datum offset,
  meosType basetype);

extern TboxGridState *tnumber_value_time_tile_init(const Temporal *temp,
  Datum vsize, const Interval *duration, Datum vorigin, TimestampTz torigin,
  int *ntiles);
extern bool tbox_tile_state_get(TboxGridState *state, TBox *box);

/*****************************************************************************/
#endif /* __TEMPORAL_TILE_H__ */


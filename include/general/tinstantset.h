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
 * @file tinstantset.h
 * Basic functions for temporal instant sets.
 */

#ifndef __TINSTANTSET_H__
#define __TINSTANTSET_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/array.h>
#include <utils/rangetypes.h>
/* MobilityDB */
#include "general/temporal.h"

/*****************************************************************************/

/* General functions */

extern const TInstant *tinstantset_inst_n(const TInstantSet *ti, int index);
extern TInstantSet *tinstantset_make(const TInstant **instants, int count,
  bool merge);
extern TInstantSet *tinstantset_make_free(TInstant **instants, int count,
  bool merge);
extern TInstantSet *tinstantset_copy(const TInstantSet *ti);

extern void *tinstantset_bbox_ptr(const TInstantSet *ti);
extern void tinstantset_bbox(const TInstantSet *ti, void *box);
extern TInstantSet *tinstantset_make1(const TInstant **instants, int count);
extern bool tinstantset_find_timestamp(const TInstantSet *ti, TimestampTz t,
  int *pos);

/* Input/output functions */

extern char *tinstantset_to_string(const TInstantSet *ti,
  char *(*value_out)(Oid, Datum));
extern void tinstantset_write(const TInstantSet *ti, StringInfo buf);
extern TInstantSet *tinstantset_read(StringInfo buf, CachedType temptype);

/* Constructor functions */

extern TInstantSet *tinstantset_from_base(Datum value, CachedType temptype,
  const TimestampSet *ts);

/* Accessor functions */

extern Datum *tinstantset_values(const TInstantSet *ti, int *count);
extern RangeType **tfloatinstset_ranges(const TInstantSet *ti, int *count);
extern PeriodSet *tinstantset_time(const TInstantSet *ti);
extern Datum tinstantset_min_value(const TInstantSet *ti);
extern Datum tinstantset_max_value(const TInstantSet *ti);
extern void tinstantset_period(const TInstantSet *ti, Period *p);
extern Interval *tinstantset_timespan(const TInstantSet *ti);
extern TSequence **tinstantset_sequences(const TInstantSet *ti);
extern const TInstant **tinstantset_instants(const TInstantSet *ti);
extern TimestampTz tinstantset_start_timestamp(const TInstantSet *ti);
extern TimestampTz tinstantset_end_timestamp(const TInstantSet *ti);
extern TimestampTz *tinstantset_timestamps(const TInstantSet *ti);

/* Cast functions */

TInstantSet *tintinstset_tfloatinstset(const TInstantSet *ti);
TInstantSet *tfloatinstset_tintinstset(const TInstantSet *ti);

/* Transformation functions */

extern TInstantSet *tinstant_tinstantset(const TInstant *inst);
extern TInstantSet *tsequence_tinstantset(const TSequence *seq);
extern TInstantSet *tsequenceset_tinstantset(const TSequenceSet *ts);
extern TInstantSet *tinstantset_shift_tscale(const TInstantSet *ti,
  const Interval *start, const Interval *duration);

/* Ever/always comparison operators */

extern bool tinstantset_ever_eq(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_eq(const TInstantSet *ti, Datum value);
extern bool tinstantset_ever_lt(const TInstantSet *ti, Datum value);
extern bool tinstantset_ever_le(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_lt(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_le(const TInstantSet *ti, Datum value);

/* Restriction Functions */

extern TInstantSet *tinstantset_restrict_value(const TInstantSet *ti,
  Datum value, bool atfunc);
extern TInstantSet *tinstantset_restrict_values(const TInstantSet *ti,
  const Datum *values, int count, bool atfunc);
extern TInstantSet *tnumberinstset_restrict_range(const TInstantSet *ti,
  const RangeType *range, bool atfunc);
extern TInstantSet *tnumberinstset_restrict_ranges(const TInstantSet *ti,
  RangeType **normranges, int count, bool atfunc);
extern const TInstant *tinstantset_min_instant(const TInstantSet *ti);
extern const TInstant *tinstantset_max_instant(const TInstantSet *ti);
extern TInstantSet *tinstantset_restrict_minmax(const TInstantSet *ti,
  bool min, bool atfunc);
extern Temporal *tinstantset_restrict_timestamp(const TInstantSet *ti,
  TimestampTz t, bool atfunc);
extern TInstantSet *tinstantset_restrict_timestampset(const TInstantSet *ti,
  const TimestampSet *ts, bool atfunc);
extern TInstantSet *tinstantset_restrict_period(const TInstantSet *ti,
  const Period *p, bool atfunc);
extern TInstantSet *tinstantset_restrict_periodset(const TInstantSet *ti,
  const PeriodSet *ps, bool atfunc);

extern bool tinstantset_value_at_timestamp(const TInstantSet *ti,
  TimestampTz t, Datum *result);

/* Append and merge functions */

extern TInstantSet *tinstantset_append_tinstant(const TInstantSet *ti,
  const TInstant *inst);
extern Temporal *tinstantset_merge(const TInstantSet *ti1,
  const TInstantSet *ti2);
extern Temporal *tinstantset_merge_array(const TInstantSet **tis, int count);

/* Intersection functions */

extern bool intersection_tinstantset_tinstant(const TInstantSet *ti,
  const TInstant *inst, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstant_tinstantset(const TInstant *inst,
  const TInstantSet *ti, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstantset_tinstantset(const TInstantSet *ti1,
  const TInstantSet *ti2, TInstantSet **inter1, TInstantSet **inter2);

/* Intersects Functions */

extern bool tinstantset_intersects_timestamp(const TInstantSet *ti,
  const TimestampTz t);
extern bool tinstantset_intersects_timestampset(const TInstantSet *ti,
  const TimestampSet *ts);
extern bool tinstantset_intersects_period(const TInstantSet *ti,
  const Period *p);
extern bool tinstantset_intersects_periodset(const TInstantSet *ti,
  const PeriodSet *ps);

/* Local aggregate functions */

extern double tnumberinstset_twavg(const TInstantSet *ti);

/* Comparison functions */

extern int tinstantset_cmp(const TInstantSet *ti1, const TInstantSet *ti2);
extern bool tinstantset_eq(const TInstantSet *ti1, const TInstantSet *ti2);

/* Function for defining hash index */

extern uint32 tinstantset_hash(const TInstantSet *ti);

/*****************************************************************************/

#endif

/*****************************************************************************
 *
 * tinstant.h
 *    Basic functions for temporal instants.
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

#ifndef __TINSTANT_H__
#define __TINSTANT_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/array.h>
#include <utils/rangetypes.h>

#include "temporal.h"
#include "postgis.h"

/*****************************************************************************/

extern TInstant *tinstant_make(Datum value, TimestampTz t, Oid valuetypid);
extern TInstant *tinstant_copy(const TInstant *inst);
extern Datum* tinstant_value_ptr(const TInstant *inst);
extern Datum tinstant_value(const TInstant *inst);
extern Datum tinstant_value_copy(const TInstant *inst);
extern void tinstant_set(TInstant *inst, Datum value, TimestampTz t);

/* Input/output functions */

extern char *tinstant_to_string(const TInstant *inst, char *(*value_out)(Oid, Datum));
extern void tinstant_write(const TInstant *inst, StringInfo buf);
extern TInstant *tinstant_read(StringInfo buf, Oid valuetypid);

/* Intersection function */

extern bool intersection_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  TInstant **inter1, TInstant **inter2);

/* Append and merge functions */

extern Temporal *tinstant_append_tinstant(const TInstant *inst1, const TInstant *inst2);
extern Temporal *tinstant_merge(const TInstant *inst1, const TInstant *inst2);
extern Temporal *tinstant_merge_array(TInstant **instants, int count);

/* Cast functions */

extern TInstant *tintinst_to_tfloatinst(const TInstant *inst);
extern TInstant *tfloatinst_to_tintinst(const TInstant *inst);

/* Transformation functions */

extern TInstant *tinstantset_to_tinstant(const TInstantSet *ti);
extern TInstant *tsequence_to_tinstant(const TSequence *seq);
extern TInstant *tsequenceset_to_tinstant(const TSequenceSet *ts);

/* Accessor functions */

extern ArrayType *tinstant_values(const TInstant *inst);
extern ArrayType *tfloatinst_ranges(const TInstant *inst);
extern PeriodSet *tinstant_get_time(const TInstant *inst);
extern void tinstant_period(Period *p, const TInstant *inst);
extern ArrayType *tinstant_timestamps(const TInstant *inst);
extern ArrayType *tinstant_instants_array(const TInstant *inst);
extern TInstant *tinstant_shift(const TInstant *inst, const Interval *interval);

extern bool tinstant_ever_eq(const TInstant *inst, Datum value);
extern bool tinstant_ever_lt(const TInstant *inst, Datum value);
extern bool tinstant_ever_le(const TInstant *inst, Datum value);

extern bool tinstant_always_eq(const TInstant *inst, Datum value);
extern bool tinstant_always_lt(const TInstant *inst, Datum value);
extern bool tinstant_always_le(const TInstant *inst, Datum value);

/* Restriction Functions */

extern TInstant *tinstant_restrict_value(const TInstant *inst,
  Datum value, bool atfunc);
extern bool tinstant_restrict_values_test(const TInstant *inst,
  const Datum *values, int count, bool atfunc);
  extern TInstant *tinstant_restrict_values(const TInstant *inst,
  const Datum *values, int count, bool atfunc);
extern bool tnumberinst_restrict_range_test(const TInstant *inst,
  RangeType *range, bool atfunc);
extern TInstant *tnumberinst_restrict_range(const TInstant *inst,
  RangeType *range, bool atfunc);
extern bool tnumberinst_restrict_ranges_test(const TInstant *inst,
  RangeType **normranges, int count, bool atfunc);
extern TInstant *tnumberinst_restrict_ranges(const TInstant *inst,
  RangeType **normranges, int count, bool atfunc);
extern TInstant *tinstant_restrict_timestamp(const TInstant *inst,
  TimestampTz t, bool atfunc);
extern bool tinstant_value_at_timestamp(const TInstant *inst,
  TimestampTz t, Datum *result);
extern bool tinstant_restrict_timestampset_test(const TInstant *inst,
  const TimestampSet *ts, bool atfunc);
extern TInstant *tinstant_restrict_timestampset(const TInstant *inst,
  const TimestampSet *ts, bool atfunc);
extern TInstant *tinstant_restrict_period(const TInstant *inst,
  const Period *p, bool atfunc);
extern bool tinstant_restrict_periodset_test(const TInstant *inst,
  const PeriodSet *ps, bool atfunc);
extern TInstant *tinstant_restrict_periodset(const TInstant *inst,
  const PeriodSet *ps, bool atfunc);

/* Intersection Functions */

extern bool tinstant_intersects_timestamp(const TInstant *inst, TimestampTz t);
extern bool tinstant_intersects_timestampset(const TInstant *inst, const TimestampSet *ts);
extern bool tinstant_intersects_period(const TInstant *inst, const Period *p);
extern bool tinstant_intersects_periodset(const TInstant *inst, const PeriodSet *ps);

/* Comparison functions */

extern int tinstant_cmp(const TInstant *inst1, const TInstant *inst2);
extern bool tinstant_eq(const TInstant *inst1, const TInstant *inst2);

/* Function for defining hash index */

extern uint32 tinstant_hash(const TInstant *inst);

/*****************************************************************************/

#endif

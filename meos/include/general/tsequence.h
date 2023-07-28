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

/**
 * @brief Basic functions for temporal sequences.
 */

#ifndef __TSEQUENCE_H__
#define __TSEQUENCE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"
#include "general/span.h"

/*****************************************************************************/

/* General functions */

extern int tcontseq_find_timestamp(const TSequence *seq, TimestampTz t);
extern int tdiscseq_find_timestamp(const TSequence *seq, TimestampTz t);
extern void tsequence_make_valid1(const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp);
extern TSequence *tsequence_make1_exp(const TInstant **instants, int count,
  int maxcount, bool lower_inc, bool upper_inc, interpType interp,
  bool normalize, void *bbox);
extern TSequence **tseqarr2_to_tseqarr(TSequence ***sequences, int *countseqs,
  int count, int totalseqs);

/* Append and merge functions */

extern double datum_distance(Datum value1, Datum value2, meosType basetype,
  int16 flags);
extern bool tsequence_join_test(const TSequence *seq1, const TSequence *seq2,
  bool *removelast, bool *removefirst);
extern TSequence *tsequence_join(const TSequence *seq1, const TSequence *seq2,
  bool removelast, bool removefirst);
extern TSequence **tseqarr_normalize(const TSequence **sequences, int count,
  int *newcount);
extern TSequence **tsequence_merge_array1(const TSequence **sequences,
  int count, int *totalcount);

/* Synchronization functions */

extern bool synchronize_tsequence_tsequence(const TSequence *seq1,
  const TSequence *seq2, TSequence **sync1, TSequence **sync2,
  bool interpoint);

/* Intersection functions */

extern bool tlinearsegm_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, meosType basetype, Datum *inter,
  TimestampTz *t);
extern bool tsegment_intersection(const TInstant *start1,
  const TInstant *end1, bool linear1, const TInstant *start2,
  const TInstant *end2, bool linear2, Datum *inter1, Datum *inter2,
  TimestampTz *t);

extern bool intersection_tdiscseq_tdiscseq(const TSequence *seq1,
  const TSequence *seq2, TSequence **inter1, TSequence **inter2);
extern bool intersection_tcontseq_tdiscseq(const TSequence *seq1,
  const TSequence *seq2, TSequence **inter1, TSequence **inter2);
extern bool intersection_tdiscseq_tcontseq(const TSequence *is,
  const TSequence *seq2, TSequence **inter1, TSequence **inter2);

extern bool intersection_tsequence_tinstant(const TSequence *seq,
  const TInstant *inst, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstant_tsequence(const TInstant *inst,
  const TSequence *seq, TInstant **inter1, TInstant **inter2);

/* Input/output functions */

extern char *tsequence_to_string(const TSequence *seq, int maxdd,
  bool component, outfunc value_out);

/* Constructor functions */

extern void ensure_increasing_timestamps(const TInstant *inst1,
  const TInstant *inst2, bool strict);
extern void bbox_expand(const void *box1, void *box2, meosType temptype);
extern void ensure_valid_tinstarr(const TInstant **instants, int count,
  bool merge, interpType interp);

/* Transformation functions */

extern void tsequence_shift_tscale_iter(TSequence *seq, TimestampTz delta,
  double scale);

/* Accessor functions */

extern int tfloatseq_spans(const TSequence *seq, Span *result);
extern int tsequence_segments_iter(const TSequence *seq, TSequence **result);
extern int tsequence_timestamps_iter(const TSequence *seq, TimestampTz *result);
extern Datum tsegment_value_at_timestamp(const TInstant *inst1,
  const TInstant *inst2, bool linear, TimestampTz t);

/* Restriction Functions */

extern int tcontseq_restrict_value_iter(const TSequence *seq, Datum value,
  bool atfunc, TSequence **result);
extern int tsequence_at_values_iter(const TSequence *seq, const Set *set,
  TSequence **result);
extern int tnumbercontseq_restrict_span_iter(const TSequence *seq,
  const Span *span, bool atfunc, TSequence **result);
extern int tnumbercontseq_restrict_spanset_iter(const TSequence *seq,
  const SpanSet *ss, bool atfunc, TSequence **result);
extern TInstant *tsegment_at_timestamp(const TInstant *inst1,
  const TInstant *inst2, bool linear, TimestampTz t);
extern int tcontseq_minus_timestamp_iter(const TSequence *seq, TimestampTz t,
  TSequence **result);
extern int tcontseq_minus_timestampset_iter(const TSequence *seq, const Set *s,
  TSequence **result);
extern int tcontseq_at_periodset1(const TSequence *seq, const SpanSet *ps,
  TSequence **result);
extern int tcontseq_minus_periodset_iter(const TSequence *seq, const SpanSet *ps,
  TSequence **result);

/*****************************************************************************/

#endif

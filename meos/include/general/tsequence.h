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

/**
 * @brief Basic functions for temporal sequences.
 */

#ifndef __TSEQUENCE_H__
#define __TSEQUENCE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"
#include "general/temporal.h"

/*****************************************************************************/

extern bool
tsequence_norm_test(Datum value1, Datum value2, Datum value3, meosType basetype,
  interpType interp, TimestampTz t1, TimestampTz t2, TimestampTz t3);
  
/* General functions */

extern int tcontseq_find_timestamptz(const TSequence *seq, TimestampTz t);
extern int tdiscseq_find_timestamptz(const TSequence *seq, TimestampTz t);
extern bool ensure_valid_tinstarr_common(const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp);
extern TSequence *tsequence_make_exp1(const TInstant **instants, int count,
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

extern bool tfloatsegm_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, meosType basetype, TimestampTz *t);
extern bool tlinearsegm_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, meosType basetype, Datum *inter,
  TimestampTz *t);
extern bool tsegment_intersection(const TInstant *start1,
  const TInstant *end1, interpType interp1, const TInstant *start2,
  const TInstant *end2, interpType interp2, Datum *inter1, Datum *inter2,
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

extern bool ensure_increasing_timestamps(const TInstant *inst1,
  const TInstant *inst2, bool strict);
extern void bbox_expand(const void *box1, void *box2, meosType temptype);
extern bool ensure_valid_tinstarr(const TInstant **instants, int count,
  bool merge, interpType interp);

/* Transformation functions */

extern void tnumberseq_shift_scale_value_iter(TSequence *seq, Datum origin,
  Datum delta, bool hasdelta, double scale);
extern void tsequence_shift_scale_time_iter(TSequence *seq, TimestampTz delta,
  double scale);
extern int tstepseq_to_linear_iter(const TSequence *seq, TSequence **result);
extern TSequenceSet *tstepseq_to_linear(const TSequence *seq);

/* Accessor functions */

extern int tfloatseq_spans(const TSequence *seq, Span *result);
extern int tsequence_segments_iter(const TSequence *seq, TSequence **result);
extern int tsequence_timestamps_iter(const TSequence *seq, TimestampTz *result);
extern Datum tsegment_value_at_timestamptz(const TInstant *inst1,
  const TInstant *inst2, interpType interp, TimestampTz t);

/* Local Aggregate Functions */

extern double tnumbercontseq_twavg(const TSequence *seq);

/*****************************************************************************/

#endif

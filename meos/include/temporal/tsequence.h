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

/**
 * @brief Basic functions for temporal sequences
 */

#ifndef __TSEQUENCE_H__
#define __TSEQUENCE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"

/*****************************************************************************/

/* Collinear function */

extern bool float_collinear(double x1, double x2, double x3, double ratio);

/* Interpolation functions */

extern double floatsegm_interpolate(double value1, double value2,
  long double value);
extern long double floatsegm_locate(double value1, double value2,
  double value);

extern int tnumbersegm_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, meosType basetype, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);

/* Normalization functions */

extern bool tsequence_norm_test(Datum value1, Datum value2, Datum value3,
  meosType basetype, interpType interp, TimestampTz t1, TimestampTz t2,
  TimestampTz t3);
extern bool tsequence_join_test(const TSequence *seq1, const TSequence *seq2,
  bool *removelast, bool *removefirst);
extern TSequence *tsequence_join(const TSequence *seq1, const TSequence *seq2,
  bool removelast, bool removefirst);
extern TInstant **tinstarr_normalize(const TInstant **instants,
  interpType interp, int count, int *newcount);

/* General functions */

extern int tcontseq_find_timestamptz(const TSequence *seq, TimestampTz t);
extern int tdiscseq_find_timestamptz(const TSequence *seq, TimestampTz t);
extern TSequence **tseqarr2_to_tseqarr(TSequence ***sequences, int *countseqs,
  int count, int totalseqs);

extern bool ensure_valid_tinstarr_common(const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp);
extern TSequence *tsequence_make_exp1(const TInstant **instants, int count,
  int maxcount, bool lower_inc, bool upper_inc, interpType interp,
  bool normalize, void *bbox);

/* Synchronization functions */

extern bool synchronize_tsequence_tsequence(const TSequence *seq1,
  const TSequence *seq2, TSequence **sync1, TSequence **sync2,
  bool interpoint);

/* Intersection functions */

extern int tfloatsegm_intersection_value(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t);
extern int tsegment_intersection_value(Datum start, Datum end, Datum value,
  meosType temptype, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2);
extern int tsegment_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, meosType temptype, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
extern Datum tsegment_value_at_timestamptz(Datum start, Datum end,
  meosType temptype, TimestampTz lower, TimestampTz upper, TimestampTz t);
  
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
extern bool tsequence_make_valid(const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp);

/* Transformation functions */

extern void tnumberseq_shift_scale_value_iter(TSequence *seq, Datum origin,
  Datum delta, bool hasdelta, double scale);
extern void tsequence_shift_scale_time_iter(TSequence *seq, TimestampTz delta,
  double scale);
extern int tstepseq_to_linear_iter(const TSequence *seq, TSequence **result);
extern TSequenceSet *tstepseq_to_linear(const TSequence *seq);

/* Accessor functions */

extern int tsequence_segments_iter(const TSequence *seq, TSequence **result);
extern int tsequence_timestamps_iter(const TSequence *seq, TimestampTz *result);

/* Local Aggregate Functions */

extern double tnumberseq_cont_twavg(const TSequence *seq);

/*****************************************************************************/

#endif

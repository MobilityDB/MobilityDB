/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @file
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

extern bool float_collinear(double x1, double x2, double x3, TimestampTz t1,
  TimestampTz t2, TimestampTz t3, bool constructed);

/* Interpolation functions */

extern double floatsegm_interpolate(double value1, double value2,
  long double value);
extern long double floatsegm_locate(double value1, double value2,
  double value);

extern int tnumbersegm_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, MeosType basetype, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);

/* Exact crossing functions */

extern int tfloatsegm_side(double start, double end, double value,
  TimestampTz lower, TimestampTz upper, TimestampTz t);
extern int tnumbersegm_side(double start1, double end1, double start2,
  double end2, TimestampTz lower, TimestampTz upper, TimestampTz t);
extern bool tfloatsegm_crossing(double start, double end, double value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t);
extern bool tnumbersegm_crossing(double start1, double end1, double start2,
  double end2, TimestampTz lower, TimestampTz upper, TimestampTz *t);

/* The durations, 2^52 microseconds, below which the offsets of a crossing
 * and their halves are doubles exactly */
#define FLOATSEGM_FILTER_MAXD (INT64CONST(1) << 52)

/**
 * @brief Return true if the crossing whose offset from the start of a period
 * of @p d microseconds is computed as @p x lies strictly inside the window of
 * one interior microsecond, and that microsecond in the last argument
 * @details The offset @p x is within a relative `5.02 u`, `u = 2^-53`, of
 * the exact offset `x*` of the crossing, and the period is shorter than
 * 2^52 microseconds, so that the microseconds and their halves are doubles
 * exactly. Then `|x - x*|` is below `x 2^-48`, which leaves more than the
 * rounding of the sum `|x - m| + x 2^-48`, the one inexact operation of the
 * test, `x - m` being exact where it is at most 1/2. Where that sum is at
 * most 1/2 for the microsecond `m`, `x*` lies strictly inside
 * `(m - 1/2, m + 1/2)`: the gap at `m + 1/2` has left its sign at the start
 * and the gap at `m - 1/2` has not, so `m` is the microsecond
 * #tfloatsegm_crossing answers, with no tie. The microsecond tried, the
 * integer part of `x + 1/2`, is interior, an offset whose rounded `x + 1/2`
 * is below 1, at least `d`, or not a number being refused, as a crossing
 * whose nearest microsecond may be a bound
 * @return False where the filter cannot tell, the caller then deciding the
 * crossing with exact signs
 */
static inline bool
floatsegm_offset_filter(double x, int64 d, int64 *m)
{
  /* Not a number fails both comparisons */
  double y = x + 0.5;
  if (! (y >= 1.0 && y < (double) d))
    return false;
  TimestampTz est = (TimestampTz) y;
  if (fabs(x - (double) est) + x * 0x1p-48 > 0.5)
    return false;
  *m = est;
  return true;
}

/**
 * @brief Return true if the crossing of two float segments whose gaps at the
 * bounds have opposite signs lies strictly inside the window of one interior
 * microsecond, decided from its offset computed in doubles, and that
 * microsecond in the output argument
 * @details A filter, as the one of #cross_product_sign: the offset of the
 * crossing is `x* = d a / (a - b)` with `a = start1 - start2` and
 * `b = end1 - end2` of opposite signs. Over doubles, a difference of two
 * doubles is exact or within a relative `u = 2^-53`, never in error below
 * the normal range; `a - b` adds two magnitudes, so its rounded operands and
 * its rounding keep it within `2u`; the quotient, in `(0, 1]`, and the
 * product by `d`, exact below 2^52, each add `u` where the offset computed,
 * `x`, is at least 1/2, the quotient being then normal. So `x` is within
 * `5.02 u x` of `x*`, which #floatsegm_offset_filter decides. An overflowing
 * difference makes the offset 0 or not a number, which it refuses
 * @param[in] start1,end1 Values defining the first segment
 * @param[in] start2,end2 Values defining the second segment
 * @param[in] d Duration of the segments in microseconds
 * @param[out] m Microsecond nearest to the crossing, from the start
 * @return False where the filter cannot tell, the caller then deciding the
 * crossing with exact signs
 */
static inline bool
floatsegm_crossing_filter(double start1, double end1, double start2,
  double end2, int64 d, int64 *m)
{
  if (d >= FLOATSEGM_FILTER_MAXD)
    return false;
  double a = start1 - start2, b = end1 - end2;
  return floatsegm_offset_filter((a / (a - b)) * (double) d, d, m);
}

/* Normalization functions */

extern bool tsequence_norm_test(Datum value1, Datum value2, Datum value3,
  MeosType basetype, interpType interp, TimestampTz t1, TimestampTz t2,
  TimestampTz t3);
extern bool tsequence_join_test(const TSequence *seq1, const TSequence *seq2,
  bool *removelast, bool *removefirst);
extern TSequence *tsequence_join(const TSequence *seq1, const TSequence *seq2,
  bool removelast, bool removefirst);
extern TInstant **tinstarr_normalize(TInstant **instants, interpType interp,
  int count, int *newcount);

/* General functions */

extern int tcontseq_find_timestamptz(const TSequence *seq, TimestampTz t);
extern int tdiscseq_find_timestamptz(const TSequence *seq, TimestampTz t);
extern TSequence **tseqarr2_to_tseqarr(TSequence ***sequences, int *countseqs,
  int count, int totalseqs);

extern bool ensure_valid_tinstarr_common(TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp);
extern TSequence *tsequence_make_exp1(TInstant **instants, int count,
  int maxcount, bool lower_inc, bool upper_inc, interpType interp,
  bool normalize, const void *bbox);

/* Synchronization functions */

extern bool synchronize_tsequence_tsequence(const TSequence *seq1,
  const TSequence *seq2, TSequence **sync1, TSequence **sync2,
  bool interpoint);

/* Intersection functions */

extern int tfloatsegm_intersection_value(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t);
extern int tsegment_intersection_value(Datum start, Datum end, Datum value,
  MeosType temptype, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2);
extern int tsegment_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, MeosType temptype, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
extern Datum tsegment_value_at_timestamptz(Datum start, Datum end,
  MeosType temptype, TimestampTz lower, TimestampTz upper, TimestampTz t);
  
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
  bool component, outfunc value_out, size_t *size_out);

/* Constructor functions */

extern bool ensure_increasing_timestamps(const TInstant *inst1,
  const TInstant *inst2, bool strict);
extern void bbox_expand(const void *box1, void *box2, MeosType temptype);
extern bool ensure_valid_tinstarr(TInstant **instants, int count, bool merge,
  interpType interp);
extern bool tsequence_make_valid(TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp);

/* Transformation functions */

extern bool tnumberseq_shift_scale_value_iter(TSequence *seq, Datum origin,
  Datum delta, bool hasdelta, double scale);
extern void tsequence_shift_scale_time_iter(TSequence *seq, TimestampTz delta,
  double scale);
extern int tstepseq_to_linear_iter(const TSequence *seq, TSequence **result);
extern TSequenceSet *tstepseq_to_linear(const TSequence *seq);

/* Accessor functions */

extern int tsequence_segments_iter(const TSequence *seq, TSequence **result);
extern int tsequence_timestamps_iter(const TSequence *seq, TimestampTz *times);

/* Local Aggregate Functions */

extern double tnumberseq_cont_twavg(const TSequence *seq);
extern double tnumber_integral_scale(const TBox *box);
extern double tnumberseq_integral_scaled(const TSequence *seq, double scale);

/*****************************************************************************/

#endif

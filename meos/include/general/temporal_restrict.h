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
 * @brief Restriction functions for temporal value.
 */

#ifndef __TEMPORAL_RESTRICT_H__
#define __TEMPORAL_RESTRICT_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"
#include "general/temporal.h"

/*****************************************************************************/

/* Restriction Functions */

extern TInstant *tdiscseq_at_timestamptz(const TSequence *seq, TimestampTz t);
extern TSequence *tdiscseq_restrict_value(const TSequence *seq, Datum value,
  bool atfunc);
extern TSequence *tdiscseq_restrict_values(const TSequence *seq, const Set *s,
  bool atfunc);
extern TSequence *tdiscseq_restrict_minmax(const TSequence *seq, bool min,
  bool atfunc);
extern TSequence *tdiscseq_minus_timestamptz(const TSequence *seq,
  TimestampTz t);
extern TSequence *tdiscseq_restrict_tstzset(const TSequence *seq,
  const Set *s, bool atfunc);
extern TSequence *tdiscseq_restrict_tstzspanset(const TSequence *seq,
  const SpanSet *ss, bool atfunc);
extern int tcontseq_restrict_value_iter(const TSequence *seq, Datum value,
  bool atfunc, TSequence **result);
extern TSequenceSet *tcontseq_restrict_minmax(const TSequence *seq, bool min,
  bool atfunc);
extern TSequence *tcontseq_delete_timestamptz(const TSequence *seq,
  TimestampTz t);
extern TSequence *tcontseq_delete_tstzset(const TSequence *seq,
  const Set *s);
extern TSequence *tcontseq_delete_tstzspanset(const TSequence *seq,
  const SpanSet *ss);
extern TSequence *tcontseq_at_tstzset(const TSequence *seq, const Set *s);
extern TSequenceSet *tcontseq_minus_timestamptz(const TSequence *seq,
  TimestampTz t);
extern TSequenceSet *tcontseq_minus_tstzset(const TSequence *seq,
  const Set *s);
extern TSequenceSet *tcontseq_minus_tstzspan(const TSequence *seq,
  const Span *s);
extern TSequenceSet *tcontseq_restrict_value(const TSequence *seq, Datum value,
  bool atfunc);
extern TSequenceSet *tcontseq_restrict_values(const TSequence *seq,
  const Set *s, bool atfunc);
extern int tsequence_at_values_iter(const TSequence *seq, const Set *set,
  TSequence **result);
extern int tnumbercontseq_restrict_span_iter(const TSequence *seq,
  const Span *span, bool atfunc, TSequence **result);
extern int tnumbercontseq_restrict_spanset_iter(const TSequence *seq,
  const SpanSet *ss, bool atfunc, TSequence **result);
extern TInstant *tsegment_at_timestamptz(const TInstant *inst1,
  const TInstant *inst2, interpType interp, TimestampTz t);
extern int tcontseq_minus_timestamp_iter(const TSequence *seq, TimestampTz t,
  TSequence **result);
extern int tcontseq_minus_tstzset_iter(const TSequence *seq, const Set *s,
  TSequence **result);
extern int tcontseq_at_tstzspanset1(const TSequence *seq, const SpanSet *ss,
  TSequence **result);
extern int tcontseq_minus_tstzspanset_iter(const TSequence *seq, const SpanSet *ss,
  TSequence **result);
extern TSequence *tcontseq_at_tstzspan(const TSequence *seq, const Span *s);
extern TInstant *tcontseq_at_timestamptz(const TSequence *seq, TimestampTz t);
extern TSequenceSet *tcontseq_restrict_tstzspanset(const TSequence *seq,
  const SpanSet *ss, bool atfunc);
extern bool tdiscseq_value_at_timestamptz(const TSequence *seq, TimestampTz t,
  Datum *result);
extern TSequence *tnumberdiscseq_restrict_span(const TSequence *seq,
  const Span *span, bool atfunc);
extern TSequence *tnumberdiscseq_restrict_spanset(const TSequence *seq,
  const SpanSet *ss, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_span(const TSequence *seq,
  const Span *span, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_spanset(const TSequence *seq,
  const SpanSet *ss, bool atfunc);
extern double tnumbercontseq_twavg(const TSequence *seq);

/*****************************************************************************/

#endif /* __TEMPORAL_RESTRICT_H__ */

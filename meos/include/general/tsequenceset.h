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
 * @brief Basic functions for temporal sequence sets.
 */

#ifndef __TSEQUENCESET_H__
#define __TSEQUENCESET_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"
#include "general/span.h"

/*****************************************************************************/

/* General functions */

extern bool tsequenceset_find_timestamp(const TSequenceSet *ss, TimestampTz t,
  int *loc);

/* Synchronize functions */

extern bool synchronize_tsequenceset_tsequence(const TSequenceSet *ss,
  const TSequence *seq, SyncMode mode,
  TSequenceSet **inter1, TSequenceSet **inter2);
extern bool synchronize_tsequenceset_tsequenceset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, SyncMode mode,
  TSequenceSet **inter1, TSequenceSet **inter2);

/* Intersection functions */

extern bool intersection_tsequenceset_tinstant(const TSequenceSet *ss,
  const TInstant *inst, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstant_tsequenceset(const TInstant *inst,
  const TSequenceSet *ss, TInstant **inter1, TInstant **inter2);
extern bool intersection_tsequenceset_tdiscseq(const TSequenceSet *ss,
  const TSequence *is, TSequence **inter1, TSequence **inter2);
extern bool intersection_tdiscseq_tsequenceset(const TSequence *is,
  const TSequenceSet *ss, TSequence **inter1, TSequence **inter2);
extern bool intersection_tsequence_tsequenceset(const TSequence *seq,
  const TSequenceSet *ss, SyncMode mode,
  TSequenceSet **inter1, TSequenceSet **inter2);

/* Input/output functions */

extern char *tsequenceset_to_string(const TSequenceSet *ss, int maxdd,
  outfunc value_out);

/*****************************************************************************/

#endif

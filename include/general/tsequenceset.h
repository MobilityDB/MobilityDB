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
 * @file tsequenceset.h
 * Basic functions for temporal sequence sets.
 */

#ifndef __TSEQUENCESET_H__
#define __TSEQUENCESET_H__

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "general/temporal.h"
#include "general/span.h"

/*****************************************************************************/

/* General functions */

extern void *tsequenceset_bbox_ptr(const TSequenceSet *ts);
extern bool tsequenceset_find_timestamp(const TSequenceSet *ts, TimestampTz t,
  int *loc);

/* Synchronize functions */

extern bool synchronize_tsequenceset_tsequence(const TSequenceSet *ts,
  const TSequence *seq, SyncMode mode,
  TSequenceSet **inter1, TSequenceSet **inter2);
extern bool synchronize_tsequenceset_tsequenceset(const TSequenceSet *ts1,
  const TSequenceSet *ts2, SyncMode mode,
  TSequenceSet **inter1, TSequenceSet **inter2);

/* Intersection functions */

extern bool intersection_tsequenceset_tinstant(const TSequenceSet *ts,
  const TInstant *inst, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstant_tsequenceset(const TInstant *inst,
  const TSequenceSet *ts, TInstant **inter1, TInstant **inter2);
extern bool intersection_tsequenceset_tinstantset(const TSequenceSet *ts,
  const TInstantSet *ti, TInstantSet **inter1, TInstantSet **inter2);
extern bool intersection_tinstantset_tsequenceset(const TInstantSet *ti,
  const TSequenceSet *ts, TInstantSet **inter1, TInstantSet **inter2);
extern bool intersection_tsequence_tsequenceset(const TSequence *seq,
  const TSequenceSet *ts, SyncMode mode,
  TSequenceSet **inter1, TSequenceSet **inter2);

/* Input/output functions */

extern char *tsequenceset_to_string(const TSequenceSet *ts,
  char *(*value_out)(CachedType, Datum));


/*****************************************************************************/

#endif

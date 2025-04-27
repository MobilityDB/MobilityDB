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
 * @file
 * @brief Functions for temporal rigid geometries with sequence subtype
 */

#ifndef __TRGEO_SEQ_H__
#define __TRGEO_SEQ_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

extern const GSERIALIZED *trgeoseq_geom_p(const TSequence *seq);

extern size_t trgeoseq_pose_varsize(const TSequence *seq);
extern void trgeoseq_set_pose(TSequence *seq);
extern TSequence *trgeoseq_tposeseq(const TSequence *seq);

/* Constructor functions */

extern bool trgeoseq_make_valid(const GSERIALIZED *geom, const TInstant **instants,
  int count, bool lower_inc, bool upper_inc, bool linear);
extern TSequence *trgeoseq_make1_exp(const GSERIALIZED *geom, const TInstant **instants,
  int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *trgeoseq_make1(const GSERIALIZED *geom, const TInstant **instants,
  int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *trgeoseq_make_exp(const GSERIALIZED *geom, const TInstant **instants,
  int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *trgeoseq_make(const GSERIALIZED *geom, const TInstant **instants,
  int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *trgeoseq_make_free_exp(const GSERIALIZED *geom, TInstant **instants,
  int count, int maxcount, bool lower_inc, bool upper_inc, interpType interp, bool normalize);
extern TSequence *trgeoseq_make_free(const GSERIALIZED *geom, TInstant **instants,
  int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize);

/* Transformation functions */

extern TSequence *trgeoinst_to_tsequence(const TInstant *inst, interpType interp);
extern TInstant *trgeoseq_to_tinstant(const TSequence *seq);

extern TSequence *trgeoseq_to_discseq(const TSequence *seq);
extern TSequence *trgeoseq_to_contseq(const TSequence *seq);
extern TSequence *trgeoseqset_to_seq(const TSequenceSet *ss);

/* Accessor functions */

extern TSequence **trgeoseq_sequences(const TSequence *seq, int *count);
extern int trgeoseq_segments1(Datum geom, const TSequence *seq,
  TSequence **result);
extern TSequence **trgeoseq_segments(const TSequence *seq, int *count);

/*****************************************************************************/

#endif /* __TRGEO_SEQ_H__ */

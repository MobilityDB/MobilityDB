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
 * @brief Functions for temporal rigid geometries with sequence set subtype
 */

#ifndef __TRGEO_SEQSET_H__
#define __TRGEO_SEQSET_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/temporal.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

extern const GSERIALIZED *trgeoseqset_geom_p(const TSequenceSet *ts);
extern TSequenceSet *trgeoseqset_tposeseqset(const TSequenceSet *ss);

/* Constructor functions */

extern TSequenceSet *trgeoseqset_make1_exp(const GSERIALIZED *geom,
  const TSequence **sequences, int count, int maxcount, bool normalize);
extern TSequenceSet *trgeoseqset_make_exp(const GSERIALIZED *geom,
  const TSequence **sequences, int count, int maxcount, bool normalize);
extern TSequenceSet *trgeoseqset_make(const GSERIALIZED *geom,
  const TSequence **sequences, int count, bool normalize);
extern TSequenceSet *trgeoseqset_make_free(const GSERIALIZED *geom,
  TSequence **sequences, int count, bool normalize);
extern TSequenceSet *trgeoseqset_make_gaps(const GSERIALIZED *geom,
  const TInstant **instants, int count, interpType interp,
  Interval *maxt, double maxdist);

/* Transformation functions */

extern TSequence *trgeoseqset_to_tsequence(const TSequenceSet *ss);
extern TSequence *trgeo_to_tsequence(const Temporal *temp,
  const char *interp_str);
extern TSequenceSet *trgeo_to_tsequenceset(const Temporal *temp,
  const char *interp_str);

extern TSequenceSet *trgeoinst_to_seqset(const TInstant *inst,
  interpType interp);
extern TSequenceSet *trgeo_discseq_to_seqset(const TSequence *seq,
  interpType interp);
extern TSequence *trgeoseqset_to_discseq(const TSequenceSet *ss);
extern TSequenceSet *trgeoseq_to_seqset(const TSequence *seq);

/* Accessor functions */

extern TSequence **trgeoseqset_sequences(const TSequenceSet *ss, int *count);
extern TSequence **trgeoseqset_segments(const TSequenceSet *ss, int *count);

/*****************************************************************************/

#endif /* __TRGEO_SEQSET_H__ */

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
 * @brief Bounding box operators for temporal types.
 */

#ifndef __TEMPORAL_BOXOPS_H__
#define __TEMPORAL_BOXOPS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/meos_catalog.h"

/*****************************************************************************/

/**
 * @brief Return the size in bytes to read from toast to get the basic
 * information from a temporal: Temporal struct (i.e., TInstant, TSequence,
 * or TSequenceSet) and bounding box size
*/
#define TEMPORAL_MAX_HEADER_SIZE \
    DOUBLE_PAD(Max(Max(sizeof(TInstant), sizeof(TSequence)), \
    sizeof(TSequenceSet))) + DOUBLE_PAD(sizeof(bboxunion))

/*****************************************************************************/

/* Functions on generic bounding boxes of temporal types */

extern bool bbox_type(meosType bboxtype);
extern size_t bbox_get_size(meosType bboxtype);
extern int bbox_max_dims(meosType bboxtype);
extern bool temporal_bbox_eq(const void *box1, const void *box2,
  meosType temptype);
extern int temporal_bbox_cmp(const void *box1, const void *box2,
  meosType temptype);

/* Compute the bounding box at the creation of temporal values */

extern size_t temporal_bbox_size(meosType tempype);
extern void tinstant_set_bbox(const TInstant *inst, void *bbox);
extern void tinstarr_set_bbox(const TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp, void *bbox);
extern void tsequence_compute_bbox(TSequence *seq);
extern void tseqarr_compute_bbox(const TSequence **seqs, int count,
  void *bbox);
extern void tsequenceset_compute_bbox(TSequenceSet *ss);

/* Bounding box operators for temporal types */

extern bool boxop_temporal_tstzspan(const Temporal *temp, const Span *s,
  bool (*func)(const Span *, const Span *), bool invert);
extern bool boxop_temporal_temporal(const Temporal *temp1,
  const Temporal *temp2, bool (*func)(const Span *, const Span *));

extern bool boxop_tnumber_numspan(const Temporal *temp, const Span *span,
  bool (*func)(const Span *, const Span *), bool invert);
extern bool boxop_tnumber_tbox(const Temporal *temp, const TBox *box,
  bool (*func)(const TBox *, const TBox *), bool invert);
extern bool boxop_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const TBox *, const TBox *));

/*****************************************************************************/

#endif /* __TEMPORAL_BOXOPS_H__ */

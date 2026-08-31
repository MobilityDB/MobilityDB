/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief TPCBox bounding-box helpers for temporal pointcloud types
 *   (tpcpoint, tpcpatch). Mirrors the tspatial_boxops.h API for the
 *   STBox case.
 */

#ifndef __TPC_BOXOPS_H__
#define __TPC_BOXOPS_H__

#include <meos.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"

/*****************************************************************************/

extern void tpcbox_expand(const TPCBox *box1, TPCBox *box2);

extern void tstzspan_set_tpcbox(const Span *s, TPCBox *box);
extern void tpointcloudinst_set_tpcbox(const TInstant *inst, TPCBox *box);
extern void tpointcloudinstarr_set_tpcbox(TInstant **instants, int count,
  bool lower_inc, bool upper_inc, interpType interp, TPCBox *box);
extern void tpointcloudseq_expand_tpcbox(TSequence *seq, const TInstant *inst);
extern void tpointcloudseqarr_set_tpcbox(TSequence **sequences, int count,
  TPCBox *box);

/* Extent aggregation transition — folds @p temp's TPCBox into @p state.
 * Returns @p state on hit, a freshly-palloc'd TPCBox on first non-null
 * @p temp with NULL @p state, or @p NULL when both inputs are NULL.
 * Mirrors @c tspatial_extent_transfn for stbox. */
extern TPCBox *tpcbox_extent_transfn(TPCBox *state, const Temporal *temp);

/* Generic bbox dispatchers — apply a tpcbox-vs-tpcbox predicate to
 * the bounding boxes of one or two temporal pointcloud values.
 * Mirrors boxop_tspatial_{stbox,tspatial} from tgeo_boxops.c. */
extern bool boxop_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box,
  bool (*func)(const TPCBox *, const TPCBox *), bool inverted);
extern bool boxop_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2,
  bool (*func)(const TPCBox *, const TPCBox *));

/* Lossy tpcbox → stbox conversion: copies bounds + period + srid + the
 * X/Z/T dimension flags, drops pcid and never sets GEODETIC. Used by
 * the SP-GiST opclasses where the index storage is stbox and pcid
 * filtering is recovered by the operator's recheck. STBox is
 * typedef'd via meos.h above. */
extern void tpcbox_set_stbox(const TPCBox *src, STBox *dst);

/* Nearest-approach distance between two TPCBox values. Returns DBL_MAX
 * when their time spans don't overlap or the pcids differ. */
extern double nad_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);

/* Nearest-approach distance between a temporal pointcloud value and a
 * tpcbox / another temporal pointcloud value. */
extern double nad_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box);
extern double nad_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************/

#endif /* __TPC_BOXOPS_H__ */

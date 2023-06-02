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
 * @brief Bounding box operators for temporal points.
 */

#ifndef __TPOINT_BOXOPS_H__
#define __TPOINT_BOXOPS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include "general/temporal.h"
#include "general/type_util.h"
#include "point/stbox.h"

/*****************************************************************************/

/* Functions computing the bounding box at the creation of a temporal point */

extern void tpointinst_set_stbox(const TInstant *inst, STBox *box);
extern void tpointinstarr_set_stbox(const TInstant **instants, int count,
  STBox *box);
extern void tpointseq_expand_stbox(TSequence *seq, const TInstant *inst);
extern void tpointseqarr_set_stbox(const TSequence **sequences, int count,
  STBox *box);

/* Boxes functions */

extern STBox *tpointseq_stboxes(const TSequence *seq, int *count);
extern STBox *tpointseqset_stboxes(const TSequenceSet *ss, int *count);
extern STBox * tpoint_stboxes(const Temporal *temp, int *count);

/* Generic box functions */

extern int boxop_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool (*func)(const STBox *, const STBox *), bool invert);
extern Datum boxop_tpoint_stbox(const Temporal *temp, const STBox *box,
  bool (*func)(const STBox *, const STBox *), bool invert);
extern bool boxop_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const STBox *, const STBox *));

/*****************************************************************************/

#endif /* __TPOINT_BOXOPS_H__ */

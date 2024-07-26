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
 * @brief Bounding box operators for temporal pose objects.
 */

#ifndef __TPOSE_BOXOPS_H__
#define __TPOSE_BOXOPS_H__

#include "general/temporal.h"
#include "pose/tpose_static.h"

/*****************************************************************************/

extern bool pose_set_stbox(const Pose *pose, STBox *box);
extern bool pose_timestamp_set_stbox(const Pose *pose, TimestampTz t,
  STBox *box);
extern bool pose_period_set_stbox(const Pose *pose, const Span *p,
  STBox *box);
/* Functions computing the bounding box at the creation of a temporal pose */

extern void tposeinst_set_stbox(const TInstant *inst, STBox *box);
extern void tposeinstarr_set_stbox(const TInstant **instants, int count,
  STBox *box);
extern void tposeseq_expand_stbox(TSequence *seq, const TInstant *inst);

/*****************************************************************************/

#endif /* __TPOSE_BOXOPS_H__ */

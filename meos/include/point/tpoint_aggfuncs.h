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
 * @brief Aggregate functions for temporal points.
 */

#ifndef __TPOINT_AGGFUNCS_H__
#define __TPOINT_AGGFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include "general/skiplist.h"
#include "general/temporal.h"

/*****************************************************************************/

/**
 * Structure storing the SRID and the dimensionality of the temporal point
 * values for aggregation. Notice that for the moment we do not aggregate
 * temporal geographic points.
 */
struct GeoAggregateState
{
  int32_t srid;
  bool hasz;
};

/*****************************************************************************/

extern void geoaggstate_check(const SkipList *state, int32_t srid, bool hasz);
extern void geoaggstate_check_temp(const SkipList *state, const Temporal *t);
extern void geoaggstate_check_state(const SkipList *state1,
  const SkipList *state2);

extern Temporal **tpoint_transform_tcentroid(const Temporal *temp, int *count);
extern SkipList * tpoint_tcentroid_transfn(SkipList *state, Temporal *temp);
extern TSequence *tpointinst_tcentroid_finalfn(TInstant **instants, int count,
  int srid);
extern TSequenceSet *tpointseq_tcentroid_finalfn(TSequence **sequences,
  int count, int srid);
extern Temporal *tpoint_tcentroid_finalfn(SkipList *state);

/*****************************************************************************/

#endif

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
 * @file
 * @brief Aggregate functions for temporal network points.
 *
 * The only function currently provided is temporal centroid.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_aggfuncs.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_aggfuncs.h"
#include "npoint/tnpoint.h"

/*****************************************************************************/

/**
 * @brief Transition function for temporal centroid aggregation of temporal
 * network points
 */
SkipList *
tnpoint_tcentroid_transfn(SkipList *state, Temporal *temp)
{
  Temporal *temp1 = tnpoint_tgeompoint(temp);
  geoaggstate_check_temp(state, temp1);
  Datum (*func)(Datum, Datum) = MEOS_FLAGS_GET_Z(temp1->flags) ?
    &datum_sum_double4 : &datum_sum_double3;

  int count;
  Temporal **temparr = tpoint_transform_tcentroid(temp1, &count);
  if (state)
    skiplist_splice(state, (void **) temparr, count, func, false);
  else
  {
    state = skiplist_make((void **) temparr, count);
    struct GeoAggregateState extra =
    {
      .srid = tpoint_srid(temp1),
      .hasz = MEOS_FLAGS_GET_Z(temp1->flags) != 0
    };
    aggstate_set_extra(state, &extra, sizeof(struct GeoAggregateState));
  }

  pfree_array((void **) temparr, count);
  pfree(temp1);
  return state;
}

/*****************************************************************************/

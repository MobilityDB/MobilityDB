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
 * @brief Aggregate functions for temporal network points.
 *
 * The only function currently provided is temporal centroid.
 */

/* C */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include "pg_general/temporal_aggfuncs.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "pg_point/tpoint_aggfuncs.h"
#include "npoint/tnpoint.h"

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_tcentroid_transfn);
/**
 * Transition function for temporal centroid aggregation of temporal network
 * points
 */
PGDLLEXPORT Datum
Tnpoint_tcentroid_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *temp1 = tnpoint_tgeompoint(temp);

  geoaggstate_check_temp(state, temp1);
  Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp1->flags) ?
    &datum_sum_double4 : &datum_sum_double3;

  int count;
  Temporal **temparr = tpoint_transform_tcentroid(temp1, &count);
  if (state)
  {
    ensure_same_tempsubtype_skiplist(state, temparr[0]);
    skiplist_splice(fcinfo, state, (void **) temparr, count, func, false);
  }
  else
  {
    state = skiplist_make(fcinfo, (void **) temparr, count, TEMPORAL);
    struct GeoAggregateState extra =
    {
      .srid = tpoint_srid(temp1),
      .hasz = MOBDB_FLAGS_GET_Z(temp1->flags) != 0
    };
    aggstate_set_extra(fcinfo, state, &extra, sizeof(struct GeoAggregateState));
  }

  pfree_array((void **) temparr, count);
  pfree(temp1);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************/

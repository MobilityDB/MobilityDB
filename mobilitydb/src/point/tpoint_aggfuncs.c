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
 * @brief Aggregate functions for temporal points.
 *
 * The only functions currently provided are extent and temporal centroid.
 */

#include "point/tpoint_aggfuncs.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/doublen.h"
#include "general/skiplist.h"
#include "general/temporal_aggfuncs.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/skiplist.h"
#include "pg_general/temporal.h"

/*****************************************************************************
 * Extent
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_extent_transfn);
/**
 * @brief Transition function for temporal extent aggregation of temporal point
 * values
 */
Datum
Tpoint_extent_transfn(PG_FUNCTION_ARGS)
{
  STBox *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);
  STBox *result = tpoint_extent_transfn(box, temp);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Centroid
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_tcentroid_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_tcentroid_transfn);
/**
 * @brief Transition function for temporal centroid aggregation of temporal
 * network points
 */
Datum
Tpoint_tcentroid_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  state = tpoint_tcentroid_transfn(state, temp);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_tcentroid_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_tcentroid_combinefn);
/**
 * @brief Combine function for temporal centroid aggregation of temporal point
 * values
 */
Datum
Tpoint_tcentroid_combinefn(PG_FUNCTION_ARGS)
{
  SkipList *state1 = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  SkipList *state2 = PG_ARGISNULL(1) ? NULL :
    (SkipList *) PG_GETARG_POINTER(1);

  store_fcinfo(fcinfo);
  geoaggstate_check_state(state1, state2);
  struct GeoAggregateState *extra = NULL;
  if (state1 && state1->extra)
    extra = state1->extra;
  if (state2 && state2->extra)
    extra = state2->extra;
  assert(extra != NULL);
  datum_func2 func = extra->hasz ? &datum_sum_double4 : &datum_sum_double3;
  SkipList *result = temporal_tagg_combinefn(state1, state2, func, false);

  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_tcentroid_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_tcentroid_finalfn);
/**
 * @brief Final function for temporal centroid aggregation of temporal point
 * values
 */
Datum
Tpoint_tcentroid_finalfn(PG_FUNCTION_ARGS)
{
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  Temporal *result = tpoint_tcentroid_finalfn(state);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

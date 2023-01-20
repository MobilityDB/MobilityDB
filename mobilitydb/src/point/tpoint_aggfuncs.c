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

PG_FUNCTION_INFO_V1(Tpoint_extent_transfn);
/**
 * Transition function for temporal extent aggregation of temporal point values
 */
PGDLLEXPORT Datum
Tpoint_extent_transfn(PG_FUNCTION_ARGS)
{
  STBox *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);

  /* Can't do anything with null inputs */
  if (! box && ! temp)
    PG_RETURN_NULL();
  STBox *result = palloc0(sizeof(STBox));
  /* Null box and non-null temporal, return the bbox of the temporal */
  if (temp && ! box )
  {
    temporal_set_bbox(temp, result);
    PG_RETURN_POINTER(result);
  }
  /* Non-null box and null temporal, return the box */
  if (box && ! temp)
  {
    memcpy(result, box, sizeof(STBox));
    PG_RETURN_POINTER(result);
  }

  /* Both box and temporal are not null */
  ensure_same_srid_tpoint_stbox(temp, box);
  ensure_same_dimensionality(temp->flags, box->flags);
  ensure_same_geodetic(temp->flags, box->flags);
  temporal_set_bbox(temp, result);
  stbox_expand(box, result);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Centroid
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_tcentroid_transfn);
/**
 * Transition function for temporal centroid aggregation of temporal network
 * points
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Tpoint_tcentroid_combinefn);
/**
 * Combine function for temporal centroid aggregation of temporal point values
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Tpoint_tcentroid_finalfn);
/**
 * Final function for temporal centroid aggregation of temporal point values
 */
PGDLLEXPORT Datum
Tpoint_tcentroid_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  Temporal **values = (Temporal **) skiplist_values(state);
  int32_t srid = ((struct GeoAggregateState *) state->extra)->srid;
  Temporal *result;
  assert(values[0]->subtype == TINSTANT || values[0]->subtype == TSEQUENCE);
  if (values[0]->subtype == TINSTANT)
    result = (Temporal *) tpointinst_tcentroid_finalfn((TInstant **) values,
      state->length, srid);
  else /* values[0]->subtype == TSEQUENCE */
    result = (Temporal *) tpointseq_tcentroid_finalfn((TSequence **) values,
      state->length, srid);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

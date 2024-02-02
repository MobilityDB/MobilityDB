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
 * @file
 * @brief Window aggregate functions for temporal types
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/temporal_aggfuncs.h"
#include "general/temporal_waggfuncs.h"
/* MobilityDB */
#include "pg_general/skiplist.h" 

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Helper macro to input the current aggregate state
 */
#define INPUT_AGG_TRANS_STATE_ARG(fcinfo, state)  \
  do {  \
    MemoryContext ctx = set_aggregation_context(fcinfo); \
    state = PG_ARGISNULL(0) ? NULL : (SkipList *) PG_GETARG_SKIPLIST_P(0);  \
    if (PG_ARGISNULL(1) || PG_ARGISNULL(2))  \
    {  \
      if (state)  \
        PG_RETURN_SKIPLIST_P(state);  \
      else  \
        PG_RETURN_NULL();  \
    }  \
    unset_aggregation_context(ctx); \
  } while (0)

/**
 * @brief Generic moving window transition function for min, max, and sum
 * aggregation
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] min True if the calling function is min, max otherwise
 * @param[in] crossings True if turning points are added in the segments
 */
Datum
Temporal_wagg_transfn(FunctionCallInfo fcinfo, datum_func2 func, bool min,
  bool crossings)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE_ARG(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Interval *interval = PG_GETARG_INTERVAL_P(2);
  if ( temp->subtype != TINSTANT && ! MEOS_FLAGS_DISCRETE_INTERP(temp->flags) &&
      temp->temptype == T_TFLOAT && func == &datum_sum_float8)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Operation not supported for temporal continuous float sequences")));
  store_fcinfo(fcinfo);
  SkipList *result = temporal_wagg_transfn(state, temp, interval, func, min,
    crossings);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_SKIPLIST_P(result);
}

/**
 * @brief Transition function for moving window count and average aggregation
 * for temporal values
 */
Datum
Temporal_wagg_transform_transfn(FunctionCallInfo fcinfo, datum_func2 func,
  TSequence ** (*transform)(const Temporal *, const Interval *, int *))
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE_ARG(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Interval *interval = PG_GETARG_INTERVAL_P(2);
  store_fcinfo(fcinfo);
  SkipList *result = temporal_wagg_transform_transfn(state, temp, interval,
    func, transform);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(interval, 2);
  PG_RETURN_SKIPLIST_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tint_wmin_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_wmin_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window minimun aggregation for
 * temporal integers
 * @sqlfn wmin()
 */
Datum
Tint_wmin_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transfn(fcinfo, &datum_min_int32, GET_MIN, CROSSINGS);
}

PGDLLEXPORT Datum Tfloat_wmin_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_wmin_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window minimun aggregation for
 * temporal floats
 * @sqlfn wmin()
 */
Datum
Tfloat_wmin_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transfn(fcinfo, &datum_min_float8, GET_MIN, CROSSINGS);
}

PGDLLEXPORT Datum Tint_wmax_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_wmax_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window maximun aggregation for
 * temporal integers
 * @sqlfn wmax()
 */
Datum
Tint_wmax_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transfn(fcinfo, &datum_max_int32, GET_MAX, CROSSINGS);
}

PGDLLEXPORT Datum Tfloat_wmax_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_wmax_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window maximun aggregation for
 * temporal floats
 * @sqlfn wmax()
 */
Datum
Tfloat_wmax_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transfn(fcinfo, &datum_max_float8, GET_MAX, CROSSINGS);
}

PGDLLEXPORT Datum Tint_wsum_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_wsum_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window sum aggregation for temporal
 * integers
 * @sqlfn wsum()
 */
Datum
Tint_wsum_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transfn(fcinfo, &datum_sum_int32, GET_MIN, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tfloat_wsum_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_wsum_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window sum aggregation for temporal
 * floats
 * @sqlfn wsum()
 */
Datum
Tfloat_wsum_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transfn(fcinfo, &datum_sum_float8, GET_MIN, CROSSINGS);
}

PGDLLEXPORT Datum Temporal_wcount_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_wcount_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window count aggregation for temporal
 * values
 * @sqlfn wcount()
 */
Datum
Temporal_wcount_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transform_transfn(fcinfo, &datum_sum_int32,
    &temporal_transform_wcount);
}

PGDLLEXPORT Datum Tnumber_wavg_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_wavg_transfn);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Transition function for moving window average aggregation for
 * temporal numbers
 * @sqlfn wavg()
 */
Datum
Tnumber_wavg_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_wagg_transform_transfn(fcinfo, &datum_sum_double2,
    &tnumber_transform_wavg);
}

/*****************************************************************************/

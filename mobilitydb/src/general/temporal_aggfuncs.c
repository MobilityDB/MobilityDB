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
 * @brief General aggregate functions for temporal types.
 */

#include "general/temporal_aggfuncs.h"

/* C */
#include <assert.h>
#include <math.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_collation.h>
#include <libpq/pqformat.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/temporaltypes.h"
#include "general/tbool_boolops.h"
#include "general/doublen.h"
#include "general/time_aggfuncs.h"
/* MobilityDB */
#include "pg_general/skiplist.h"
#include "pg_general/temporal.h"

/*****************************************************************************
 * Generic aggregate functions for TInstant and TSequence
 *****************************************************************************/

/**
 * @brief Generic transition function for aggregating temporal values
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 */
static Datum
Temporal_tagg_transfn(FunctionCallInfo fcinfo, datum_func2 func, bool crossings)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  SkipList *result = temporal_tagg_transfn(state, temp, func, crossings);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Generic combine function for aggregating temporal alphanumeric values
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] crossings True if turning points are added in the segments
 */
static Datum
Temporal_tagg_combinefn(FunctionCallInfo fcinfo, datum_func2 func,
  bool crossings)
{
  SkipList *state1, *state2;
  INPUT_AGG_COMB_STATE(fcinfo, state1, state2);
  store_fcinfo(fcinfo);
  SkipList *result = temporal_tagg_combinefn(state1, state2, func, crossings);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_tagg_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_tagg_finalfn);
/**
 * @brief Generic final function for temporal aggregation
 */
Datum
Temporal_tagg_finalfn(PG_FUNCTION_ARGS)
{
  MemoryContext ctx = set_aggregation_context(fcinfo);
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  unset_aggregation_context(ctx);
  Temporal *result = temporal_tagg_finalfn(state);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * @brief Transition function for aggregating temporal values that require a
 * transformation to each composing instant/sequence
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @param[in] transform Transform function
 */
Datum
Temporal_tagg_transform_transfn(FunctionCallInfo fcinfo, datum_func2 func,
  bool crossings, TInstant *(*transform)(const TInstant *))
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  state = temporal_tagg_transform_transfn(state, temp, func, crossings,
    transform);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************
 * Temporal count
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_tcount_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_tcount_transfn);
/**
 * @brief Generic transition function for temporal aggregation
 */
Datum
Temporal_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  state = temporal_tcount_transfn(state, temp);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Temporal_tcount_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_tcount_combinefn);
/**
 * @brief Combine function for temporal count aggregation
 */
Datum
Temporal_tcount_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_sum_int32, false);
}

/*****************************************************************************
 * Temporal extent
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_extent_transfn);
/**
 * @brief Transition function for temporal extent aggregation of temporal
 * values with period bounding box
 */
Datum
Temporal_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);
  Span *result = temporal_extent_transfn(p, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tnumber_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_extent_transfn);
/**
 * @brief Transition function for temporal extent aggregation for temporal
 * numbers
 */
Datum
Tnumber_extent_transfn(PG_FUNCTION_ARGS)
{
  TBox *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);
  TBox *result = tnumber_extent_transfn(box, temp);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal boolean functions
 *****************************************************************************/

PGDLLEXPORT Datum Tbool_tand_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbool_tand_transfn);
/**
 * @brief Transition function for temporal and aggregation of temporal boolean
 * values
 */
Datum
Tbool_tand_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_and, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tbool_tand_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbool_tand_combinefn);
/**
 * @brief Combine function for temporal and aggregation of temporal boolean
 * values
 */
Datum
Tbool_tand_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_and, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tbool_tor_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbool_tor_transfn);
/**
 * @brief Transition function for temporal or aggregation of temporal boolean
 * values
 */
Datum
Tbool_tor_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_or, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tbool_tor_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbool_tor_combinefn);
/**
 * @brief Combine function for temporal or aggregation of temporal boolean
 * values
 */
Datum
Tbool_tor_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_or, CROSSINGS_NO);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tint_tmin_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_tmin_transfn);
/**
 * @brief Transition function for temporal minimum aggregation of temporal
 * integer values
 */
Datum
Tint_tmin_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tint_tmin_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_tmin_combinefn);
/**
 * @brief Combine function for temporal minimum aggregation of temporal
 * integer values
 */
Datum
Tint_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tfloat_tmin_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_tmin_transfn);
/**
 * @brief Transition function for temporal minimum aggregation of temporal
 * float values
 */
Datum
Tfloat_tmin_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_min_float8, CROSSINGS);
}

PGDLLEXPORT Datum Tfloat_tmin_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_tmin_combinefn);
/**
 * @brief Combine function for temporal minimum aggregation of temporal float
 * values
 */
Datum
Tfloat_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_min_float8, CROSSINGS);
}

PGDLLEXPORT Datum Tint_tmax_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_tmax_transfn);
/**
 * @brief Transition function for temporal maximum aggregation of temporal
 * integer values
 */
Datum
Tint_tmax_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tint_tmax_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_tmax_combinefn);
/**
 * @brief Combine function for temporal maximum aggregation of temporal integer
 * values
 */
Datum
Tint_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tfloat_tmax_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_tmax_transfn);
/**
 * @brief Transition function for temporal maximum aggregation of temporal
 * float values
 */
Datum
Tfloat_tmax_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_max_float8, CROSSINGS);
}

PGDLLEXPORT Datum Tfloat_tmax_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_tmax_combinefn);
/**
 * @brief Combine function for temporal maximum aggregation of temporal float
 * values
 */
Datum
Tfloat_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_max_float8, CROSSINGS);
}

PGDLLEXPORT Datum Tint_tsum_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_tsum_transfn);
/**
 * @brief Transition function for temporal sum aggregation of temporal integer
 * values
 */
Datum
Tint_tsum_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tint_tsum_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_tsum_combinefn);
/**
 * @brief Combine function for temporal sum aggregation of temporal integer
 * values
 */
Datum
Tint_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tfloat_tsum_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_tsum_transfn);
/**
 * @brief Transition function for temporal sum aggregation of temporal float
 * values
 */
Datum
Tfloat_tsum_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tfloat_tsum_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_tsum_combinefn);
/**
 * @brief Combine function for temporal sum aggregation of temporal float values
 */
Datum
Tfloat_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ttext_tmin_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_tmin_transfn);
/**
 * @brief Transition function for temporal minimum aggregation of temporal text
 * values
 */
Datum
Ttext_tmin_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PGDLLEXPORT Datum Ttext_tmin_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_tmin_combinefn);
/**
 * @brief Combine function for temporal minimum aggregation of temporal text
 * values
 */
Datum
Ttext_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PGDLLEXPORT Datum Ttext_tmax_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_tmax_transfn);
/**
 * @brief Transition function for temporal maximum aggregation of temporal text
 * values
 */
Datum
Ttext_tmax_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

PGDLLEXPORT Datum Ttext_tmax_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_tmax_combinefn);
/**
 * @brief Combine function for temporal maximum aggregation of temporal text
 * values
 */
Datum
Ttext_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

/*****************************************************************************
 * Temporal average
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_tavg_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_tavg_transfn);
/**
 * @brief Transition function for temporal average aggregation
 */
Datum
Tnumber_tavg_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transform_transfn(fcinfo, &datum_sum_double2,
    CROSSINGS_NO, &tnumberinst_transform_tavg);
}

PGDLLEXPORT Datum Tnumber_tavg_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_tavg_combinefn);
/**
 * @brief Combine function for temporal average aggregation
 */
Datum
Tnumber_tavg_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, &datum_sum_double2, false);
}

PGDLLEXPORT Datum Tnumber_tavg_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_tavg_finalfn);
/**
 * @brief Final function for temporal average aggregation
 */
Datum
Tnumber_tavg_finalfn(PG_FUNCTION_ARGS)
{
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  Temporal *result = tnumber_tavg_finalfn(state);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_merge_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_merge_transfn);
/**
 * @brief Transition function for union aggregate of periods
 */
Datum
Temporal_merge_transfn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_transfn(fcinfo, NULL, CROSSINGS_NO);
}

PGDLLEXPORT Datum Temporal_merge_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_merge_combinefn);
/**
 * @brief Combine function for union aggregate of time types
 */
Datum
Temporal_merge_combinefn(PG_FUNCTION_ARGS)
{
  return Temporal_tagg_combinefn(fcinfo, NULL, CROSSINGS_NO);
}

/*****************************************************************************
 * Append aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Transition function for append temporal instant aggregate
 */
Temporal *
temporal_app_tinst_transfn(Temporal *state, const TInstant *inst,
  double maxdist, Interval *maxt)
{
  /* Null state: create a new temporal sequence with the instant */
  if (! state)
  {
#if ! MEOS
    MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    /* Default interpolation depending on the base type */
    interpType interp = MEOS_FLAGS_GET_CONTINUOUS(inst->flags) ? LINEAR : STEP;
    /* Arbitrary initialization to 64 elements */
    Temporal *result = (Temporal *) tsequence_make_exp(
      (const TInstant **) &inst, 1, 64, true, true, interp, NORMALIZE_NO);
#if ! MEOS
    unset_aggregation_context(ctx);
#endif /* ! MEOS */
    return result;
  }

  return temporal_append_tinstant(state, inst, maxdist, maxt, true);
}

/*****************************************************************************/

/**
 * @brief Transition function for append temporal instant aggregate
 */
// TODO generalize for discrete interpolation
PGDLLEXPORT Datum Temporal_app_tinst_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_app_tinst_transfn);
/**
 * @brief Transition function for append temporal instant aggregate
 */
Datum
Temporal_app_tinst_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext ctx = set_aggregation_context(fcinfo);
  Temporal *state = PG_ARGISNULL(0) ? NULL : PG_GETARG_TEMPORAL_P(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }
  Temporal *inst = PG_GETARG_TEMPORAL_P(1);
  unset_aggregation_context(ctx);
  double maxdist = -1.0;
  Interval *maxt = NULL;
  /* Take into account the arguments for the gaps */
  if (PG_NARGS() > 2)
  {
    if (PG_NARGS() == 3)
    {
      if (! PG_ARGISNULL(2))
        maxt = PG_GETARG_INTERVAL_P(2);
    }
    else /* PG_NARGS() == 4 */
    {
      if (! PG_ARGISNULL(2))
        maxdist = PG_GETARG_FLOAT8(2);
      if (! PG_ARGISNULL(3))
        maxt = PG_GETARG_INTERVAL_P(3);
    }
  }
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  state = temporal_app_tinst_transfn(state, (TInstant *) inst, maxdist, maxt);
  PG_FREE_IF_COPY(inst, 1);
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Temporal_append_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_append_finalfn);
/**
 * @brief Combine function for append temporal instant/sequence aggregate
 */
Datum
Temporal_append_finalfn(PG_FUNCTION_ARGS)
{
  MemoryContext ctx = set_aggregation_context(fcinfo);
  Temporal *state = PG_GETARG_TEMPORAL_P(0);
  unset_aggregation_context(ctx);
  Temporal *result = temporal_compact(state);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Transition function for append temporal sequence aggregate
 */
Temporal *
temporal_app_tseq_transfn(Temporal *state, const TSequence *seq)
{
  /* Null state: create a new temporal sequence with the sequence */
  if (! state)
  {
#if ! MEOS
    MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    /* Arbitrary initialization to 64 elements */
    Temporal *result = (Temporal *) tsequenceset_make_exp(
      (const TSequence **) &seq, 1, 64, NORMALIZE_NO);
#if ! MEOS
    unset_aggregation_context(ctx);
#endif /* ! MEOS */
    return result;
  }

  return temporal_append_tsequence(state, seq, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_app_tseq_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_app_tseq_transfn);
/**
 * @brief Transition function for append temporal sequence aggregate
 */
Datum
Temporal_app_tseq_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext ctx = set_aggregation_context(fcinfo);
  Temporal *state = PG_ARGISNULL(0) ? NULL : PG_GETARG_TEMPORAL_P(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }
  unset_aggregation_context(ctx);
  Temporal *seq = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  state = temporal_app_tseq_transfn(state, (TSequence *) seq);
  PG_FREE_IF_COPY(seq, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************/

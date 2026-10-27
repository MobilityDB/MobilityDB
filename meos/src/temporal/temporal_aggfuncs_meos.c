/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief General aggregate functions for temporal types
 */

#include "temporal/temporal_aggfuncs.h"

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/skiplist.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_restrict.h"
#include "temporal/temporal_waggfuncs.h"
#include "temporal/tbool_ops.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * MEOS aggregate transition functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal and of temporal booleans
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tbool_tand_transfn()
 * @csqlaggfn #tAndTransition()
 */
SkipList *
tbool_tand_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_and, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal or of temporal booleans
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tbool_tor_transfn()
 * @csqlaggfn #tOrTransition()
 */
SkipList *
tbool_tor_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_or, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tint_tmin_transfn()
 * @csqlaggfn #tMinTransition()
 */
SkipList *
tint_tmin_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_min_int32, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tbigint_tmin_transfn()
 * @csqlaggfn #tMinTransition()
 */
SkipList *
tbigint_tmin_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBIGINT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_min_int64, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tfloat_tmin_transfn()
 * @csqlaggfn #tMinTransition()
 */
SkipList *
tfloat_tmin_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_min_float8, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tint_tmax_transfn()
 * @csqlaggfn #tMaxTransition()
 */
SkipList *
tint_tmax_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_max_int32, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tbigint_tmax_transfn()
 * @csqlaggfn #tMaxTransition()
 */
SkipList *
tbigint_tmax_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBIGINT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_max_int64, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tfloat_tmax_transfn()
 * @csqlaggfn #tMaxTransition()
 */
SkipList *
tfloat_tmax_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_max_float8, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tint_tsum_transfn()
 * @csqlaggfn #tSumTransition()
 */
SkipList *
tint_tsum_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_sum_int32, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tbigint_tsum_transfn()
 * @csqlaggfn #tSumTransition()
 */
SkipList *
tbigint_tsum_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBIGINT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_sum_int64, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tfloat_tsum_transfn()
 * @csqlaggfn #tSumTransition()
 */
SkipList *
tfloat_tsum_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_sum_float8, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal average of temporal numbers
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Tnumber_tavg_transfn()
 * @csqlaggfn #tAvgTransition()
 */
SkipList *
tnumber_tavg_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_tnumber_type(temp->temptype))
    return NULL;
  return temporal_tagg_transform_transfn(state, temp, &datum_sum_double2,
    CROSSINGS_NO, &tnumberinst_transform_tavg);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal text values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Ttext_tmin_transfn()
 * @csqlaggfn #tMinTransition()
 */
SkipList *
ttext_tmin_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_min_text, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal text values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Ttext_tmax_transfn()
 * @csqlaggfn #tMaxTransition()
 */
SkipList *
ttext_tmax_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return temporal_tagg_transfn(state, temp, &datum_max_text, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for the merge aggregate of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value to aggregate @nullable
 * @csqlfn #Temporal_merge_transfn()
 * @csqlaggfn #mergeTransition()
 */
SkipList *
temporal_merge_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  return temporal_tagg_transfn(state, temp, NULL, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the merge aggregate of temporal values
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Temporal_merge_combinefn()
 */
SkipList *
temporal_merge_combinefn(SkipList *state1, SkipList *state2)
{
  return temporal_tagg_combinefn(state1, state2, NULL, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the temporal count aggregate of temporal values
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Temporal_tcount_combinefn()
 */
SkipList *
temporal_tcount_combinefn(SkipList *state1, SkipList *state2)
{
  return temporal_tagg_combinefn(state1, state2, &datum_sum_int32, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the temporal and aggregate of temporal booleans
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Tbool_tand_combinefn()
 */
SkipList *
tbool_tand_combinefn(SkipList *state1, SkipList *state2)
{
  return temporal_tagg_combinefn(state1, state2, &datum_and, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the temporal or aggregate of temporal booleans
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Tbool_tor_combinefn()
 */
SkipList *
tbool_tor_combinefn(SkipList *state1, SkipList *state2)
{
  return temporal_tagg_combinefn(state1, state2, &datum_or, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the temporal minimum aggregate of temporal
 * integers, big integers, floats and texts
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Temporal_tmin_combinefn()
 */
SkipList *
temporal_tmin_combinefn(SkipList *state1, SkipList *state2)
{
  datum_func2 func;
  bool crossings;
  if (! tagg_combine_func(state1, state2, TAGG_MIN, &func, &crossings))
    return NULL;
  return temporal_tagg_combinefn(state1, state2, func, crossings);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the temporal maximum aggregate of temporal
 * integers, big integers, floats and texts
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Temporal_tmax_combinefn()
 */
SkipList *
temporal_tmax_combinefn(SkipList *state1, SkipList *state2)
{
  datum_func2 func;
  bool crossings;
  if (! tagg_combine_func(state1, state2, TAGG_MAX, &func, &crossings))
    return NULL;
  return temporal_tagg_combinefn(state1, state2, func, crossings);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the temporal sum aggregate of temporal
 * integers, big integers and floats
 * @details The states of the temporal count hold temporal integers whatever
 * the type of the values counted, and are combined as theirs
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Tnumber_tsum_combinefn()
 */
SkipList *
tnumber_tsum_combinefn(SkipList *state1, SkipList *state2)
{
  datum_func2 func;
  bool crossings;
  if (! tagg_combine_func(state1, state2, TAGG_SUM, &func, &crossings))
    return NULL;
  return temporal_tagg_combinefn(state1, state2, func, crossings);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Combine function for the temporal average aggregate of temporal
 * numbers
 * @param[in,out] state1, state2 Current aggregate states
 * @csqlfn #Tnumber_tavg_combinefn()
 */
SkipList *
tnumber_tavg_combinefn(SkipList *state1, SkipList *state2)
{
  return temporal_tagg_combinefn(state1, state2, &datum_sum_double2, CROSSINGS_NO);
}

/*****************************************************************************
 * MEOS window aggregate transition functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tint_wmin_transfn()
 * @csqlaggfn #wMinTransition()
 */
SkipList *
tint_wmin_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_min_int32,
    GET_MIN, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tbigint_wmin_transfn()
 * @csqlaggfn #wMinTransition()
 */
SkipList *
tbigint_wmin_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBIGINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_min_int64,
    GET_MIN, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wmin_transfn()
 * @csqlaggfn #wMinTransition()
 */
SkipList *
tfloat_wmin_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_min_float8,
    GET_MIN, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tint_wmax_transfn()
 * @csqlaggfn #wMaxTransition()
 */
SkipList *
tint_wmax_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_max_int32,
    GET_MAX, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tbigint_wmax_transfn()
 * @csqlaggfn #wMaxTransition()
 */
SkipList *
tbigint_wmax_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBIGINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_max_int64,
    GET_MAX, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wmax_transfn()
 * @csqlaggfn #wMaxTransition()
 */
SkipList *
tfloat_wmax_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_max_float8,
    GET_MAX, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tint_wsum_transfn()
 * @csqlaggfn #wSumTransition()
 */
SkipList *
tint_wsum_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_sum_int32,
    GET_MIN, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tbigint_wsum_transfn()
 * @csqlaggfn #wSumTransition()
 */
SkipList *
tbigint_wsum_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TBIGINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_sum_int64,
    GET_MIN, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wsum_transfn()
 * @csqlaggfn #wSumTransition()
 */
SkipList *
tfloat_wsum_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_sum_float8,
    GET_MIN, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal average of temporal numbers
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Tnumber_wavg_transfn()
 * @csqlaggfn #wAvgTransition()
 */
SkipList *
tnumber_wavg_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_tnumber_type(temp->temptype))
    return NULL;
  return
  temporal_wagg_transform_transfn(state, temp, interv, &datum_sum_double2,
    &tnumber_transform_wavg);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal count of temporal values over a
 * moving window
 * @param[in,out] state Current aggregate state @nullable
 * @param[in] temp Temporal value @nullable
 * @param[in] interv Interval
 * @csqlfn #Temporal_wcount_transfn()
 * @csqlaggfn #wCountTransition()
 */
SkipList *
temporal_wcount_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  return
  temporal_wagg_transform_transfn(state, temp, interv, &datum_sum_int32,
    &temporal_transform_wcount);
}

/*****************************************************************************/

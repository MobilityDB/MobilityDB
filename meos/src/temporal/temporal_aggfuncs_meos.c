/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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

#if ! MEOS
  extern FunctionCallInfo fetch_fcinfo();
  extern void store_fcinfo(FunctionCallInfo fcinfo);
  extern MemoryContext set_aggregation_context(FunctionCallInfo fcinfo);
  extern void unset_aggregation_context(MemoryContext ctx);
#endif /* ! MEOS */

/*****************************************************************************
 * MEOS aggregate transition functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal and of temporal booleans
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tbool_tand_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tbool_tor_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tint_tmin_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tfloat_tmin_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tint_tmax_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tfloat_tmax_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tint_tsum_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tfloat_tsum_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tnumber_tavg_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Ttext_tmin_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Ttext_tmax_transfn()
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

#endif /* MEOS */

/*****************************************************************************
 * MEOS window aggregate transition functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tint_wmin_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wmin_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tint_wmax_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wmax_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tint_wsum_transfn()
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
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wsum_transfn()
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
    GET_MIN, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal average of temporal numbers
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tnumber_wavg_transfn()
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
#endif /* MEOS */

/*****************************************************************************/

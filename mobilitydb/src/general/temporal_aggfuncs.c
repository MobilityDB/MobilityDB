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
 * @brief General aggregate functions for temporal types.
 */

#include "general/temporal_aggfuncs.h"

/* C */
#include <assert.h>
#include <math.h>
#include <string.h>
/* PostgreSQL */
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
 * Generic transition function for aggregating temporal values
 *
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 */
static Datum
temporal_tagg_transfn(FunctionCallInfo fcinfo, datum_func2 func, bool crossings)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  ensure_valid_tempsubtype(temp->subtype);
  SkipList *result;
  if (temp->subtype == TINSTANT)
    result =  tinstant_tagg_transfn(state, (TInstant *) temp, func);
  else if (temp->subtype == TSEQUENCE)
    result = MOBDB_FLAGS_GET_DISCRETE(temp->flags) ?
      tdiscseq_tagg_transfn(state, (TSequence *) temp, func) :
      tsequence_tagg_transfn(state, (TSequence *) temp, func, crossings);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_tagg_transfn(state, (TSequenceSet *) temp,
      func, crossings);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Generic combine function for aggregating temporal values
 *
 * @param[in] state1, state2 State values
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @note This function is called for aggregating temporal points and thus
 * after checking the dimensionality and the SRID of the values
 */
SkipList *
temporal_tagg_combinefn1(SkipList *state1, SkipList *state2,
  datum_func2 func, bool crossings)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  Temporal *head2 = (Temporal *) skiplist_headval(state2);
  ensure_same_tempsubtype_skiplist(state1, head2);
  int count2 = state2->length;
  void **values2 = skiplist_values(state2);
  skiplist_splice(state1, values2, count2, func, crossings);
  pfree_array(values2, count2);
  return state1;
}

/**
 * Generic combine function for aggregating temporal alphanumeric values
 *
 * @param[in] func Function
 * @param[in] crossings True if turning points are added in the segments
 */
static Datum
temporal_tagg_combinefn(FunctionCallInfo fcinfo, datum_func2 func, bool crossings)
{
  SkipList *state1, *state2;
  INPUT_AGG_COMB_STATE(state1, state2);
  store_fcinfo(fcinfo);
  SkipList *result = temporal_tagg_combinefn1(state1, state2, func,
    crossings);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_tagg_finalfn);
/**
 * Generic final function for temporal aggregation
 */
PGDLLEXPORT Datum
Temporal_tagg_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  Temporal **values = (Temporal **) skiplist_values(state);
  Temporal *result;
  assert(values[0]->subtype == TINSTANT || values[0]->subtype == TSEQUENCE);
  if (values[0]->subtype == TINSTANT)
    result = (Temporal *) tsequence_make((const TInstant **) values,
      state->length, state->length, true, true, DISCRETE, NORMALIZE_NO);
  else /* values[0]->subtype == TSEQUENCE */
    result = (Temporal *) tsequenceset_make((const TSequence **) values,
      state->length, NORMALIZE);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transition function for aggregating temporal values that require a
 * transformation to each composing instant/sequence
 *
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @param[in] transform Transform function
 */
Datum
temporal_tagg_transform_transfn(FunctionCallInfo fcinfo, datum_func2 func,
  bool crossings, TInstant *(*transform)(const TInstant *))
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  store_fcinfo(fcinfo);
  int count;
  Temporal **temparr = temporal_transform_tagg(temp, &count, transform);
  if (state)
  {
    ensure_same_tempsubtype_skiplist(state, temparr[0]);
    skiplist_splice(state, (void **) temparr, count, func, crossings);
  }
  else
  {
    state = skiplist_make((void **) temparr, count, TEMPORAL);
  }

  pfree_array((void **) temparr, count);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************
 * Temporal count
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_tcount_transfn);
/**
 * Generic transition function for temporal aggregation
 */
PGDLLEXPORT Datum
Temporal_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);

  store_fcinfo(fcinfo);
  int count;
  Temporal **temparr = temporal_transform_tcount(temp, &count);
  if (state)
  {
    ensure_same_tempsubtype_skiplist(state, temparr[0]);
    skiplist_splice(state, (void **) temparr, count, &datum_sum_int32,
      false);
  }
  else
  {
    state = skiplist_make((void **) temparr, count, TEMPORAL);
  }

  pfree_array((void **) temparr, count);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Temporal_tcount_combinefn);
/**
 * Generic combine function for temporal aggregation
 */
PGDLLEXPORT Datum
Temporal_tcount_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_int32, false);
}

/*****************************************************************************
 * Temporal extent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_extent_transfn);
/**
 * Transition function for temporal extent aggregation of temporal values
 * with period bounding box
 */
PGDLLEXPORT Datum
Temporal_extent_transfn(PG_FUNCTION_ARGS)
{
  Period *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);
  Period *result;

  /* Can't do anything with null inputs */
  if (! p && ! temp)
    PG_RETURN_NULL();
  /* Null period and non-null temporal, return the bbox of the temporal */
  if (! p)
  {
    result = palloc0(sizeof(Period));
    temporal_set_bbox(temp, result);
    PG_RETURN_POINTER(result);
  }
  /* Non-null period and null temporal, return the period */
  if (! temp)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, p, sizeof(Period));
    PG_RETURN_POINTER(result);
  }

  Period p1;
  temporal_set_bbox(temp, &p1);
  result = union_span_span(p, &p1, false);

  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_extent_transfn);
/**
 * Transition function for temporal extent aggregation for temporal numbers
 */
PGDLLEXPORT Datum
Tnumber_extent_transfn(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);

  /* Can't do anything with null inputs */
  if (!box && !temp)
    PG_RETURN_NULL();
  TBOX *result = palloc0(sizeof(TBOX));
  /* Null box and non-null temporal, return the bbox of the temporal */
  if (!box)
  {
    temporal_set_bbox(temp, result);
    PG_RETURN_POINTER(result);
  }
  /* Non-null box and null temporal, return the box */
  if (!temp)
  {
    memcpy(result, box, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }

  /* Both box and temporal are not null */
  temporal_set_bbox(temp, result);
  tbox_expand(box, result);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal boolean functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbool_tand_transfn);
/**
 * Transition function for temporal and aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
Tbool_tand_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_and, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tbool_tand_combinefn);
/**
 * Combine function for temporal and aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
Tbool_tand_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_and, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tbool_tor_transfn);
/**
 * Transition function for temporal or aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
Tbool_tor_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_or, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tbool_tor_combinefn);
/**
 * Combine function for temporal or aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
Tbool_tor_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_or, CROSSINGS_NO);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tint_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
Tint_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tint_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
Tint_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tfloat_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal float values
 */
PGDLLEXPORT Datum
Tfloat_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(Tfloat_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal float values
 */
PGDLLEXPORT Datum
Tfloat_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(Tint_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
Tint_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tint_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
Tint_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tfloat_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal float values
 */
PGDLLEXPORT Datum
Tfloat_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(Tfloat_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal float values
 */
PGDLLEXPORT Datum
Tfloat_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(Tint_tsum_transfn);
/**
 * Transition function for temporal sum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
Tint_tsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tint_tsum_combinefn);
/**
 * Combine function for temporal sum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
Tint_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tfloat_tsum_transfn);
/**
 * Transition function for temporal sum aggregation of temporal float values
 */
PGDLLEXPORT Datum
Tfloat_tsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Tfloat_tsum_combinefn);
/**
 * Combine function for temporal sum aggregation of temporal float values
 */
PGDLLEXPORT Datum
Tfloat_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Ttext_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal text values
 */
PGDLLEXPORT Datum
Ttext_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Ttext_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal text values
 */
PGDLLEXPORT Datum
Ttext_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Ttext_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal text values
 */
PGDLLEXPORT Datum
Ttext_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Ttext_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal text values
 */
PGDLLEXPORT Datum
Ttext_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

/*****************************************************************************
 * Temporal average
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_tavg_transfn);
/**
 * Transition function for temporal average aggregation
 */
PGDLLEXPORT Datum
Tnumber_tavg_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transform_transfn(fcinfo, &datum_sum_double2,
    CROSSINGS_NO, &tnumberinst_transform_tavg);
}

PG_FUNCTION_INFO_V1(Tnumber_tavg_combinefn);
/**
 * Combine function for temporal average aggregation
 */
PGDLLEXPORT Datum
Tnumber_tavg_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_double2, false);
}

PG_FUNCTION_INFO_V1(Tnumber_tavg_finalfn);
/**
 * Final function for temporal average aggregation
 */
PGDLLEXPORT Datum
Tnumber_tavg_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  Temporal **values = (Temporal **) skiplist_values(state);
  Temporal *result;
  assert(values[0]->subtype == TINSTANT || values[0]->subtype == TSEQUENCE);
  if (values[0]->subtype == TINSTANT)
    result = (Temporal *) tinstant_tavg_finalfn((TInstant **) values,
      state->length);
  else /* values[0]->subtype == TSEQUENCE */
    result = (Temporal *) tsequence_tavg_finalfn((TSequence **) values,
      state->length);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_merge_transfn);
/**
 * Transition function for union aggregate of periods
 */
PGDLLEXPORT Datum
Temporal_merge_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, NULL, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(Temporal_merge_combinefn);
/**
 * Combine function for union aggregate of time types
 */
PGDLLEXPORT Datum
Temporal_merge_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, NULL, CROSSINGS_NO);
}

/*****************************************************************************/

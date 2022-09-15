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
 * @brief Aggregate functions for temporal points.
 *
 * The only functions currently provided are extent and temporal centroid.
 */

#include "pg_point/tpoint_aggfuncs.h"

/* C */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/doublen.h"
#include "pg_general/skiplist.h"
#include "pg_general/temporal_aggfuncs.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Check the validity of the temporal point values for aggregation
 */
static void
geoaggstate_check(const SkipList *state, int32_t srid, bool hasz)
{
  if(! state)
    return;
  struct GeoAggregateState *extra = state->extra;
  if (extra && extra->srid != srid)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Geometries must have the same SRID for temporal aggregation")));
  if (extra && extra->hasz != hasz)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Geometries must have the same dimensionality for temporal aggregation")));
  return;
}

/**
 * Check the validity of the temporal point values for aggregation
 */
static void
geoaggstate_check_state(const SkipList *state1, const SkipList *state2)
{
  if(! state2)
    return;
  struct GeoAggregateState *extra2 = state2->extra;
  if (extra2)
    geoaggstate_check(state1, extra2->srid, extra2->hasz);
  return;
}

/**
 * Check the validity of the temporal point values for aggregation
 */
void
geoaggstate_check_temp(const SkipList *state, const Temporal *t)
{
  geoaggstate_check(state, tpoint_srid(t), MOBDB_FLAGS_GET_Z(t->flags) != 0);
  return;
}

/*****************************************************************************/

/**
 * Transform a temporal point value of instant type into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TInstant *
tpointinst_transform_tcentroid(const TInstant *inst)
{
  TInstant *result;
  if (MOBDB_FLAGS_GET_Z(inst->flags))
  {
    const POINT3DZ *point = datum_point3dz_p(tinstant_value(inst));
    double4 dvalue;
    double4_set(point->x, point->y, point->z, 1, &dvalue);
    result = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE4, inst->t);
  }
  else
  {
    const POINT2D *point = datum_point2d_p(tinstant_value(inst));
    double3 dvalue;
    double3_set(point->x, point->y, 1, &dvalue);
    result = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE3, inst->t);
  }
  return result;
}

/**
 * Transform a temporal point discrete sequence into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TInstant **
tpointdiscseq_transform_tcentroid(const TSequence *seq)
{
  TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    result[i] = tpointinst_transform_tcentroid(inst);
  }
  return result;
}

/**
 * Transform a temporal point value of sequence type into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TSequence *
tpointseq_transform_tcentroid(const TSequence *seq)
{
  TInstant **instants = tpointdiscseq_transform_tcentroid(seq);
  return tsequence_make_free(instants, seq->count, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * Transform a temporal point value of sequence set type into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TSequence **
tpointseqset_transform_tcentroid(const TSequenceSet *ts)
{
  TSequence **result = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    result[i] = tpointseq_transform_tcentroid(seq);
  }
  return result;
}

/**
 * Transform a temporal point value for performing temporal centroid aggregation
 */
Temporal **
tpoint_transform_tcentroid(const Temporal *temp, int *count)
{
  Temporal **result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *) tpointinst_transform_tcentroid((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (MOBDB_FLAGS_GET_DISCRETE(temp->flags))
    {
      result = (Temporal **) tpointdiscseq_transform_tcentroid((TSequence *) temp);
      *count = ((TSequence *) temp)->count;
    }
    else
    {
      result = palloc(sizeof(Temporal *));
      result[0] = (Temporal *) tpointseq_transform_tcentroid((TSequence *) temp);
      *count = 1;
    }

  }
  else /* temp->subtype == TSEQUENCESET */
  {
    result = (Temporal **) tpointseqset_transform_tcentroid((TSequenceSet *) temp);
    *count = ((TSequenceSet *) temp)->count;
  }
  assert(result != NULL);
  return result;
}

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
  STBOX *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);

  /* Can't do anything with null inputs */
  if (! box && ! temp)
    PG_RETURN_NULL();
  STBOX *result = palloc0(sizeof(STBOX));
  /* Null box and non-null temporal, return the bbox of the temporal */
  if (temp && ! box )
  {
    temporal_set_bbox(temp, result);
    PG_RETURN_POINTER(result);
  }
  /* Non-null box and null temporal, return the box */
  if (box && ! temp)
  {
    memcpy(result, box, sizeof(STBOX));
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
 * Transition function for temporal centroid aggregation of temporal point values
 */
PGDLLEXPORT Datum
Tpoint_tcentroid_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);

  geoaggstate_check_temp(state, temp);
  datum_func2 func = MOBDB_FLAGS_GET_Z(temp->flags) ?
    &datum_sum_double4 : &datum_sum_double3;

  int count;
  Temporal **temparr = tpoint_transform_tcentroid(temp, &count);
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
      .srid = tpoint_srid(temp),
      .hasz = MOBDB_FLAGS_GET_Z(temp->flags) != 0
    };
    aggstate_set_extra(fcinfo, state, &extra, sizeof(struct GeoAggregateState));
  }

  pfree_array((void **) temparr, count);
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

  geoaggstate_check_state(state1, state2);
  struct GeoAggregateState *extra = NULL;
  if (state1 && state1->extra)
    extra = state1->extra;
  if (state2 && state2->extra)
    extra = state2->extra;
  assert(extra != NULL);
  datum_func2 func = extra->hasz ? &datum_sum_double4 : &datum_sum_double3;
  SkipList *result = temporal_tagg_combinefn1(fcinfo, state1, state2,
    func, false);

  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transforms a temporal doubleN instant into a point
 */
static Datum
doublen_to_point(const TInstant *inst, int srid)
{
  assert(inst->temptype == T_TDOUBLE3 || inst->temptype == T_TDOUBLE4);
  LWPOINT *point;
  if (inst->temptype == T_TDOUBLE3)
  {
    double3 *value3 = (double3 *) DatumGetPointer(&inst->value);
    assert(value3->c != 0);
    double valuea = value3->a / value3->c;
    double valueb = value3->b / value3->c;
    point = lwpoint_make2d(srid, valuea, valueb);
  }
  else /* inst->temptype == T_TDOUBLE4 */
  {
    double4 *value4 = (double4 *) DatumGetPointer(&inst->value);
    assert(value4->d != 0);
    double valuea = value4->a / value4->d;
    double valueb = value4->b / value4->d;
    double valuec = value4->c / value4->d;
    point = lwpoint_make3dz(srid, valuea, valueb, valuec);
  }
  /* Notice that for the moment we do not aggregate temporal geographic points */
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) point));
  lwpoint_free(point);
  return result;
}

/**
 * Final function for temporal centroid aggregation of temporal point values
 * with instant type
 *
 * @param[in] instants Temporal values
 * @param[in] count Number of elements in the array
 * @param[in] srid SRID of the values
 */
static TSequence *
tpointinst_tcentroid_finalfn(TInstant **instants, int count, int srid)
{
  TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    TInstant *inst = instants[i];
    Datum value = doublen_to_point(inst, srid);
    newinstants[i] = tinstant_make(value, T_TGEOMPOINT, inst->t);
    pfree(DatumGetPointer(value));
  }
  return tsequence_make_free(newinstants, count, count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * Final function for temporal centroid aggregation of temporal point values
 * with sequence type
 *
 * @param[in] sequences Temporal values
 * @param[in] count Number of elements in the array
 * @param[in] srid SRID of the values
 */
static TSequenceSet *
tpointseq_tcentroid_finalfn(TSequence **sequences, int count, int srid)
{
  TSequence **newsequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    TSequence *seq = sequences[i];
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = tsequence_inst_n(seq, j);
      Datum value = doublen_to_point(inst, srid);
      instants[j] = tinstant_make(value, T_TGEOMPOINT, inst->t);
      pfree(DatumGetPointer(value));
    }
    newsequences[i] = tsequence_make_free(instants, seq->count, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      MOBDB_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  }
  return tsequenceset_make_free(newsequences, count, NORMALIZE);
}

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

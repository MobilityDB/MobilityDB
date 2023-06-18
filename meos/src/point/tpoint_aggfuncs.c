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

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Check the validity of the temporal point values for aggregation
 */
void
geoaggstate_check(const SkipList *state, int32_t srid, bool hasz)
{
  if(! state)
    return;
  struct GeoAggregateState *extra = state->extra;
  if (extra && extra->srid != srid)
    elog(ERROR, "Geometries must have the same SRID for temporal aggregation");
  if (extra && extra->hasz != hasz)
    elog(ERROR, "Geometries must have the same dimensionality for temporal aggregation");
  return;
}

/**
 * @brief Check the validity of the temporal point values for aggregation
 */
void
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
 * @brief Check the validity of the temporal point values for aggregation
 */
void
geoaggstate_check_temp(const SkipList *state, const Temporal *t)
{
  geoaggstate_check(state, tpoint_srid(t), MEOS_FLAGS_GET_Z(t->flags) != 0);
  return;
}

/*****************************************************************************/

/**
 * @brief Transform a temporal point value of instant type into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TInstant *
tpointinst_transform_tcentroid(const TInstant *inst)
{
  TInstant *result;
  if (MEOS_FLAGS_GET_Z(inst->flags))
  {
    const POINT3DZ *point = DATUM_POINT3DZ_P(tinstant_value(inst));
    double4 dvalue;
    double4_set(point->x, point->y, point->z, 1, &dvalue);
    result = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE4, inst->t);
  }
  else
  {
    const POINT2D *point = DATUM_POINT2D_P(tinstant_value(inst));
    double3 dvalue;
    double3_set(point->x, point->y, 1, &dvalue);
    result = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE3, inst->t);
  }
  return result;
}

/**
 * @brief Transform a temporal point discrete sequence into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TInstant **
tpointseq_disc_transform_tcentroid(const TSequence *seq)
{
  TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    result[i] = tpointinst_transform_tcentroid(inst);
  }
  return result;
}

/**
 * @brief Transform a temporal point value of sequence type into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TSequence *
tpointseq_cont_transform_tcentroid(const TSequence *seq)
{
  TInstant **instants = tpointseq_disc_transform_tcentroid(seq);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc,  MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Transform a temporal point value of sequence set type into a temporal
 * double3/double4 value for performing temporal centroid aggregation
 */
static TSequence **
tpointseqset_transform_tcentroid(const TSequenceSet *ss)
{
  TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    result[i] = tpointseq_cont_transform_tcentroid(seq);
  }
  return result;
}

/**
 * @brief Transform a temporal point value for performing temporal centroid aggregation
 */
Temporal **
tpoint_transform_tcentroid(const Temporal *temp, int *count)
{
  Temporal **result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *) tpointinst_transform_tcentroid((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(temp->flags))
    {
      result = (Temporal **) tpointseq_disc_transform_tcentroid((TSequence *) temp);
      *count = ((TSequence *) temp)->count;
    }
    else
    {
      result = palloc(sizeof(Temporal *));
      result[0] = (Temporal *) tpointseq_cont_transform_tcentroid((TSequence *) temp);
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

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal centroid aggregation of temporal
 * point values
 */
SkipList *
tpoint_tcentroid_transfn(SkipList *state, Temporal *temp)
{
  geoaggstate_check_temp(state, temp);
  bool hasz = MEOS_FLAGS_GET_Z(temp->flags);
  datum_func2 func = hasz ? &datum_sum_double4 : &datum_sum_double3;

  int count;
  Temporal **temparr = tpoint_transform_tcentroid(temp, &count);
  if (state)
    skiplist_splice(state, (void **) temparr, count, func, false);
  else
  {
    state = skiplist_make((void **) temparr, count);
    struct GeoAggregateState extra =
    {
      .srid = tpoint_srid(temp),
      .hasz = hasz
    };
    aggstate_set_extra(state, &extra, sizeof(struct GeoAggregateState));
  }

  pfree_array((void **) temparr, count);
  return state;
}

/*****************************************************************************
 * Extent
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal extent aggregation of temporal point values
 */
STBox *
tpoint_extent_transfn(STBox *box, const Temporal *temp)
{
  /* Can't do anything with null inputs */
  if (! box && ! temp)
    return NULL;
  STBox *result = palloc0(sizeof(STBox));
  /* Null box and non-null temporal, return the bbox of the temporal */
  if (temp && ! box )
  {
    temporal_set_bbox(temp, result);
    return result;
  }
  /* Non-null box and null temporal, return the box */
  if (box && ! temp)
  {
    memcpy(result, box, sizeof(STBox));
    return result;
  }

  /* Both box and temporal are not null */
  ensure_same_srid(tpoint_srid(temp), stbox_srid(box));
  ensure_same_dimensionality(temp->flags, box->flags);
  ensure_same_geodetic(temp->flags, box->flags);
  temporal_set_bbox(temp, result);
  stbox_expand(box, result);
  return result;
}

/*****************************************************************************
 * Centroid
 *****************************************************************************/

/**
 * @brief Transform a temporal doubleN instant into a point
 */
static Datum
doublen_to_point(const TInstant *inst, int srid)
{
  assert(inst->temptype == T_TDOUBLE3 || inst->temptype == T_TDOUBLE4);
  LWPOINT *point;
  if (inst->temptype == T_TDOUBLE3)
  {
    double3 *value3 = (double3 *) DatumGetPointer(tinstant_value(inst));
    assert(value3->c != 0);
    double valuea = value3->a / value3->c;
    double valueb = value3->b / value3->c;
    point = lwpoint_make2d(srid, valuea, valueb);
  }
  else /* inst->temptype == T_TDOUBLE4 */
  {
    double4 *value4 = (double4 *) DatumGetPointer(tinstant_value(inst));
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
 * @brief Final function for temporal centroid aggregation of temporal point
 * value with instant type
 * @param[in] instants Temporal values
 * @param[in] count Number of elements in the array
 * @param[in] srid SRID of the values
 */
TSequence *
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
  return tsequence_make_free(newinstants, count,  true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Final function for temporal centroid aggregation of temporal point
 * values with sequence type
 * @param[in] sequences Temporal values
 * @param[in] count Number of elements in the array
 * @param[in] srid SRID of the values
 */
TSequenceSet *
tpointseq_tcentroid_finalfn(TSequence **sequences, int count, int srid)
{
  TSequence **newsequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    TSequence *seq = sequences[i];
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      Datum value = doublen_to_point(inst, srid);
      instants[j] = tinstant_make(value, T_TGEOMPOINT, inst->t);
      pfree(DatumGetPointer(value));
    }
    newsequences[i] = tsequence_make_free(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  }
  TSequenceSet *result = tsequenceset_make_free(newsequences, count, NORMALIZE);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Final function for temporal centroid aggregation of temporal point
 * values
 */
Temporal *
tpoint_tcentroid_finalfn(SkipList *state)
{
  if (state == NULL || state->length == 0)
    return NULL;

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
  skiplist_free(state);
  return result;
}

/*****************************************************************************/

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
 * @brief Aggregate functions for temporal points
 *
 * The only functions currently provided are extent and temporal centroid.
 */

#include "point/tpoint_aggfuncs.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doublen.h"
#include "general/skiplist.h"
#include "general/temporal_aggfuncs.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Check the validity of two temporal points for aggregation
 */
bool
ensure_geoaggstate(const SkipList *state, int32_t srid, bool hasz)
{
  if (! state)
    return true;
  struct GeoAggregateState *extra = state->extra;
  if (extra && extra->srid != srid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Geometries must have the same SRID for temporal aggregation");
    return false;
  }
  if (extra && extra->hasz != hasz)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Geometries must have the same dimensionality for temporal aggregation");
    return false;
  }
  return true;
}

/**
 * @brief Check the validity of two temporal points for aggregation
 */
bool
ensure_geoaggstate_state(const SkipList *state1, const SkipList *state2)
{
  if(! state2)
    return true;
  struct GeoAggregateState *extra2 = state2->extra;
  if (extra2)
    return ensure_geoaggstate(state1, extra2->srid, extra2->hasz);
  return true;
}

/*****************************************************************************/

/**
 * @brief Transform a temporal point instant into a temporal @p double3 of
 * @p double4 for performing temporal centroid aggregation
 */
static TInstant *
tpointinst_transform_tcentroid(const TInstant *inst)
{
  if (MEOS_FLAGS_GET_Z(inst->flags))
  {
    const POINT3DZ *point = DATUM_POINT3DZ_P(tinstant_val(inst));
    double4 dvalue;
    double4_set(point->x, point->y, point->z, 1, &dvalue);
    return tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE4, inst->t);
  }
  else
  {
    const POINT2D *point = DATUM_POINT2D_P(tinstant_val(inst));
    double3 dvalue;
    double3_set(point->x, point->y, 1, &dvalue);
    return tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE3, inst->t);
  }
}

/**
 * @brief Transform a temporal point discrete sequence into a temporal
 * @p double3 or @p double4 for performing temporal centroid aggregation
 */
static TInstant **
tpointdiscseq_transform_tcentroid(const TSequence *seq)
{
  TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = tpointinst_transform_tcentroid(TSEQUENCE_INST_N(seq, i));
  return result;
}

/**
 * @brief Transform a temporal point sequence into a temporal
 * @p double3 or @p double4 for performing temporal centroid aggregation
 */
static TSequence *
tpointcontseq_transform_tcentroid(const TSequence *seq)
{
  TInstant **instants = tpointdiscseq_transform_tcentroid(seq);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc,  MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Transform a temporal point sequence set into a temporal
 * @p double3 or @p double4 for performing temporal centroid aggregation
 */
static TSequence **
tpointseqset_transform_tcentroid(const TSequenceSet *ss)
{
  TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    result[i] = tpointcontseq_transform_tcentroid(TSEQUENCESET_SEQ_N(ss, i));
  return result;
}

/**
 * @brief Transform a temporal point for performing temporal centroid
 * aggregation
 */
Temporal **
tpoint_transform_tcentroid(const Temporal *temp, int *count)
{
  Temporal **result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      result = palloc(sizeof(Temporal *));
      result[0] = (Temporal *) tpointinst_transform_tcentroid((TInstant *) temp);
      *count = 1;
      break;
    }
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
      {
        result = (Temporal **) tpointdiscseq_transform_tcentroid((TSequence *) temp);
        *count = ((TSequence *) temp)->count;
      }
      else
      {
        result = palloc(sizeof(Temporal *));
        result[0] = (Temporal *) tpointcontseq_transform_tcentroid((TSequence *) temp);
        *count = 1;
      }
      break;
    }
    default: /* TSEQUENCESET */
    {
      result = (Temporal **) tpointseqset_transform_tcentroid((TSequenceSet *) temp);
      *count = ((TSequenceSet *) temp)->count;
    }
  }
  assert(result != NULL);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal centroid aggregation of temporal
 * points
 * @param[in] state Current aggregate value
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_tcentroid_transfn()
 */
SkipList *
tpoint_tcentroid_transfn(SkipList *state, Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  bool hasz = MEOS_FLAGS_GET_Z(temp->flags);
  /* Ensure validity of the arguments */
  if (! ensure_tgeo_type(temp->temptype) ||
      ! ensure_geoaggstate(state, tpoint_srid(temp), hasz))
    return NULL;

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
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal extent aggregation of temporal
 * points
 * @param[in] box Current aggregate value
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_extent_transfn()
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
  if (! ensure_same_srid(tpoint_srid(temp), stbox_srid(box)) ||
      ! ensure_same_dimensionality(temp->flags, box->flags) ||
      ! ensure_same_geodetic(temp->flags, box->flags))
    return NULL;

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
    double3 *value3 = (double3 *) DatumGetPointer(tinstant_val(inst));
    assert(value3->c != 0);
    double valuea = value3->a / value3->c;
    double valueb = value3->b / value3->c;
    point = lwpoint_make2d(srid, valuea, valueb);
  }
  else /* inst->temptype == T_TDOUBLE4 */
  {
    double4 *value4 = (double4 *) DatumGetPointer(tinstant_val(inst));
    assert(value4->d != 0);
    double valuea = value4->a / value4->d;
    double valueb = value4->b / value4->d;
    double valuec = value4->c / value4->d;
    point = lwpoint_make3dz(srid, valuea, valueb, valuec);
  }
  /* Notice that for the moment we do not aggregate temporal geography points */
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) point));
  lwpoint_free(point);
  return result;
}

/**
 * @brief Final function for temporal centroid aggregation of temporal point
 * instants
 * @param[in] instants Temporal instants
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
    newinstants[i] = tinstant_make_free(value, T_TGEOMPOINT, inst->t);
  }
  return tsequence_make_free(newinstants, count,  true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Final function for temporal centroid aggregation of temporal point
 * sequences
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
      instants[j] = tinstant_make_free(value, T_TGEOMPOINT, inst->t);
    }
    newsequences[i] = tsequence_make_free(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  }
  return tsequenceset_make_free(newsequences, count, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_agg
 * @brief Final function for temporal centroid aggregation of temporal points
 * @param[in] state Current aggregate value
 * @csqlfn #Tpoint_tcentroid_finalfn()
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

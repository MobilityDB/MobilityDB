/*****************************************************************************
 *
 * tpoint_boxops.c
 *    Bounding box operators for temporal points.
 *
 * These operators test the bounding boxes of temporal points, which are
 * STBOX, where the x, y, and optional z coordinates are for the space (value)
 * dimension and the t coordinate is for the time dimension.
 * The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_boxops.h"

#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "timestampset.h"
#include "periodset.h"
#include "temporaltypes.h"
#include "temporal_util.h"
#include "temporal_boxops.h"
#include "tpoint.h"
#include "stbox.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Functions computing the bounding box at the creation of a temporal point
 *****************************************************************************/

/**
 * Set the spatiotemporal box from the temporal instant point value
 */
void
tpointinst_make_stbox(STBOX *box, const TInstant *inst)
{
  Datum value = tinstant_value(inst);
  GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(value);
  assert(geo_to_stbox_internal(box, gs));
  box->tmin = box->tmax = inst->t;
  MOBDB_FLAGS_SET_T(box->flags, true);
}

/**
 * Set the spatiotemporal box from the array of temporal instant point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @note Temporal instant values do not have a precomputed bounding box 
 */
void
tpointinstarr_to_stbox(STBOX *box, TInstant **instants, int count)
{
  tpointinst_make_stbox(box, instants[0]);
  for (int i = 1; i < count; i++)
  {
    STBOX box1;
    memset(&box1, 0, sizeof(STBOX));
    tpointinst_make_stbox(&box1, instants[i]);
    stbox_expand(box, &box1);
  }
}

/**
 * Set the spatiotemporal box from the array of temporal sequence point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] sequences Temporal instant values
 * @param[in] count Number of elements in the array
 */
void
tpointseqarr_to_stbox(STBOX *box, TSequence **sequences, int count)
{
  memcpy(box, tsequence_bbox_ptr(sequences[0]), sizeof(STBOX));
  for (int i = 1; i < count; i++)
  {
    STBOX *box1 = tsequence_bbox_ptr(sequences[i]);
    stbox_expand(box, box1);
  }
}

/*****************************************************************************
 * Boxes functions
 * These functions are currently not used but can be used for defining 
 * VODKA indexes https://www.pgcon.org/2014/schedule/events/696.en.html
 *****************************************************************************/

/**
 * Returns an array of spatiotemporal boxes from the segments of the 
 * temporal sequence point value
 *
 * @param[out] result Spatiotemporal box
 * @param[in] seq Temporal value
 * @return Number of elements in the array
 */
static int
tpointseq_stboxes1(STBOX *result, const TSequence *seq)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tsequence_inst_n(seq, 0);
    tpointinst_make_stbox(&result[0], inst);
    return 1;
  }

  /* Temporal sequence has at least 2 instants */
  TInstant *inst1 = tsequence_inst_n(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    tpointinst_make_stbox(&result[i], inst1);
    TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    STBOX box;
    memset(&box, 0, sizeof(STBOX));
    tpointinst_make_stbox(&box, inst2);
    stbox_expand(&result[i], &box);
    inst1 = inst2;
  }
  return seq->count - 1;
}

/**
 * Returns an array of spatiotemporal boxes from the segments of the 
 * temporal sequence point value
 *
 * @param[in] seq Temporal value
 */
ArrayType *
tpointseq_stboxes(const TSequence *seq)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  int count = seq->count - 1;
  if (count == 0)
    count = 1;
  STBOX *boxes = palloc0(sizeof(STBOX) * count);
  tpointseq_stboxes1(boxes, seq);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  return result;
}

/**
 * Returns an array of spatiotemporal boxes from the segments of the 
 * temporal sequence set point value
 *
 * @param[in] ts Temporal value
 */
ArrayType *
tpointseqset_stboxes(const TSequenceSet *ts)
{
  assert(MOBDB_FLAGS_GET_LINEAR(ts->flags));
  STBOX *boxes = palloc0(sizeof(STBOX) * ts->totalcount);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tpointseq_stboxes1(&boxes[k], seq);
  }
  ArrayType *result = stboxarr_to_array(boxes, k);
  pfree(boxes);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_stboxes);
/**
 * Returns an array of spatiotemporal boxes from the temporal point
 */
PGDLLEXPORT Datum
tpoint_stboxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  ArrayType *result = NULL;
  ensure_valid_duration(temp->duration);
  if (temp->duration == INSTANT || temp->duration == INSTANTSET)
    ;
  else if (temp->duration == SEQUENCE)
    result = tpointseq_stboxes((TSequence *)temp);
  else /* temp->duration == SEQUENCESET */
    result = tpointseqset_stboxes((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

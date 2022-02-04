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
 * @file tpoint_boxops.c
 * Bounding box operators for temporal points.
 *
 * These operators test the bounding boxes of temporal points, which are an
 * `STBOX`, where the *x*, *y*, and optional *z* coordinates are for the space
 * (value) dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: `overlaps`, `contains`, `contained`,
 * `same`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "point/tpoint_boxops.h"

#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "general/timestampset.h"
#include "general/periodset.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "point/tpoint.h"
#include "point/stbox.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Functions computing the bounding box at the creation of a temporal point
 *****************************************************************************/

/**
 * Set the spatiotemporal box from the temporal point value
 */
void
tpointinst_stbox(const TInstant *inst, STBOX *box)
{
  Datum value = tinstant_value(inst);
  GSERIALIZED *gs = (GSERIALIZED *) PointerGetDatum(value);
  /* Non-empty geometries have a bounding box */
  geo_stbox(gs, box);
  box->tmin = box->tmax = inst->t;
  MOBDB_FLAGS_SET_T(box->flags, true);
}

/**
 * Set the spatiotemporal box from the array of temporal point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @note Temporal instant values do not have a precomputed bounding box
 */
void
tpointinstarr_stbox(const TInstant **instants, int count, STBOX *box)
{
  tpointinst_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBOX box1;
    tpointinst_stbox(instants[i], &box1);
    stbox_expand(box, &box1);
  }
  return;
}

/**
 * Set the spatiotemporal box from the array of temporal point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] sequences Temporal instant values
 * @param[in] count Number of elements in the array
 */
void
tpointseqarr_stbox(const TSequence **sequences, int count, STBOX *box)
{
  memcpy(box, tsequence_bbox_ptr(sequences[0]), sizeof(STBOX));
  for (int i = 1; i < count; i++)
  {
    const STBOX *box1 = tsequence_bbox_ptr(sequences[i]);
    stbox_expand(box, box1);
  }
  return;
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
tpointseq_stboxes1(const TSequence *seq, STBOX *result)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = tsequence_inst_n(seq, 0);
    tpointinst_stbox(inst1, &result[0]);
    return 1;
  }

  /* Temporal sequence has at least 2 instants */
  inst1 = tsequence_inst_n(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    tpointinst_stbox(inst1, &result[i]);
    const TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    STBOX box;
    tpointinst_stbox(inst2, &box);
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
  tpointseq_stboxes1(seq, boxes);
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
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tpointseq_stboxes1(seq, &boxes[k]);
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
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET)
    ;
  else if (temp->subtype == SEQUENCE)
    result = tpointseq_stboxes((TSequence *)temp);
  else /* temp->subtype == SEQUENCESET */
    result = tpointseqset_stboxes((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * Generic box function for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_geo_tpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  STBOX box1, box2;
  geo_stbox(gs, &box1);
  temporal_bbox(temp, &box2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for a temporal point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tpoint_geo(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX box1, box2;
  temporal_bbox(temp, &box1);
  geo_stbox(gs, &box2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for a spatiotemporal box and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_stbox_tpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  STBOX box1;
  temporal_bbox(temp, &box1);
  bool result = func(box, &box1);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for a temporal point and a spatiotemporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tpoint_stbox(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  STBOX box1;
  temporal_bbox(temp, &box1);
  bool result = func(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for two temporal points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
boxop_tpoint_tpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  STBOX box1, box2;
  temporal_bbox(temp1, &box1);
  temporal_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the
 * temporal point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal point and the geometry/geography overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point and the spatiotemporal box overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal points overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal box of the geometry/geography contains
 * the spatiotemporal box of the temporal point
 */
PGDLLEXPORT Datum
contains_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box contains the spatiotemporal box of the
 * temporal point
 */
PGDLLEXPORT Datum
contains_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal box of the temporal point contains the
 * one of the geometry/geography
 */
PGDLLEXPORT Datum
contains_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point contains the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
contains_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal box of the first temporal point contains
 * the one of the second temporal point
 */
PGDLLEXPORT Datum
contains_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal box of the geometry/geography is
 * contained in the spatiotemporal box of the temporal point
 */
PGDLLEXPORT Datum
contained_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is contained in the spatiotemporal
 * box of the temporal point
 */
PGDLLEXPORT Datum
contained_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal box of the temporal point is contained
 * in the one of the geometry/geography
 */
PGDLLEXPORT Datum
contained_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point is contained
 * in the spatiotemporal box
 */
PGDLLEXPORT Datum
contained_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal box of the first temporal point is contained
 * in the one of the second temporal point
 */
PGDLLEXPORT Datum
contained_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the
 * temporal point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal point and
 * geometry/geography are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point and the
 * spatiotemporal box are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal points are equal
 * in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &same_stbox_stbox_internal);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the
 * temporal point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal point and
 * geometry/geography are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point and the
 * spatiotemporal box are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal points are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

/*****************************************************************************/

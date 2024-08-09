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
 * @brief Bounding box operators for temporal points
 *
 * These operators test the bounding boxes of temporal points, which are an
 * `STBox`, where the *x*, *y*, and optional *z* coordinates are for the space
 * (value) dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: `overlaps`, `contains`, `contained`,
 * `same`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "point/tpoint_boxops.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "point/stbox.h"
#include "general/temporal.h"
#include "point/tpoint_spatialfuncs.h"

extern void ll2cart(const POINT2D *g, POINT3D *p);
extern int edge_calculate_gbox(const POINT3D *A1, const POINT3D *A2, GBOX *gbox);

/*****************************************************************************
 * Functions computing the bounding box at the creation of a temporal point
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the spatiotemporal box from
 * a temporal point instant
 * @param[in] inst Temporal instant
 * @param[in] box Spatiotemporal box
 */
void
tpointinst_set_stbox(const TInstant *inst, STBox *box)
{
  GSERIALIZED *point = DatumGetGserializedP(tinstant_val(inst));
  geo_set_stbox(point, box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the spatiotemporal box of a
 * temporal point sequence
 * @param[in] seq Temporal sequence
 * @param[in] box Spatiotemporal box
 * @note The function copes with both temporal points and temporal network
 * points
 */
void
tspatialseq_set_stbox(const TSequence *seq, STBox *box)
{
  assert(seq); assert(box); assert(tspatial_type(seq->temptype));
  memcpy(box, TSEQUENCE_BBOX_PTR(seq), sizeof(STBox));
  return;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the spatiotemporal box of a
 * temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] box Spatiotemporal box
 * @note The function copes with both temporal points and temporal network
 * points
 */
void
tspatialseqset_set_stbox(const TSequenceSet *ss, STBox *box)
{
  assert(ss); assert(box); assert(tspatial_type(ss->temptype));
  memcpy(box, TSEQUENCESET_BBOX_PTR(ss), sizeof(STBox));
  return;
}

/**
 * @brief Return the last argument initialized with the spatiotemporal box of
 * an array of temporal point instants
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the output array
 * @param[out] box Spatiotemporal box
 * @note Temporal instant values do not have a precomputed bounding box
 */
void
tpointinstarr_set_stbox(const TInstant **instants, int count, STBox *box)
{
  /* Initialize the bounding box with the first instant */
  tpointinst_set_stbox(instants[0], box);
  /* Prepare for the iteration */
  bool hasz = MEOS_FLAGS_GET_Z(instants[0]->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(instants[0]->flags);
  for (int i = 1; i < count; i++)
  {
    GSERIALIZED *point = DatumGetGserializedP(tinstant_val(instants[i]));
    double x, y, z;
    point_get_coords(point, hasz, &x, &y, &z);
    box->xmin = Min(box->xmin, x);
    box->xmax = Max(box->xmax, x);
    box->ymin = Min(box->ymin, y);
    box->ymax = Max(box->ymax, y);
    if (hasz)
    {
      box->zmin = Min(box->zmin, z);
      box->zmax = Max(box->zmax, z);
    }
    box->period.lower = TimestampTzGetDatum(
      Min(DatumGetTimestampTz(box->period.lower), instants[i]->t));
    box->period.upper = TimestampTzGetDatum(
      Max(DatumGetTimestampTz(box->period.upper), instants[i]->t));
  }
  MEOS_FLAGS_SET_Z(box->flags, hasz);
  MEOS_FLAGS_SET_GEODETIC(box->flags, geodetic);
  return;
}

/**
 * @brief Expand the bounding box of a temporal point sequence with an instant
 * @param[in] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tpointseq_expand_stbox(TSequence *seq, const TInstant *inst)
{
  STBox box;
  tpointinst_set_stbox(inst, &box);
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/**
 * @brief Return the last argument initialized with the the spatiotemporal box
 * from an array of temporal point sequences
 * @param[in] sequences Temporal instant values
 * @param[in] count Number of elements in the output array
 * @param[out] box Spatiotemporal box
 */
void
tpointseqarr_set_stbox(const TSequence **sequences, int count, STBox *box)
{
  memcpy(box, TSEQUENCE_BBOX_PTR(sequences[0]), sizeof(STBox));
  for (int i = 1; i < count; i++)
  {
    const STBox *box1 = TSEQUENCE_BBOX_PTR(sequences[i]);
    stbox_expand(box1, box);
  }
  return;
}

/*****************************************************************************
 * Boxes functions
 * These functions can be used for defining MultiEntry Search Trees indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 * https://github.com/MobilityDB/mest
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return a singleton array of spatiotemporal boxes from a temporal
 * point instant
 * @param[in] inst Temporal instant
 */
STBox *
tpointinst_stboxes(const TInstant *inst)
{
  assert(inst); assert(tgeo_type(inst->temptype));
  /* One bounding box per instant */
  STBox *result = palloc(sizeof(STBox));
  tpointinst_set_stbox(inst, &result[0]);
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes from the instants
 * of a temporal point sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 */
static STBox *
tpointseq_disc_stboxes(const TSequence *seq)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* One bounding box per instant */
  STBox *result = palloc(sizeof(STBox) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tpointinst_set_stbox(TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal point sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[out] result Spatiotemporal box
 * @return Number of elements in the ouput array
 */
static int
tpointseq_cont_stboxes_iter(const TSequence *seq, STBox *result)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tpointinst_set_stbox(TSEQUENCE_INST_N(seq, 0), &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    tpointinst_set_stbox(inst, &result[i]);
    inst = TSEQUENCE_INST_N(seq, i + 1);
    STBox box;
    tpointinst_set_stbox(inst, &box);
    stbox_expand(&box, &result[i]);
  }
  return seq->count - 1;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a temporal point sequence, where the choice between instants or segments
 * depends, respectively, on whether the interpolation is discrete or
 * continuous
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 */
STBox *
tpointseq_stboxes(const TSequence *seq, int *count)
{
  assert(seq); assert(count); assert(tgeo_type(seq->temptype));

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return tpointseq_disc_stboxes(seq);
  }

  /* Continuous case */
  int nboxes = (seq->count == 1) ? 1 : seq->count - 1;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  *count = tpointseq_cont_stboxes_iter(seq, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 */
STBox *
tpointseqset_stboxes(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count); assert(tgeo_type(ss->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));

  /* One bounding box per segment */
  STBox *result = palloc(sizeof(STBox) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
    nboxes += tpointseq_cont_stboxes_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[nboxes]);
  assert(nboxes <= ss->totalcount);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_temporal_bbox
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a temporal point, where the choice between instants or segments depends,
 * respectively, on whether the interpolation is discrete or continuous
 * @param[in] temp Temporal point
 * @param[out] count Number of values of the output array
 * @return On error return @p NULL
 * @csqlfn #Tpoint_stboxes()
 */
STBox *
tpoint_stboxes(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tpointinst_stboxes((TInstant *) temp);
    case TSEQUENCE:
      return tpointseq_stboxes((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tpointseqset_stboxes((TSequenceSet *) temp, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return an array of N spatiotemporal boxes from the instants
 * of a temporal point sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
static STBox *
tpointseq_disc_split_n_stboxes(const TSequence *seq, int box_count, int *count)
{
  assert(seq); assert(count); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(box_count > 0);

  /* One bounding box per instant */
  if (seq->count <= box_count)
  {
    *count = seq->count;
    return tpointseq_disc_stboxes(seq);
  }

  /* One bounding box per several consecutive instants */
  STBox *result = palloc(sizeof(STBox) * box_count);
  /* Minimum number of input instants merged together in an output box */
  int size = seq->count / box_count;
  /* Number of output boxes that result from merging (size + 1) instants */
  int remainder = seq->count % box_count;
  int i = 0; /* Loop variable for input instants */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tpointinst_set_stbox(TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      STBox box;
      tpointinst_set_stbox(TSEQUENCE_INST_N(seq, l), &box);
      stbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == seq->count);
  *count = box_count;
  return result;
}

/**
 * @brief Return an array of N spatiotemporal boxes from the segments of a 
 * temporal point sequence with continuous interpolation (iterator function)
 * @param[in] seq Temporal point
 * @param[in] box_count Number of boxes
 * @param[out] result Number of elements in the ouput array
 */
static int
tpointseq_cont_split_n_stboxes_iter(const TSequence *seq, int box_count,
  STBox *result)
{
  assert(seq); assert(result); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(box_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tpointinst_set_stbox(TSEQUENCE_INST_N(seq, 0), &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  int nsegs = seq->count - 1;
  if (nsegs <= box_count)
    return tpointseq_cont_stboxes_iter(seq, result);

  /* One bounding box per several consecutive segments */
  /* Minimum number of input segments merged together in an output box */
  int size = nsegs / box_count;
  /* Number of output boxes that result from merging (size + 1) segments */
  int remainder = nsegs % box_count;
  int i = 0; /* Loop variable for input segments */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tpointinst_set_stbox(TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l <= j; l++)
    {
      STBox box;
      tpointinst_set_stbox(TSEQUENCE_INST_N(seq, l), &box);
      stbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == nsegs);
  return box_count;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spatiotemporal boxes from the instants or
 * segments of a temporal point sequence, where the choice between instants or
 * segments depends, respectively, on whether the interpolation is discrete or
 * continuous
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `seq->count <= box_count`, the result contains one box
 * per instant or segment. Otherwise, consecutive instants or segments are
 * combined into a single box in the result to reach the given number of
 * boxes.
 */
STBox *
tpointseq_split_n_stboxes(const TSequence *seq, int box_count, int *count)
{
  assert(seq); assert(count); assert(tgeo_type(seq->temptype));
  assert(box_count > 0);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tpointseq_disc_split_n_stboxes(seq, box_count, count);

  /* Continuous case */
  int nboxes = (seq->count <= box_count) ?
    (seq->count == 1 ? 1 : seq->count - 1) : box_count;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  *count = tpointseq_cont_split_n_stboxes_iter(seq, box_count, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spatiotemporal boxes from the segments of a
 * temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `ss->totalcount <= box_count`, the result contains one box per
 * segment. Otherwise, consecutive input segments are combined into a single box in
 * the result to reach the given number of boxes.
 */
STBox *
tpointseqset_split_n_stboxes(const TSequenceSet *ss, int box_count, int *count)
{
  assert(ss); assert(count); assert(tgeo_type(ss->temptype));
  assert(box_count > 0);

  /* One bounding box per segment */
  int nboxes = (ss->totalcount <= box_count) ? ss->totalcount : box_count;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  if (ss->totalcount <= box_count)
    return tpointseqset_stboxes(ss, count);

  /* Number of bounding boxes per composing sequence determined from the
   * proportion of seq->count and ss->totalcount */
  if (ss->count <= box_count)
  {
    int nboxes1 = 0;
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int nboxes_seq = (int) (box_count * seq->count * 1.0 / ss->totalcount);
      if (! nboxes_seq)
        nboxes_seq = 1;
      nboxes1 += tpointseq_cont_split_n_stboxes_iter(seq, nboxes_seq,
        &result[nboxes1]);
    }
    assert(nboxes1 <= ss->totalcount);
    *count = nboxes1;
    return result;
  }

  /* Merge consecutive sequences to reach the maximum number of boxes */
  /* Minimum number of sequences merged together in an output box */
  int size = ss->count / box_count;
  /* Number of output boxes that result from merging (size + 1) sequences */
  int remainder = ss->count % box_count;
  int i = 0; /* Loop variable for input sequences */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    tpointseq_cont_split_n_stboxes_iter(TSEQUENCESET_SEQ_N(ss, i), 1,
      &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      STBox box;
      tpointseq_cont_split_n_stboxes_iter(TSEQUENCESET_SEQ_N(ss, l), 1, &box);
      stbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == ss->count);
  *count = box_count;
  return result;
}

/**
 * @ingroup meos_temporal_bbox
 * @brief Return an array of N spatiotemporal boxes obtained by merging
 * consecutive instants or segments of a temporal point, where the choice
 * between instants or segments depends, respectively, on whether the
 * interpolation is discrete or continuous
 * @param[in] temp Temporal point
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 * @csqlfn #Tpoint_split_n_stboxes()
 */
STBox *
tpoint_split_n_stboxes(const Temporal *temp, int box_count, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_tgeo_type(temp->temptype) || ! ensure_positive(box_count))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tpointinst_stboxes((TInstant *) temp);
    case TSEQUENCE:
      return tpointseq_split_n_stboxes((TSequence *) temp, box_count, count);
    default: /* TSEQUENCESET */
      return tpointseqset_split_n_stboxes((TSequenceSet *) temp, box_count,
        count);
  }
}

/*****************************************************************************
 * split_each_n_stboxes function
 *****************************************************************************/

/**
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive instants of a temporal point sequence with discrete
 * interpolation 
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_box Number of input instants merged into an output box
 * @param[out] count Number of elements in the output array
 */
static STBox *
tpointseq_disc_split_each_n_stboxes(const TSequence *seq, int elems_per_box,
  int *count)
{
  assert(seq); assert(count); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(elems_per_box > 0);

  int nboxes = ceil((double) seq->count / (double) elems_per_box);
  STBox *result = palloc(sizeof(STBox) * nboxes);
  int k = 0;
  for (int i = 0; i < seq->count; ++i)
  {
    if (i % elems_per_box == 0)
      tinstant_set_bbox(TSEQUENCE_INST_N(seq, i), &result[k++]);
    else
    {
      STBox box;
      tinstant_set_bbox(TSEQUENCE_INST_N(seq, i), &box);
      stbox_expand(&box, &result[k - 1]);
    }
  }
  assert(k == nboxes);
  *count = k;
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive segments of a temporal point sequence with continuous
 * interpolation (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_box Number of input segments merged into an output box
 * @param[out] result Array of boxes
 * @return Number of elements in the output array
 */
static int
tpointseq_cont_split_each_n_stboxes_iter(const TSequence *seq, int elems_per_box,
  STBox *result)
{
  assert(seq); assert(result); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(elems_per_box > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tsequence_set_bbox(seq, &result[0]);
    return 1;
  }

  /* General case */
  int k = 0;
  tinstant_set_bbox(TSEQUENCE_INST_N(seq, 0), &result[k]);
  for (int i = 1; i < seq->count; ++i)
  {
    STBox box;
    tinstant_set_bbox(TSEQUENCE_INST_N(seq, i), &box);
    stbox_expand(&box, &result[k]);
    if ((i % elems_per_box == 0) && (i < seq->count - 1))
      result[++k] = box;
  }
  int nboxes = ceil((double) (seq->count - 1) / (double) elems_per_box);
  assert(k + 1 == nboxes);
  return nboxes;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive instants or segments of a temporal point sequence, where the
 * choice between instants or segments depends, respectively, on whether the
 * interpolation is discrete or continuous
 * @param[in] seq Temporal sequence
 * @param[in] elems_per_box Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
static STBox *
tpointseq_split_each_n_stboxes(const TSequence *seq, int elems_per_box,
  int *count)
{
  assert(seq); assert(count); assert(tgeo_type(seq->temptype));
  assert(elems_per_box > 0);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return tpointseq_disc_split_each_n_stboxes(seq, elems_per_box, count);

  /* Number of instants or segments */
  int nelems = (seq->count == 1) ? 1 : seq->count - 1;
  int nboxes = ceil((double) nelems / (double) elems_per_box);
  STBox *result = palloc(sizeof(STBox) * nboxes);
  *count = tpointseq_cont_split_each_n_stboxes_iter(seq, elems_per_box,
    result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive segments of a temporal point sequence set 
 * @param[in] ss Temporal sequence set
 * @param[in] elems_per_box Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
static STBox *
tpointseqset_split_each_n_stboxes(const TSequenceSet *ss, int elems_per_box,
  int *count)
{
  assert(ss); assert(count); assert(tgeo_type(ss->temptype));
  assert(elems_per_box > 0);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_split_each_n_stboxes(TSEQUENCESET_SEQ_N(ss, 0),
      elems_per_box, count);

  /* Iterate for every composing sequence */
  int nboxes = 0;
  STBox *result = palloc(sizeof(STBox) * ss->totalcount);
  for (int i = 0; i < ss->count; ++i)
    nboxes += tpointseq_cont_split_each_n_stboxes_iter(
      TSEQUENCESET_SEQ_N(ss, i), elems_per_box, &result[nboxes]);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_temporal_bbox
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive instants or segments of a temporal point, where the choice
 * between instants or segments depends, respectively, on whether the
 * interpolation is discrete or continuous.
 * @param[in] temp Temporal value
 * @param[in] elems_per_box Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 * @return On error return @p NULL
 * @csqlfn #Tpoint_split_each_n_stboxes()
 */
STBox *
tpoint_split_each_n_stboxes(const Temporal *temp, int elems_per_box,
  int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_tgeo_type(temp->temptype) || ! ensure_positive(elems_per_box))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tpoint_to_stbox(temp);
    case TSEQUENCE:
      return tpointseq_split_each_n_stboxes((TSequence *) temp, elems_per_box,
        count);
    default: /* TSEQUENCESET */
      return tpointseqset_split_each_n_stboxes((TSequenceSet *) temp,
        elems_per_box, count);
  }
}

/*****************************************************************************
 * Gboxes function
 *****************************************************************************/

/**
 * @ingroup meos_internal_box
 * @brief Return the @p GBOX in the last argument initialized with a @p LWPOINT
 * @param[in] p Point
 * @param[in] hasz True when the box has Z dimension
 * @param[in] hasm True when the box has M dimension
 * @param[in] geodetic True when the box has geodetic coordinates
 * @param[out] box GBOX
 */
void
lwpoint_init_gbox(const POINT4D *p, bool hasz, bool hasm, bool geodetic,
  GBOX *box)
{
  assert(p); assert(box);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(GBOX));
  /* Initialize existing dimensions */
  box->xmin = box->xmax = p->x;
  box->ymin = box->ymax = p->y;
  if (hasz || geodetic)
    box->zmin = box->zmax = p->z;
  if (hasm)
    box->mmin = box->mmax = p->m;
  FLAGS_SET_Z(box->flags, hasz);
  FLAGS_SET_M(box->flags, hasm);
  FLAGS_SET_GEODETIC(box->flags, geodetic);
  return;
}

/**
 * @ingroup meos_internal_box
 * @brief Return the @p GBOX in the last argument merged with a @p LWPOINT
 * @param[in] p Point
 * @param[in] hasz True when the box has Z dimension
 * @param[in] hasm True when the box has M dimension
 * @param[in] geodetic True when the box has geodetic coordinates
 * @param[out] box GBOX
 */
void
lwpoint_merge_gbox(const POINT4D *p, bool hasz, bool hasm, bool geodetic,
  GBOX *box)
{
  assert(p); assert(box);
  if (box->xmin > p->x) box->xmin = p->x;
  if (box->xmax < p->x) box->xmax = p->x;
  if (box->ymin > p->y) box->ymin = p->y;
  if (box->ymax < p->y) box->ymax = p->y;
  if (hasz || geodetic)
  {
    if (box->zmin > p->z) box->zmin = p->z;
    if (box->zmax < p->z) box->zmax = p->z;
  }
  if (hasm)
  {
    if (box->mmin > p->m) box->mmin = p->m;
    if (box->mmax < p->m) box->mmax = p->m;
  }
  return;
}

/**
 * @brief Return an array of N spatial boxes from the segments of a line
 * (iterator function)
 * @param[in] lwline Line
 * @param[out] result Spatial box
 * @param[in] geodetic True when the line is geodetic
 * @return Number of elements in the ouput array
 * @note Temporal instant values do not have a precomputed bounding box
 */
static int
line_gboxes_iter(const LWLINE *lwline, bool geodetic, GBOX *result)
{
  assert(lwline); assert(result); assert(lwline->points->npoints > 1);
  POINTARRAY *pa = lwline->points;
  int npoints = lwline->points->npoints;
  bool hasz = FLAGS_GET_Z(pa->flags);
  bool hasm = FLAGS_GET_M(pa->flags);
  const POINT4D *pt;

  /* Line has only 1 point */
  if (npoints == 1)
  {
    pt = (POINT4D *) getPoint_internal(pa, 0);
    lwpoint_init_gbox(pt, hasz, hasm, geodetic, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  pt = (POINT4D *) getPoint_internal(pa, 0);
  for (int i = 0; i < npoints - 1; i++)
  {
    lwpoint_init_gbox(pt, hasz, hasm, geodetic, &result[i]);
    pt = (POINT4D *) getPoint_internal(pa, i + 1);
    lwpoint_merge_gbox(pt, hasz, hasm, geodetic, &result[i]);
  }
  return npoints - 1;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spatial boxes from the segments of a line
 * @param[in] gs Line
 * @param[out] count Number of elements in the output array
 */
GBOX *
line_gboxes(const GSERIALIZED *gs, int *count)
{
  assert(gs); assert(count); assert(gserialized_get_type(gs) == LINETYPE);
  assert(! gserialized_is_empty(gs));

  bool geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  LWLINE *lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  int npoints = lwline->points->npoints;
  /* No points in the point array */
  if (! npoints)
  {
    lwline_free(lwline);
    return NULL;
  }

  int nboxes = npoints == 1 ? 1 : npoints - 1;
  GBOX *result = palloc(sizeof(GBOX) * nboxes);
  *count = line_gboxes_iter(lwline, geodetic, result);
  lwline_free(lwline);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spatial boxes from the segments of a multiline
 * @param[in] gs Multiline
 * @param[out] count Number of elements in the output array
 */
GBOX *
multiline_gboxes(const GSERIALIZED *gs, int *count)
{
  assert(gs); assert(count); assert(gserialized_get_type(gs) == MULTILINETYPE);
  assert(! gserialized_is_empty(gs));

  bool geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  LWMLINE *lwmline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
  int nlines = lwmline->ngeoms;
  /* No points in the point array */
  if (! nlines)
  {
    lwmline_free(lwmline);
    return NULL;
  }

  int totalpoints = 0;
  const LWLINE *lwline;
  for (int i = 0; i < nlines; i++)
  {
    lwline = lwmline->geoms[i];
    totalpoints += lwline->points->npoints;
  }
  GBOX *result = palloc(sizeof(GBOX) * totalpoints);
  /* One bounding box per segment */
  int nboxes = 0;
  for (int i = 0; i < nlines; i++)
    nboxes += line_gboxes_iter(lwmline->geoms[i], geodetic, &result[nboxes]);
  lwmline_free(lwmline);
  *count = nboxes;
  assert(nboxes <= totalpoints);
  return result;
}

/**
 * @ingroup meos_temporal_bbox
 * @brief Return an array of N spatial boxes obtained by merging consecutive
 * segments of a (multi)line
 * @param[in] gs (Multi)line
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 */
GBOX *
geo_gboxes(const GSERIALIZED *gs, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) count) ||
      ! ensure_not_empty(gs))
    return NULL;
  uint32_t geotype = gserialized_get_type(gs);
  if (geotype != LINETYPE && geotype != MULTILINETYPE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Argument is not a (multi) line");
    return NULL;
  }

  return (geotype == LINETYPE) ?
    line_gboxes(gs, count) : multiline_gboxes(gs, count);
}

/*****************************************************************************/

/**
 * @brief Return an array of N spatial boxes obtained by merging consecutive 
 * segments of a line (iterator function)
 * @param[in] lwline Line
 * @param[in] box_count Number of boxes
 * @param[in] geodetic True when the line is geodetic
 * @param[out] result Array of boxes. If `totalpoints <= box_count`, the result
 * contains one box per segment. Otherwise, consecutive input segments are
 * combined into a single box in the result to reach the given number of boxes.
 * @return Number of elements in the output array
 * @note We must pass the `geodetic` flag since `LWLINE` does not have this
 * flag set
 */
static int
line_split_n_gboxes_iter(const LWLINE *lwline, int box_count, bool geodetic,
  GBOX *result)
{
  assert(lwline); assert(result); assert(lwline->points->npoints > 1);
  assert(box_count > 0);

  POINTARRAY *pa = lwline->points;
  int npoints = lwline->points->npoints;
  bool hasz = FLAGS_GET_Z(pa->flags);
  bool hasm = FLAGS_GET_M(pa->flags);
  const POINT4D *pt;

  /* Line has only 1 point */
  if (npoints == 1)
  {
    pt = (POINT4D *) getPoint_internal(pa, 0);
    lwpoint_init_gbox(pt, hasz, hasm, geodetic, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  int nsegs = npoints - 1;
  if (nsegs <= box_count)
    return line_gboxes_iter(lwline, geodetic, result);

  /* One bounding box per several consecutive segments */
  /* Minimum number of input segments merged together in an output box */
  int size = nsegs / box_count;
  /* Number of output boxes that result from merging (size + 1) segments */
  int remainder = nsegs % box_count;
  int i = 0; /* Loop variable for input segments */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    assert(i < j);
    pt = (POINT4D *) getPoint_internal(pa, i);
    lwpoint_init_gbox(pt, hasz, hasm, geodetic, &result[k]);
    for (int l = i + 1; l <= j; l++)
    {
      pt = (POINT4D *) getPoint_internal(pa, l);
      lwpoint_merge_gbox(pt, hasz, hasm, geodetic, &result[k]);
    }
    i = j;
  }
  assert(i == nsegs);
  return box_count;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spatial boxes from the segments of a line
 * @param[in] gs Line
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `totalpoints <= box_count`, the result contains one box per
 * segment. Otherwise, consecutive input segments are combined into a single
 * box in the result to reach the given number of boxes.
 */
GBOX *
line_split_n_gboxes(const GSERIALIZED *gs, int box_count, int *count)
{
  assert(gs); assert(count); assert(gserialized_get_type(gs) == LINETYPE);
  assert(! gserialized_is_empty(gs)); assert(box_count > 0);

  bool geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  LWLINE *lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  int npoints = lwline->points->npoints;
  /* No points in the point array */
  if (! npoints)
  {
    lwline_free(lwline);
    return NULL;
  }

  int nboxes = (npoints <= box_count) ?
    ( npoints == 1 ? 1 : npoints - 1 ) : box_count;
  GBOX *result = palloc(sizeof(GBOX) * nboxes);
  *count = line_split_n_gboxes_iter(lwline, box_count, geodetic, result);
  lwline_free(lwline);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of N spatial boxes from the segments of a multiline
 * @param[in] gs Multiline
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `totalpoints <= box_count`, the result contains one box per
 * segment. Otherwise, consecutive input segments are combined into a single
 * box in the result to reach the given number of boxes.
 */
GBOX *
multiline_split_n_gboxes(const GSERIALIZED *gs, int box_count, int *count)
{
  assert(gs); assert(count); assert(gserialized_get_type(gs) == MULTILINETYPE);
  assert(! gserialized_is_empty(gs)); assert(box_count > 0);

  bool geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  LWMLINE *lwmline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
  int nlines = lwmline->ngeoms;
  /* No points in the point array */
  if ( ! nlines )
    return NULL;

  int totalpoints = 0;
  const LWLINE *lwline;
  for (int i = 0; i < nlines; i++)
  {
    lwline = lwmline->geoms[i];
    totalpoints += lwline->points->npoints;
  }
  int nboxes = (totalpoints <= box_count) ? totalpoints : box_count;
  GBOX *result = palloc(sizeof(GBOX) * nboxes);
  int nboxes1;

  /* One bounding box per segment */
  if (totalpoints <= box_count)
  {
    nboxes1 = 0;
    for (int i = 0; i < nlines; i++)
      nboxes1 += line_split_n_gboxes_iter(lwmline->geoms[i], box_count,
        geodetic, &result[nboxes1]);
    *count = nboxes1;
    assert(nboxes1 <= totalpoints);
    return result;
  }

  /* Number of bounding boxes per composing lines determined from the
   * proportion of nlines and totalpoints */
  if (nlines <= box_count)
  {
    nboxes1 = 0;
    for (int i = 0; i < nlines; i++)
    {
      lwline = lwmline->geoms[i];
      int nboxes_line = (int) (box_count * lwline->points->npoints * 1.0 /
        totalpoints);
      if (! nboxes_line)
        nboxes_line = 1;
      nboxes1 += line_split_n_gboxes_iter(lwline, nboxes_line,
        geodetic, &result[nboxes1]);
    }
    assert(nboxes1 <= box_count);
    *count = nboxes1;
    return result;
  }

  /* Merge consecutive sequences to reach the maximum number of boxes */
  /* Minimum number of sequences merged together in an output box */
  int size = nlines / box_count;
  /* Number of output boxes that result from merging (size + 1) sequences */
  int remainder = nlines % box_count;
  int i = 0; /* Loop variable for input sequences */
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    line_split_n_gboxes_iter(lwmline->geoms[i], 1, geodetic, &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      GBOX box;
      line_split_n_gboxes_iter(lwmline->geoms[l], 1, geodetic, &box);
      gbox_merge(&box, &result[k]);
    }
    i = j;
  }
  assert(i == nlines);
  *count = box_count;
  return result;
}

/**
 * @ingroup meos_temporal_bbox
 * @brief Return an array of N spatial boxes obtained by merging consecutive
 * segments of a (multi)line
 * @param[in] gs (Multi)line
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 */
GBOX *
geo_split_n_gboxes(const GSERIALIZED *gs, int box_count, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) count) ||
      ! ensure_not_empty(gs) || ! ensure_positive(box_count))
    return NULL;
  uint32_t geotype = gserialized_get_type(gs);
  if (geotype != LINETYPE && geotype != MULTILINETYPE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Argument is not a (multi)line");
    return NULL;
  }

  return (geotype == LINETYPE) ?
    line_split_n_gboxes(gs, box_count, count) :
    multiline_split_n_gboxes(gs, box_count, count);
}

/*****************************************************************************/

/**
 * @brief Return an array of spatial boxes from the segments of a line
 * (iterator function)
 * @param[in] lwline Line
 * @param[in] elems_per_box Number of input segments merged into an output box
 * @param[in] geodetic True when the line is geodetic
 * @param[out] result Array of boxes
 * @return Number of elements in the output array. If the number of segments <=
 * `elems_per_box`, the result contains a single box. Otherwise, consecutive
 * input segments are combined into a single output box.
 * @note We must pass the `geodetic` flag since `LWLINE` does not have this
 * flag set
 */
static int
line_split_each_n_gboxes_iter(const LWLINE *lwline, int elems_per_box,
  bool geodetic, GBOX *result)
{
  assert(lwline); assert(result); assert(elems_per_box > 0);

  POINTARRAY *pa = lwline->points;
  int npoints = lwline->points->npoints;
  bool hasz = FLAGS_GET_Z(pa->flags);
  bool hasm = FLAGS_GET_M(pa->flags);
  const POINT4D *pt;

  /* Line has only 1 point */
  if (npoints == 1)
  {
    pt = (POINT4D *) getPoint_internal(pa, 0);
    lwpoint_init_gbox(pt, hasz, hasm, geodetic, &result[0]);
    return 1;
  }

  /* General case */
  int k = 0;
  pt = (POINT4D *) getPoint_internal(pa, 0);
  lwpoint_init_gbox(pt, hasz, hasm, geodetic, &result[k]);
  for (int i = 1; i < npoints; ++i)
  {
    GBOX box;
    pt = (POINT4D *) getPoint_internal(pa, i);
    lwpoint_init_gbox(pt, hasz, hasm, geodetic, &box);
    gbox_merge(&box, &result[k]);
    if ((i % elems_per_box == 0) && (i < npoints - 1))
      result[++k] = box;
  }
  int nboxes = ceil((double) (npoints - 1) / (double) elems_per_box);
  assert(k + 1 == nboxes);
  return nboxes;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spatial boxes from the segments of a line
 * @param[in] gs Line
 * @param[in] elems_per_box Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 * @return If the number of segments <= `elems_per_box`, the result contains a 
 * single box. Otherwise, consecutive input segments are combined into a single
 * output box.
 */
GBOX *
line_split_each_n_gboxes(const GSERIALIZED *gs, int elems_per_box, int *count)
{
  assert(gs); assert(gserialized_get_type(gs) == LINETYPE);
  assert(! gserialized_is_empty(gs)); assert(elems_per_box > 0);

  LWLINE *lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  int npoints = lwline->points->npoints;
  /* No points in the point array */
  if (! npoints)
  {
    lwline_free(lwline);
    return NULL;
  }

  /* Number of segments */
  int nsegs = (npoints == 1) ? 1 : npoints - 1;
  int nboxes = ceil((double) nsegs / (double) elems_per_box);
  GBOX *result = palloc(sizeof(GBOX) * nboxes);
  bool geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  *count = line_split_each_n_gboxes_iter(lwline, elems_per_box, geodetic,
    result);
  lwline_free(lwline);
  return result;
}

/**
 * @ingroup meos_internal_temporal_bbox
 * @brief Return an array of spatial boxes from the segments of a multiline
 * @param[in] gs Multiline
 * @param[in] elems_per_box Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
GBOX *
multiline_split_each_n_gboxes(const GSERIALIZED *gs, int elems_per_box,
  int *count)
{
  assert(gs); assert(gserialized_get_type(gs) == MULTILINETYPE);
  assert(! gserialized_is_empty(gs)); assert(elems_per_box > 0);

  LWMLINE *lwmline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
  int nlines = lwmline->ngeoms;
  /* No points in the point array */
  if (! nlines)
  {
    lwmline_free(lwmline);
    return NULL;
  }

  /* Compute the total number of points */
  int totalpoints = 0;
  const LWLINE *lwline;
  for (int i = 0; i < nlines; i++)
  {
    lwline = lwmline->geoms[i];
    totalpoints += lwline->points->npoints;
  }

  /* Iterate for every composing line */
  int nboxes = 0;
  GBOX *result = palloc(sizeof(GBOX) * totalpoints);
  bool geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  for (int i = 0; i < nlines; ++i)
    nboxes += line_split_each_n_gboxes_iter(lwmline->geoms[i], elems_per_box,
      geodetic, &result[nboxes]);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_temporal_bbox
 * @brief Return an array of spatial boxes obtained by merging consecutive
 * segments of a (multi)line
 * @param[in] gs (Multi)line
 * @param[in] elems_per_box Number of input segments combined in an output box
 * @param[out] count Number of boxes in the output array
 * @return If the number of segments is <= `elems_per_box`, the result contains
 * a single box. Otherwise, consecutive input segments are combined into an
 * output box in the result. On error return @p NULL
 */
GBOX *
geo_split_each_n_gboxes(const GSERIALIZED *gs, int elems_per_box, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) count) ||
      ! ensure_not_empty(gs) || ! ensure_positive(elems_per_box))
    return NULL;
  uint32_t geotype = gserialized_get_type(gs);
  if (geotype != LINETYPE && geotype != MULTILINETYPE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Argument is not a (multi)line");
    return NULL;
  }

  return (geotype == LINETYPE) ?
    line_split_each_n_gboxes(gs, elems_per_box, count) :
    multiline_split_each_n_gboxes(gs, elems_per_box, count);
}

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a temporal point and a
 * spatiotemporal box
 */
bool
boxop_tpoint_stbox(const Temporal *temp, const STBox *box,
  bool (*func)(const STBox *, const STBox *), bool inverted)
{
  STBox box1;
  temporal_set_bbox(temp, &box1);
  return inverted ? func(box, &box1) : func(&box1, box);
}

/**
 * @brief Generic topological function for two temporal points
 */
bool
boxop_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const STBox *, const STBox *))
{
  STBox box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  return func(&box1, &box2);
}


/*****************************************************************************/

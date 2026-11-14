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
 * @brief Bounding box operators for temporal rigid geometries
 */

#include "rgeo/trgeo_boxops.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "geo/stbox.h"
#include "pose/pose.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_inst.h"
#include "rgeo/trgeo_utils.h"
#include "temporal/bbox_index.h"
#include "geo/stbox_index.h"

/*****************************************************************************
 * Transform a temporal geometry to a STBox
 *****************************************************************************/

/**
 * @brief Set the spatiotemporal box from the geometry value
 * @param[in] geom Reference geometry
 * @param[in] inst Temporal network point
 * @param[out] box Spatiotemporal box
 */
void
trgeoinst_set_stbox(const GSERIALIZED *geom, const TInstant *inst, STBox *box)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(box, 0, sizeof(STBox));
  bool hasz = (bool) FLAGS_GET_Z(geom->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(geom->gflags);
  box->srid = gserialized_get_srid(geom);
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, hasz);
  MEOS_FLAGS_SET_T(box->flags, true);
  MEOS_FLAGS_SET_GEODETIC(box->flags, geodetic);

  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);

  LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
  LWGEOM *lwgeom_copy = lwgeom_clone_deep(lwgeom);
  lwgeom_apply_pose(pose, lwgeom_copy);

  GBOX gbox;
  lwgeom_calculate_gbox(lwgeom_copy, &gbox);
  lwgeom_free(lwgeom_copy);
  lwgeom_free(lwgeom);

  box->xmin = gbox.xmin;
  box->xmax = gbox.xmax;
  box->ymin = gbox.ymin;
  box->ymax = gbox.ymax;
  if (hasz || geodetic)
  {
    box->zmin = gbox.zmin;
    box->zmax = gbox.zmax;
  }
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal rigid geometry values
 * @param[in] geom Reference geometry
 * @param[in] instants Temporal geometry values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
trgeoinstarr_static_stbox(const GSERIALIZED *geom, TInstant **instants,
  int count, STBox *box)
{
  trgeoinst_set_stbox(geom, instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    memset(&box1, 0, sizeof(STBox));
    trgeoinst_set_stbox(geom, instants[i], &box1);
    stbox_expand(&box1, box);
  }
}

/**
 * @brief Set the spatiotemporal box from the pose value
 * @param[out] box Spatiotemporal box
 * @param[in] inst Temporal network point
 */
static void
trgeoinst_make_pose_stbox(const TInstant *inst, STBox *box)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
  box->xmin = box->xmax = pose->data[0];
  box->ymin = box->ymax = pose->data[1];
  if (MEOS_FLAGS_GET_Z(pose->flags))
    box->zmin = box->zmax = pose->data[2];
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_X(box->flags, true);
  MEOS_FLAGS_SET_Z(box->flags, MEOS_FLAGS_GET_Z(pose->flags));
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal rigid geometry
 * values
 * @param[in] geom Geometry
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
trgeoinstarr_rotating_stbox(const GSERIALIZED *geom, TInstant **instants,
  int count, STBox *box)
{
  double r = geom_radius(geom);
  trgeoinst_make_pose_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    memset(&box1, 0, sizeof(STBox));
    trgeoinst_make_pose_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  box->xmin -= r;
  box->xmax += r;
  box->ymin -= r;
  box->ymax += r;
  if (MEOS_FLAGS_GET_Z(box->flags))
  {
    box->zmin -= r;
    box->zmax += r;
  }
  box->srid = gserialized_get_srid(geom);
  MEOS_FLAGS_SET_GEODETIC(box->flags, false);
  return;
}

/*****************************************************************************/

/**
 * @brief Set the bounding box from the array of temporal instant values
 * (dispatch function)
 * @param[in] geom Geometry
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[in] interp Interpolation
 * @param[out] box Box
 */
void
trgeoinstarr_compute_bbox(const GSERIALIZED *geom, TInstant **instants,
  int count, interpType interp, void *box)
{
  /* Only external types have bounding box */
  assert(temporal_type(instants[0]->temptype));
  if (instants[0]->temptype == T_TRGEOMETRY)
  {
    if (interp == LINEAR)
      trgeoinstarr_rotating_stbox(geom, instants, count, (STBox *) box);
    else
      trgeoinstarr_static_stbox(geom, instants, count, (STBox *) box);
  }
  else
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "unknown bounding box function for temporal type: %s",
      meostype_name(instants[0]->temptype));
  return;
}

/*****************************************************************************/

/*****************************************************************************
 * stboxes function
 *****************************************************************************/

/**
 * @brief Set a spatiotemporal box to the box of the instants first to last
 * of a temporal rigid geometry sequence
 * @details The box is the one #trgeoinstarr_compute_bbox gives the sequence,
 * so a segment where the geometry turns holds the geometry at every instant
 * of the segment, not only at its ends
 * @param[in] geom Reference geometry of the sequence
 * @param[in] seq Temporal sequence
 * @param[in] first,last Positions of the first and last instants
 * @param[out] box Spatiotemporal box
 */
static void
trgeoseq_slice_stbox(const GSERIALIZED *geom, const TSequence *seq, int first,
  int last, STBox *box)
{
  assert(geom); assert(seq); assert(box);
  assert(0 <= first); assert(first <= last); assert(last < seq->count);
  int count = last - first + 1;
  const TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
    instants[i] = TSEQUENCE_INST_N(seq, first + i);
  /* The box is filled field by field, so it starts from zero, as the loop of
   * #trgeoinstarr_rotating_stbox starts each of its boxes */
  memset(box, 0, sizeof(STBox));
  trgeoinstarr_compute_bbox(geom, (TInstant **) instants, count,
    MEOS_FLAGS_GET_INTERP(seq->flags), box);
  pfree(instants);
  return;
}

/* GENERATED-BOXSPLIT-BEGIN trgeometry — tools/codegen/inherited/generate.py from templates/boxsplit.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxsplit.yaml and re-run. */
/**
 * @ingroup meos_internal_geo_bbox
 * @brief Return a singleton array of spatiotemporal boxes from a
 * temporal rigid geometry instant
 * @param[in] inst Temporal instant
 */
STBox *
trgeoinst_stboxes(const TInstant *inst, const GSERIALIZED *geom)
{
  assert(inst); assert(tspatial_type(inst->temptype));
  /* One bounding box per instant */
  STBox *result = palloc(sizeof(STBox));
  trgeoinst_set_stbox(geom, inst, &result[0]);
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes from the instants
 * of a temporal rigid geometry sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 */
static STBox *
trgeoseq_disc_stboxes(const TSequence *seq, const GSERIALIZED *geom)
{
  assert(seq); assert(tspatial_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  /* One bounding box per instant */
  STBox *result = palloc(sizeof(STBox) * seq->count);
  for (int i = 0; i < seq->count; i++)
    trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal rigid geometry sequence with continuous interpolation (iterator
 * function)
 * @param[in] seq Temporal sequence
 * @param[out] result Array of boxes
 * @return Number of elements in the ouput array
 */
static int
trgeoseq_cont_stboxes_iter(const TSequence *seq, STBox *result,
  const GSERIALIZED *geom)
{
  assert(seq); assert(result); assert(tspatial_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    trgeoseq_slice_stbox(geom, seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  for (int i = 0; i < seq->count - 1; i++)
    trgeoseq_slice_stbox(geom, seq, i, i + 1, &result[i]);
  return seq->count - 1;
}

/**
 * @ingroup meos_internal_geo_bbox
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a temporal rigid geometry sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[out] count Number of elements in the output array
 */
STBox *
trgeoseq_stboxes(const TSequence *seq, int *count, const GSERIALIZED *geom)
{
  assert(seq); assert(count); assert(tspatial_type(seq->temptype));

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return trgeoseq_disc_stboxes(seq, geom);
  }

  /* Continuous case */
  int nboxes = (seq->count == 1) ? 1 : seq->count - 1;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  *count = trgeoseq_cont_stboxes_iter(seq, result, geom);
  return result;
}

/**
 * @ingroup meos_internal_geo_bbox
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal rigid geometry sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] count Number of elements in the output array
 */
STBox *
trgeoseqset_stboxes(const TSequenceSet *ss, int *count, const GSERIALIZED *geom)
{
  assert(ss); assert(count); assert(tspatial_type(ss->temptype));

  /* One bounding box per segment */
  STBox *result = palloc(sizeof(STBox) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
    nboxes += trgeoseq_cont_stboxes_iter(TSEQUENCESET_SEQ_N(ss, i),
      &result[nboxes], geom);
  assert(nboxes <= ss->totalcount);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a temporal rigid geometry
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal rigid geometry
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Trgeometry_stboxes()
 */
STBox *
trgeometry_stboxes(const Temporal *temp, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  const GSERIALIZED *geom = trgeometry_geom_p(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return trgeoinst_stboxes((TInstant *) temp, geom);
    case TSEQUENCE:
      return trgeoseq_stboxes((TSequence *) temp, count, geom);
    default: /* TSEQUENCESET */
      return trgeoseqset_stboxes((TSequenceSet *) temp, count, geom);
  }
}

/*****************************************************************************/

/**
 * @brief Return an array of N spatiotemporal boxes from the instants
 * of a temporal rigid geometry sequence with discrete interpolation
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 */
static STBox *
trgeoseq_disc_split_n_stboxes(const TSequence *seq, int box_count, int *count,
  const GSERIALIZED *geom)
{
  assert(seq); assert(count); assert(tspatial_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(box_count > 0);

  /* One bounding box per instant */
  if (seq->count <= box_count)
  {
    *count = seq->count;
    return trgeoseq_disc_stboxes(seq, geom);
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
    trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l < j; l++)
    {
      STBox box;
      trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, l), &box);
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
 * temporal rigid geometry sequence with continuous interpolation (iterator
 * function)
 * @param[in] seq Temporal rigid geometry
 * @param[in] box_count Number of boxes
 * @param[out] result Array of boxes
 */
static int
trgeoseq_cont_split_n_stboxes_iter(const TSequence *seq, int box_count,
  STBox *result, const GSERIALIZED *geom)
{
  assert(seq); assert(result); assert(tspatial_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(box_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    trgeoseq_slice_stbox(geom, seq, 0, 0, &result[0]);
    return 1;
  }

  /* One bounding box per segment */
  int nsegs = seq->count - 1;
  if (nsegs <= box_count)
    return trgeoseq_cont_stboxes_iter(seq, result, geom);

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
    trgeoseq_slice_stbox(geom, seq, i, j, &result[k]);
    i = j;
  }
  assert(i == nsegs);
  return box_count;
}

/**
 * @ingroup meos_internal_geo_bbox
 * @brief Return an array of N spatiotemporal boxes from the instants or
 * segments of a temporal rigid geometry sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `seq->count <= box_count`, the result contains one box
 * per instant or segment. Otherwise, consecutive instants or segments are
 * combined into a single box in the result to reach the given number of
 * boxes.
 */
STBox *
trgeoseq_split_n_stboxes(const TSequence *seq, int box_count, int *count,
  const GSERIALIZED *geom)
{
  assert(seq); assert(count); assert(tspatial_type(seq->temptype));
  assert(box_count > 0);

  /* Discrete case */
  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return trgeoseq_disc_split_n_stboxes(seq, box_count, count, geom);

  /* Continuous case */
  int nboxes = (seq->count <= box_count) ?
    (seq->count == 1 ? 1 : seq->count - 1) : box_count;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  *count = trgeoseq_cont_split_n_stboxes_iter(seq, box_count, result, geom);
  return result;
}

/**
 * @ingroup meos_internal_geo_bbox
 * @brief Return an array of N spatiotemporal boxes from the segments of a
 * temporal rigid geometry sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return If `ss->totalcount <= box_count`, the result contains one box per
 * segment. Otherwise, consecutive input segments are combined into a single box
 * in
 * the result to reach the given number of boxes.
 */
STBox *
trgeoseqset_split_n_stboxes(const TSequenceSet *ss, int box_count, int *count,
  const GSERIALIZED *geom)
{
  assert(ss); assert(count); assert(tspatial_type(ss->temptype));
  assert(box_count > 0);

  /* One bounding box per segment */
  int nboxes = (ss->totalcount <= box_count) ? ss->totalcount : box_count;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  if (ss->totalcount <= box_count)
    return trgeoseqset_stboxes(ss, count, geom);

  /* Number of bounding boxes per composing sequence determined from the
   * proportion of seq->count and ss->totalcount */
  if (ss->count <= box_count)
  {
    int nboxes1 = 0;
    for (int i = 0; i < ss->count; i++)
    {
      bool end = false;
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int nboxes_seq = (int) (box_count * seq->count * 1.0 / ss->totalcount);
      if (! nboxes_seq)
        nboxes_seq = 1;
      if (nboxes_seq + nboxes1 >= box_count)
      {
        end = true;
        nboxes_seq = box_count - nboxes1;
      }
      nboxes1 += trgeoseq_cont_split_n_stboxes_iter(seq, nboxes_seq,
        &result[nboxes1], geom);
      if (end)
        break;
    }
    assert(nboxes1 <= box_count);
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
    trgeoseq_cont_split_n_stboxes_iter(TSEQUENCESET_SEQ_N(ss, i), 1,
      &result[k], geom);
    for (int l = i + 1; l < j; l++)
    {
      STBox box;
      trgeoseq_cont_split_n_stboxes_iter(TSEQUENCESET_SEQ_N(ss, l), 1, &box,
        geom);
      stbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == ss->count);
  *count = box_count;
  return result;
}

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of N spatiotemporal boxes obtained by merging
 * consecutive instants or segments of a temporal rigid geometry
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal rigid geometry
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @errval NULL
 * @csqlfn #Trgeometry_split_n_stboxes()
 */
STBox *
trgeometry_split_n_stboxes(const Temporal *temp, int box_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_positive(box_count))
    return NULL;

  const GSERIALIZED *geom = trgeometry_geom_p(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return trgeoinst_stboxes((TInstant *) temp, geom);
    case TSEQUENCE:
      return trgeoseq_split_n_stboxes((TSequence *) temp, box_count, count,
        geom);
    default: /* TSEQUENCESET */
      return trgeoseqset_split_n_stboxes((TSequenceSet *) temp, box_count,
        count, geom);
  }
}

/*****************************************************************************
 * split_each_n_stboxes function
 *****************************************************************************/

/**
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive instants of a temporal rigid geometry sequence with discrete
 * interpolation
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input instants merged into an output box
 * @param[out] count Number of elements in the output array
 */
static STBox *
trgeoseq_disc_split_each_n_stboxes(const TSequence *seq, int elem_count,
  int *count, const GSERIALIZED *geom)
{
  assert(seq); assert(count); assert(tspatial_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  assert(elem_count > 0);

  int nboxes = ceil((double) seq->count / (double) elem_count);
  STBox *result = palloc(sizeof(STBox) * nboxes);
  int k = 0;
  for (int i = 0; i < seq->count; ++i)
  {
    if (i % elem_count == 0)
      trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[k++]);
    else
    {
      STBox box;
      trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &box);
      stbox_expand(&box, &result[k - 1]);
    }
  }
  assert(k == nboxes);
  *count = k;
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive segments of a temporal rigid geometry sequence (iterator
 * function)
 * @details The sequence has continuous interpolation.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] result Array of boxes
 * @return Number of elements in the output array
 */
static int
trgeoseq_cont_split_each_n_stboxes_iter(const TSequence *seq,
  int elem_count, STBox *result, const GSERIALIZED *geom)
{
  assert(seq); assert(result); assert(tspatial_type(seq->temptype));
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  assert(elem_count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tspatialseq_set_stbox(seq, &result[0]);
    return 1;
  }

  /* General case */
  int nboxes = ceil((double) (seq->count - 1) / (double) elem_count);
  for (int k = 0; k < nboxes; k++)
  {
    int first = k * elem_count;
    int last = Min(first + elem_count, seq->count - 1);
    trgeoseq_slice_stbox(geom, seq, first, last, &result[k]);
  }
  return nboxes;
}

/**
 * @ingroup meos_internal_geo_bbox
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive instants or segments of a temporal rigid geometry sequence
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] seq Temporal sequence
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
STBox *
trgeoseq_split_each_n_stboxes(const TSequence *seq, int elem_count,
  int *count, const GSERIALIZED *geom)
{
  assert(seq); assert(count); assert(tspatial_type(seq->temptype));
  assert(elem_count > 0);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    return trgeoseq_disc_split_each_n_stboxes(seq, elem_count, count, geom);

  /* Number of instants or segments */
  int nelems = (seq->count == 1) ? 1 : seq->count - 1;
  int nboxes = ceil((double) nelems / (double) elem_count);
  STBox *result = palloc(sizeof(STBox) * nboxes);
  *count = trgeoseq_cont_split_each_n_stboxes_iter(seq, elem_count,
    result, geom);
  return result;
}

/**
 * @ingroup meos_internal_geo_bbox
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive segments of a temporal rigid geometry sequence set 
 * @param[in] ss Temporal sequence set
 * @param[in] elem_count Number of input segments merged into an output box
 * @param[out] count Number of elements in the output array
 */
STBox *
trgeoseqset_split_each_n_stboxes(const TSequenceSet *ss, int elem_count,
  int *count, const GSERIALIZED *geom)
{
  assert(ss); assert(count); assert(tspatial_type(ss->temptype));
  assert(elem_count > 0);

  /* Singleton sequence set */
  if (ss->count == 1)
    return trgeoseq_split_each_n_stboxes(TSEQUENCESET_SEQ_N(ss, 0),
      elem_count, count, geom);

  /* Iterate for every composing sequence */
  int nboxes = 0;
  STBox *result = palloc(sizeof(STBox) * ss->totalcount);
  for (int i = 0; i < ss->count; ++i)
    nboxes += trgeoseq_cont_split_each_n_stboxes_iter(
      TSEQUENCESET_SEQ_N(ss, i), elem_count, &result[nboxes], geom);
  *count = nboxes;
  return result;
}

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of spatiotemporal boxes obtained by merging
 * consecutive instants or segments of a temporal rigid geometry
 * @details The choice between instants or segments depends, respectively, on
 * whether the interpolation is discrete or continuous.
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Trgeometry_split_each_n_stboxes()
 */
STBox *
trgeometry_split_each_n_stboxes(const Temporal *temp, int elem_count,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;

  const GSERIALIZED *geom = trgeometry_geom_p(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return trgeoinst_stboxes((TInstant *) temp, geom);
    case TSEQUENCE:
      return trgeoseq_split_each_n_stboxes((TSequence *) temp, elem_count,
        count, geom);
    default: /* TSEQUENCESET */
      return trgeoseqset_split_each_n_stboxes((TSequenceSet *) temp,
        elem_count, count, geom);
  }
}

/*****************************************************************************
 * merge_n_stboxes and merge_each_n_stboxes functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of N spatiotemporal boxes obtained by merging the
 * boxes of
 * the consecutive instants or segments of a temporal rigid geometry, each time
 * the
 * consecutive pair whose union adds the least size to theirs
 * @details The merge twin of #trgeometry_split_n_stboxes, by #bbox_merge_n
 * @param[in] temp Temporal value
 * @param[in] box_count Number of boxes
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Trgeometry_merge_n_stboxes()
 */
STBox *
trgeometry_merge_n_stboxes(const Temporal *temp, int box_count, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_positive(box_count))
    return NULL;

  STBox *result = trgeometry_stboxes(temp, count);
  if (result)
    *count = bbox_merge_n(T_STBOX, result, *count, box_count, &stbox_adjust,
      &stbox_size);
  return result;
}

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of spatiotemporal boxes obtained by merging the boxes
 * of the
 * consecutive instants or segments of a temporal rigid geometry, each time the
 * consecutive
 * pair whose union adds the least size to theirs, until a box holds on
 * average a number of them
 * @details The merge twin of #trgeometry_split_each_n_stboxes, by
 * #bbox_merge_each_n
 * @param[in] temp Temporal value
 * @param[in] elem_count Number of input instants or segments merged into an
 * output box on average
 * @param[out] count Number of values of the output array
 * @errval NULL
 * @csqlfn #Trgeometry_merge_each_n_stboxes()
 */
STBox *
trgeometry_merge_each_n_stboxes(const Temporal *temp, int elem_count,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_positive(elem_count))
    return NULL;

  STBox *result = trgeometry_stboxes(temp, count);
  if (result)
    *count = bbox_merge_each_n(T_STBOX, result, *count, elem_count,
      &stbox_adjust, &stbox_size);
  return result;
}
/* GENERATED-BOXSPLIT-END trgeometry */

/*****************************************************************************/

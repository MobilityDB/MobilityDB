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
 * @param[in] count Number of elements in the array
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
 * @param[in] count Number of elements in the array
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
 * These functions can be used for defining MultiEntry Search Trees (a.k.a.
 * VODKA) indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 * https://github.com/MobilityDB/mest
 *****************************************************************************/

/**
 * @brief Return an array of maximumn n spatiotemporal boxes from the segments
 * of a temporal point sequence (iterator function)
 * @param[in] seq Temporal value
 * @param[in] max_count Maximum number of elements in the output array
 * If the value is < 1, the result is one box per segment
 * @param[out] result Spatiotemporal box
 * @return Number of elements in the array
 */
static int
tpointseq_stboxes_from_segs_iter(const TSequence *seq, int max_count,
  STBox *result)
{
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    tpointinst_set_stbox(TSEQUENCE_INST_N(seq, 0), &result[0]);
    return 1;
  }

  /* Temporal sequence has at least 2 instants */
  int num_segs = seq->count - 1;
  if (max_count < 1 || num_segs <= max_count)
  {
    /* One bounding box per segment */
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
    for (int i = 0; i < seq->count - 1; i++)
    {
      tpointinst_set_stbox(inst1, &result[i]);
      const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
      STBox box;
      tpointinst_set_stbox(inst2, &box);
      stbox_expand(&box, &result[i]);
      inst1 = inst2;
    }
    return num_segs;
  }
  else
  {
    /* One bounding box per several consecutive segments */
    /* Minimum number of input segments merged together in an output box */
    int size = num_segs / max_count;
    /* Number of output boxes that result from merging (size + 1) segments */
    int remainder = num_segs % max_count;
    int i = 0; /* Loop variable for input segments */
    int k = 0; /* Loop variable for output boxes */
    while (k < max_count)
    {
      int j = i + size;
      if (k < remainder)
        j++;
      assert(i < j);
      tpointinst_set_stbox(TSEQUENCE_INST_N(seq, i), &result[k]);
      for (int l = i + 1; l <= j; l++)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, l);
        STBox box;
        tpointinst_set_stbox(inst, &box);
        stbox_expand(&box, &result[k]);
      }
      k++;
      i = j;
    }
    return max_count;
  }
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return an array of maximumn n spatiotemporal boxes from the segments
 * of a temporal point sequence
 * @param[in] seq Temporal sequence
 * @param[in] max_count Maximum number of elements in the output array
 * If the value is < 1, the result is one box per segment
 * @param[out] count Number of elements in the output array
 */
STBox *
tpointseq_stboxes_from_segs(const TSequence *seq, int max_count, int *count)
{
  assert(seq); assert(count); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  int nboxes = (max_count < 1) ?
    ( seq->count == 1 ? 1 : seq->count - 1 ) : max_count;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  *count = tpointseq_stboxes_from_segs_iter(seq, max_count, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @param[in] max_count Maximum number of elements in the output array
 * If the value is < 1, the result is one box per segment
 * @param[out] count Number of elements in the output array
 */
STBox *
tpointseqset_stboxes(const TSequenceSet *ss, int max_count, int *count)
{
  assert(ss); assert(count); assert(tgeo_type(ss->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));
  int nboxes = (max_count < 1) ? ss->totalcount : max_count;
  STBox *result = palloc(sizeof(STBox) * nboxes);
  int nboxes1;
  if (max_count < 1 || ss->totalcount <= max_count)
  {
    /* One bounding box per segment */
    nboxes1 = 0;
    for (int i = 0; i < ss->count; i++)
      nboxes1 += tpointseq_stboxes_from_segs_iter(TSEQUENCESET_SEQ_N(ss, i),
        max_count, &result[nboxes1]);
    *count = nboxes1;
    return result;
  }
  else if (ss->count <= max_count)
  {
    /* Amount of bounding boxes per composing sequence determined from the
     * proportion of seq->count and ss->totalcount */
    nboxes1 = 0;
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int nboxes_seq = (int) (max_count * seq->count * 1.0 / ss->totalcount);
      if (! nboxes_seq)
        nboxes_seq = 1;
      nboxes1 += tpointseq_stboxes_from_segs_iter(seq, nboxes_seq,
        &result[nboxes1]);
    }
    *count = nboxes1;
    return result;
  }
  else
  {
    /* Merge consecutive sequences to reach the maximum number of boxes */
    /* Minimum number of sequences merged together in an output box */
    int size = ss->count / max_count;
    /* Number of output boxes that result from merging (size + 1) sequences */
    int remainder = ss->count % max_count;
    int i = 0; /* Loop variable for input sequences */
    int k = 0; /* Loop variable for output boxes */
    while (k < max_count)
    {
      int j = i + size - 1;
      if (k < remainder)
        j++;
      if (i < j)
      {
        tpointseq_stboxes_from_segs_iter(TSEQUENCESET_SEQ_N(ss, i), 1,
          &result[k]);
        for (int l = i + 1; l <= j; l++)
        {
          STBox box;
          tpointseq_stboxes_from_segs_iter(TSEQUENCESET_SEQ_N(ss, l), 1, &box);
          stbox_expand(&box, &result[k]);
        }
        i = j + 1;
        k++;
      }
      else
        tpointseq_stboxes_from_segs_iter(TSEQUENCESET_SEQ_N(ss, i++), 1,
          &result[k++]);
    }
    *count = max_count;
    return result;
  }
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal point
 * @param[in] temp Temporal value
 * @param[in] max_count Maximum number of elements in the output array.
 * If the value is < 1, the result is one box per segment
 * @param[out] count Number of values of the output array
 * @return On error return @p NULL
 * @csqlfn #Tpoint_stboxes()
 */
STBox *
tpoint_stboxes_from_segs(const Temporal *temp, int max_count, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  if (! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return NULL;
  else if (temp->subtype == TSEQUENCE)
    return tpointseq_stboxes_from_segs((TSequence *)temp, max_count, count);
  else /* TSEQUENCESET */
    return tpointseqset_stboxes((TSequenceSet *)temp, max_count, count);
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

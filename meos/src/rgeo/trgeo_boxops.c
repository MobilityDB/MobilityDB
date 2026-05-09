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
  const Pose *pose = DatumGetPoseP(tinstant_value(inst));

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
  Pose *pose = DatumGetPoseP(tinstant_value(inst));
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
      "unknown bounding box function for temporal type: %d",
      meostype_name(instants[0]->temptype));
  return;
}

/*****************************************************************************/

/*****************************************************************************
 * stboxes function
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of spatiotemporal boxes from the instants or segments
 * of a temporal rigid geometry, where the choice depends on whether the
 * interpolation is discrete or continuous
 * @param[in] temp Temporal rigid geometry
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 */
STBox *
trgeo_stboxes(const Temporal *temp, int *count)
{
  assert(temp); assert(count);
  assert(temp->temptype == T_TRGEOMETRY);
  const GSERIALIZED *geom = trgeo_geom_p(temp);
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      *count = 1;
      STBox *result = palloc(sizeof(STBox));
      trgeoinst_set_stbox(geom, (TInstant *) temp, result);
      return result;
    }
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
      {
        *count = seq->count;
        STBox *result = palloc(sizeof(STBox) * seq->count);
        for (int i = 0; i < seq->count; i++)
          trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[i]);
        return result;
      }
      int nboxes = (seq->count == 1) ? 1 : seq->count - 1;
      STBox *result = palloc(sizeof(STBox) * nboxes);
      if (seq->count == 1)
      {
        trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, 0), &result[0]);
      }
      else
      {
        for (int i = 0; i < seq->count - 1; i++)
        {
          trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[i]);
          STBox box;
          trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i + 1), &box);
          stbox_expand(&box, &result[i]);
        }
      }
      *count = nboxes;
      return result;
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      STBox *result = palloc(sizeof(STBox) * ss->totalcount);
      int nboxes = 0;
      for (int i = 0; i < ss->count; i++)
      {
        const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
        if (seq->count == 1)
        {
          trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, 0),
            &result[nboxes++]);
        }
        else
        {
          for (int j = 0; j < seq->count - 1; j++)
          {
            trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, j),
              &result[nboxes]);
            STBox box;
            trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, j + 1), &box);
            stbox_expand(&box, &result[nboxes++]);
          }
        }
      }
      *count = nboxes;
      return result;
    }
  }
}

/*****************************************************************************
 * split_n_stboxes function
 *****************************************************************************/

/**
 * @brief Iterator for continuous-interpolation sequences: split into at most
 * box_count boxes for a temporal rigid geometry sequence
 */
static int
trgeo_seq_cont_split_n_iter(const TSequence *seq,
  const GSERIALIZED *geom, int box_count, STBox *result)
{
  assert(seq); assert(result);
  if (seq->count == 1)
  {
    trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, 0), &result[0]);
    return 1;
  }
  int nsegs = seq->count - 1;
  if (nsegs <= box_count)
  {
    for (int i = 0; i < nsegs; i++)
    {
      trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[i]);
      STBox box;
      trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i + 1), &box);
      stbox_expand(&box, &result[i]);
    }
    return nsegs;
  }
  int size = nsegs / box_count;
  int remainder = nsegs % box_count;
  int i = 0;
  for (int k = 0; k < box_count; k++)
  {
    int j = i + size;
    if (k < remainder)
      j++;
    trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[k]);
    for (int l = i + 1; l <= j; l++)
    {
      STBox box;
      trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, l), &box);
      stbox_expand(&box, &result[k]);
    }
    i = j;
  }
  assert(i == nsegs);
  return box_count;
}

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of N spatiotemporal boxes obtained by merging
 * consecutive instants or segments of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] box_count Number of boxes
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 */
STBox *
trgeo_split_n_stboxes(const Temporal *temp, int box_count, int *count)
{
  assert(temp); assert(count);
  assert(temp->temptype == T_TRGEOMETRY);
  if (! ensure_positive(box_count))
    return NULL;
  const GSERIALIZED *geom = trgeo_geom_p(temp);
  switch (temp->subtype)
  {
    case TINSTANT:
      return trgeo_stboxes(temp, count);
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
      {
        if (seq->count <= box_count)
          return trgeo_stboxes(temp, count);
        int size = seq->count / box_count;
        int remainder = seq->count % box_count;
        STBox *result = palloc(sizeof(STBox) * box_count);
        int i = 0;
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
      int nboxes = (seq->count <= box_count) ?
        (seq->count == 1 ? 1 : seq->count - 1) : box_count;
      STBox *result = palloc(sizeof(STBox) * nboxes);
      *count = trgeo_seq_cont_split_n_iter(seq, geom, box_count, result);
      return result;
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      if (ss->totalcount <= box_count)
        return trgeo_stboxes(temp, count);
      int nboxes = (ss->totalcount <= box_count) ? ss->totalcount : box_count;
      STBox *result = palloc(sizeof(STBox) * nboxes);
      if (ss->count <= box_count)
      {
        int nboxes1 = 0;
        for (int i = 0; i < ss->count; i++)
        {
          bool end = false;
          const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
          int nboxes_seq = (int) (box_count * seq->count * 1.0 /
            ss->totalcount);
          if (! nboxes_seq)
            nboxes_seq = 1;
          if (nboxes_seq + nboxes1 >= box_count)
          {
            end = true;
            nboxes_seq = box_count - nboxes1;
          }
          nboxes1 += trgeo_seq_cont_split_n_iter(seq, geom, nboxes_seq,
            &result[nboxes1]);
          if (end)
            break;
        }
        assert(nboxes1 <= box_count);
        *count = nboxes1;
        return result;
      }
      int size = ss->count / box_count;
      int remainder = ss->count % box_count;
      int i = 0;
      for (int k = 0; k < box_count; k++)
      {
        int j = i + size;
        if (k < remainder)
          j++;
        trgeo_seq_cont_split_n_iter(TSEQUENCESET_SEQ_N(ss, i), geom, 1,
          &result[k]);
        for (int l = i + 1; l < j; l++)
        {
          STBox box;
          trgeo_seq_cont_split_n_iter(TSEQUENCESET_SEQ_N(ss, l), geom, 1,
            &box);
          stbox_expand(&box, &result[k]);
        }
        i = j;
      }
      assert(i == ss->count);
      *count = box_count;
      return result;
    }
  }
}

/*****************************************************************************
 * split_each_n_stboxes function
 *****************************************************************************/

/**
 * @brief Iterator for continuous-interpolation sequences: merge each
 * elems_per_box consecutive segments into one box for a trgeo sequence
 */
static int
trgeo_seq_cont_split_each_n_iter(const TSequence *seq,
  const GSERIALIZED *geom, int elems_per_box, STBox *result)
{
  assert(seq); assert(result);
  if (seq->count == 1)
  {
    tspatialseq_set_stbox(seq, &result[0]);
    return 1;
  }
  int k = 0;
  trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, 0), &result[k]);
  for (int i = 1; i < seq->count; i++)
  {
    STBox box;
    trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &box);
    stbox_expand(&box, &result[k]);
    if ((i % elems_per_box == 0) && (i < seq->count - 1))
      result[++k] = box;
  }
  int nboxes = (int) ceil((double) (seq->count - 1) / (double) elems_per_box);
  assert(k + 1 == nboxes);
  return nboxes;
}

/**
 * @ingroup meos_rgeo_bbox_split
 * @brief Return an array of spatiotemporal boxes obtained by merging each N
 * consecutive instants or segments of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] elems_per_box Number of input elements merged per output box
 * @param[out] count Number of elements in the output array
 * @return On error return @p NULL
 */
STBox *
trgeo_split_each_n_stboxes(const Temporal *temp, int elems_per_box, int *count)
{
  assert(temp); assert(count);
  assert(temp->temptype == T_TRGEOMETRY);
  if (! ensure_positive(elems_per_box))
    return NULL;
  const GSERIALIZED *geom = trgeo_geom_p(temp);
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tspatial_to_stbox(temp);
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
      {
        int nboxes = (int) ceil((double) seq->count / (double) elems_per_box);
        STBox *result = palloc(sizeof(STBox) * nboxes);
        int k = 0;
        for (int i = 0; i < seq->count; i++)
        {
          if (i % elems_per_box == 0)
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
      int nelems = (seq->count == 1) ? 1 : seq->count - 1;
      int nboxes = (int) ceil((double) nelems / (double) elems_per_box);
      STBox *result = palloc(sizeof(STBox) * nboxes);
      *count = trgeo_seq_cont_split_each_n_iter(seq, geom, elems_per_box,
        result);
      return result;
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      int nboxes = 0;
      STBox *result = palloc(sizeof(STBox) * ss->totalcount);
      for (int i = 0; i < ss->count; i++)
        nboxes += trgeo_seq_cont_split_each_n_iter(TSEQUENCESET_SEQ_N(ss, i),
          geom, elems_per_box, &result[nboxes]);
      *count = nboxes;
      return result;
    }
  }
}

/*****************************************************************************/

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
#include "rgeo/trgeo_inst.h"
#include "rgeo/trgeo_seq.h"
#include "rgeo/trgeo_seqset.h"
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
 * stboxes / splitNStboxes / splitEachNStboxes for trgeometry
 *****************************************************************************/

/**
 * @brief Return an array of one spatiotemporal box from a temporal rigid
 * geometry instant
 */
static STBox *
trgeo_inst_stboxes(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TRGEOMETRY);
  STBox *result = palloc(sizeof(STBox));
  trgeoinst_set_stbox(trgeoinst_geom_p(inst), inst, result);
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes from the instants of a
 * temporal rigid geometry sequence with discrete interpolation
 */
static STBox *
trgeo_disc_seq_stboxes(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TRGEOMETRY);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE);
  const GSERIALIZED *geom = trgeoseq_geom_p(seq);
  STBox *result = palloc(sizeof(STBox) * seq->count);
  for (int i = 0; i < seq->count; i++)
    trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, i), &result[i]);
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal rigid geometry sequence with continuous interpolation
 */
static int
trgeo_cont_seq_stboxes_iter(const TSequence *seq, STBox *result)
{
  assert(seq); assert(result); assert(seq->temptype == T_TRGEOMETRY);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);
  const GSERIALIZED *geom = trgeoseq_geom_p(seq);

  if (seq->count == 1)
  {
    trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, 0), &result[0]);
    return 1;
  }

  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    trgeoinst_set_stbox(geom, inst, &result[i]);
    inst = TSEQUENCE_INST_N(seq, i + 1);
    STBox box;
    trgeoinst_set_stbox(geom, inst, &box);
    stbox_expand(&box, &result[i]);
  }
  return seq->count - 1;
}

/**
 * @brief Return an array of spatiotemporal boxes from a temporal rigid
 * geometry sequence
 */
static STBox *
trgeo_seq_stboxes(const TSequence *seq, int *count)
{
  assert(seq); assert(count); assert(seq->temptype == T_TRGEOMETRY);

  if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
  {
    *count = seq->count;
    return trgeo_disc_seq_stboxes(seq);
  }

  *count = (seq->count == 1) ? 1 : seq->count - 1;
  STBox *result = palloc(sizeof(STBox) * *count);
  trgeo_cont_seq_stboxes_iter(seq, result);
  return result;
}

/**
 * @brief Return an array of spatiotemporal boxes from a temporal rigid
 * geometry sequence set
 */
static STBox *
trgeo_seqset_stboxes(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count); assert(ss->temptype == T_TRGEOMETRY);

  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
      nboxes += seq->count;
    else
      nboxes += (seq->count == 1) ? 1 : seq->count - 1;
  }

  STBox *result = palloc(sizeof(STBox) * nboxes);
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (MEOS_FLAGS_GET_INTERP(seq->flags) == DISCRETE)
    {
      const GSERIALIZED *geom = trgeoseq_geom_p(seq);
      for (int j = 0; j < seq->count; j++)
        trgeoinst_set_stbox(geom, TSEQUENCE_INST_N(seq, j), &result[k++]);
    }
    else
    {
      k += trgeo_cont_seq_stboxes_iter(seq, &result[k]);
    }
  }
  *count = k;
  return result;
}

/**
 * @ingroup meos_rgeo_bbox
 * @brief Return an array of spatiotemporal boxes from the instants or
 * segments of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[out] count Number of elements in the output array
 */
STBox *
trgeo_stboxes(const Temporal *temp, int *count)
{
  assert(temp); assert(count); assert(temp->temptype == T_TRGEOMETRY);

  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return trgeo_inst_stboxes((TInstant *) temp);
    case TSEQUENCE:
      return trgeo_seq_stboxes((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return trgeo_seqset_stboxes((TSequenceSet *) temp, count);
  }
}

/**
 * @ingroup meos_rgeo_bbox
 * @brief Return N spatiotemporal boxes from a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] box_count Target number of boxes
 * @param[out] count Number of elements in the output array
 */
STBox *
trgeo_split_n_stboxes(const Temporal *temp, int box_count, int *count)
{
  assert(temp); assert(count); assert(temp->temptype == T_TRGEOMETRY);
  assert(box_count > 0);

  int nboxes;
  STBox *all = trgeo_stboxes(temp, &nboxes);

  if (nboxes <= box_count)
  {
    *count = nboxes;
    return all;
  }

  STBox *result = palloc(sizeof(STBox) * box_count);
  int size = nboxes / box_count;
  int remainder = nboxes % box_count;
  int src = 0;
  for (int k = 0; k < box_count; k++)
  {
    int grp = size + (k < remainder ? 1 : 0);
    result[k] = all[src];
    for (int j = 1; j < grp; j++)
      stbox_expand(&all[src + j], &result[k]);
    src += grp;
  }
  pfree(all);
  *count = box_count;
  return result;
}

/**
 * @ingroup meos_rgeo_bbox
 * @brief Return spatiotemporal boxes from a temporal rigid geometry,
 * one per every N segments
 * @param[in] temp Temporal rigid geometry
 * @param[in] elems_per_box Number of segments per output box
 * @param[out] count Number of elements in the output array
 */
STBox *
trgeo_split_each_n_stboxes(const Temporal *temp, int elems_per_box, int *count)
{
  assert(temp); assert(count); assert(temp->temptype == T_TRGEOMETRY);
  assert(elems_per_box > 0);

  int nboxes;
  STBox *all = trgeo_stboxes(temp, &nboxes);

  int nout = (nboxes + elems_per_box - 1) / elems_per_box;
  STBox *result = palloc(sizeof(STBox) * nout);
  int k = 0;
  for (int i = 0; i < nboxes; i += elems_per_box)
  {
    result[k] = all[i];
    int end = Min(i + elems_per_box, nboxes);
    for (int j = i + 1; j < end; j++)
      stbox_expand(&all[j], &result[k]);
    k++;
  }
  pfree(all);
  *count = nout;
  return result;
}

/*****************************************************************************/

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
 * @brief Bounding box operators for temporal points.
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
#include "general/temporaltypes.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

extern void ll2cart(const POINT2D *g, POINT3D *p);
extern int edge_calculate_gbox(const POINT3D *A1, const POINT3D *A2, GBOX *gbox);

/*****************************************************************************
 * Functions computing the bounding box at the creation of a temporal point
 *****************************************************************************/

/**
 * @brief Set the spatiotemporal box from a temporal instant point
 */
void
tpointinst_set_stbox(const TInstant *inst, STBox *box)
{
  GSERIALIZED *point = DatumGetGserializedP(tinstant_value(inst));
  geo_set_stbox(point, box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
}

/**
 * @brief Set the spatiotemporal box from an array of temporal instant points
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
    GSERIALIZED *point = DatumGetGserializedP(tinstant_value(instants[i]));
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
 * @brief Set the spatiotemporal box from an array of temporal sequence points
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
 * These functions can be used for defining MultiEntry (a.k.a.) VODKA indexes
 * https://www.pgcon.org/2014/schedule/events/696.en.html
 * https://github.com/mschoema/mgist
 *****************************************************************************/

/**
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal sequence point (iterator function)
 * @param[in] seq Temporal value
 * @param[out] result Spatiotemporal box
 * @return Number of elements in the array
 */
static int
tpointseq_stboxes_iter(const TSequence *seq, STBox *result)
{
  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));
  const TInstant *inst1;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    tpointinst_set_stbox(inst1, &result[0]);
    return 1;
  }

  /* Temporal sequence has at least 2 instants */
  inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 0; i < seq->count - 1; i++)
  {
    tpointinst_set_stbox(inst1, &result[i]);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    STBox box;
    tpointinst_set_stbox(inst2, &box);
    stbox_expand(&box, &result[i]);
    inst1 = inst2;
  }
  return seq->count - 1;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal sequence point.
 * @param[in] seq Temporal value
 * @param[out] count Number of elements in the output array
 */
STBox *
tpointseq_stboxes(const TSequence *seq, int *count)
{
  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));
  int newcount = seq->count == 1 ? 1 : seq->count - 1;
  STBox *result = palloc(sizeof(STBox) * newcount);
  tpointseq_stboxes_iter(seq, result);
  *count = newcount;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from the segments of a
 * temporal sequence set point.
 * @param[in] ss Temporal value
 * @param[out] count Number of elements in the output array
 */
STBox *
tpointseqset_stboxes(const TSequenceSet *ss, int *count)
{
  assert(MEOS_FLAGS_GET_LINEAR(ss->flags));
  STBox *result = palloc(sizeof(STBox) * ss->totalcount);
  int nboxes = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    nboxes += tpointseq_stboxes_iter(seq, &result[nboxes]);
  }
  *count = nboxes;
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return an array of spatiotemporal boxes from a temporal point
 * @sqlfunc stboxes()
 */
STBox *
tpoint_stboxes(const Temporal *temp, int *count)
{
  STBox *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || MEOS_FLAGS_GET_DISCRETE(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = tpointseq_stboxes((TSequence *)temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tpointseqset_stboxes((TSequenceSet *)temp, count);
  return result;
}

/*****************************************************************************/

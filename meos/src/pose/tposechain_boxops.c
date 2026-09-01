/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Bounding box operators for temporal pose chains
 */

/* C */
#include <assert.h>
/* Postgres */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <string.h>
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "pose/posechain.h"
#include "pose/tposechain_boxops.h"

/*****************************************************************************
 * Boxes constructed from a pose chain and a time
 *****************************************************************************/

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * pose chain and a timestamptz
 * @param[in] pc Pose chain
 * @param[in] t Timestamp
 * @param[out] box Spatiotemporal box
 */
bool
posechain_timestamptz_set_stbox(const PoseChain *pc, TimestampTz t, STBox *box)
{
  assert(pc); assert(box);
  posechain_set_stbox(pc, box);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @ingroup meos_posechain_base_bbox
 * @brief Return a spatiotemporal box constructed from a pose chain and a
 * timestamptz
 * @param[in] pc Pose chain
 * @param[in] t Timestamp
 * @csqlfn #Posechain_timestamptz_to_stbox()
 */
STBox *
posechain_timestamptz_to_stbox(const PoseChain *pc, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  STBox box;
  if (! posechain_timestamptz_set_stbox(pc, t, &box))
    return NULL;
  return stbox_copy(&box);
}

/**
 * @ingroup meos_internal_box_constructor
 * @brief Return in the last argument a spatiotemporal box constructed from a
 * pose chain and a timestamptz span
 * @param[in] pc Pose chain
 * @param[in] s Timestamptz span
 * @param[out] box Spatiotemporal box
 */
bool
posechain_tstzspan_set_stbox(const PoseChain *pc, const Span *s, STBox *box)
{
  assert(pc); assert(s); assert(box);
  posechain_set_stbox(pc, box);
  memcpy(&box->period, s, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @ingroup meos_posechain_base_bbox
 * @brief Return a spatiotemporal box constructed from a pose chain and a
 * timestamptz span
 * @param[in] pc Pose chain
 * @param[in] s Timestamptz span
 * @csqlfn #Posechain_tstzspan_to_stbox()
 */
STBox *
posechain_tstzspan_to_stbox(const PoseChain *pc, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_TSTZSPAN(s, NULL);
  STBox box;
  if (! posechain_tstzspan_set_stbox(pc, s, &box))
    return NULL;
  return stbox_copy(&box);
}

/*****************************************************************************/

/*****************************************************************************
 * Transform a temporal pose chain to a spatiotemporal box
 *
 * The box of a chain spans the composed position of every one of its
 * prefixes, which is where each of its joints sits, and not the position its
 * links store. #posechain_set_stbox is what answers that.
 *****************************************************************************/

/**
 * @brief Return in the last argument the spatiotemporal box of a temporal
 * pose chain instant
 * @param[in] inst Temporal instant
 * @param[out] box Spatiotemporal box
 */
void
tposechaininst_set_stbox(const TInstant *inst, STBox *box)
{
  assert(inst); assert(box);
  posechain_set_stbox(DatumGetPoseChainP(tinstant_value_p(inst)), box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Return in the last argument the spatiotemporal box of an array of
 * temporal pose chain instants
 * @param[in] instants Temporal instants
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tposechaininstarr_set_stbox(TInstant **instants, int count, STBox *box)
{
  assert(instants); assert(box); assert(count > 0);
  /* Initialize the bounding box with the first instant */
  tposechaininst_set_stbox(instants[0], box);
  for (int i = 1; i < count; i++)
  {
    STBox box1;
    tposechaininst_set_stbox(instants[i], &box1);
    stbox_expand(&box1, box);
  }
  return;
}

/**
 * @brief Expand the bounding box of a temporal pose chain sequence with an
 * additional instant
 * @param[in,out] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tposechainseq_expand_stbox(TSequence *seq, const TInstant *inst)
{
  assert(seq); assert(inst);
  STBox box;
  tposechaininst_set_stbox(inst, &box);
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/*****************************************************************************/

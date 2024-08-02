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
 * @brief General functions for temporal pose objects.
 */

#include "pose/tpose.h"

/* C */
#include <assert.h>
/* Postgres */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "pose/tpose_static.h"

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

Temporal *
tpose_to_tgeompoint(const Temporal *temp)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_geom;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_spatial_accessor
 * @brief Return the SRID of a temporal instant point.
 * @sqlfn SRID()
 */
static int
tposeinst_srid(const TInstant *inst)
{
  Pose *gs = DatumGetPoseP(tinstant_value(inst));
  return pose_get_srid(gs);
}

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point.
 * @sqlfn SRID()
 */
int
tpose_srid(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || temp->temptype != T_TPOSE)
    return SRID_INVALID;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tposeinst_srid((TInstant *) temp);
    case TSEQUENCE:
      return tpointseq_srid((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tpointseqset_srid((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Set the SRID of a temporal instant point
 * @sqlfn setSRID()
 */
static TInstant *
tposeinst_set_srid(const TInstant *inst, int32 srid)
{
  TInstant *result = tinstant_copy(inst);
  Pose *pose = DatumGetPoseP(tinstant_value(result));
  pose_set_srid(pose, srid);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Set the SRID of a temporal sequence point
 * @sqlfn setSRID()
 */
static TSequence *
tposeseq_set_srid(const TSequence *seq, int32 srid)
{
  TSequence *result = tsequence_copy(seq);
  /* Set the SRID of the composing points */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(result, i);
    Pose *pose = DatumGetPoseP(tinstant_value(inst));
    pose_set_srid(pose, srid);
  }
  /* Set the SRID of the bounding box */
  STBox *box = TSEQUENCE_BBOX_PTR(result);
  box->srid = srid;
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_spatial_transf
 * @brief Set the SRID of a temporal sequence set point
 * @sqlfn setSRID()
 */
static TSequenceSet *
tposeseqset_set_srid(const TSequenceSet *ss, int32 srid)
{
  STBox *box;
  TSequenceSet *result = tsequenceset_copy(ss);
  /* Loop for every composing sequence */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(result, i);
    for (int j = 0; j < seq->count; j++)
    {
      /* Set the SRID of the composing points */
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      Pose *pose = DatumGetPoseP(tinstant_value(inst));
      pose_set_srid(pose, srid);
    }
    /* Set the SRID of the bounding box */
    box = TSEQUENCE_BBOX_PTR(seq);
    box->srid = srid;
  }
  /* Set the SRID of the bounding box */
  box = TSEQUENCESET_BBOX_PTR(result);
  box->srid = srid;
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Set the SRID of a temporal pose.
 * @see tposeinst_set_srid
 * @see tposeseq_set_srid
 * @see tposeseqset_set_srid
 * @sqlfn setSRID()
 */
Temporal *
tpose_set_srid(const Temporal *temp, int32 srid)
{
  Temporal *result;
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tposeinst_set_srid((TInstant *) temp, srid);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tposeseq_set_srid((TSequence *) temp, srid);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tposeseqset_set_srid((TSequenceSet *) temp, srid);

  assert(result != NULL);
  return result;
}

/*****************************************************************************/

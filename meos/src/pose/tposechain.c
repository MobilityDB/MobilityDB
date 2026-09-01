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
 * @brief General functions for temporal pose chains
 */

/* C */
#include <assert.h>
#include <limits.h>
/* Postgres */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "pose/posechain.h"
#include "pose/tposechain.h"

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure the validity of a temporal pose chain and a geometry
 */
bool
ensure_valid_tposechain_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TPOSECHAIN(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal pose chain and a pose chain
 */
bool
ensure_valid_tposechain_posechain(const Temporal *temp, const PoseChain *pc)
{
  VALIDATE_TPOSECHAIN(temp, false); VALIDATE_NOT_NULL(pc, false);
  if (! ensure_same_srid(tspatial_srid(temp), posechain_srid(pc)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal pose chain and a spatiotemporal box
 */
bool
ensure_valid_tposechain_stbox(const Temporal *temp, const STBox *box)
{
  VALIDATE_TPOSECHAIN(temp, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_same_srid(tspatial_srid(temp), box->srid))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two temporal pose chains
 */
bool
ensure_valid_tposechain_tposechain(const Temporal *temp1,
  const Temporal *temp2)
{
  VALIDATE_TPOSECHAIN(temp1, false); VALIDATE_TPOSECHAIN(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return false;
  return true;
}

/**
 * @brief Ensure that two temporal pose chain instants hold the same number of
 * links
 * @details The link count of a temporal pose chain is the same at every
 * instant: a chain that gains a joint is a different structure rather than a
 * later value of the same one. The check holds whatever the interpolation,
 * since it is the identity of the value that requires it and not the blend
 * between two of them.
 * @param[in] inst1,inst2 Temporal instants
 */
bool
ensure_same_count_tposechaininst(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2);
  return ensure_same_count_posechain(
    DatumGetPoseChainP(tinstant_value_p(inst1)),
    DatumGetPoseChainP(tinstant_value_p(inst2)));
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * @brief Return 1 or 0 if a temporal pose chain segment intersects a pose
 * chain during the period defined by the output timestamps
 * @param[in] start,end Base values defining the segment
 * @param[in] value Base value
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 */
int
tposechainsegm_intersection_value(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  /* Locate the value in the segment: every link locates at the ratio the
   * chain moves through, so a chain that differs from the segment in any one
   * of its links is not on it */
  long double fraction = posechainsegm_locate(DatumGetPoseChainP(start),
    DatumGetPoseChainP(end), DatumGetPoseChainP(value));
  if (fraction < 0.0)
    return 0;
  double duration = (double) (upper - lower);
  *t1 = lower + (TimestampTz) (duration * (double) fraction);
  if (t2)
    *t2 = *t1;
  return 1;
}

/**
 * @brief Return 1 if two temporal pose chain segments intersect during the
 * period defined by the output timestamps, return 0 otherwise
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @note The candidate instant is derived from the composed position of the
 * two chains; the lifting infrastructure verifies equality of the whole
 * value at the returned timestamp
 */
int
tposechainsegm_intersection(Datum start1, Datum end1, Datum start2,
  Datum end2, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  GSERIALIZED *gs1 = posechain_to_point(DatumGetPoseChainP(start1));
  GSERIALIZED *gs2 = posechain_to_point(DatumGetPoseChainP(end1));
  GSERIALIZED *gs3 = posechain_to_point(DatumGetPoseChainP(start2));
  GSERIALIZED *gs4 = posechain_to_point(DatumGetPoseChainP(end2));
  int result = tgeompointsegm_intersection(PointerGetDatum(gs1),
    PointerGetDatum(gs2), PointerGetDatum(gs3), PointerGetDatum(gs4),
    lower, upper, t1, t2);
  pfree(gs1); pfree(gs2); pfree(gs3); pfree(gs4);
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_inout
 * @brief Return a temporal pose chain from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Tposechain_in()
 */
Temporal *
tposechain_in(const char *str)
{
  VALIDATE_NOT_NULL(str, NULL);
  return tspatial_parse(&str, T_TPOSECHAIN);
}

/**
 * @ingroup meos_internal_posechain_inout
 * @brief Return a temporal pose chain instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tposechaininst_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tposechain_in(str);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_posechain_inout
 * @brief Return a temporal pose chain sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tposechainseq_in(const char *str, interpType interp UNUSED)
{
  /* Call the superclass function */
  Temporal *temp = tposechain_in(str);
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_posechain_inout
 * @brief Return a temporal pose chain sequence set from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tposechainseqset_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tposechain_in(str);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_posechain_inout
 * @brief Return a temporal pose chain from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tposechain_from_mfjson(const char *mfjson)
{
  VALIDATE_NOT_NULL(mfjson, NULL);
  return temporal_from_mfjson(mfjson, T_TPOSECHAIN);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_constructor
 * @brief Return a temporal pose chain instant from a pose chain and a
 * timestamptz
 * @param[in] pc Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tposechaininst_make(const PoseChain *pc, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL);
  return tinstant_make(PointerGetDatum(pc), T_TPOSECHAIN, t);
}

/**
 * @ingroup meos_posechain_constructor
 * @brief Return a temporal pose chain from a pose chain and the time frame of
 * another temporal value
 * @param[in] pc Value
 * @param[in] temp Temporal value
 */
Temporal *
tposechain_from_base_temp(const PoseChain *pc, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_NOT_NULL(temp, NULL);
  return temporal_from_base_temp(PointerGetDatum(pc), T_TPOSECHAIN, temp);
}

/**
 * @ingroup meos_posechain_constructor
 * @brief Return a temporal pose chain discrete sequence from a pose chain and
 * a timestamptz set
 * @param[in] pc Value
 * @param[in] s Set
 */
TSequence *
tposechainseq_from_base_tstzset(const PoseChain *pc, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_TSTZSET(s, NULL);
  return tsequence_from_base_tstzset(PointerGetDatum(pc), T_TPOSECHAIN, s);
}

/**
 * @ingroup meos_posechain_constructor
 * @brief Return a temporal pose chain sequence from a pose chain and a
 * timestamptz span
 * @param[in] pc Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tposechainseq_from_base_tstzspan(const PoseChain *pc, const Span *s,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_TSTZSPAN(s, NULL);
  return tsequence_from_base_tstzspan(PointerGetDatum(pc), T_TPOSECHAIN, s,
    interp);
}

/**
 * @ingroup meos_posechain_constructor
 * @brief Return a temporal pose chain sequence set from a pose chain and a
 * timestamptz span set
 * @param[in] pc Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tposechainseqset_from_base_tstzspanset(const PoseChain *pc, const SpanSet *ss,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pc, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(pc),
    T_TPOSECHAIN, ss, interp);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_conversion
 * @brief Return a temporal pose from a temporal pose chain
 * @details Every instant holds the pose the whole chain composes to, which is
 * where its innermost frame sits in the frame of the chain.
 * @param[in] temp Temporal pose chain
 * @note The result is a step function. Composing a chain multiplies the
 * rotations of its links, so the pose of the chain interpolated between two
 * instants is not the pose interpolated between the two composed poses, and a
 * linear result would answer a pose the chain never holds. A chain of one
 * link composes to itself, so there the interpolation of the input carries
 * over unchanged.
 * @csqlfn #Tposechain_to_tpose()
 */
Temporal *
tposechain_to_tpose(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSECHAIN(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_posechain_pose;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TPOSE;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags) &&
    tposechain_num_poses(temp) == 1;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_posechain_accessor
 * @brief Return the number of links every value of a temporal pose chain
 * holds
 * @details The link count is the same at every instant, so one instant
 * answers for the whole value.
 * @param[in] temp Temporal pose chain
 * @return On error return @p INT_MAX
 * @csqlfn #Tposechain_num_poses()
 */
int
tposechain_num_poses(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSECHAIN(temp, INT_MAX);
  const TInstant *inst = temporal_start_inst(temp);
  return posechain_num_poses(DatumGetPoseChainP(tinstant_value_p(inst)));
}

/*****************************************************************************/

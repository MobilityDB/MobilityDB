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
 * @brief Temporal spatial relationships for temporal rigid geometries
 * @details These relationships are applied at each instant and result in a
 * temporal Boolean. The spatial relationship is evaluated by applying the
 * pose at each instant to the reference polygon, then testing the resulting
 * body against the other argument.
 *
 * The following relationships are supported: `tContains`, `tCovers`,
 * `tDisjoint`, `tIntersects`, `tTouches`, and `tDwithin`.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_rgeo.h>
#include "temporal/lifting.h"
#include "temporal/tbool_ops.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialrels.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_tempspatialrels.h"
#include "pose/pose.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_inst.h"
#include "rgeo/trgeo_tempspatialrels.h"

/*****************************************************************************
 * Internal helpers — single-instant evaluation
 *****************************************************************************/

/**
 * @brief Enforce the STEP-sequence invariant: when upper_inc is false, the
 * last instant must carry the same value as the second-to-last instant.
 * The tbool instants are allocated; if we have to duplicate, we free the
 * last element and replace it.
 */
static void
tbool_instants_fix_upper(TInstant **instants, int count, bool upper_inc)
{
  if (upper_inc || count < 2)
    return;
  bool prev_val = DatumGetBool(tinstant_value_p(instants[count - 2]));
  bool last_val = DatumGetBool(tinstant_value_p(instants[count - 1]));
  if (prev_val != last_val)
  {
    TimestampTz t = instants[count - 1]->t;
    pfree(instants[count - 1]);
    instants[count - 1] = tinstant_make(BoolGetDatum(prev_val), T_TBOOL, t);
  }
}

/**
 * @brief Evaluate a 2-argument spatial relationship at a single trgeo instant
 * vs a fixed geometry
 * @param[in] inst Temporal instant (trgeo)
 * @param[in] ref Reference polygon of the temporal rigid geometry
 * @param[in] gs Fixed geometry
 * @param[in] func Spatial relationship function (Datum, Datum) -> Datum bool
 * @param[in] invert When true, call func(gs, body); otherwise func(body, gs)
 */
static TInstant *
tspatialrel_trgeoinst_geo(const TInstant *inst, const GSERIALIZED *ref,
  const GSERIALIZED *gs, datum_func2 func, bool invert)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
  GSERIALIZED *body = geom_apply_pose(ref, pose);
  Datum bd = PointerGetDatum(body);
  Datum gd = PointerGetDatum(gs);
  bool val = DatumGetBool(invert ? func(gd, bd) : func(bd, gd));
  pfree(body);
  return tinstant_make(BoolGetDatum(val), T_TBOOL, inst->t);
}

/**
 * @brief Evaluate the dwithin relationship at a single trgeo instant vs a
 * fixed geometry
 * @param[in] inst Temporal instant (trgeo)
 * @param[in] ref Reference polygon of the temporal rigid geometry
 * @param[in] gs Fixed geometry
 * @param[in] dist Distance threshold
 */
static TInstant *
tdwithin_trgeoinst_geo(const TInstant *inst, const GSERIALIZED *ref,
  const GSERIALIZED *gs, double dist)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
  GSERIALIZED *body = geom_apply_pose(ref, pose);
  bool val = DatumGetBool(datum_geom_dwithin2d(PointerGetDatum(body),
    PointerGetDatum(gs), Float8GetDatum(dist)));
  pfree(body);
  return tinstant_make(BoolGetDatum(val), T_TBOOL, inst->t);
}

/**
 * @brief Evaluate a 2-argument spatial relationship at a single pair of
 * synchronized trgeo instants
 * @param[in] inst1 First temporal instant (trgeo)
 * @param[in] ref1 Reference polygon of the first temporal rigid geometry
 * @param[in] inst2 Second temporal instant (trgeo)
 * @param[in] ref2 Reference polygon of the second temporal rigid geometry
 * @param[in] func Spatial relationship function
 */
static TInstant *
tspatialrel_trgeoinst_trgeoinst(const TInstant *inst1,
  const GSERIALIZED *ref1, const TInstant *inst2, const GSERIALIZED *ref2,
  datum_func2 func)
{
  const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
  const Pose *pose2 = DatumGetPoseP(tinstant_value_p(inst2));
  GSERIALIZED *body1 = geom_apply_pose(ref1, pose1);
  GSERIALIZED *body2 = geom_apply_pose(ref2, pose2);
  bool val = DatumGetBool(func(PointerGetDatum(body1), PointerGetDatum(body2)));
  pfree(body1);
  pfree(body2);
  return tinstant_make(BoolGetDatum(val), T_TBOOL, inst1->t);
}

/**
 * @brief Evaluate the dwithin relationship at a single pair of synchronized
 * trgeo instants
 * @param[in] inst1 First temporal instant (trgeo)
 * @param[in] ref1 Reference polygon of the first temporal rigid geometry
 * @param[in] inst2 Second temporal instant (trgeo)
 * @param[in] ref2 Reference polygon of the second temporal rigid geometry
 * @param[in] dist Distance threshold
 */
static TInstant *
tdwithin_trgeoinst_trgeoinst(const TInstant *inst1, const GSERIALIZED *ref1,
  const TInstant *inst2, const GSERIALIZED *ref2, double dist)
{
  const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
  const Pose *pose2 = DatumGetPoseP(tinstant_value_p(inst2));
  GSERIALIZED *body1 = geom_apply_pose(ref1, pose1);
  GSERIALIZED *body2 = geom_apply_pose(ref2, pose2);
  bool val = DatumGetBool(datum_geom_dwithin2d(PointerGetDatum(body1),
    PointerGetDatum(body2), Float8GetDatum(dist)));
  pfree(body1);
  pfree(body2);
  return tinstant_make(BoolGetDatum(val), T_TBOOL, inst1->t);
}

/*****************************************************************************
 * Internal dispatch — trgeo vs geometry
 *****************************************************************************/

/**
 * @brief Dispatch a 2-argument spatial relationship over all instants of a
 * trgeo value against a fixed geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Fixed geometry
 * @param[in] func Spatial relationship function
 * @param[in] invert When true, call func(gs, body)
 */
static Temporal *
tspatialrel_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  datum_func2 func, bool invert)
{
  const GSERIALIZED *ref = trgeo_geom_p(temp);
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tspatialrel_trgeoinst_geo(
        (const TInstant *) temp, ref, gs, func, invert);
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
      for (int i = 0; i < seq->count; i++)
        instants[i] = tspatialrel_trgeoinst_geo(TSEQUENCE_INST_N(seq, i),
          ref, gs, func, invert);
      tbool_instants_fix_upper(instants, seq->count, seq->period.upper_inc);
      return (Temporal *) tsequence_make_free(instants, seq->count,
        seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
      for (int i = 0; i < ss->count; i++)
      {
        const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
        TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
        for (int j = 0; j < seq->count; j++)
          instants[j] = tspatialrel_trgeoinst_geo(TSEQUENCE_INST_N(seq, j),
            ref, gs, func, invert);
        tbool_instants_fix_upper(instants, seq->count, seq->period.upper_inc);
        sequences[i] = tsequence_make_free(instants, seq->count,
          seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE_NO);
      }
      return (Temporal *) tsequenceset_make_free(sequences, ss->count,
        NORMALIZE);
    }
  }
}

/**
 * @brief Dispatch the dwithin relationship over all instants of a trgeo
 * value against a fixed geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Fixed geometry
 * @param[in] dist Distance threshold
 */
static Temporal *
tdwithin_trgeo_geo_dispatch(const Temporal *temp, const GSERIALIZED *gs,
  double dist)
{
  const GSERIALIZED *ref = trgeo_geom_p(temp);
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tdwithin_trgeoinst_geo(
        (const TInstant *) temp, ref, gs, dist);
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
      for (int i = 0; i < seq->count; i++)
        instants[i] = tdwithin_trgeoinst_geo(TSEQUENCE_INST_N(seq, i),
          ref, gs, dist);
      tbool_instants_fix_upper(instants, seq->count, seq->period.upper_inc);
      return (Temporal *) tsequence_make_free(instants, seq->count,
        seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
      for (int i = 0; i < ss->count; i++)
      {
        const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
        TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
        for (int j = 0; j < seq->count; j++)
          instants[j] = tdwithin_trgeoinst_geo(TSEQUENCE_INST_N(seq, j),
            ref, gs, dist);
        tbool_instants_fix_upper(instants, seq->count, seq->period.upper_inc);
        sequences[i] = tsequence_make_free(instants, seq->count,
          seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE_NO);
      }
      return (Temporal *) tsequenceset_make_free(sequences, ss->count,
        NORMALIZE);
    }
  }
}

/*****************************************************************************
 * Internal dispatch — synchronized trgeo vs trgeo
 *****************************************************************************/

/**
 * @brief Dispatch a 2-argument spatial relationship over synchronized pairs
 * of trgeo instants
 * @param[in] sync1 First synchronized temporal rigid geometry
 * @param[in] sync2 Second synchronized temporal rigid geometry
 * @param[in] ref1 Reference polygon of the first trgeo
 * @param[in] ref2 Reference polygon of the second trgeo
 * @param[in] func Spatial relationship function
 */
static Temporal *
tspatialrel_trgeo_trgeo_sync(const Temporal *sync1, const Temporal *sync2,
  const GSERIALIZED *ref1, const GSERIALIZED *ref2, datum_func2 func)
{
  assert(sync1->subtype == sync2->subtype);
  switch (sync1->subtype)
  {
    case TINSTANT:
      return (Temporal *) tspatialrel_trgeoinst_trgeoinst(
        (const TInstant *) sync1, ref1, (const TInstant *) sync2, ref2, func);
    case TSEQUENCE:
    {
      const TSequence *seq1 = (const TSequence *) sync1;
      const TSequence *seq2 = (const TSequence *) sync2;
      TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
      for (int i = 0; i < seq1->count; i++)
        instants[i] = tspatialrel_trgeoinst_trgeoinst(
          TSEQUENCE_INST_N(seq1, i), ref1,
          TSEQUENCE_INST_N(seq2, i), ref2, func);
      tbool_instants_fix_upper(instants, seq1->count, seq1->period.upper_inc);
      return (Temporal *) tsequence_make_free(instants, seq1->count,
        seq1->period.lower_inc, seq1->period.upper_inc, STEP, NORMALIZE);
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss1 = (const TSequenceSet *) sync1;
      const TSequenceSet *ss2 = (const TSequenceSet *) sync2;
      TSequence **sequences = palloc(sizeof(TSequence *) * ss1->count);
      for (int i = 0; i < ss1->count; i++)
      {
        const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
        const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, i);
        TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
        for (int j = 0; j < seq1->count; j++)
          instants[j] = tspatialrel_trgeoinst_trgeoinst(
            TSEQUENCE_INST_N(seq1, j), ref1,
            TSEQUENCE_INST_N(seq2, j), ref2, func);
        tbool_instants_fix_upper(instants, seq1->count, seq1->period.upper_inc);
        sequences[i] = tsequence_make_free(instants, seq1->count,
          seq1->period.lower_inc, seq1->period.upper_inc, STEP, NORMALIZE_NO);
      }
      return (Temporal *) tsequenceset_make_free(sequences, ss1->count,
        NORMALIZE);
    }
  }
}

/**
 * @brief Dispatch the dwithin relationship over synchronized pairs of trgeo
 * instants
 * @param[in] sync1 First synchronized temporal rigid geometry
 * @param[in] sync2 Second synchronized temporal rigid geometry
 * @param[in] ref1 Reference polygon of the first trgeo
 * @param[in] ref2 Reference polygon of the second trgeo
 * @param[in] dist Distance threshold
 */
static Temporal *
tdwithin_trgeo_trgeo_sync(const Temporal *sync1, const Temporal *sync2,
  const GSERIALIZED *ref1, const GSERIALIZED *ref2, double dist)
{
  assert(sync1->subtype == sync2->subtype);
  switch (sync1->subtype)
  {
    case TINSTANT:
      return (Temporal *) tdwithin_trgeoinst_trgeoinst(
        (const TInstant *) sync1, ref1, (const TInstant *) sync2, ref2, dist);
    case TSEQUENCE:
    {
      const TSequence *seq1 = (const TSequence *) sync1;
      const TSequence *seq2 = (const TSequence *) sync2;
      TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
      for (int i = 0; i < seq1->count; i++)
        instants[i] = tdwithin_trgeoinst_trgeoinst(
          TSEQUENCE_INST_N(seq1, i), ref1,
          TSEQUENCE_INST_N(seq2, i), ref2, dist);
      tbool_instants_fix_upper(instants, seq1->count, seq1->period.upper_inc);
      return (Temporal *) tsequence_make_free(instants, seq1->count,
        seq1->period.lower_inc, seq1->period.upper_inc, STEP, NORMALIZE);
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss1 = (const TSequenceSet *) sync1;
      const TSequenceSet *ss2 = (const TSequenceSet *) sync2;
      TSequence **sequences = palloc(sizeof(TSequence *) * ss1->count);
      for (int i = 0; i < ss1->count; i++)
      {
        const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
        const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, i);
        TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
        for (int j = 0; j < seq1->count; j++)
          instants[j] = tdwithin_trgeoinst_trgeoinst(
            TSEQUENCE_INST_N(seq1, j), ref1,
            TSEQUENCE_INST_N(seq2, j), ref2, dist);
        tbool_instants_fix_upper(instants, seq1->count, seq1->period.upper_inc);
        sequences[i] = tsequence_make_free(instants, seq1->count,
          seq1->period.lower_inc, seq1->period.upper_inc, STEP, NORMALIZE_NO);
      }
      return (Temporal *) tsequenceset_make_free(sequences, ss1->count,
        NORMALIZE);
    }
  }
}

/**
 * @brief Apply a 2-argument spatial relationship to two trgeo values,
 * synchronizing first
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @param[in] func Spatial relationship function
 */
static Temporal *
tspatialrel_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func)
{
  Temporal *sync1, *sync2;
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;
  const GSERIALIZED *ref1 = trgeo_geom_p(temp1);
  const GSERIALIZED *ref2 = trgeo_geom_p(temp2);
  Temporal *result = tspatialrel_trgeo_trgeo_sync(sync1, sync2, ref1, ref2,
    func);
  pfree(sync1);
  pfree(sync2);
  return result;
}

/**
 * @brief Apply the dwithin relationship to two trgeo values, synchronizing
 * first
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @param[in] dist Distance threshold
 */
static Temporal *
tdwithin_trgeo_trgeo_dispatch(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  Temporal *sync1, *sync2;
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;
  const GSERIALIZED *ref1 = trgeo_geom_p(temp1);
  const GSERIALIZED *ref2 = trgeo_geom_p(temp2);
  Temporal *result = tdwithin_trgeo_trgeo_sync(sync1, sync2, ref1, ref2,
    dist);
  pfree(sync1);
  pfree(sync2);
  return result;
}

/*****************************************************************************
 * tContains
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry contains a
 * temporal rigid geometry at each instant
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Tcontains_geo_trgeo()
 */
Temporal *
tcontains_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs, (datum_func2) datum_geom_contains,
    INVERT);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry contains a geometry at each instant
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Tcontains_trgeo_geo()
 */
Temporal *
tcontains_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs, (datum_func2) datum_geom_contains,
    INVERT_NO);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether the first temporal
 * rigid geometry contains the second at each synchronized instant
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @csqlfn #Tcontains_trgeo_trgeo()
 */
Temporal *
tcontains_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_TRGEOMETRY(temp2, NULL);
  if (! ensure_valid_trgeo_trgeo(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  return tspatialrel_trgeo_trgeo(temp1, temp2,
    (datum_func2) datum_geom_contains);
}

/*****************************************************************************
 * tCovers
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry covers a
 * temporal rigid geometry at each instant
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Tcovers_geo_trgeo()
 */
Temporal *
tcovers_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs, (datum_func2) datum_geom_covers,
    INVERT);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry covers a geometry at each instant
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Tcovers_trgeo_geo()
 */
Temporal *
tcovers_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs, (datum_func2) datum_geom_covers,
    INVERT_NO);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether the first temporal
 * rigid geometry covers the second at each synchronized instant
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @csqlfn #Tcovers_trgeo_trgeo()
 */
Temporal *
tcovers_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_TRGEOMETRY(temp2, NULL);
  if (! ensure_valid_trgeo_trgeo(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  return tspatialrel_trgeo_trgeo(temp1, temp2,
    (datum_func2) datum_geom_covers);
}

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry intersects a
 * temporal rigid geometry at each instant
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Tintersects_geo_trgeo()
 */
Temporal *
tintersects_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs,
    (datum_func2) datum_geom_intersects2d, INVERT_NO);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry intersects a geometry at each instant
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Tintersects_trgeo_geo()
 */
Temporal *
tintersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs,
    (datum_func2) datum_geom_intersects2d, INVERT_NO);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries intersect at each synchronized instant
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @csqlfn #Tintersects_trgeo_trgeo()
 */
Temporal *
tintersects_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_TRGEOMETRY(temp2, NULL);
  if (! ensure_valid_trgeo_trgeo(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  return tspatialrel_trgeo_trgeo(temp1, temp2,
    (datum_func2) datum_geom_intersects2d);
}

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry is disjoint
 * from a temporal rigid geometry at each instant
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Tdisjoint_geo_trgeo()
 */
Temporal *
tdisjoint_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  Temporal *inter = tintersects_geo_trgeo(gs, temp);
  if (! inter)
    return NULL;
  Temporal *result = tnot_tbool(inter);
  pfree(inter);
  return result;
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry is disjoint from a geometry at each instant
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Tdisjoint_trgeo_geo()
 */
Temporal *
tdisjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  Temporal *inter = tintersects_trgeo_geo(temp, gs);
  if (! inter)
    return NULL;
  Temporal *result = tnot_tbool(inter);
  pfree(inter);
  return result;
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries are disjoint at each synchronized instant
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @csqlfn #Tdisjoint_trgeo_trgeo()
 */
Temporal *
tdisjoint_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *inter = tintersects_trgeo_trgeo(temp1, temp2);
  if (! inter)
    return NULL;
  Temporal *result = tnot_tbool(inter);
  pfree(inter);
  return result;
}

/*****************************************************************************
 * tTouches
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry touches a
 * temporal rigid geometry at each instant
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Ttouches_geo_trgeo()
 */
Temporal *
ttouches_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs, (datum_func2) datum_geom_touches,
    INVERT);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry touches a geometry at each instant
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Ttouches_trgeo_geo()
 */
Temporal *
ttouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return tspatialrel_trgeo_geo(temp, gs, (datum_func2) datum_geom_touches,
    INVERT_NO);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries touch at each synchronized instant
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @csqlfn #Ttouches_trgeo_trgeo()
 */
Temporal *
ttouches_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_TRGEOMETRY(temp2, NULL);
  if (! ensure_valid_trgeo_trgeo(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  return tspatialrel_trgeo_trgeo(temp1, temp2,
    (datum_func2) datum_geom_touches);
}

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry and a
 * temporal rigid geometry are within a given distance at each instant
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] dist Distance threshold
 * @csqlfn #Tdwithin_geo_trgeo()
 */
Temporal *
tdwithin_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp, double dist)
{
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;
  return tdwithin_trgeo_geo_dispatch(temp, gs, dist);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry and a geometry are within a given distance at each instant
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance threshold
 * @csqlfn #Tdwithin_trgeo_geo()
 */
Temporal *
tdwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;
  return tdwithin_trgeo_geo_dispatch(temp, gs, dist);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries are within a given distance at each synchronized instant
 * @param[in] temp1 First temporal rigid geometry
 * @param[in] temp2 Second temporal rigid geometry
 * @param[in] dist Distance threshold
 * @csqlfn #Tdwithin_trgeo_trgeo()
 */
Temporal *
tdwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2, double dist)
{
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_TRGEOMETRY(temp2, NULL);
  if (! ensure_valid_trgeo_trgeo(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;
  return tdwithin_trgeo_trgeo_dispatch(temp1, temp2, dist);
}

/*****************************************************************************/

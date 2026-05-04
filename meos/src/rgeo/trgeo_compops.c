/***********************************************************************
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
 * @brief Ever/always and temporal comparisons for temporal rigid geometries
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_rgeo.h>
#include "temporal/postgres_types.h"
#include "temporal/lifting.h"
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "pose/pose.h"
#include "rgeo/trgeo.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal rigid geometry and a geometry satisfy the
 * ever/always comparison
 * @details The base type of trgeo is T_POSE, so the generic eacomp_temporal_base
 * would treat gs as a Pose datum, producing SRID errors.  Instead, materialise
 * the instantaneous geometry at each sample instant using geom_apply_pose and
 * compare against gs as T_GEOMETRY.
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;
  assert(func);
  const GSERIALIZED *ref_geom = trgeo_geom_p(temp);
  Datum gs_datum = PointerGetDatum(gs);
  int count;
  const TInstant **instants = temporal_insts_p(temp, &count);
  int result = ever ? 0 : 1;
  for (int i = 0; i < count; i++)
  {
    GSERIALIZED *inst_geom = geom_apply_pose(ref_geom,
      DatumGetPoseP(tinstant_value_p(instants[i])));
    bool cmp = DatumGetBool(func(PointerGetDatum(inst_geom), gs_datum,
      T_GEOMETRY));
    pfree(inst_geom);
    if (ever && cmp)  { result = 1; break; }
    if (!ever && !cmp) { result = 0; break; }
  }
  pfree((void *) instants);
  return result;
}

/**
 * @brief Return true if two temporal rigid geometries satisfy the ever/always
 * comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return -1;
  assert(func);
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a geometry is ever equal to a temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_geo_trgeo()
 */
inline int
ever_eq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever equal to a rigid
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Ever_eq_trgeo_geo()
 */
inline int
ever_eq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a geometry is ever different from a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_geo_trgeo()
 */
inline int
ever_ne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever different from a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Ever_ne_trgeo_geo()
 */
inline int
ever_ne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a geometry is always equal to a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_geo_trgeo()
 */
inline int
always_eq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always equal to a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Always_eq_trgeo_geo()
 */
inline int
always_eq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a geometry is always different from a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_geo_trgeo()
 */
inline int
always_ne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always different from a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Always_ne_trgeo_geo()
 */
inline int
always_ne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are ever equal
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Ever_eq_trgeo_trgeo()
 */
inline int
ever_eq_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are ever different
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Ever_ne_trgeo_trgeo()
 */
inline int
ever_ne_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are always equal
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Always_eq_trgeo_trgeo()
 */
inline int
always_eq_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are always different
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Always_ne_trgeo_trgeo()
 */
inline int
always_ne_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Apply a comparison to each sample instant of a trgeo TSequence
 * @details Applies geom_apply_pose at each instant, compares to gs, and
 * returns the result as a TSequence (DISCRETE) or a single-element TSequence
 * array for the continuous (STEP/LINEAR) case.
 * @param[in] seq TSequence of trgeo
 * @param[in] ref_geom Reference geometry of the trgeo
 * @param[in] gs Static geometry for comparison
 * @param[in] func Comparison function
 * @param[in] invert True when gs is the left argument
 */
static TSequence *
tcomp_trgeosequence_geo(const TSequence *seq, const GSERIALIZED *ref_geom,
  const GSERIALIZED *gs, Datum (*func)(Datum, Datum, MeosType), bool invert)
{
  Datum gs_datum = PointerGetDatum(gs);
  TInstant **new_insts = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    GSERIALIZED *g = geom_apply_pose(ref_geom,
      DatumGetPoseP(tinstant_value_p(inst)));
    Datum geom_d = PointerGetDatum(g);
    Datum cmp = invert ? func(gs_datum, geom_d, T_GEOMETRY)
                       : func(geom_d, gs_datum, T_GEOMETRY);
    pfree(g);
    new_insts[i] = tinstant_make(cmp, T_TBOOL, inst->t);
  }
  bool is_discrete = MEOS_FLAGS_DISCRETE_INTERP(seq->flags);
  interpType interp = is_discrete ? DISCRETE : STEP;
  return tsequence_make_free(new_insts, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
}

/**
 * @brief Return the temporal comparison of a temporal rigid geometry and a
 * geometry
 * @details The base type of trgeo is T_POSE, so the generic lifting
 * infrastructure would treat gs as a Pose datum, producing SRID errors.
 * Instead, materialise the instantaneous geometry at each sample instant using
 * geom_apply_pose and compare against gs as T_GEOMETRY.  For continuous
 * (STEP/LINEAR) sequences the result is wrapped in a TSequenceSet to match
 * the standard behaviour of temporal comparison operators.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 * @param[in] invert True when gs is the left argument of func
 */
static Temporal *
tcomp_trgeo_geo_int(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType), bool invert)
{
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;
  assert(func);
  const GSERIALIZED *ref_geom = trgeo_geom_p(temp);
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      const TInstant *inst = (const TInstant *) temp;
      GSERIALIZED *g = geom_apply_pose(ref_geom,
        DatumGetPoseP(tinstant_value_p(inst)));
      Datum geom_d = PointerGetDatum(g);
      Datum gs_d = PointerGetDatum(gs);
      Datum cmp = invert ? func(gs_d, geom_d, T_GEOMETRY)
                         : func(geom_d, gs_d, T_GEOMETRY);
      pfree(g);
      return (Temporal *) tinstant_make(cmp, T_TBOOL, inst->t);
    }
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      TSequence *boolseq = tcomp_trgeosequence_geo(seq, ref_geom, gs, func,
        invert);
      /* Continuous sequences: wrap in TSequenceSet to match tcomp semantics */
      if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
        return (Temporal *) boolseq;
      return (Temporal *) tsequenceset_make((TSequence **) &boolseq, 1,
        NORMALIZE);
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      TSequence **new_seqs = palloc(sizeof(TSequence *) * ss->count);
      for (int j = 0; j < ss->count; j++)
        new_seqs[j] = tcomp_trgeosequence_geo(TSEQUENCESET_SEQ_N(ss, j),
          ref_geom, gs, func, invert);
      return (Temporal *) tsequenceset_make_free(new_seqs, ss->count,
        NORMALIZE);
    }
  }
}

/**
 * @brief Return the temporal comparison of a geometry and a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  return tcomp_trgeo_geo_int(temp, gs, func, true);
}

/**
 * @brief Return the temporal comparison of a temporal rigid geometry and a
 * geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType))
{
  return tcomp_trgeo_geo_int(temp, gs, func, false);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_comp_temp
 * @brief Return the temporal equality of a geometry and a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Teq_geo_trgeo()
 */
inline Temporal *
teq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_geo_trgeo(gs, temp, &datum2_eq);
}

/**
 * @ingroup meos_rgeo_comp_temp
 * @brief Return the temporal inequality of a geometry and a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Tne_geo_trgeo()
 */
inline Temporal *
tne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_geo_trgeo(gs, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_comp_temp
 * @brief Return the temporal equality of a temporal rigid geometry and a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Teq_trgeo_geo()
 */
inline Temporal *
teq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_trgeo_geo(temp, gs, &datum2_eq);
}

/**
 * @ingroup meos_rgeo_comp_temp
 * @brief Return the temporal inequality of a temporal rigid geometry and a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Tne_trgeo_geo()
 */
inline Temporal *
tne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_trgeo_geo(temp, gs, &datum2_ne);
}

/*****************************************************************************/

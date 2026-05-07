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
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "rgeo/trgeo.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Evaluate a comparison function against a materialized instant
 * @returns true if func(materialized_geom, gs) holds
 */
static bool
eacomp_trgeo_geo_inst(const Temporal *temp, TimestampTz t,
  const GSERIALIZED *gs, Datum (*func)(Datum, Datum, MeosType))
{
  Datum mat;
  if (! trgeo_value_at_timestamptz(temp, t, false, &mat))
    return false;
  bool result = DatumGetBool(func(mat, PointerGetDatum(gs), T_GEOMETRY));
  pfree(DatumGetPointer(mat));
  return result;
}

/**
 * @brief Return true if a temporal rigid geometry and a geometry satisfy the
 * ever/always comparison
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 * @note The generic lifting infrastructure cannot be used here because the
 * base type of T_TRGEOMETRY is T_POSE, not T_GEOMETRY. Instead, we iterate
 * over all instants and compare the materialized geometry at each one.
 */
static int
eacomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;
  assert(func);

  if (temp->subtype == TINSTANT)
  {
    bool val = eacomp_trgeo_geo_inst(temp, ((TInstant *) temp)->t, gs, func);
    return (int) val;
  }
  if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    for (int i = 0; i < seq->count; i++)
    {
      bool val = eacomp_trgeo_geo_inst(temp,
        TSEQUENCE_INST_N(seq, i)->t, gs, func);
      if (ever && val)   return 1;
      if (! ever && ! val) return 0;
    }
    return ever ? 0 : 1;
  }
  /* TSEQUENCESET */
  const TSequenceSet *ss = (const TSequenceSet *) temp;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      bool val = eacomp_trgeo_geo_inst(temp,
        TSEQUENCE_INST_N(seq, j)->t, gs, func);
      if (ever && val)   return 1;
      if (! ever && ! val) return 0;
    }
  }
  return ever ? 0 : 1;
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
 * @brief Return the temporal comparison of a temporal rigid geometry and a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 * @note The generic lifting infrastructure cannot be used here because the
 * base type of T_TRGEOMETRY is T_POSE, not T_GEOMETRY. We iterate over all
 * instants, materializing each one, and build a STEP-interpolated TBool.
 */
static Temporal *
tcomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;
  assert(func);

  Datum gsdat = PointerGetDatum(gs);

  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    Datum mat;
    trgeo_value_at_timestamptz(temp, inst->t, false, &mat);
    bool val = DatumGetBool(func(mat, gsdat, T_GEOMETRY));
    pfree(DatumGetPointer(mat));
    return (Temporal *) tinstant_make(BoolGetDatum(val), T_TBOOL, inst->t);
  }

  if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      Datum mat;
      trgeo_value_at_timestamptz(temp, inst->t, false, &mat);
      bool val = DatumGetBool(func(mat, gsdat, T_GEOMETRY));
      pfree(DatumGetPointer(mat));
      instants[i] = tinstant_make(BoolGetDatum(val), T_TBOOL, inst->t);
    }
    if (interp == DISCRETE)
      return (Temporal *) tsequence_make_free(instants, seq->count,
        seq->period.lower_inc, seq->period.upper_inc, DISCRETE, NORMALIZE);
    if (interp == STEP)
      return (Temporal *) tsequence_make_free(instants, seq->count,
        seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
    /* LINEAR: wrap in TSequenceSet to match generic lifting infrastructure */
    TSequence *seq_bool = tsequence_make_free(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
    TSequence **seqs = palloc(sizeof(TSequence *));
    seqs[0] = seq_bool;
    return (Temporal *) tsequenceset_make_free(seqs, 1, NORMALIZE);
  }

  /* TSEQUENCESET: each inner linear sequence becomes a STEP TSequence */
  const TSequenceSet *ss = (const TSequenceSet *) temp;
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      Datum mat;
      trgeo_value_at_timestamptz(temp, inst->t, false, &mat);
      bool val = DatumGetBool(func(mat, gsdat, T_GEOMETRY));
      pfree(DatumGetPointer(mat));
      instants[j] = tinstant_make(BoolGetDatum(val), T_TBOOL, inst->t);
    }
    sequences[i] = tsequence_make_free(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  }
  return (Temporal *) tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return the temporal comparison of a geometry and a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* datum2_eq/datum2_ne are commutative, so geo #= trgeo = trgeo #= geo */
  return tcomp_trgeo_geo(temp, gs, func);
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

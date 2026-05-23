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
#include "rgeo/trgeo.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal rigid geometry and a geometry satisfy the
 * ever/always comparison
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 * @note The generic lifting infrastructure routes through the trgeo basetype
 * (T_POSE), causing type confusion when comparing against a geometry.
 * This function materializes the world-frame geometry at each instant and
 * compares using T_GEOMETRY semantics instead.
 */
static int
eacomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;
  assert(func);

  const GSERIALIZED *body = trgeo_geom_p(temp);
  Datum gs_datum = PointerGetDatum(gs);

  switch (temp->subtype)
  {
    case TINSTANT:
    {
      const TInstant *inst = (const TInstant *) temp;
      GSERIALIZED *world = geom_apply_pose(body,
        DatumGetPoseP(tinstant_value(inst)));
      bool result = DatumGetBool(func(PointerGetDatum(world), gs_datum,
        T_GEOMETRY));
      pfree(world);
      return (int) result;
    }
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      for (int i = 0; i < seq->count; i++)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, i);
        GSERIALIZED *world = geom_apply_pose(body,
          DatumGetPoseP(tinstant_value(inst)));
        bool cmp = DatumGetBool(func(PointerGetDatum(world), gs_datum,
          T_GEOMETRY));
        pfree(world);
        if (ever && cmp) return 1;
        if (! ever && ! cmp) return 0;
      }
      return ever ? 0 : 1;
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
      {
        const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
        for (int j = 0; j < seq->count; j++)
        {
          const TInstant *inst = TSEQUENCE_INST_N(seq, j);
          GSERIALIZED *world = geom_apply_pose(body,
            DatumGetPoseP(tinstant_value(inst)));
          bool cmp = DatumGetBool(func(PointerGetDatum(world), gs_datum,
            T_GEOMETRY));
          pfree(world);
          if (ever && cmp) return 1;
          if (! ever && ! cmp) return 0;
        }
      }
      return ever ? 0 : 1;
    }
  }
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
 * @csqlfn #Ever_eq_geo_trgeometry()
 */
inline int
ever_eq_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever equal to a rigid
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Ever_eq_trgeometry_geo()
 */
inline int
ever_eq_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a geometry is ever different from a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_geo_trgeometry()
 */
inline int
ever_ne_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is ever different from a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Ever_ne_trgeometry_geo()
 */
inline int
ever_ne_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a geometry is always equal to a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_geo_trgeometry()
 */
inline int
always_eq_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always equal to a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Always_eq_trgeometry_geo()
 */
inline int
always_eq_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a geometry is always different from a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_geo_trgeometry()
 */
inline int
always_ne_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if a temporal rigid geometry is always different from a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Always_ne_trgeometry_geo()
 */
inline int
always_ne_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return eacomp_trgeo_geo(temp, gs, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are ever equal
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Ever_eq_trgeometry_trgeometry()
 */
inline int
ever_eq_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are ever different
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Ever_ne_trgeometry_trgeometry()
 */
inline int
ever_ne_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are always equal
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Always_eq_trgeometry_trgeometry()
 */
inline int
always_eq_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_rgeo_comp_ever
 * @brief Return true if two temporal rigid geometries are always different
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Always_ne_trgeometry_trgeometry()
 */
inline int
always_ne_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_trgeo_trgeo(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Build a TBool TSequence from one sequence of a trgeo vs geometry
 * comparison
 */
static TSequence *
tcomp_trgeo_geo_seq_inner(const TSequence *seq, const GSERIALIZED *body,
  Datum gs_datum, Datum (*func)(Datum, Datum, MeosType))
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    GSERIALIZED *world = geom_apply_pose(body,
      DatumGetPoseP(tinstant_value(inst)));
    Datum cmp = func(PointerGetDatum(world), gs_datum, T_GEOMETRY);
    pfree(world);
    instants[i] = tinstant_make(cmp, T_TBOOL, inst->t);
  }
  /* Preserve DISCRETE interpolation; use STEP for STEP/LINEAR. */
  interpType out_interp = MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ? DISCRETE : STEP;
  return tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, out_interp, NORMALIZE);
}

/**
 * @brief Return the temporal comparison of a temporal rigid geometry and a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] func Comparison function
 * @note The generic lifting infrastructure routes through the trgeo basetype
 * (T_POSE), causing type confusion when comparing against a geometry.
 * This function materializes the world-frame geometry at each instant and
 * compares using T_GEOMETRY semantics instead.
 */
static Temporal *
tcomp_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;
  assert(func);

  const GSERIALIZED *body = trgeo_geom_p(temp);
  Datum gs_datum = PointerGetDatum(gs);

  switch (temp->subtype)
  {
    case TINSTANT:
    {
      const TInstant *inst = (const TInstant *) temp;
      GSERIALIZED *world = geom_apply_pose(body,
        DatumGetPoseP(tinstant_value(inst)));
      Datum cmp = func(PointerGetDatum(world), gs_datum, T_GEOMETRY);
      pfree(world);
      return (Temporal *) tinstant_make(cmp, T_TBOOL, inst->t);
    }
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      TSequence *result = tcomp_trgeo_geo_seq_inner(seq, body, gs_datum, func);
      if (MEOS_FLAGS_LINEAR_INTERP(seq->flags))
      {
        TSequence **seqs = palloc(sizeof(TSequence *));
        seqs[0] = result;
        return (Temporal *) tsequenceset_make_free(seqs, 1, NORMALIZE);
      }
      return (Temporal *) result;
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      TSequence **seqs = palloc(sizeof(TSequence *) * ss->count);
      for (int i = 0; i < ss->count; i++)
        seqs[i] = tcomp_trgeo_geo_seq_inner(TSEQUENCESET_SEQ_N(ss, i), body,
          gs_datum, func);
      return (Temporal *) tsequenceset_make_free(seqs, ss->count, NORMALIZE);
    }
  }
}

/**
 * @brief Return the temporal comparison of a geometry and a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @param[in] func Comparison function
 * @note eq and ne are commutative, so this simply delegates to tcomp_trgeo_geo.
 */
static Temporal *
tcomp_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  return tcomp_trgeo_geo(temp, gs, func);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_comp_temp
 * @brief Return the temporal equality of a geometry and a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Teq_geo_trgeometry()
 */
inline Temporal *
teq_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_geo_trgeo(gs, temp, &datum2_eq);
}

/**
 * @ingroup meos_rgeo_comp_temp
 * @brief Return the temporal inequality of a geometry and a temporal rigid
 * geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 * @csqlfn #Tne_geo_trgeometry()
 */
inline Temporal *
tne_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp)
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
 * @csqlfn #Teq_trgeometry_geo()
 */
inline Temporal *
teq_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_trgeo_geo(temp, gs, &datum2_eq);
}

/**
 * @ingroup meos_rgeo_comp_temp
 * @brief Return the temporal inequality of a temporal rigid geometry and a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Tne_trgeometry_geo()
 */
inline Temporal *
tne_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_trgeo_geo(temp, gs, &datum2_ne);
}

/*****************************************************************************/

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
 * @brief General functions for temporal rigid geometries
 */

#include "rgeo/trgeo.h"

/* C */
#include <assert.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/meos_catalog.h"
#include "general/temporal.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"
#include "rgeo/trgeo_out.h"
#include "rgeo/trgeo_utils.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a trgeometry has a reference geometry
 */
bool
ensure_has_geom(int16 flags)
{
  if (MEOS_FLAGS_GET_GEOM(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Cannot access geometry from temporal rigid geometry");
  return false;
}

/*****************************************************************************/

/**
 * @brief Returns the reference geometry of a temporal rigid geometry
 */
Datum
trgeo_geom_p(const Temporal *temp)
{
  Datum result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = trgeoinst_geom_p((const TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = trgeoseq_geom_p((const TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = trgeoseqset_geom_p((const TSequenceSet *) temp);
  return result;
}

/**
 * @brief Returns a copy of the reference geometry of a temporal rigid geometry
 */
inline Datum
trgeo_geom(const Temporal *temp)
{
  return datum_copy(trgeo_geom_p(temp), T_GEOMETRY);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a temporal rigid geometry
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 * @param[in] extended True when the leading SRID string is output
 */
char *
trgeo_out(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  char *geom = geo_out(DatumGetGserializedP(trgeo_geom_p(temp)));
  char *pose = temporal_out(temp, OUT_DEFAULT_DECIMAL_DIGITS);
  /* Write the representations with the ';' delimiter and the end '\0' */
  size_t len = strlen(geom) + strlen(pose) + 2;
  char *result = palloc(len);
  snprintf(result, len, "%s;%s", geom, pose);
  return result;
}

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a temporal rigid geometry
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 * @param[in] extended True when the leading SRID string is output
 */
char *
trgeo_wkt_out(const Temporal *temp, int maxdd, bool extended)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  /* Write the geometry */
  const GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  char *wkt_geom = lwgeom_to_wkt(geom, extended ? WKT_EXTENDED : WKT_ISO,
    maxdd, NULL);
  lwgeom_free(geom);
  /* Write the pose */
  char *wkt_pose = tspatial_as_text(temp, maxdd);
  /* Write the representations with the ';' delimiter and the end '\0' */
  size_t len = strlen(wkt_geom) + strlen(wkt_pose) + 2;
  char *result = palloc(len);
  snprintf(result, len, "%s;%s", wkt_geom, wkt_pose);
  return result;
}

/**
 * @ingroup meos_internal_rgeo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal rigid
 * geometry
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 */
inline char *
trgeo_as_text(const Temporal *temp, int maxdd)
{
  return trgeo_wkt_out(temp, maxdd, false);
}

/**
 * @ingroup meos_internal_rgeo_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal rigid geometry
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 */
inline char *
trgeo_as_ewkt(const Temporal *temp, int maxdd)
{
  return trgeo_wkt_out(temp, maxdd, true);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Returns a temporal pose obtained by removing the reference
 * geometry of a temporal rigid geometry
 */
Temporal *
trgeo_tpose(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_has_geom(temp->flags))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    return (Temporal *) trgeoinst_tposeinst((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    return (Temporal *) trgeoseq_tposeseq((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    return (Temporal *) trgeoseqset_tposeseqset((TSequenceSet *) temp);
}

/**
 * @brief Returns a temporal point obtained from the points of the temporal
 * pose of a temporal rigid geometry
 */
Temporal *
trgeo_tpoint(const Temporal *temp)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_point;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] inst Temporal value
 */
TInstant *
geo_tposeinst_to_trgeo(const GSERIALIZED *gs, const TInstant *inst)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void **) gs) || ! ensure_not_null((void **) inst))
    return NULL;
#else
  assert(gs); assert(inst);
#endif /* MEOS */
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  return trgeoinst_make(PointerGetDatum(gs), tinstant_value_p(inst),
    T_TRGEOMETRY, inst->t);
}

/**
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] seq Temporal value
 */
TSequence *
geo_tposeseq_to_trgeo(const GSERIALIZED *gs, const TSequence *seq)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void **) gs) || ! ensure_not_null((void **) seq))
    return NULL;
#else
  assert(gs); assert(seq);
#endif /* MEOS */
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = geo_tposeinst_to_trgeo(gs, TSEQUENCE_INST_N(seq, i));
  return trgeoseq_make_free(PointerGetDatum(gs), instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] ss Temporal value
 */
TSequenceSet *
geo_tposeseqset_to_trgeo(const GSERIALIZED *gs, const TSequenceSet *ss)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void **) gs) || ! ensure_not_null((void **) ss))
    return NULL;
#else
  assert(gs); assert(ss);
#endif /* MEOS */
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = geo_tposeseq_to_trgeo(gs, TSEQUENCESET_SEQ_N(ss, i));
  return trgeoseqset_make_free(PointerGetDatum(gs), sequences, ss->count,
    NORMALIZE_NO);
}

/**
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] temp Temporal value
 */
Temporal *
geo_tpose_to_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void **) gs) || ! ensure_not_null((void **) temp))
    return NULL;
#else
  assert(gs); assert(temp);
#endif /* MEOS */
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    return (Temporal *) geo_tposeinst_to_trgeo(gs, (TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    return (Temporal *) geo_tposeseq_to_trgeo(gs, (TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    return (Temporal *) geo_tposeseqset_to_trgeo(gs, (TSequenceSet *) temp);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @brief Return a geometry obtained by appling a pose to a geometry
 * @param[in] pose Pose
 * @param[inout] gs Geometry
 */
GSERIALIZED *
geom_apply_pose(const Pose *pose, GSERIALIZED *gs)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  LWGEOM *result_geom = lwgeom_clone_deep(geom);
  lwgeom_apply_pose(pose, result_geom);
  if (result_geom->bbox)
    lwgeom_refresh_bbox(result_geom);
  GSERIALIZED *result = geo_serialize(result_geom);
  lwgeom_free(geom); lwgeom_free(result_geom);
  return result;
}

/**
 * @brief Return a copy of the start value of a temporal rigid geometry
 * @param[in] temp Temporal value
 * @csqlfn #Trgeometry_start_value()
 */
Datum
trgeo_start_value(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
  Datum pose;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      pose = tinstant_value((TInstant *) temp);
      break;
    case TSEQUENCE:
      pose = tinstant_value(TSEQUENCE_INST_N((TSequence *) temp, 0));
      break;
    default: /* TSEQUENCESET */
      pose = tinstant_value(
        TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0), 0));
  }
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom(temp));
  GSERIALIZED *res = geom_apply_pose(DatumGetPoseP(pose), gs);
  return GserializedPGetDatum(res);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a copy of the end base value of a temporal rigid geometry
 * @param[in] temp Temporal value
 */
Datum
trgeo_end_value(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
  Datum pose;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      pose = tinstant_value((TInstant *) temp);
      break;
    case TSEQUENCE:
      pose = tinstant_value(TSEQUENCE_INST_N((TSequence *) temp,
        ((TSequence *) temp)->count - 1));
      break;
    default: /* TSEQUENCESET */
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
        ((TSequenceSet *) temp)->count - 1);
      pose = tinstant_value(TSEQUENCE_INST_N(seq, seq->count - 1));
    }
  }
  GSERIALIZED *gs = geo_copy(DatumGetGserializedP(trgeo_geom_p(temp)));
  return GserializedPGetDatum(geom_apply_pose(DatumGetPoseP(pose), gs));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return in the last argument a copy of the n-th value of a temporal
 * value 
 * @param[in] temp Temporal value
 * @param[in] n Number (1-based)
 * @param[out] result Resulting timestamp
 * @return On error return false
 * @csqlfn #Trgeometry_value_n()
 */
bool
trgeo_value_n(const Temporal *temp, int n, Datum *result)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) result))
    return false;
#else
  assert(temp); assert(result);
#endif /* MEOS */
  if (! ensure_positive(n))
    return false;

  Datum pose;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      if (n != 1)
        return false;
      pose = tinstant_value((TInstant *) temp);
      break;
    }
    case TSEQUENCE:
    {
      if (n < 1 || n > ((TSequence *) temp)->count)
        return false;
      pose = tinstant_value(TSEQUENCE_INST_N((TSequence *) temp, n - 1));
      break;
    }
    default: /* TSEQUENCESET */
      if (! tsequenceset_value_n((TSequenceSet *) temp, n, &pose))
        return false;
  } 
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom(temp));
  GSERIALIZED *res = geom_apply_pose(DatumGetPoseP(pose), gs);
  *result = GserializedPGetDatum(res);
  return true;
}

/**
 * @ingroup libmeos_internal_rgeo_restrict
 * @brief Return the value of a temporal rigid geometry at a timestamptz
 * @sqlfn valueAtTimestamp
 */
bool
trgeo_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Datum *result)
{
  assert(temp->temptype == T_TRGEOMETRY);
  Datum pose_datum;
  bool found = temporal_value_at_timestamptz(temp, t, strict, &pose_datum);
  if (found)
  {
    /* Apply pose to reference geometry */
    const Pose *pose = DatumGetPoseP(pose_datum);
    GSERIALIZED *gs = geo_copy(DatumGetGserializedP(trgeo_geom_p(temp)));
    GSERIALIZED *result_gs = geom_apply_pose(pose, gs);
    *result = PointerGetDatum(result_gs);
  }
  return found;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the start instant of a temporal rigid geometry
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_instant()
 */
TInstant *
trgeo_start_instant(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  /* A temporal rigid geometry always has a start instant */
  return geo_tposeinst_to_trgeo(DatumGetGserializedP(trgeo_geom_p(temp)),
    temporal_start_inst(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end instant of a temporal rigid geometry
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_instant()
 */
TInstant *
trgeo_end_instant(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  /* A temporal rigid geometry always has an end instant */
  return geo_tposeinst_to_trgeo(DatumGetGserializedP(trgeo_geom_p(temp)),
    temporal_end_inst(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end instant of a temporal value
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @note This function is used for validity testing.
 * @csqlfn #Temporal_end_instant()
 */
TInstant *
trgeo_instant_n(const Temporal *temp, int n)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  TInstant *inst = temporal_instant_n(temp, n);
  if (! inst)
    return NULL;
  return geo_tposeinst_to_trgeo(DatumGetGserializedP(trgeo_geom_p(temp)),
    inst);
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the distinct instants of a temporal value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_instants()
 */
TInstant **
trgeo_instants(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(count); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  const TInstant **instants = temporal_instants_p(temp, count);
  TInstant **result = palloc(sizeof(TInstant *) * *count);
  const GSERIALIZED *res_geo = DatumGetGserializedP(trgeo_geom_p(temp));
  for (int i = 0; i < *count; i ++)
    result[i] = geo_tposeinst_to_trgeo(res_geo, instants[i]);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the start sequence of a temporal sequence (set)
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_sequence()
 */
TSequence *
trgeo_start_sequence(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  /* Ensure the validity of the arguments */
  if (! ensure_continuous(temp))
    return NULL;

  const TSequence *res_pose = (temp->subtype == TSEQUENCE) ?
    (TSequence *) temp : TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0);
  const GSERIALIZED *res_geo = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tposeseq_to_trgeo(res_geo, res_pose);
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end sequence of a temporal sequence (set)
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_sequence()
 */
TSequence *
trgeo_end_sequence(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  if (! ensure_continuous(temp))
    return NULL;

  const TSequence *res_pose = (temp->subtype == TSEQUENCE) ?
    (TSequence *) temp : TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
  const GSERIALIZED *res_geo = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tposeseq_to_trgeo(res_geo, res_pose);
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the n-th sequence of a temporal sequence (set)
 * @param[in] temp Temporal value
 * @param[in] n Number (1-based)
 * @return On error return @p NULL
 * @csqlfn #Temporal_sequence_n()
 */
TSequence *
trgeo_sequence_n(const Temporal *temp, int n)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  if (! ensure_continuous(temp))
    return NULL;

  const TSequence *res_pose;
  if (temp->subtype == TSEQUENCE)
    res_pose = (n == 1) ? (const TSequence *) temp : NULL;
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    res_pose = (n >= 1 && n <= ss->count) ? TSEQUENCESET_SEQ_N(ss, n - 1) : 
      NULL;
  }
  if (! res_pose)
    return NULL;
  const GSERIALIZED *res_geo = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tposeseq_to_trgeo(res_geo, res_pose);
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return an array of copies of the sequences of a temporal sequence
 * (set)
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_sequences()
 */
TSequence **
trgeo_sequences(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */
  if (! ensure_continuous(temp))
    return NULL;

  const GSERIALIZED *res_geo = DatumGetGserializedP(trgeo_geom_p(temp));
  const TSequence **sequences = temporal_sequences_p(temp, count);
  TSequence **result = palloc(sizeof(TSequence *) * *count);
  for (int i = 0; i < *count; i ++)
    result[i] = geo_tposeseq_to_trgeo(res_geo, sequences[i]);
  pfree(sequences);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_transf
 * @brief Return a temporal rigid geometry rounded to a given number of decimal places
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits to output
 * @csqlfn #Temporal_round()
 */
Temporal *
trgeo_round(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *temp1 = temporal_copy(temp);
  Temporal *res_pose = temporal_round(temp1, maxdd);
  GSERIALIZED *res_geo = geo_round(DatumGetGserializedP(trgeo_geom_p(temp)),
    maxdd);
  pfree(temp1);
  return geo_tpose_to_trgeo(res_geo, res_pose);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_transf
 * @brief Return a temporal rigid geometry transformed to a temporal instant
 * @param[in] temp Temporal value
 * @csqlfn #Trgeometry_to_tinstant()
 */
TInstant *
trgeo_to_tinstant(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp))
    return NULL;
#else
  assert(temp);
#endif /* MEOS */

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return trgeoseq_to_tinstant((TSequence *) temp);
    default: /* TSEQUENCESET */
      return trgeoseqset_to_tinstant((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_rgeo_transf
 * @brief Return a temporal rigid geometry transformed to a temporal sequence
 * @param[in] temp Temporal value
 * @param[in] interp_str Interpolation string, may be NULL
 * @csqlfn #Trgeometry_to_tsequence()
 */
TSequence *
trgeo_to_tsequence(const Temporal *temp, const char *interp_str)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  interpType interp;
  /* If the interpolation is not NULL */
  if (interp_str)
    interp = interptype_from_string(interp_str);
  else
  {
    if (temp->subtype == TSEQUENCE)
      interp = MEOS_FLAGS_GET_INTERP(temp->flags);
    else
      interp = MEOS_FLAGS_GET_CONTINUOUS(temp->flags) ? LINEAR : STEP;
  }
  TSequence *res = temporal_tsequence(temp, interp);
  const GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  TSequence *result = geo_tposeseq_to_trgeo(gs, res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_transf
 * @brief Return a temporal rigid geometry transformed to a temporal sequence set
 * @param[in] temp Temporal value
 * @param[in] interp_str Interpolation string
 * @csqlfn #Trgeometry_to_tsequenceset()
 */
TSequenceSet *
trgeo_to_tsequenceset(const Temporal *temp, const char *interp_str)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  interpType interp;
  /* If the interpolation is not NULL */
  if (interp_str)
    interp = interptype_from_string(interp_str);
  else
  {
    interp = MEOS_FLAGS_GET_INTERP(temp->flags);
    if (interp == INTERP_NONE || interp == DISCRETE)
      interp = MEOS_FLAGS_GET_CONTINUOUS(temp->flags) ? LINEAR : STEP;
  }
  TSequenceSet *res = temporal_tsequenceset(temp, interp);
  const GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  TSequenceSet *result = geo_tposeseqset_to_trgeo(gs, res);
  pfree(res);
  return result;
}

/*****************************************************************************/

Temporal *
trgeo_set_interp(const Temporal *temp, interpType interp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_set_interp(temp, interp);
  return geo_tpose_to_trgeo(DatumGetGserializedP(trgeo_geom_p(temp)), res);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) a base value
 * @param[in] temp Temporal value
 * @param[in] value Value
 * @param[in] atfunc True if the restriction is at, false for minus
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 * @csqlfn #Temporal_at_value(), #Temporal_minus_value()
 */
Temporal *
trgeo_restrict_value(const Temporal *temp, Datum value, bool atfunc)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_restrict_value(temp, value, atfunc);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) an array of base
 * values
 * @param[in] temp Temporal value
 * @param[in] s Set
 * @param[in] atfunc True if the restriction is at, false for minus
 * @csqlfn #Temporal_at_values(), #Temporal_minus_values()
 */
Temporal *
trgeo_restrict_values(const Temporal *temp, const Set *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_restrict_values(temp, s, atfunc);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup libmeos_internal_rgeo_restrict
 * @brief Return the value of a temporal rigid geometry at a timestamptz
 * @sqlfn valueAtTimestamp
 */
Temporal *
trgeo_restrict_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_restrict_timestamptz(temp, t, atfunc);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamp set from a temporal rigid geometry atfuncing the
 * instants before and after the given timestamp, if any
 * @param[in] temp Temporal value
 * @param[in] s Timestamp set
 * @param[in] atfunc True when the instants before and after the timestamp
 * set are atfunced in the result
 * @csqlfn #Trgeo_restrict_tstzset()
 */
Temporal *
trgeo_restrict_tstzset(const Temporal *temp, const Set *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(s); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_restrict_tstzset(temp, s, atfunc);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamptz span from a temporal rigid geometry
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @param[in] atfunc True when the instants before and after the span, if any,
 * are atfunced in the result
 * @csqlfn #Trgeo_restrict_tstzspan()
 */
Temporal *
trgeo_restrict_tstzspan(const Temporal *temp, const Span *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(s); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_restrict_tstzspan(temp, s, atfunc);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamptz span set from a temporal rigid geometry
 * @param[in] temp Temporal value
 * @param[in] ss Span set
 * @param[in] atfunc True when the instants before and after the span set, if
 * any, are atfunced in the result
 * @csqlfn #Trgeo_restrict_tstzspanset()
 */
Temporal *
trgeo_restrict_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ss) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(ss); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_restrict_tstzspanset(temp, ss, atfunc);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_modif
 * @brief Append an instant to a temporal value
 * @param[in,out] temp Temporal value
 * @param[in] inst Temporal instant
 * @param[in] interp Interpolation
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap
 * @param[in] expand True when reserving space for additional instants
 * @csqlfn #Temporal_append_tinstant()
 * @return When the temporal value passed as first argument has space for 
 * adding the instant, the function returns the temporal value. Otherwise,
 * a NEW temporal value is returned and the input value is freed.
 * @note Always use the function to overwrite the existing temporal value as in: 
 * @code
 * temp = temporal_append_tinstant(temp, inst, ...);
 * @endcode
 */
Temporal *
trgeo_append_tinstant(Temporal *temp, const TInstant *inst, 
  interpType interp, double maxdist, const Interval *maxt, bool expand)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) inst) ||
      ! ensure_same_temporal_type(temp, (Temporal *) inst))
    return NULL;
#else
  assert(temp); assert(inst); assert(temp->temptype == inst->temptype);
#endif /* MEOS */
  if (! ensure_spatial_validity(temp, (const Temporal *) inst) ||
      ! ensure_temporal_isof_subtype((Temporal *) inst, TINSTANT))
    return NULL;

  Temporal *res = temporal_append_tinstant(temp, inst, interp, maxdist, maxt,
    expand);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_temporal_modif
 * @brief Append a sequence to a temporal value
 * @param[in,out] temp Temporal value
 * @param[in] seq Temporal sequence
 * @param[in] expand True when reserving space for additional sequences
 * @csqlfn #Temporal_append_tsequence()
 */
Temporal *
trgeo_append_tsequence(Temporal *temp, const TSequence *seq, bool expand)
{
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) seq) ||
      ! ensure_same_temporal_type(temp, (Temporal *) seq))
    return NULL;
#else
  assert(temp); assert(seq); assert(temp->temptype == seq->temptype);
#endif /* MEOS */
  /* Ensure the validity of the arguments */
  if ((temp->subtype != TINSTANT && ! ensure_same_interp(temp, (Temporal *) seq)) ||
      ! ensure_spatial_validity(temp, (Temporal *) seq) ||
      ! ensure_temporal_isof_subtype((Temporal *) seq, TSEQUENCE))
    return NULL;

  Temporal *res = temporal_append_tsequence(temp, seq, expand);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_rgeo_restrict
 * @brief Return the value of a temporal rigid geometry at a timestamptz
 * @sqlfn valueAtTimestamp
 */
Temporal *
trgeo_delete_timestamptz(const Temporal *temp, TimestampTz t, bool connect)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_delete_timestamptz(temp, t, connect);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamp set from a temporal rigid geometry connecting the
 * instants before and after the given timestamp, if any
 * @param[in] temp Temporal value
 * @param[in] s Timestamp set
 * @param[in] connect True when the instants before and after the timestamp
 * set are connected in the result
 * @csqlfn #Trgeo_delete_tstzset()
 */
Temporal *
trgeo_delete_tstzset(const Temporal *temp, const Set *s, bool connect)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(s); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_delete_tstzset(temp, s, connect);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamptz span from a temporal rigid geometry
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @param[in] connect True when the instants before and after the span, if any,
 * are connected in the result
 * @csqlfn #Trgeo_delete_tstzspan()
 */
Temporal *
trgeo_delete_tstzspan(const Temporal *temp, const Span *s, bool connect)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(s); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_delete_tstzspan(temp, s, connect);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamptz span set from a temporal rigid geometry
 * @param[in] temp Temporal value
 * @param[in] ss Span set
 * @param[in] connect True when the instants before and after the span set, if
 * any, are connected in the result
 * @csqlfn #Trgeo_delete_tstzspanset()
 */
Temporal *
trgeo_delete_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool connect)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ss) ||
      ! ensure_temporal_isof_type(temp, T_TRGEOMETRY))
    return NULL;
#else
  assert(temp); assert(ss); assert(temp->temptype == T_TRGEOMETRY);
#endif /* MEOS */

  Temporal *res = temporal_delete_tstzspanset(temp, ss, connect);
  if (! res)
    return NULL;
  GSERIALIZED *gs = DatumGetGserializedP(trgeo_geom_p(temp));
  return geo_tpose_to_trgeo(gs, res);
}

/*****************************************************************************/

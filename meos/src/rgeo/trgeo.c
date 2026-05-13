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
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "temporal/lifting.h"
#include "temporal/meos_catalog.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"
#include "rgeo/trgeo_utils.h"

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure that a temporal rigid geometry has a reference geometry
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

/**
 * @brief Ensure the validity of a temporal rigid geometry and a geometry
 */
bool
ensure_valid_trgeo_stbox(const Temporal *temp, const STBox *box)
{
  VALIDATE_TRGEOMETRY(temp, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_same_srid(tspatial_srid(temp), stbox_srid(box)) ||
      ! ensure_same_spatial_dimensionality(temp->flags, box->flags))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal rigid geometry and a geometry
 */
bool
ensure_valid_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TRGEOMETRY(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two temporal rigid geometries
 */
bool
ensure_valid_trgeo_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TRGEOMETRY(temp1, false); VALIDATE_TPOINT(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two temporal rigid geometries
 */
bool
ensure_valid_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TRGEOMETRY(temp1, false); VALIDATE_TRGEOMETRY(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return false;
  return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_rgeo_inout
 * @brief Return a temporal rigid geometry from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
inline Temporal *
trgeo_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return tspatial_parse(&str, T_TRGEOMETRY);
}

/**
 * @ingroup meos_rgeo_inout
 * @brief Return a temporal rigid geometry from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
inline Temporal *
trgeo_from_mfjson(const char *mfjson)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(mfjson, NULL);
  return temporal_from_mfjson(mfjson, T_TRGEOMETRY);
}
#endif /* MEOS */

/**
 * @ingroup meos_rgeo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal rigid
 * geometry
 * @param[in] temp Temporal rigid geometry
 */
char *
trgeo_out(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  char *geom = geo_out(trgeo_geom_p(temp));
  char *pose = temporal_out(temp, OUT_DEFAULT_DECIMAL_DIGITS);
  /* Write the representations with the ';' delimiter and the end '\0' */
  size_t len = strlen(geom) + strlen(pose) + 2;
  char *result = palloc(len);
  snprintf(result, len, "%s;%s", geom, pose);
  return result;
}

/**
 * @ingroup meos_internal_rgeo_inout
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] maxdd Maximum number of decimal digits
 * @param[in] extended True when the leading SRID string is output
 */
char *
trgeo_wkt_out(const Temporal *temp, int maxdd, bool extended)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  /* Write the geometry */
  LWGEOM *geom = lwgeom_from_gserialized(trgeo_geom_p(temp));
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
 * @ingroup meos_rgeo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal rigid
 * geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] maxdd Maximum number of decimal digits
 */
inline char *
trgeo_as_text(const Temporal *temp, int maxdd)
{
  return trgeo_wkt_out(temp, maxdd, false);
}

/**
 * @ingroup meos_rgeo_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
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
 * @ingroup meos_rgeo_conversion
 * @brief Return a temporal pose obtained by removing the reference geometry
 * of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 */
Temporal *
trgeo_to_tpose(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
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
 * @ingroup meos_rgeo_conversion
 * @brief Return a temporal point obtained from the points of the temporal
 * pose of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 */
Temporal *
trgeo_to_tpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_point;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TGEOMPOINT;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/**
 * @ingroup meos_internal_rgeo_conversion
 * @brief Return the reference geometry of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 */
const GSERIALIZED *
trgeo_geom_p(const Temporal *temp)
{
  assert(temp);
  const GSERIALIZED *result;
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
 * @ingroup meos_rgeo_conversion
 * @brief Return a copy of the reference geometry of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 */
GSERIALIZED *
trgeo_geom(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_has_geom(temp->flags))
    return NULL;
  return geo_copy(trgeo_geom_p(temp));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_constructor
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] inst Temporal pose
 */
TInstant *
geo_tposeinst_to_trgeo(const GSERIALIZED *gs, const TInstant *inst)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(inst, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  return trgeoinst_make(gs, DatumGetPoseP(tinstant_value_p(inst)), inst->t);
}

/**
 * @ingroup meos_internal_rgeo_constructor
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] seq Temporal pose
 */
TSequence *
geo_tposeseq_to_trgeo(const GSERIALIZED *gs, const TSequence *seq)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(seq, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = geo_tposeinst_to_trgeo(gs, TSEQUENCE_INST_N(seq, i));
  return trgeoseq_make_free(gs, instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_rgeo_constructor
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] ss Temporal pose
 */
TSequenceSet *
geo_tposeseqset_to_trgeo(const GSERIALIZED *gs, const TSequenceSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(ss, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = geo_tposeseq_to_trgeo(gs, TSEQUENCESET_SEQ_N(ss, i));
  return trgeoseqset_make_free(gs, sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_rgeo_constructor
 * @brief Construct a temporal rigid geometry from a geometry and a temporal
 * pose
 * @param[in] gs Geometry
 * @param[in] temp Temporal pose
 */
Temporal *
geo_tpose_to_trgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
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
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a geometry obtained by appling a pose to a geometry
 * @param[in] pose Pose
 * @param[inout] gs Geometry
 */
GSERIALIZED *
geom_apply_pose(const GSERIALIZED *gs, const Pose *pose)
{
  assert(pose); assert(gs);
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
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the start value of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_start_value()
 */
GSERIALIZED *
trgeo_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

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
  return geom_apply_pose(trgeo_geom_p(temp), DatumGetPoseP(pose));
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the end base value of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 */
GSERIALIZED *
trgeo_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

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
  return geom_apply_pose(trgeo_geom_p(temp), DatumGetPoseP(pose));
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return in the last argument a copy of the n-th value of a temporal
 * value 
 * @param[in] temp Temporal rigid geometry
 * @param[in] n Number (1-based)
 * @param[out] result Resulting timestamp
 * @return On error return false
 * @csqlfn #Trgeometry_value_n()
 */
bool
trgeo_value_n(const Temporal *temp, int n, GSERIALIZED **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, false); VALIDATE_NOT_NULL(result, false);
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
  *result = geom_apply_pose(trgeo_geom_p(temp), DatumGetPoseP(pose));
  return true;
}

/**
 * @ingroup meos_internal_rgeo_accessor
 * @brief Return the value of a temporal rigid geometry at a timestamptz
 */
bool
trgeo_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Datum *result)
{
  assert(temp); assert(result); assert(temp->temptype == T_TRGEOMETRY);
  Datum pose;
  bool found = temporal_value_at_timestamptz(temp, t, strict, &pose);
  if (found)
  {
    /* Apply pose to reference geometry */
    GSERIALIZED *gs = geom_apply_pose(trgeo_geom_p(temp), DatumGetPoseP(pose));
    *result = PointerGetDatum(gs);
  }
  return found;
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the start instant of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_instant()
 */
TInstant *
trgeo_start_instant(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  /* A temporal rigid geometry always has a start instant */
  const TInstant *inst = temporal_start_inst(temp);
  TInstant *res = trgeoinst_tposeinst(inst);
  TInstant *result = geo_tposeinst_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the end instant of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_instant()
 */
TInstant *
trgeo_end_instant(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  /* A temporal rigid geometry always has an end instant */
  const TInstant *inst = temporal_end_inst(temp);
  TInstant *res = trgeoinst_tposeinst(inst);
  TInstant *result = geo_tposeinst_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the n-th instant of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] n Number (1-based)
 * @return On error return @p NULL
 * @csqlfn #Temporal_instant_n()
 */
TInstant *
trgeo_instant_n(const Temporal *temp, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_positive(n))
    return NULL;

  TInstant *inst = temporal_instant_n(temp, n);
  if (! inst)
    return NULL;
  TInstant *res = trgeoinst_tposeinst(inst);
  TInstant *result = geo_tposeinst_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the distinct instants of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_instants()
 */
TInstant **
trgeo_instants(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

  const TInstant **instants = temporal_insts_p(temp, count);
  TInstant **result = palloc(sizeof(TInstant *) * *count);
  const GSERIALIZED *geo = trgeo_geom_p(temp);
  for (int i = 0; i < *count; i ++)
  {
    TInstant *inst = trgeoinst_tposeinst(instants[i]);
    result[i] = geo_tposeinst_to_trgeo(geo, inst);
    pfree(inst);
  }
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the start sequence of a temporal sequence (set)
 * @param[in] temp Temporal rigid geometry
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_sequence()
 */
TSequence *
trgeo_start_sequence(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_continuous(temp))
    return NULL;

  const TSequence *res_pose = (temp->subtype == TSEQUENCE) ?
    (TSequence *) temp : TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0);
  TSequence *res = trgeoseq_tposeseq(res_pose);
  TSequence *result = geo_tposeseq_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the end sequence of a temporal sequence (set)
 * @param[in] temp Temporal rigid geometry
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_sequence()
 */
TSequence *
trgeo_end_sequence(const Temporal *temp)
{
  /* Ensure the validity of the arguments */

  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_continuous(temp))
    return NULL;

  const TSequence *res_pose = (temp->subtype == TSEQUENCE) ?
    (TSequence *) temp : TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
  TSequence *res = trgeoseq_tposeseq(res_pose);
  TSequence *result = geo_tposeseq_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the n-th sequence of a temporal sequence (set)
 * @param[in] temp Temporal rigid geometry
 * @param[in] n Number (1-based)
 * @return On error return @p NULL
 * @csqlfn #Temporal_sequence_n()
 */
TSequence *
trgeo_sequence_n(const Temporal *temp, int n)
{
  /* Ensure the validity of the arguments */

  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_continuous(temp) || ! ensure_positive(n))
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
  TSequence *res = trgeoseq_tposeseq(res_pose);
  TSequence *result = geo_tposeseq_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return an array of copies of the sequences of a temporal sequence
 * (set)
 * @param[in] temp Temporal rigid geometry
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_sequences()
 */
TSequence **
trgeo_sequences(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */

  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_continuous(temp))
    return NULL;

  const GSERIALIZED *geo = trgeo_geom_p(temp);
  const TSequence **sequences = temporal_sequences_p(temp, count);
  TSequence **result = palloc(sizeof(TSequence *) * *count);
  for (int i = 0; i < *count; i ++)
  {
    TSequence *seq = trgeoseq_tposeseq(sequences[i]);
    result[i] = geo_tposeseq_to_trgeo(geo, seq);
    pfree(seq);
  }
  pfree(sequences);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_transf
 * @brief Return a temporal rigid geometry rounded to a given number of decimal places
 * @param[in] temp Temporal rigid geometry
 * @param[in] maxdd Maximum number of decimal digits to output
 * @csqlfn #Temporal_round()
 */
Temporal *
trgeo_round(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */

  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  Temporal *tpose = trgeo_to_tpose(temp);
  GSERIALIZED *res_geo = geo_round(trgeo_geom_p(temp), maxdd);
  Temporal *res_tpose = temporal_round(tpose, maxdd);
  Temporal *result = geo_tpose_to_trgeo(res_geo, res_tpose);
  pfree(tpose); pfree(res_geo); pfree(res_tpose);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_transf
 * @brief Return a temporal rigid geometry transformed to a temporal instant
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_to_tinstant()
 */
TInstant *
trgeo_to_tinstant(const Temporal *temp)
{
  /* Ensure the validity of the arguments */

  VALIDATE_TRGEOMETRY(temp, NULL);

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
 * @param[in] temp Temporal rigid geometry
 * @param[in] interp_str Interpolation string, may be NULL
 * @csqlfn #Trgeometry_to_tsequence()
 */
TSequence *
trgeo_to_tsequence(const Temporal *temp, const char *interp_str)
{
  /* Ensure the validity of the arguments */

  VALIDATE_TRGEOMETRY(temp, NULL);

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
  Temporal *tpose = trgeo_to_tpose(temp);
  TSequence *res = temporal_tsequence(tpose, interp);
  TSequence *result = geo_tposeseq_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res); pfree(tpose);
  return result;
}

/**
 * @ingroup meos_rgeo_transf
 * @brief Return a temporal rigid geometry transformed to a temporal sequence set
 * @param[in] temp Temporal rigid geometry
 * @param[in] interp_str Interpolation string, may be @p NULL
 * @csqlfn #Trgeometry_to_tsequenceset()
 */
TSequenceSet *
trgeo_to_tsequenceset(const Temporal *temp, const char *interp_str)
{
  /* Ensure the validity of the arguments */

  VALIDATE_TRGEOMETRY(temp, NULL);

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
  Temporal *tpose = trgeo_to_tpose(temp);
  TSequenceSet *res = temporal_tsequenceset(tpose, interp);
  TSequenceSet *result = geo_tposeseqset_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res); pfree(tpose);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_transf
 * @brief Restrict a temporal rigid geometry transformed to an interpolation
 * @param[in] temp Temporal rigid geometry
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_set_interp()
 */
Temporal *
trgeo_set_interp(const Temporal *temp, interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_set_interp(tpose, interp);
  if (! res)
    return NULL;
  /* We need to explicitly set the temporal type to T_TPOSE */
  res->temptype = T_TPOSE;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res); pfree(tpose);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] value Value
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 * @csqlfn #Temporal_restrict_value()
 */
Temporal *
trgeo_restrict_value(const Temporal *temp, Datum value, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *res = temporal_restrict_value(temp, value, atfunc);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Temporal_at_value()
 */
Temporal *
trgeo_at_value(const Temporal *temp, const GSERIALIZED *gs)
{
  return trgeo_restrict_value(temp, PointerGetDatum(gs), REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to the complement of a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
trgeo_minus_value(const Temporal *temp, const GSERIALIZED *gs)
{
  return trgeo_restrict_value(temp, PointerGetDatum(gs), REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) a set of
 * geometries
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Set of values
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Temporal_restrict_values()
 */
Temporal *
trgeo_restrict_values(const Temporal *temp, const Set *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_GEOMSET(s, NULL); 
  Temporal *res = temporal_restrict_values(temp, s, atfunc);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to a set of geometries
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Set of values
 * @csqlfn #Temporal_at_values()
 */
inline Temporal *
trgeo_at_values(const Temporal *temp, const Set *s)
{
  return trgeo_restrict_values(temp, s, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to the complement of a set of
 * geometries
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Temporal_minus_values()
 * @param[in] s Set of values
 */
inline Temporal *
trgeo_minus_values(const Temporal *temp, const Set *s)
{
  return trgeo_restrict_values(temp, s, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) a
 * timestamptz
 * @param[in] temp Temporal rigid geometry
 * @param[in] t Timestamptz
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Temporal_restrict_timestamptz()
 */
Temporal *
trgeo_restrict_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_restrict_timestamptz(tpose, t, atfunc);
  pfree(tpose); 
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to a timestamptz
 * @param[in] temp Temporal rigid geometry
 * @param[in] t Timestamptz
 * @csqlfn #Temporal_at_timestamptz()
 */
inline Temporal *
trgeo_at_timestamptz(const Temporal *temp, TimestampTz t)
{
  return trgeo_restrict_timestamptz(temp, t, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to the complement of a
 * timestamptz
 * @param[in] temp Temporal rigid geometry
 * @param[in] t Timestamptz
 * @csqlfn #Temporal_minus_timestamptz()
 */
inline Temporal *
trgeo_minus_timestamptz(const Temporal *temp, TimestampTz t)
{
  return trgeo_restrict_timestamptz(temp, t, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) a
 * timestamptz set
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Timestamp set
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Temporal_restrict_tstzset()
 */
Temporal *
trgeo_restrict_tstzset(const Temporal *temp, const Set *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TSTZSET(s, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_restrict_tstzset(tpose, s, atfunc);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to a timestamptz set
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Set
 * @csqlfn #Temporal_at_tstzspanset()
 */
inline Temporal *
trgeo_at_tstzset(const Temporal *temp, const Set *s)
{
  return trgeo_restrict_tstzset(temp, s, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to the complement of a
 * timestamptz set
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Set
 * @csqlfn #Temporal_minus_tstzspanset()
 */
inline Temporal *
trgeo_minus_tstzset(const Temporal *temp, const Set *s)
{
  return trgeo_restrict_tstzset(temp, s, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) a
 * timestamptz span
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Span
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Temporal_restrict_tstzspan()
 */
Temporal *
trgeo_restrict_tstzspan(const Temporal *temp, const Span *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TSTZSPAN(s, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_restrict_tstzspan(tpose, s, atfunc);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to a timestamptz span
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Span
 * @csqlfn #Temporal_at_tstzspan()
 */
inline Temporal *
trgeo_at_tstzspan(const Temporal *temp, const Span *s)
{
  return trgeo_restrict_tstzspan(temp, s, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to the complement of a timestamptz
 * span
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Span
 * @csqlfn #Temporal_minus_tstzspan()
 */
inline Temporal *
trgeo_minus_tstzspan(const Temporal *temp, const Span *s)
{
  return trgeo_restrict_tstzspan(temp, s, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to (the complement of) a
 * timestamptz span set
 * @param[in] temp Temporal rigid geometry
 * @param[in] ss Span set
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Temporal_restrict_tstzspanset()
 */
Temporal *
trgeo_restrict_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_restrict_tstzspanset(tpose, ss, atfunc);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to a timestamptz span set
 * @param[in] temp Temporal rigid geometry
 * @param[in] ss Span set
 * @csqlfn #Temporal_at_tstzspanset()
 */
inline Temporal *
trgeo_at_tstzspanset(const Temporal *temp, const SpanSet *ss)
{
  return trgeo_restrict_tstzspanset(temp, ss, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to the complement of a
 * timestamptz span set
 * @param[in] temp Temporal rigid geometry
 * @param[in] ss Span set
 * @csqlfn #Temporal_minus_tstzspanset()
 */
inline Temporal *
trgeo_minus_tstzspanset(const Temporal *temp, const SpanSet *ss)
{
  return trgeo_restrict_tstzspanset(temp, ss, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to instants before or equal a
 * timestamptz
 * @param[in] temp Temporal rigid geometry
 * @param[in] t Timestamptz
 * @param[in] strict True if the restriction is strictly before, false when
 * the restriction is before or equal
 * @csqlfn #Temporal_before_timestamptz()
 */
Temporal *
trgeo_before_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_before_timestamptz(tpose, t, atfunc);
  pfree(tpose); 
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Restrict a temporal rigid geometry to instants after or equal a
 * timestamptz
 * @param[in] temp Temporal rigid geometry
 * @param[in] t Timestamptz
 * @param[in] strict True if the restriction is strictly after, false when
 * the restriction is before or equal
 * @csqlfn #Temporal_after_timestamptz()
 */
Temporal *
trgeo_after_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_after_timestamptz(tpose, t, atfunc);
  pfree(tpose); 
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_modif
 * @brief Append an instant to a temporal value
 * @param[in,out] temp Temporal rigid geometry
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
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TRGEOMETRY(inst, NULL);
  if (! ensure_spatial_validity(temp, (const Temporal *) inst) ||
      ! ensure_temporal_isof_subtype((Temporal *) inst, TINSTANT))
    return NULL;

  Temporal *tpose = trgeo_to_tpose(temp);
  TInstant *tpose_inst = trgeoinst_tposeinst(inst);
  Temporal *res = temporal_append_tinstant(tpose, tpose_inst, interp, maxdist,
    maxt, expand);
  if (! res)
    return NULL;
  
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res); pfree(tpose); pfree(tpose_inst);
  return result;
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Append a sequence to a temporal value
 * @param[in,out] temp Temporal rigid geometry
 * @param[in] seq Temporal sequence
 * @param[in] expand True when reserving space for additional sequences
 * @csqlfn #Temporal_append_tsequence()
 */
Temporal *
trgeo_append_tsequence(Temporal *temp, const TSequence *seq, bool expand)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TRGEOMETRY(seq, NULL);
  if ((temp->subtype != TINSTANT && 
      ! ensure_same_interp(temp, (Temporal *) seq)) ||
      ! ensure_spatial_validity(temp, (Temporal *) seq) ||
      ! ensure_temporal_isof_subtype((Temporal *) seq, TSEQUENCE))
    return NULL;

  Temporal *tpose = trgeo_to_tpose(temp);
  TSequence *tpose_seq = trgeoseq_tposeseq(seq);
  Temporal *res = temporal_append_tsequence(tpose, tpose_seq, expand);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res); pfree(tpose); pfree(tpose_seq);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_modif
 * @brief Return the value of a temporal rigid geometry at a timestamptz
 * @csqlfn #Temporal_delete_timestamptz
 */
Temporal *
trgeo_delete_timestamptz(const Temporal *temp, TimestampTz t, bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_delete_timestamptz(tpose, t, connect);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamp set from a temporal rigid geometry connecting the
 * instants before and after the given timestamp, if any
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Timestamp set
 * @param[in] connect True when the instants before and after the timestamp
 * set are connected in the result
 * @csqlfn #Temporal_delete_tstzset()
 */
Temporal *
trgeo_delete_tstzset(const Temporal *temp, const Set *s, bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TSTZSET(s, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_delete_tstzset(tpose, s, connect);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamptz span from a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Span
 * @param[in] connect True when the instants before and after the span, if any,
 * are connected in the result
 * @csqlfn #Temporal_delete_tstzspan()
 */
Temporal *
trgeo_delete_tstzspan(const Temporal *temp, const Span *s, bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TSTZSPAN(s, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_delete_tstzspan(tpose, s, connect);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_rgeo_modif
 * @brief Delete a timestamptz span set from a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] ss Span set
 * @param[in] connect True when the instants before and after the span set, if
 * any, are connected in the result
 * @csqlfn #Temporal_delete_tstzspanset()
 */
Temporal *
trgeo_delete_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool connect)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  Temporal *tpose = trgeo_to_tpose(temp);
  Temporal *res = temporal_delete_tstzspanset(tpose, ss, connect);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geo_tpose_to_trgeo(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/*****************************************************************************/

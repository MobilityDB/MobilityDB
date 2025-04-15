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
 * @brief SRID functions for spatial types
 */

/* PostgreSQL */
#include <postgres.h>
/* PROJ */
#include <proj.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/type_util.h"
#include "geo/stbox.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include <meos_cbuffer.h>
  #include "cbuffer/cbuffer.h"
#endif
#if NPOINT
  #include "npoint/tnpoint.h"
#endif
#if POSE
  #include <meos_pose.h>
  #include "pose/pose.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
#endif

/*
 * Maximum length of an ESPG string to lookup
 * Notice that SRID_MAXIMUM is defined by PostGIS as 999999
 */
#define MAX_AUTH_SRID_STR 12 /* EPSG:999999 */

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the SRID of a spatial value
 */
int32_t
spatial_srid(Datum d, meosType basetype)
{
  assert(spatial_basetype(basetype));
  switch (basetype)
  {
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return gserialized_get_srid(DatumGetGserializedP(d));
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_srid(DatumGetCbufferP(d));
#endif
#if NPOINT
    case T_NPOINT:
      return npoint_srid(DatumGetNpointP(d));
#endif
#if POSE
    case T_POSE:
      return pose_srid(DatumGetPoseP(d));
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown SRID function for type: %s", meostype_name(basetype));
    return SRID_INVALID;
  }
}

/**
 * @brief Return true if the first argument has been successfully transformed
 * to another SRID
 */
bool
spatial_set_srid(Datum d, meosType basetype, int32_t srid)
{
  assert(spatial_basetype(basetype));
  switch (basetype)
  {
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      gserialized_set_srid(DatumGetGserializedP(d), srid);
      return true;
#if CBUFFER
    case T_CBUFFER:
      cbuffer_set_srid(DatumGetCbufferP(d), srid);
      return true;
#endif
#if POSE || RGEO
    case T_POSE:
      pose_set_srid(DatumGetPoseP(d), srid);
      return true;
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown setSRID function for type: %s", meostype_name(basetype));
    return false;
  }
}

/*****************************************************************************
 * Functions for spatial reference systems for spatial set types
 *****************************************************************************/

/**
 * @ingroup meos_geo_set_srid
 * @brief Return the SRID of a spatial set
 * @param[in] s Spatial set
 * @csqlfn #Spatialset_srid()
 */
int32_t
spatialset_srid(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_SPATIALSET(s, SRID_INVALID);
  return spatial_srid(SET_VAL_N(s, 0), s->basetype);
}

/**
 * @ingroup meos_geo_set_srid
 * @brief Return a spatial set with the coordinates set to an SRID
 * @param[in] s Spatial set
 * @param[in] srid SRID
 * @return On error return @p NULL
 * @csqlfn #Spatialset_set_srid()
 */
Set *
spatialset_set_srid(const Set *s, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_SPATIALSET(s, NULL);
  if (! ensure_srid_known(srid))
    return NULL;

  Set *result = set_copy(s);
  /* Set the SRID of the composing points */
  for (int i = 0; i < s->count; i++)
  {
    if (! spatial_set_srid(SET_VAL_N(result, i), s->basetype, srid))
    {
      pfree(result); 
      return NULL;
    }
  }
  /* Set the SRID of the bounding box */
  STBox *box = SET_BBOX_PTR(result);
  box->srid = srid;
  return result;
}

/*****************************************************************************
 * Functions for spatial reference systems for temporal spatial types
 *****************************************************************************/

/**
 * @ingroup meos_internal_geo_srid
 * @brief Return the SRID of a temporal spatial instant
 * @return On error return @p SRID_INVALID
 * @param[in] inst Temporal spatial instant
 */
int32_t
tspatialinst_srid(const TInstant *inst)
{
  assert(inst); assert(tspatial_type(inst->temptype));
  meosType basetype = temptype_basetype(inst->temptype);
  return spatial_srid(tinstant_value_p(inst), basetype);
}

/**
 * @ingroup meos_geo_srid
 * @brief Return the SRID of a temporal spatial value
 * @return On error return @p SRID_INVALID
 * @param[in] temp Temporal spatial value
 * @csqlfn #Tspatial_srid()
 */
int32_t
tspatial_srid(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, SRID_INVALID);
  const STBox *box;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tspatialinst_srid((TInstant *) temp);
    case TSEQUENCE:
      box = ((STBox *) TSEQUENCE_BBOX_PTR((TSequence *) temp));
      return box->srid;
    default: /* TSEQUENCESET */
      box = ((STBox *) TSEQUENCESET_BBOX_PTR((TSequenceSet *) temp));
      return box->srid;
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_srid
 * @brief Set the coordinates of a temporal spatial instant to an SRID
 * @param[in] inst Temporal spatial instant
 * @param[in] srid SRID
 */
void
tspatialinst_set_srid(TInstant *inst, int32_t srid)
{
  assert(inst); assert(tspatial_type(inst->temptype));
  meosType basetype = temptype_basetype(inst->temptype);
  spatial_set_srid(tinstant_value_p(inst), basetype, srid);
  return;
}

/**
 * @ingroup meos_internal_geo_srid
 * @brief Set the coordinates of a temporal spatial sequence to an SRID
 * @param[in] seq Temporal spatial sequence
 * @param[in] srid SRID
 */
void
tspatialseq_set_srid(TSequence *seq, int32_t srid)
{
  assert(seq); assert(tspatial_type(seq->temptype));
  /* Set the SRID of the composing points */
  for (int i = 0; i < seq->count; i++)
    tspatialinst_set_srid((TInstant *) TSEQUENCE_INST_N(seq, i), srid);
  /* Set the SRID of the bounding box */
  STBox *box = TSEQUENCE_BBOX_PTR(seq);
  box->srid = srid;
  return;
}

/**
 * @ingroup meos_internal_geo_srid
 * @brief Set the coordinates of a temporal spatial sequence set to an SRID
 * @param[in] ss Temporal spatial sequence set
 * @param[in] srid SRID
 */
void
tspatialseqset_set_srid(TSequenceSet *ss, int32_t srid)
{
  assert(ss); assert(tspatial_type(ss->temptype));
  /* Loop for every composing sequence */
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
    tspatialseq_set_srid(seq, srid);
  }
  /* Set the SRID of the bounding box */
  STBox *box = TSEQUENCESET_BBOX_PTR(ss);
  box->srid = srid;
  return;
}

/**
 * @ingroup meos_geo_srid
 * @brief Return a temporal spatial value with the coordinates set to an SRID
 * @param[in] temp Temporal spatial value
 * @param[in] srid SRID
 * @return On error return @p NULL
 * @see #tspatialinst_set_srid()
 * @see #tspatialseq_set_srid()
 * @see #tspatialseqset_set_srid()
 * @csqlfn #Tspatial_set_srid()
 */
Temporal *
tspatial_set_srid(const Temporal *temp, int32_t srid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL);
  if (! ensure_srid_known(srid))
    return NULL;

  Temporal *result = temporal_copy(temp);
  assert(temptype_subtype(result->subtype));
  switch (result->subtype)
  {
    case TINSTANT:
      tspatialinst_set_srid((TInstant *) result, srid);
      break;
    case TSEQUENCE:
      tspatialseq_set_srid((TSequence *) result, srid);
      break;
    default: /* TSEQUENCESET */
      tspatialseqset_set_srid((TSequenceSet *) result, srid);
  }
  return result;
}

/*****************************************************************************
 * Defitions taken from file lwgeom_transform.c
 *****************************************************************************/

/**
 * @brief Convert decimal degress to radians
 */
static void
to_rad(POINT4D *pt)
{
  pt->x *= M_PI/180.0;
  pt->y *= M_PI/180.0;
}

/**
 * @brief Convert radians to decimal degress
 */
static void
to_dec(POINT4D *pt)
{
  pt->x *= 180.0/M_PI;
  pt->y *= 180.0/M_PI;
}

/*****************************************************************************
 * Functions fetching an LWPROJ structure containing transform information
 *****************************************************************************/

/**
 * @brief Return a structure with the information to perform a transformation
 * @param[in] srid_from,srid_to SRIDs
 * @note We are avoiding to have a list of recognized SRIDs cached as done in
 * PostGIS. We didn't find a way to get the authority name from an SRID.
 * Given that all (but one) entries in PostGIS spatial_ref_sys table have
 * either authority name equal to 'EPSG' or 'ESRI', we try finding the two
 * combinations for the input and output SRIDs.
 */
LWPROJ *
lwproj_get(int32_t srid_from, int32_t srid_to)
{
  char srid_from_str[MAX_AUTH_SRID_STR];
  char srid_to_str[MAX_AUTH_SRID_STR];
  /* From SRID */
  snprintf(srid_from_str, MAX_AUTH_SRID_STR, "EPSG:%d", srid_from);
  PJ *pj1 = proj_create(proj_get_context(), srid_from_str);
  if (! pj1)
  {
    snprintf(srid_from_str, MAX_AUTH_SRID_STR, "ESRI:%d", srid_from);
    pj1 = proj_create(proj_get_context(), srid_from_str);
    if (! pj1)
    {
      /* Error */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Transform: Could not form projection from 'srid=%d'", srid_from);
      return NULL;
    }
  }
  proj_destroy(pj1);
  /* To SRID */
  snprintf(srid_to_str, MAX_AUTH_SRID_STR, "EPSG:%d", srid_to);
  PJ *pj2 = proj_create(proj_get_context(), srid_to_str);
  if (! pj2)
  {
    snprintf(srid_to_str, MAX_AUTH_SRID_STR, "ESRI:%d", srid_to);
    pj2 = proj_create(proj_get_context(), srid_to_str);
    if (! pj2)
    {
      /* Error */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Transform: Could not form projection to 'srid=%d'", srid_to);
      return NULL;
    }
  }
  proj_destroy(pj2);
  LWPROJ *result = lwproj_from_str(srid_from_str, srid_to_str);
  if (result)
    return result;
  /* Error */
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "Transform: Could not form projection from 'srid=%d' to 'srid=%d'",
    srid_from, srid_to);
  return NULL;
}

/**
 * @brief Return a structure with the information to perform a transformation
 * pipeline
 * @param[in] pipeline Pipeline string
 * @param[in] is_forward True when the transformation is forward
 */
LWPROJ *
lwproj_get_pipeline(const char *pipeline, bool is_forward)
{
  assert(pipeline);
  LWPROJ *result = lwproj_from_str_pipeline(pipeline, is_forward);
  if (result)
    return result;
  /* Error */
  PJ *pj_in = proj_create(proj_get_context(), pipeline);
  if (! pj_in)
  {
    proj_errno_reset(NULL);
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Transform: Could not parse coordinate operation '%s'", pipeline);
      return NULL;
    }
  }
  proj_destroy(pj_in);
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "Transform: Failed to transform '%s'", pipeline);
  return NULL;
}

/*****************************************************************************/

#if MEOS
/**
 * @brief Return the spheroid in the last argument initialized from an SRID
 * @param[in] srid SRID
 * @param[in] s Spheroid
 * @note Based on the PostGIS function of the same name in directory
 * /libpgcommon
 */
int
spheroid_init_from_srid(int32_t srid, SPHEROID *s)
{
  // TODO implement a cache system for PROJ entries or reuse PostGIS one
  // if ( lwproj_lookup(srid, srid, &pj) == LW_FAILURE)
  LWPROJ *pj = lwproj_get(srid, srid);
  if (! pj)
    return false;

  if (! pj->source_is_latlong)
    return LW_FAILURE;
  spheroid_init(s, pj->source_semi_major_metre, pj->source_semi_minor_metre);

  return LW_SUCCESS;
}
#endif /* MEOS */

/**
 * @brief Determine whether an SRID is geodetic
 * @param[in] srid SRID
 */
bool
srid_is_latlong(int32_t srid)
{
  LWPROJ *pj = lwproj_get(srid, srid);
  if (! pj)
    return false;
  bool result = pj->source_is_latlong;
  pfree(pj);
  return result;
}

/**
 * @brief Ensure that an SRID is geodetic
 */
bool
ensure_srid_is_latlong(int32_t srid)
{
  if (srid_is_latlong(srid))
    return true;
  meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
    "Only lon/lat coordinate systems are supported in geography.");
  return false;
}

/**
 * @brief Transform the point to another SRID
 * @param[in] gs Point
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 * @note This function MODIFIES the input point in the first argument
 * @note Derived from PostGIS version 3.4.0 function ptarray_transform(),
 * file `lwgeom_transform.c`
 */
bool
point_transf_pj(GSERIALIZED *gs, int32_t srid_to, const LWPROJ *pj)
{
  assert(gs); assert(pj);
  int has_z = FLAGS_GET_Z(gs->gflags);
  POINT4D *p = (POINT4D *) GS_POINT_PTR(gs);
  double *pa_double = (double *) (GS_POINT_PTR(gs));
  PJ_DIRECTION direction = pj->pipeline_is_forward ? PJ_FWD : PJ_INV;

  /* Convert to radians if necessary */
  if (proj_angular_input(pj->pj, direction))
    to_rad(p);

  /* For single points it's faster to call proj_trans */
  PJ_XYZT v = {pa_double[0], pa_double[1], has_z ? pa_double[2] : 0.0, 0.0};
  PJ_COORD c;
  c.xyzt = v;
  PJ_COORD t = proj_trans(pj->pj, direction, c);

  int pj_errno_val = proj_errno_reset(pj->pj);
  if (pj_errno_val)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG,
      "Transform: %s (%d)", proj_errno_string(pj_errno_val), pj_errno_val);
    return false;
  }
  pa_double[0] = (t.xyzt).x;
  pa_double[1] = (t.xyzt).y;
  if (has_z)
    pa_double[2] = (t.xyzt).z;

  /* Convert radians to degrees if necessary */
  if (proj_angular_output(pj->pj, direction))
    to_dec(p);

  gserialized_set_srid(gs, srid_to);
  return true;
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return true if the first argument has been successfully transformed
 * to another SRID
 */
Datum
#if CBUFFER
  datum_transf_pj(Datum d, meosType basetype, int32_t srid_to, const LWPROJ *pj)
#else
  datum_transf_pj(Datum d, meosType basetype,
    int32_t srid_to __attribute__((unused)), const LWPROJ *pj)
#endif /* CBUFFER */
{
  assert(spatial_basetype(basetype));
  switch (basetype)
  {
    case T_GEOMETRY:
    case T_GEOGRAPHY:
    {
      /* TODO This DOES NOT transform the geometry, it should be fixed */
      LWGEOM *geo = lwgeom_from_gserialized(DatumGetGserializedP(d));
      if (! lwgeom_transform(geo, (LWPROJ *) pj))
        return PointerGetDatum(NULL);
      geo->srid = srid_to;
      return PointerGetDatum(geo_serialize(geo));
    }
#if CBUFFER
    case T_CBUFFER:
      return PointerGetDatum(cbuffer_transf_pj(DatumGetCbufferP(d), srid_to,
        pj));
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown transformation function for type: %s",
        meostype_name(basetype));
    return false;
  }
}

/*****************************************************************************/

/**
 * @brief Return a spatial set transformed to another SRID
 * @param[in] s Set
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
static Set *
spatialset_transf_pj(const Set *s, int32_t srid_to, const LWPROJ *pj)
{
  assert(s); assert(pj); assert(spatialset_type(s->settype));
  /* Copy the set to be able to transform the points of the set in place */
  Datum *datums = palloc(sizeof(Datum) * s->count);
  /* Transform the points of the set */
  for (int i = 0; i < s->count; i++)
  {
    datums[i] = datum_transf_pj(SET_VAL_N(s, i), s->basetype, srid_to, pj);
    if (!datums[i])
    {
      pfree_array((void **) datums, i);
      return NULL;
    }
  }
  return set_make_free(datums, s->count, s->basetype, ORDER_NO);
}

/**
 * @ingroup meos_geo_set_srid
 * @brief Return a spatial set transformed to another SRID
 * @param[in] s Spatial set
 * @param[in] srid_to Target SRID
 * @csqlfn #Spatialset_transform()
 */
Set *
spatialset_transform(const Set *s, int32_t srid_to)
{
  int32_t srid_from = spatialset_srid(s);
  /* Ensure the validity of the arguments */
  VALIDATE_SPATIALSET(s, NULL);
  if (! ensure_srid_known(srid_from) || ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return set_copy(s);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get(srid_from, srid_to);
  if (! pj)
    return NULL;

  /* Transform the geo set */
  Set *result = spatialset_transf_pj(s, srid_to, pj);

  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_geo_set_srid
 * @brief Return a spatial set transformed to another SRID using a
 * pipeline
 * @param[in] s Spatial set
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 * @csqlfn #Spatialset_transform_pipeline()
 */
Set *
spatialset_transform_pipeline(const Set *s, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  int32_t srid_from = spatialset_srid(s);
  /* Ensure the validity of the arguments */
  VALIDATE_SPATIALSET(s, NULL); VALIDATE_NOT_NULL(pipeline, NULL);
  if (! ensure_srid_known(srid_from) || ! ensure_srid_known(srid_to))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the geo set */
  Set *result = set_copy(s);
  return spatialset_transf_pj(result, srid_to, pj);
}

/*****************************************************************************/

/**
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] inst Temporal spatial instant
 * @param[in] srid_to SRID, may be @p SRID_UNKNOWN for pipeline
 * transformations
 * @param[in] pj Information about the transformation
 */
TInstant *
tspatialinst_transf_pj(const TInstant *inst, int32_t srid_to, const LWPROJ *pj)
{
  assert(inst); assert(pj); assert(tspatial_type(inst->temptype));
  meosType basetype = temptype_basetype(inst->temptype);
  /* The SRID of the geometry is set in the following function */
  Datum d = datum_transf_pj(tinstant_value_p(inst), basetype, srid_to, pj);
  if (! DatumGetPointer(d))
    return NULL;
  return tinstant_make_free(d, inst->temptype, inst->t);
}

/**
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] seq Temporal spatial sequence
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 */
TSequence *
tspatialseq_transf_pj(const TSequence *seq, int32_t srid_to, const LWPROJ *pj)
{
  assert(seq); assert(pj); assert(tspatial_type(seq->temptype));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    instants[i] = tspatialinst_transf_pj(TSEQUENCE_INST_N(seq, i), srid_to, pj);
    if (! instants[i])
    {
      pfree_array((void **) instants, i);
      return NULL;
    }
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] ss Temporal spatial sequence set
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 */
TSequenceSet *
tspatialseqset_transf_pj(const TSequenceSet *ss, int32_t srid_to,
  const LWPROJ *pj)
{
  assert(ss); assert(pj); assert(tspatial_type(ss->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    sequences[i] = tspatialseq_transf_pj(TSEQUENCESET_SEQ_N(ss, i),
      srid_to, pj);
    if (! sequences[i])
    {
      pfree_array((void **) sequences, i);
      return NULL;
    }
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @brief Return a temporal spatial value transformed to another SRID
 * @param[in] temp Temporal spatial
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
Temporal *
tspatial_transf_pj(const Temporal *temp, int32_t srid_to, const LWPROJ *pj)
{
  assert(temp); assert(pj);
  /* Copy the temporal spatial value to transform its points in place */
  Temporal *result = temporal_copy(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tspatialinst_transf_pj((TInstant *) result, srid_to,
        pj);
      break;
    case TSEQUENCE:
      return (Temporal *) tspatialseq_transf_pj((TSequence *) result, srid_to,
        pj);
      break;
    default: /* TSEQUENCESET */
      return (Temporal *) tspatialseqset_transf_pj((TSequenceSet *) result,
        srid_to, pj);
  }
}

/**
 * @ingroup meos_geo_srid
 * @brief Return a temporal spatial value transformed to another SRID
 * @param[in] temp Temporal spatial value
 * @param[in] srid_to Target SRID
 */
Temporal *
tspatial_transform(const Temporal *temp, int32_t srid_to)
{
  int32_t srid_from = tspatial_srid(temp);
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL);
  if (! ensure_srid_known(srid_from) || ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return temporal_copy(temp);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get(srid_from, srid_to);
  if (! pj)
    return NULL;

  /* Function lwproj_get does not set pj->source_is_latlong */
  if (tgeodetic_type(temp->temptype))
    pj->source_is_latlong = true;

  /* Transform the temporal spatial type */
  Temporal *result = tspatial_transf_pj(temp, srid_to, pj);

  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_geo_srid
 * @brief Return a temporal spatial value transformed to another SRID using a
 * pipeline
 * @param[in] temp Temporal spatial value
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Temporal *
tspatial_transform_pipeline(const Temporal *temp, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(pipeline, NULL);
  // TODO The following currently break the tests, this should be fixed
  // if (! ensure_srid_known(srid_to))
    // return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_get_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the temporal spatial type */
  Temporal *result = tspatial_transf_pj(temp, srid_to, pj);

  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/*****************************************************************************/


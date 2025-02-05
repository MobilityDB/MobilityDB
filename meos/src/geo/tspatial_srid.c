/***********************************************************************
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
  #include "cbuffer/tcbuffer.h"
#endif
#if NPOINT
  #include "npoint/tnpoint.h"
#endif
#if POSE
  #include "pose/pose.h"
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
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown setSRID function for type: %s", meostype_name(basetype));
    return false;
  }
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the SRID of a geo set
 * @param[in] s Set
 * @csqlfn #Spatialset_srid()
 */
int32_t
spatialset_srid(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_spatialset_type(s->settype))
    return SRID_INVALID;

  return spatial_srid(SET_VAL_N(s, 0), s->basetype);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a geo set with the coordinates set to an SRID
 * @param[in] s Set
 * @param[in] srid SRID
 * @return On error return @p NULL
 * @csqlfn #Spatialset_set_srid()
 */
Set *
spatialset_set_srid(const Set *s, int32_t srid)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_spatialset_type(s->settype))
    return NULL;

  Set *result = set_cp(s);
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

/*****************************************************************************/

/**
 * @ingroup meos_box_accessor
 * @brief Return the SRID of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @csqlfn #Stbox_srid()
 */
int32_t
stbox_srid(const STBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box))
    return SRID_INVALID;
  return box->srid;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a spatiotemporal box with the coordinates set to an SRID
 * @param[in] box Spatiotemporal box
 * @param[in] srid SRID
 * @csqlfn #Stbox_set_srid()
 */
STBox *
stbox_set_srid(const STBox *box, int32_t srid)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_has_X_stbox(box))
    return NULL;
  STBox *result = stbox_cp(box);
  result->srid = srid;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the SRID of a temporal spatial instant
 * @return On error return @p SRID_INVALID
 * @param[in] temp Temporal spatial type
 * @csqlfn #Tspatial_srid()
 */
int
tspatialinst_srid(const TInstant *inst)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) inst) || 
      ! ensure_tspatial_type(inst->temptype))
    return SRID_INVALID;

  Datum value = tinstant_value(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  return spatial_srid(value, basetype);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the SRID of a temporal spatial type
 * @return On error return @p SRID_INVALID
 * @param[in] temp Temporal value
 * @csqlfn #Tspatial_srid()
 */
int
tspatial_srid(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_tspatial_type(temp->temptype))
    return SRID_INVALID;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tspatialinst_srid((TInstant *) temp);
    case TSEQUENCE:
      return ((STBox *) TSEQUENCE_BBOX_PTR((TSequence *) temp))->srid;
    default: /* TSEQUENCESET */
      return ((STBox *) TSEQUENCESET_BBOX_PTR((TSequenceSet *) temp))->srid;
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Set the coordinates of a temporal spatial instant to an SRID
 * @param[in] inst Temporal instant
 * @param[in] srid SRID
 * @csqlfn #Tspatial_set_srid()
 */
void
tspatialinst_set_srid(TInstant *inst, int32_t srid)
{
  assert(inst); assert(tspatial_type(inst->temptype));
  meosType basetype = temptype_basetype(inst->temptype);
  spatial_set_srid(tinstant_val(inst), basetype, srid);
  return;
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Set the coordinates of a temporal spatial sequence to an SRID
 * @param[in] seq Temporal sequence
 * @param[in] srid SRID
 * @csqlfn #Tspatial_set_srid()
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
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Set the coordinates of a temporal spatial sequence set to an SRID
 * @param[in] ss Temporal sequence set
 * @param[in] srid SRID
 * @csqlfn #Tspatial_set_srid()
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
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal spatial vqlue with the coordinates set to an SRID
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_tspatial_type(temp->temptype))
    return NULL;

  Temporal *result = temporal_cp(temp);
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
lwproj_transform(int32_t srid_from, int32_t srid_to)
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
static LWPROJ *
lwproj_transform_pipeline(const char *pipeline, bool is_forward)
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

/**
 * @brief Determine whether an SRID is geodetic
 * @param[in] srid SRID
 */
bool
meos_srid_is_latlong(int32_t srid)
{
	LWPROJ *pj = lwproj_transform(srid, srid);
	if (! pj)
		return false;
	bool result = pj->source_is_latlong;
  pfree(pj);
	return result;
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
static bool
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

#if MEOS
/**
 * @brief Return a point transformed to another SRID
 * @param[in] gs Point
 * @param[in] srid_to Target SRID
 * @param[in] pj Information about the transformation
 */
GSERIALIZED *
point_transform_pj(const GSERIALIZED *gs, int32_t srid_to, LWPROJ *pj)
{
  GSERIALIZED *result = geo_copy(gs);
  if (! point_transf_pj(result, srid_to, pj))
  {
    pfree(result); result = NULL;
  }
  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_setspan_spatial_transf
 * @brief Return a point transformed to another SRID
 * @param[in] gs Point
 * @param[in] srid_to Target SRID
 */
GSERIALIZED *
point_transform(const GSERIALIZED *gs, int32_t srid_to)
{
  int32_t srid_from;
  /* Verify validity of arguments */
  if (! ensure_not_null((void *) gs) ||
      ! ensure_srid_known(srid_from = gserialized_get_srid(gs)) ||
      ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return geo_copy(gs);

  /* Transform the point */
  LWPROJ *pj = lwproj_transform(srid_from, srid_to);
  if (! pj)
    return NULL;
  return point_transform_pj(gs, srid_to, pj);
}

/**
 * @ingroup meos_setspan_spatial_transf
 * @brief Return a temporal point transformed to another SRID using a
 * pipeline
 * @param[in] gs Point
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
GSERIALIZED *
point_transform_pipeline(const GSERIALIZED *gs, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  int32_t srid_from;
  /* Verify validity of arguments */
  if (! ensure_srid_known(srid_from = gserialized_get_srid(gs)))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Transform the point */
  LWPROJ *pj = lwproj_transform_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;
  return point_transform_pj(gs, srid_to, pj);
}
#endif /* MEOS */

/*****************************************************************************/

#if CBUFFER
/**
 * @brief Return a circular buffer transformed to another SRID using a
 * pipeline
 * @param[in] cbuf Circular buffer
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
static bool
cbuffer_transf_pj(Cbuffer *cbuf, int32_t srid_to, const LWPROJ *pj)
{
  assert(cbuf); assert(pj);
  /* We are working on a copy of the input circular buffer for the 
   * transformation so we can remove the const qualifier */
  GSERIALIZED *gs = 
    (GSERIALIZED *) DatumGetGserializedP(PointerGetDatum(&cbuf->point));
  return point_transf_pj(gs, srid_to, pj);
}

/**
 * @brief Return a circular buffer transformed to another SRID
 * @param[in] cbuf Circular buffer
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
Cbuffer *
cbuffer_transform_pj(const Cbuffer *cbuf, int32_t srid_to, const LWPROJ *pj)
{
  assert(cbuf); assert(pj);
  /* Copy the circular buffer to transform its composing points in place */
  Cbuffer *result = cbuffer_cp(cbuf);
  if (! cbuffer_transf_pj(result, srid_to, pj))
  {
    pfree(result);
    result = NULL;
  }
  return result;
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a circular buffer transformed to another SRID
 * @param[in] cbuf Circular buffer
 * @param[in] srid_to Target SRID
 */
Cbuffer *
cbuffer_transform(const Cbuffer *cbuf, int32_t srid_to)
{
  int32_t srid_from;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || 
      ! ensure_srid_known(srid_from = cbuffer_srid(cbuf)) ||
      ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return cbuffer_cp(cbuf);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform(srid_from, srid_to);
  if (! pj)
    return NULL;

  /* Transform the circular buffer */
  Cbuffer * result = cbuffer_transform_pj(cbuf, srid_to, pj);

  proj_destroy(pj->pj); pfree(pj);

  return result;
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a circular buffer transformed to another SRID using a
 * pipeline
 * @param[in] cbuf Circular buffer
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Cbuffer *
cbuffer_transform_pipeline(const Cbuffer *cbuf, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) ||
      ! ensure_not_null((void *) pipeline))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the circular buffer */
  Cbuffer * result = cbuffer_transform_pj(cbuf, srid_to, pj);

  proj_destroy(pj->pj); pfree(pj);

  return result;
}
#endif /* CBUFFER */

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return true if the first argument has been successfully transformed
 * to another SRID
 */
bool
datum_transf_pj(Datum d, meosType basetype, int32_t srid_to, const LWPROJ *pj)
{
  assert(spatial_type(basetype));
  switch (basetype)
  {
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return point_transf_pj(DatumGetGserializedP(d), srid_to, pj);
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_transf_pj(DatumGetCbufferP(d), srid_to, pj);
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
spatialset_transform_pj(const Set *s, int32_t srid_to, LWPROJ *pj)
{
  assert(s); assert(pj); assert(spatialset_type(s->settype));
  /* Copy the set to be able to transform the points of the set in place */
  Set *result = set_cp(s);
  /* Transform the points of the set */
  for (int i = 0; i < s->count; i++)
  {
    if (! datum_transf_pj(SET_VAL_N(s, i), s->basetype, srid_to, pj))
    {
      pfree(result); proj_destroy(pj->pj); pfree(pj); 
      return NULL;
    }
  }
  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a spatial set transformed to another SRID
 * @param[in] s Set
 * @param[in] srid_to Target SRID
 */
Set *
spatialset_transform(const Set *s, int32_t srid_to)
{
  int32_t srid_from;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_spatialset_type(s->settype) ||
      ! ensure_srid_known(srid_from = spatialset_srid(s)) ||
      ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return set_cp(s);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform(srid_from, srid_to);
  if (! pj)
    return NULL;

  /* Transform the geo set */
  Set *result = set_cp(s);
  return spatialset_transform_pj(result, srid_to, pj);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a spatial set transformed to another SRID using a
 * pipeline
 * @param[in] s Set point
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Set *
spatialset_transform_pipeline(const Set *s, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  int32_t srid_from;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) pipeline) ||
      ! ensure_spatialset_type(s->settype) ||
      ! ensure_srid_known(srid_from = spatialset_srid(s)))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the geo set */
  Set *result = set_cp(s);
  return spatialset_transform_pj(result, srid_to, pj);
}

/*****************************************************************************/

/**
 * @brief Return a spatiotemporal box transformed to another SRID using a
 * pipeline
 * @param[in] box Spatiotemporal box
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
static bool
stbox_transf_pj(STBox *box, int32_t srid_to, const LWPROJ *pj)
{
  assert(box); assert(pj);
  /* Create the points corresponding to the bounds */
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(box->flags);
  GSERIALIZED *min = geopoint_make(box->xmin, box->ymin, box->zmin,
    hasz, geodetic, box->srid);
  GSERIALIZED *max = geopoint_make(box->xmax, box->ymax, box->zmax,
    hasz, geodetic, box->srid);

  /* Transform the points */
  if (! point_transf_pj(min, srid_to, pj) ||
      ! point_transf_pj(max, srid_to, pj))
    return false;

  /* Set the bounds of the box from the transformed points */
  box->srid = srid_to;
  if (hasz)
  {
    const POINT3DZ *ptmin = GSERIALIZED_POINT3DZ_P(min);
    const POINT3DZ *ptmax = GSERIALIZED_POINT3DZ_P(max);
    box->xmin = ptmin->x;
    box->ymin = ptmin->y;
    box->zmin = ptmin->z;
    box->xmax = ptmax->x;
    box->ymax = ptmax->y;
    box->zmax = ptmax->z;
  }
  else
  {
    const POINT2D *ptmin = GSERIALIZED_POINT2D_P(min);
    const POINT2D *ptmax = GSERIALIZED_POINT2D_P(max);
    box->xmin = ptmin->x;
    box->ymin = ptmin->y;
    box->xmax = ptmax->x;
    box->ymax = ptmax->y;
  }

  /* Clean up and return */
  pfree(min); pfree(max);
  return true;
}

/**
 * @brief Return a spatiotemporal box transformed to another SRID using a
 * pipeline
 * @param[in] box Spatiotemporal box
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
static STBox *
stbox_transform_pj(const STBox *box, int32_t srid_to, LWPROJ *pj)
{
  assert(box); assert(pj);
  /* Copy the spatiotemporal box to transform its composing points in place */
  STBox *result = stbox_cp(box);
  if (! stbox_transf_pj(result, srid_to, pj))
  {
    pfree(result); result = NULL;
  }
  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_box_transf
 * @brief Return a spatiotemporal box transformed to another SRID
 * @param[in] box STBox point
 * @param[in] srid_to Target SRID
 */
STBox *
stbox_transform(const STBox *box, int32_t srid_to)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_srid_known(box->srid) ||
      ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (box->srid == srid_to)
    return stbox_cp(box);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform(box->srid, srid_to);
  if (! pj)
    return NULL;

  /* Transform the temporal point */
  return stbox_transform_pj(box, srid_to, pj);
}

/**
 * @ingroup meos_box_transf
 * @brief Return a tspatiotemporal box transformed to another SRID using a
 * pipeline
 * @param[in] box STBox point
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
STBox *
stbox_transform_pipeline(const STBox *box, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) pipeline) ||
      ! ensure_srid_known(box->srid))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the temporal point */
  return stbox_transform_pj(box, srid_to, pj);
}

/*****************************************************************************/

/**
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] inst Temporal spatial instant
 * @param[in] srid_to SRID, may be @p SRID_UNKNOWN for pipeline
 * transformations
 * @param[in] pj Information about the transformation
 */
static bool
tspatialinst_transf_pj(const TInstant *inst, int32_t srid_to, const LWPROJ *pj)
{
  assert(inst); assert(pj); assert(tspatial_type(inst->temptype));
  meosType basetype = temptype_basetype(inst->temptype);
  /* The SRID of the geometry is set in the following function */
  if (! datum_transf_pj(tinstant_val(inst), basetype, srid_to, pj))
    return false;
  return true;
}

/**
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] seq Temporal spatial sequence
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 */
static bool
tspatialseq_transf_pj(TSequence *seq, int32_t srid_to, const LWPROJ *pj)
{
  assert(seq); assert(pj); assert(tspatial_type(seq->temptype));
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) TSEQUENCE_INST_N(seq, i);
    if (! tspatialinst_transf_pj(inst, srid_to, pj))
      return false;
  }
  /* Transform and set the SRID of the bounding box */
  STBox *box = TSEQUENCE_BBOX_PTR(seq);
  if (! stbox_transf_pj(box, srid_to, pj))
    return false;
  box->srid = srid_to;
  return true;
}

/**
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] ss Temporal spatial sequence set
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 */
static bool
tspatialseqset_transf_pj(TSequenceSet *ss, int32_t srid_to, const LWPROJ *pj)
{
  assert(ss); assert(pj); assert(tspatial_type(ss->temptype));
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
    if (! tspatialseq_transf_pj(seq, srid_to, pj))
      return false;
  }
  /* Transform and set the SRID of the bounding box */
  STBox *box = TSEQUENCESET_BBOX_PTR(ss);
  if (! stbox_transf_pj(box, srid_to, pj))
    return false;
  box->srid = srid_to;
  return true;
}

/**
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] temp Temporal spatial
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
Temporal *
tspatial_transform_pj(const Temporal *temp, int32_t srid_to, const LWPROJ *pj)
{
  assert(temp); assert(pj);
  /* Copy the temporal spatial type to transform its composing points in place */
  Temporal *result = temporal_cp(temp);
  assert(temptype_subtype(temp->subtype));
  bool success;
  switch (temp->subtype)
  {
    case TINSTANT:
      success = tspatialinst_transf_pj((TInstant *) result, srid_to, pj);
      break;
    case TSEQUENCE:
      success = tspatialseq_transf_pj((TSequence *) result, srid_to, pj);
      break;
    default: /* TSEQUENCESET */
      success = tspatialseqset_transf_pj((TSequenceSet *) result, srid_to, pj);
  }
  if (! success)
  {
    pfree(result);
    result = NULL;
  }
  /* Clean up and return */
  return result;
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal spatial type transformed to another SRID
 * @param[in] temp Temporal spatial
 * @param[in] srid_to Target SRID
 */
Temporal *
tspatial_transform(const Temporal *temp, int32_t srid_to)
{
  int32_t srid_from = tspatial_srid(temp);
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_tspatial_type(temp->temptype) ||
      ! ensure_srid_known(srid_from) || ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return temporal_cp(temp);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform(srid_from, srid_to);
  if (! pj)
    return NULL;

  /* Transform the temporal spatial type */
  Temporal * result = tspatial_transform_pj(temp, srid_to, pj);

  proj_destroy(pj->pj); pfree(pj);

  return result;
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal spatial type transformed to another SRID using a
 * pipeline
 * @param[in] temp Temporal spatial
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Temporal *
tspatial_transform_pipeline(const Temporal *temp, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_not_null((void *) pipeline) ||
      ! ensure_tspatial_type(temp->temptype))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the temporal spatial type */
  Temporal * result = tspatial_transform_pj(temp, srid_to, pj);

  proj_destroy(pj->pj); pfree(pj);

  return result;
}

/*****************************************************************************/


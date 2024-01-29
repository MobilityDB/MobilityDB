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
 * @brief SRID functions for geo sets, spatiotemporal boxes and temporal points
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
#include "general/type_util.h"
#include "point/stbox.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_static.h"

/*
 * Maximum length of an ESPG string to lookup
 * Notice that SRID_MAXIMUM is defined by PostGIS as 999999
 */
#define MAX_EPSG_STR 12 /* EPSG:999999 */

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the SRID of a geo set
 * @param[in] s Set
 * @csqlfn #Geoset_get_srid()
 */
int
geoset_srid(const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return SRID_INVALID;

  GSERIALIZED *gs = DatumGetGserializedP(SET_VAL_N(s, 0));
  return gserialized_get_srid(gs);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a geo set with the coordinates set to an SRID
 * @param[in] s Set
 * @param[in] srid SRID
 * @return On error return @p NULL
 * @csqlfn #Geoset_set_srid()
 */
Set *
geoset_set_srid(const Set *s, int32 srid)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype))
    return NULL;


  Set *result = set_cp(s);
  /* Set the SRID of the composing points */
  for (int i = 0; i < s->count; i++)
  {
    GSERIALIZED *gs = DatumGetGserializedP(SET_VAL_N(result, i));
    gserialized_set_srid(gs, srid);
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
 * @csqlfn #Stbox_get_srid()
 */
int32
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
stbox_set_srid(const STBox *box, int32 srid)
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
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point instant
 * @param[in] inst Temporal instant
 * @csqlfn #Tpoint_get_srid()
 */
int
tpointinst_srid(const TInstant *inst)
{
  assert(inst); assert(tgeo_type(inst->temptype));
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_val(inst));
  return gserialized_get_srid(gs);
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point sequence
 * @param[in] seq Temporal sequence
 * @csqlfn #Tpoint_get_srid()
 */
int
tpointseq_srid(const TSequence *seq)
{
  assert(seq);
  /* This function is also called for tnpoint */
  assert(tspatial_type(seq->temptype));
  STBox *box = TSEQUENCE_BBOX_PTR(seq);
  return box->srid;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_get_srid()
 */
int
tpointseqset_srid(const TSequenceSet *ss)
{
  assert(ss);
  /* This function is also called for tnpoint */
  assert(tspatial_type(ss->temptype));
  STBox *box = TSEQUENCESET_BBOX_PTR(ss);
  return box->srid;
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point
 * @return On error return @p SRID_INVALID
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_get_srid()
 */
int
tpoint_srid(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return SRID_INVALID;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tpointinst_srid((TInstant *) temp);
    case TSEQUENCE:
      return tpointseq_srid((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tpointseqset_srid((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal point instant with the coordinates set to an SRID
 * @param[in] inst Temporal instant
 * @param[in] srid SRID
 * @csqlfn #Tpoint_set_srid()
 */
TInstant *
tpointinst_set_srid(const TInstant *inst, int32 srid)
{
  assert(inst); assert(tgeo_type(inst->temptype));
  TInstant *result = tinstant_copy(inst);
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_val(result));
  gserialized_set_srid(gs, srid);
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal point sequence with the coordinates set to an SRID
 * @param[in] seq Temporal sequence
 * @param[in] srid SRID
 * @csqlfn #Tpoint_set_srid()
 */
TSequence *
tpointseq_set_srid(const TSequence *seq, int32 srid)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  TSequence *result = tsequence_copy(seq);
  /* Set the SRID of the composing points */
  for (int i = 0; i < seq->count; i++)
  {
    GSERIALIZED *gs = DatumGetGserializedP(
      tinstant_val(TSEQUENCE_INST_N(result, i)));
    gserialized_set_srid(gs, srid);
  }
  /* Set the SRID of the bounding box */
  STBox *box = TSEQUENCE_BBOX_PTR(result);
  box->srid = srid;
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal point sequence set with the coordinates set to an
 * SRID
 * @param[in] ss Temporal sequence set
 * @param[in] srid SRID
 * @csqlfn #Tpoint_set_srid()
 */
TSequenceSet *
tpointseqset_set_srid(const TSequenceSet *ss, int32 srid)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  STBox *box;
  TSequenceSet *result = tsequenceset_copy(ss);
  /* Loop for every composing sequence */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(result, i);
    for (int j = 0; j < seq->count; j++)
    {
      /* Set the SRID of the composing points */
      GSERIALIZED *gs = DatumGetGserializedP(
        tinstant_val(TSEQUENCE_INST_N(seq, j)));
      gserialized_set_srid(gs, srid);
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
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal point with the coordinates set to an SRID
 * @param[in] temp Temporal point
 * @param[in] srid SRID
 * @return On error return @p NULL
 * @see #tpointinst_set_srid()
 * @see #tpointseq_set_srid()
 * @see #tpointseqset_set_srid()
 * @csqlfn #Tpoint_set_srid()
 */
Temporal *
tpoint_set_srid(const Temporal *temp, int32 srid)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpointinst_set_srid((TInstant *) temp, srid);
    case TSEQUENCE:
      return (Temporal *) tpointseq_set_srid((TSequence *) temp, srid);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_set_srid((TSequenceSet *) temp, srid);
  }
}

/*****************************************************************************
 * Functions for spatial reference systems of temporal network points
 * For temporal points of duration distinct from TInstant the Spatial
 * reference system identifier (SRID) is obtained from the bounding box.
 *****************************************************************************/

#if NPOINT
/**
 * @brief Return the SRID of a temporal network point of subtype instant
 */
int
tnpointinst_srid(const TInstant *inst)
{
  const Npoint *np = DatumGetNpointP(tinstant_val(inst));
  GSERIALIZED *line = route_geom(np->rid);
  int result = gserialized_get_srid(line);
  pfree(line);
  return result;
}

/**
 * @brief Return the SRID of a temporal network point
 */
int
tnpoint_srid(const Temporal *temp)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tnpointinst_srid((const TInstant *) temp);
    case TSEQUENCE:
      return tpointseq_srid((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tpointseqset_srid((TSequenceSet *) temp);
  }
}
#endif /* NPOINT */

/*****************************************************************************
 * Transformation functions for spatial reference systems
 *****************************************************************************/

/* Defitions taken from file lwgeom_transform.c */

/** convert decimal degress to radians */
static void
to_rad(POINT4D *pt)
{
  pt->x *= M_PI/180.0;
  pt->y *= M_PI/180.0;
}

/** convert radians to decimal degress */
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
 */
static LWPROJ *
lwproj_transform(int32 srid_from, int32 srid_to)
{
  char srid_from_str[MAX_EPSG_STR];
  char srid_to_str[MAX_EPSG_STR];
  int len = snprintf(srid_from_str, MAX_EPSG_STR, "EPSG:%d", srid_from);
  srid_from_str[len] = '\0';
  len = snprintf(srid_to_str, MAX_EPSG_STR, "EPSG:%d", srid_to);
  srid_to_str[len] = '\0';
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
lwproj_transform_pipeline(char *pipeline, bool is_forward)
{
  assert(pipeline);
  LWPROJ *result = lwproj_from_str_pipeline(pipeline, is_forward);
  if (result)
    return result;
  /* Error */
  PJ *pj_in = proj_create(PJ_DEFAULT_CTX, pipeline);
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
 * @brief Transform the point to another SRID
 * @param[in] gs Point
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 * @note This function MODIFIES the input point in the first argument
 * @note Derived from PostGIS version 3.4.0 function ptarray_transform(),
 * file `lwgeom_transform.c`
 */
static bool
point_transf_pj(GSERIALIZED *gs, int32 srid_to, const LWPROJ *pj)
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
point_transform_pj(const GSERIALIZED *gs, int32 srid_to, LWPROJ *pj)
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
point_transform(const GSERIALIZED *gs, int32 srid_to)
{
  int32 srid_from;
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
point_transform_pipeline(const GSERIALIZED *gs, char *pipeline,
  int32 srid_to, bool is_forward)
{
  int32 srid_from;
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

/**
 * @brief Return a temporal point transformed to another SRID
 * @param[in] s Set point
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
static Set *
geoset_transform_pj(const Set *s, int32 srid_to, LWPROJ *pj)
{
  assert(s); assert(pj); assert(geoset_type(s->settype));
  /* Copy the set to be able to transform the points of the set in place */
  Set *result = set_cp(s);
  /* Transform the points of the set */
  for (int i = 0; i < s->count; i++)
  {
    GSERIALIZED *gs = DatumGetGserializedP(SET_VAL_N(s, i));
    if (! point_transf_pj(gs, srid_to, pj))
    {
      pfree(result); proj_destroy(pj->pj); pfree(pj); return NULL;
    }
  }
  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a temporal point transformed to another SRID
 * @param[in] s Set point
 * @param[in] srid_to Target SRID
 */
Set *
geoset_transform(const Set *s, int32 srid_to)
{
  int32 srid_from;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_geoset_type(s->settype) ||
      ! ensure_srid_known(srid_from = geoset_srid(s)) ||
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
  return geoset_transform_pj(result, srid_to, pj);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a temporal point transformed to another SRID using a
 * pipeline
 * @param[in] s Set point
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Set *
geoset_transform_pipeline(const Set *s, char *pipeline, int32 srid_to,
  bool is_forward)
{
  int32 srid_from;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) pipeline) ||
      ! ensure_geoset_type(s->settype) ||
      ! ensure_srid_known(srid_from = geoset_srid(s)))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the geo set */
  Set *result = set_cp(s);
  return geoset_transform_pj(result, srid_to, pj);
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
stbox_transf_pj(STBox *box, int32 srid_to, LWPROJ *pj)
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
stbox_transform_pj(const STBox *box, int32 srid_to, LWPROJ *pj)
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
stbox_transform(const STBox *box, int32 srid_to)
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
stbox_transform_pipeline(const STBox *box, char *pipeline,
  int32 srid_to, bool is_forward)
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
 * @brief Return a temporal point transformed to another SRID
 * @param[in] inst Temporal point instant
 * @param[in] srid_to SRID, may be @p SRID_UNKNOWN for pipeline
 * transformations
 * @param[in] pj Information about the transformation
 */
static bool
tpointinst_transf_pj(TInstant *inst, int32 srid_to, LWPROJ *pj)
{
  assert(inst); assert(pj); assert(tgeo_type(inst->temptype));
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_val(inst));
  /* The SRID of the geometry is set in the following function */
  if (! point_transf_pj(gs, srid_to, pj))
    return false;
  return true;
}

/**
 * @brief Return a temporal point transformed to another SRID
 * @param[in] seq Temporal point sequence
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 */
static bool
tpointseq_transf_pj(TSequence *seq, int32 srid_to, LWPROJ *pj)
{
  assert(seq); assert(pj); assert(tgeo_type(seq->temptype));
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = (TInstant *) TSEQUENCE_INST_N(seq, i);
    if (! tpointinst_transf_pj(inst, srid_to, pj))
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
 * @brief Return a temporal point transformed to another SRID
 * @param[in] ss Temporal point sequence set
 * @param[in] srid_to SRID
 * @param[in] pj Information about the transformation
 */
static bool
tpointseqset_transf_pj(TSequenceSet *ss, int32 srid_to, LWPROJ *pj)
{
  assert(ss); assert(pj); assert(tgeo_type(ss->temptype));
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = (TSequence *) TSEQUENCESET_SEQ_N(ss, i);
    if (! tpointseq_transf_pj(seq, srid_to, pj))
      return false;
  }
  /* Transform and set the SRID of the bounding box */
  STBox *box = TSEQUENCESET_BBOX_PTR(ss);
  if (! stbox_transf_pj(box, srid_to, pj))
    return false;
  box->srid = srid_to;
  return true;
}

/*****************************************************************************/

/**
 * @brief Return a temporal point transformed to another SRID
 * @param[in] temp Temporal point
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
static Temporal *
tpoint_transform_pj(const Temporal *temp, int32 srid_to, LWPROJ *pj)
{
  assert(temp); assert(pj);
  /* Copy the temporal point to transform its composing points in place */
  Temporal *result = temporal_cp(temp);
  assert(temptype_subtype(temp->subtype));
  bool ok;
  switch (temp->subtype)
  {
    case TINSTANT:
      ok = tpointinst_transf_pj((TInstant *) result, srid_to, pj);
      break;
    case TSEQUENCE:
      ok = tpointseq_transf_pj((TSequence *) result, srid_to, pj);
      break;
    default: /* TSEQUENCESET */
      ok = tpointseqset_transf_pj((TSequenceSet *) result, srid_to, pj);
  }
  if (! ok)
  {
    pfree(result);
    result = NULL;
  }
  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal point transformed to another SRID
 * @param[in] temp Temporal point
 * @param[in] srid_to Target SRID
 */
Temporal *
tpoint_transform(const Temporal *temp, int32 srid_to)
{
  int32 srid_from;
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype) ||
      ! ensure_srid_known(srid_from = tpoint_srid(temp)) ||
      ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return temporal_cp(temp);

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform(srid_from, srid_to);
  if (! pj)
    return NULL;

  /* Transform the temporal point */
  return tpoint_transform_pj(temp, srid_to, pj);
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal point transformed to another SRID using a
 * pipeline
 * @param[in] temp Temporal point
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Temporal *
tpoint_transform_pipeline(const Temporal *temp, char *pipeline,
  int32 srid_to, bool is_forward)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_not_null((void *) pipeline) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_transform_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the temporal point */
  return tpoint_transform_pj(temp, srid_to, pj);
}

/*****************************************************************************/


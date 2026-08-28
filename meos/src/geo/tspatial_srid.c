/***********************************************************************
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
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/type_util.h"
#include "geo/meos_transform.h"
#include "geo/stbox.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include <meos_cbuffer.h>
  #include "cbuffer/cbuffer.h"
#endif
#if H3
  #include <h3api.h>
#endif
#if NPOINT
  #include "npoint/tnpoint.h"
#endif
#if POINTCLOUD
  #include "pointcloud/pcpoint.h"
  #include "pointcloud/pcpatch.h"
  #include "pointcloud/meos_schema_hook.h"
#endif
#if POSE
  #include <meos_pose.h>
  #include "pose/pose.h"
#endif
#if POSECHAIN
  #include "posechain/posechain.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
#endif
#if H3
  #include <h3api.h>
#endif
#if POINTCLOUD
  #include "pointcloud/pcpoint.h"
  #include "pointcloud/pcpatch.h"
  #include "pointcloud/meos_schema_hook.h"
#endif

/*
 * Maximum length of an ESPG string to lookup
 * SRID_MAXIMUM is defined by PostGIS as 999999
 */
#define MAX_AUTH_SRID_STR 12 /* EPSG:999999 */

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the SRID of a spatial value
 */
int32_t
spatial_srid(Datum d, MeosType basetype)
{
  /* The pgpointcloud base types are deliberately outside spatial_basetype():
   * the other spatial_* dispatchers have no pgpointcloud case, and
   * deriving a pcpoint's or pcpatch's flags needs its schema, unlike the
   * SRID which both resolve through the same pcid-keyed schema lookup. The
   * dispatch admits them through pointcloud_basetype(), the base-type
   * counterpart of tpointcloud_temptype() that the boxops dispatch already
   * uses the same way to admit tpcpoint and tpcpatch beside
   * tspatial_type(). */
#if POINTCLOUD
  assert(spatial_basetype(basetype) || pointcloud_basetype(basetype));
#else
  assert(spatial_basetype(basetype));
#endif
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
#if POSECHAIN
    case T_POSECHAIN:
      return posechain_srid(DatumGetPoseChainP(d));
#endif
#if H3
    case T_H3INDEX:
      /* H3 cells are inherently WGS84 (EPSG:4326) */
      (void) d;
      return SRID_DEFAULT;
#endif
#if POINTCLOUD
    case T_PCPOINT: {
      const Pcpoint *pt = DatumGetPcpointP(d);
      return meos_pc_schema_srid(pcpoint_get_pcid(pt));
    }
    case T_PCPATCH: {
      const Pcpatch *pa = DatumGetPcpatchP(d);
      return meos_pc_schema_srid(pcpatch_get_pcid(pa));
    }
#endif
#if QUADBIN
    case T_QUADBIN:
      /* Quadbin cells are emitted as planar lon/lat (EPSG:4326) */
      (void) d;
      return SRID_DEFAULT;
#endif
#if S2CELL
    case T_S2CELL:
      /* S2 cells are geodetic (EPSG:4326) */
      (void) d;
      return SRID_DEFAULT;
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
spatial_set_srid(Datum d, MeosType basetype, int32_t srid)
{
  assert(spatial_basetype(basetype));
  switch (basetype)
  {
#if POINTCLOUD
    case T_PCPOINT:
    case T_PCPATCH:
      /* A point cloud value holds its SRID by reference: the schema its pcid
       * names declares it, so there is nothing in the value to set */
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "The SRID of a %s is the one its schema declares and cannot be set",
        meostype_name(basetype));
      return false;
#endif
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      gserialized_set_srid(DatumGetGserializedP(d), srid);
      return true;
#if CBUFFER
    case T_CBUFFER:
      cbuffer_set_srid_int(DatumGetCbufferP(d), srid);
      return true;
#endif
#if POSE || RGEO
    case T_POSE:
      pose_set_srid_int(DatumGetPoseP(d), srid);
      return true;
#endif
#if POSECHAIN
    case T_POSECHAIN:
      posechain_set_srid_int(DatumGetPoseChainP(d), srid);
      return true;
#endif
#if H3
    case T_H3INDEX:
      /* H3 cells are inherently WGS84; only SRID 4326 is accepted */
      (void) d;
      return (srid == SRID_DEFAULT);
#endif
#if QUADBIN
    case T_QUADBIN:
      /* Quadbin cells are planar lon/lat; only SRID 4326 is accepted */
      (void) d;
      return (srid == SRID_DEFAULT);
#endif
#if S2CELL
    case T_S2CELL:
      /* S2 cells are geodetic; only SRID 4326 is accepted */
      (void) d;
      return (srid == SRID_DEFAULT);
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
 * Functions for spatial reference systems for spatiotemporal values
 *****************************************************************************/

/**
 * @ingroup meos_internal_geo_srid
 * @brief Return the SRID of a spatiotemporal instant
 * @return On error return @p SRID_INVALID
 * @param[in] inst Spatiotemporal instant
 */
int32_t
tspatialinst_srid(const TInstant *inst)
{
  assert(inst);
  assert(tspatial_type(inst->temptype));
  MeosType basetype = temptype_basetype(inst->temptype);
  return spatial_srid(tinstant_value_p(inst), basetype);
}

/**
 * @ingroup meos_geo_srid
 * @brief Return the SRID of a spatiotemporal value
 * @return On error return @p SRID_INVALID
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Tspatial_srid()
 */
int32_t
tspatial_srid(const Temporal *temp)
{
  /* Ensure the validity of the arguments. The pgpointcloud temporal types
   * are accepted alongside the spatiotemporal ones: they are excluded from
   * tspatial_type() because their bounding box is a TPCBox rather than an
   * STBox, but a TPCBox begins with a whole STBox (see the static_asserts in
   * meos_pointcloud.h), so the SRID is read from the same offset. This
   * mirrors how temporal_boxops.c admits them beside tspatial_type(). */
#if POINTCLOUD
  if (! ensure_not_null((void *) temp))
    return SRID_INVALID;
  if (! tpointcloud_temptype(temp->temptype))
    VALIDATE_TSPATIAL(temp, SRID_INVALID);
#else
  VALIDATE_TSPATIAL(temp, SRID_INVALID);
#endif
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
 * @brief Set the coordinates of a spatiotemporal instant to an SRID
 * @param[in] inst Spatiotemporal instant
 * @param[in] srid SRID
 */
void
tspatialinst_set_srid(TInstant *inst, int32_t srid)
{
  assert(inst); assert(tspatial_type(inst->temptype));
  MeosType basetype = temptype_basetype(inst->temptype);
  spatial_set_srid(tinstant_value_p(inst), basetype, srid);
  return;
}

/**
 * @ingroup meos_internal_geo_srid
 * @brief Set the coordinates of a spatiotemporal sequence to an SRID
 * @param[in] seq Spatiotemporal sequence
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
 * @brief Set the coordinates of a spatiotemporal sequence set to an SRID
 * @param[in] ss Spatiotemporal sequence set
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
 * @brief Return a spatiotemporal value with the coordinates set to an SRID
 * @param[in] temp Spatiotemporal value
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

/*****************************************************************************Add commentMore actions
 * Defitions taken from file lwgeom_transform.c
 *****************************************************************************/

#if CBUFFER || POSE
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
#endif /* CBUFFER || POSE */

/*****************************************************************************
 * Functions fetching an LWPROJ structure containing transform information
 *****************************************************************************/

/**
 * @brief Reference ellipsoid of a geographic (lon/lat) SRID
 */
typedef struct
{
  int32_t srid;     /**< EPSG code of a geographic coordinate system */
  double a;         /**< Semi-major axis in metres */
  double rf;        /**< Inverse flattening */
} srid_ellipsoid;

/**
 * @brief Reference ellipsoids of the geographic SRIDs in common use, ordered
 * by SRID
 * @note The values are those of the `SPHEROID` clause of the `srtext` column
 * of the corresponding record of the `spatial_ref_sys` table, so that a lookup
 * here and a lookup through PROJ agree. Every entry is a geographic coordinate
 * system, which is what makes the table usable both for obtaining the
 * ellipsoid and for answering whether the SRID is geodetic. Projected
 * coordinate systems are deliberately absent: `EPSG:3857` and the like are
 * defined on an ellipsoid but are not lon/lat, and both functions below must
 * keep reporting them as not geodetic.
 */
static const srid_ellipsoid MEOS_SRID_ELLIPSOIDS[] =
{
  {4152, 6378137.0, 298.257222101},      /* NAD83(HARN), GRS 1980 */
  {4171, 6378137.0, 298.257222101},      /* RGF93, GRS 1980 */
  {4230, 6378388.0, 297.0},              /* ED50, International 1924 */
  {4258, 6378137.0, 298.257222101},      /* ETRS89, GRS 1980 */
  {4267, 6378206.4, 294.9786982138982},  /* NAD27, Clarke 1866 */
  {4269, 6378137.0, 298.257222101},      /* NAD83, GRS 1980 */
  {4283, 6378137.0, 298.257222101},      /* GDA94, GRS 1980 */
  {4322, 6378135.0, 298.26},             /* WGS 72 */
  {4326, 6378137.0, 298.257223563},      /* WGS 84 */
  {4617, 6378137.0, 298.257222101},      /* NAD83(CSRS), GRS 1980 */
  {4674, 6378137.0, 298.257222101},      /* SIRGAS 2000, GRS 1980 */
  {4759, 6378137.0, 298.257222101},      /* NAD83(NSRS2007), GRS 1980 */
};

/**
 * @brief Return 1 if the SRID is a geographic coordinate system with a
 * built-in reference ellipsoid, and in that case set the semi-major axis and
 * the inverse flattening, return 0 otherwise
 */
int
srid_builtin_ellipsoid(int32_t srid, double *a, double *rf)
{
  for (size_t i = 0; i < sizeof(MEOS_SRID_ELLIPSOIDS) /
    sizeof(MEOS_SRID_ELLIPSOIDS[0]); i++)
  {
    if (MEOS_SRID_ELLIPSOIDS[i].srid == srid)
    {
      *a = MEOS_SRID_ELLIPSOIDS[i].a;
      *rf = MEOS_SRID_ELLIPSOIDS[i].rf;
      return LW_SUCCESS;
    }
  }
  return LW_FAILURE;
}

/**
 * @brief Return 1 if the SRID is geodetic, return 0 otherwise
 */
int
srid_is_latlong(int32_t srid)
{
  /* Every SRID of the built-in table is a geographic coordinate system */
  double a, rf;
  if (srid_builtin_ellipsoid(srid, &a, &rf) == LW_SUCCESS)
    return LW_TRUE;

  LWPROJ *pj;
  if (lwproj_lookup(srid, srid, &pj) == LW_FAILURE)
    return LW_FALSE;
  return pj->source_is_latlong;
}


#if CBUFFER || POSE
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
    /* The error text comes from the context the transformation is created in.
     * proj_errno_string reads a global that another thread may be writing,
     * which PROJ states as the reason it replaces the function; MEOS answers
     * from several threads at once, so the text is read from the context */
#if POSTGIS_PROJ_VERSION >= 80
    const char *pj_errno_str = proj_context_errno_string(meos_proj_get_context(),
      pj_errno_val);
#else
    const char *pj_errno_str = proj_errno_string(pj_errno_val);
#endif /* POSTGIS_PROJ_VERSION >= 80 */
    meos_error(ERROR, MEOS_ERR_INVALID_ARG,
      "Transform: %s (%d)", pj_errno_str, pj_errno_val);
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
#endif /* CBUFFER || POSE */

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the first argument has been successfully transformed
 * to another SRID
 */
Datum
#if CBUFFER || POSE || RGEO
  datum_transf_pj(Datum d, MeosType basetype, int32_t srid_to,
    const LWPROJ *pj)
#else
  datum_transf_pj(Datum d, MeosType basetype, int32_t srid_to UNUSED,
    const LWPROJ *pj)
#endif /* CBUFFER || POSE || RGEO */
{
  assert(spatial_basetype(basetype));
  switch (basetype)
  {
#if POINTCLOUD
    case T_PCPOINT:
    case T_PCPATCH:
      /* A point cloud value holds its SRID by reference, so transforming it
       * would rewrite coordinates the schema its pcid names describes */
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "A %s cannot be transformed to another SRID",
        meostype_name(basetype));
      return (Datum) 0;
#endif
    case T_GEOMETRY:
    case T_GEOGRAPHY:
    {
      LWGEOM *geo = lwgeom_from_gserialized(DatumGetGserializedP(d));
      if (! lwgeom_transform(geo, (LWPROJ *) pj))
        return PointerGetDatum(NULL);
      geo->srid = srid_to;
      /* Re-compute bbox if input had one (COMPUTE_BBOX TAINTING) */
      if (geo->bbox)
        lwgeom_refresh_bbox(geo);
      Datum result = PointerGetDatum(geo_serialize(geo));
      lwgeom_free(geo);
      return result;
    }
#if CBUFFER
    case T_CBUFFER:
      return PointerGetDatum(cbuffer_transf_pj(DatumGetCbufferP(d), srid_to,
        pj));
#endif
#if POSE || RGEO
    case T_POSE:
      return PointerGetDatum(pose_transf_pj(DatumGetPoseP(d), srid_to, pj));
#endif
#if POSECHAIN
    case T_POSECHAIN:
      return PointerGetDatum(posechain_transf_pj(DatumGetPoseChainP(d),
        srid_to, pj));
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
    if (! datums[i])
    {
      pfree_array((void **) datums, i);
      return NULL;
    }
  }
  Set *result = set_make(datums, s->count, s->basetype, ORDER_NO);
  for (int i = 0; i < s->count; i++)
    pfree(DatumGetPointer(datums[i]));
  pfree(datums);
  return result;
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
  LWPROJ *pj;
  if (! lwproj_lookup(srid_from, srid_to, &pj))
    return NULL;

  /* Transform the geo set */
  return spatialset_transf_pj(s, srid_to, pj);
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
  LWPROJ *pj = lwproj_from_str_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the geo set */
  Set *result = spatialset_transf_pj(s, srid_to, pj);
  proj_destroy(pj->pj);
  pfree(pj);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a spatiotemporal type transformed to another SRID
 * @param[in] inst Spatiotemporal instant
 * @param[in] srid_to SRID, may be @p SRID_UNKNOWN for pipeline
 * transformations
 * @param[in] pj Information about the transformation
 */
TInstant *
tspatialinst_transf_pj(const TInstant *inst, int32_t srid_to, const LWPROJ *pj)
{
  assert(inst); assert(pj); assert(tspatial_type(inst->temptype));
  MeosType basetype = temptype_basetype(inst->temptype);
  /* The SRID of the geometry is set in the following function */
  Datum d = datum_transf_pj(tinstant_value_p(inst), basetype, srid_to, pj);
  if (! DatumGetPointer(d))
    return NULL;
  return tinstant_make_free(d, inst->temptype, inst->t);
}

/**
 * @brief Return a spatiotemporal type transformed to another SRID
 * @param[in] seq Spatiotemporal sequence
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
 * @brief Return a spatiotemporal type transformed to another SRID
 * @param[in] ss Spatiotemporal sequence set
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
 * @brief Return a spatiotemporal value transformed to another SRID
 * @param[in] temp Spatiotemporal
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
Temporal *
tspatial_transf_pj(const Temporal *temp, int32_t srid_to, const LWPROJ *pj)
{
  assert(temp); assert(pj);
  /* Copy the spatiotemporal value to transform its points in place */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tspatialinst_transf_pj((TInstant *) temp, srid_to,
        pj);
      break;
    case TSEQUENCE:
      return (Temporal *) tspatialseq_transf_pj((TSequence *) temp, srid_to,
        pj);
      break;
    default: /* TSEQUENCESET */
      return (Temporal *) tspatialseqset_transf_pj((TSequenceSet *) temp,
        srid_to, pj);
  }
}

#if RGEO
/**
 * @brief Ensure that a spatiotemporal value is not a temporal rigid geometry
 * @details A transformation rebuilds the value from its transformed instants,
 * while a temporal rigid geometry also carries a reference geometry beside
 * them, whose frame must agree with that of the poses
 * @param[in] temp Spatiotemporal value
 */
static bool
ensure_not_trgeometry(const Temporal *temp)
{
  assert(temp);
  if (temp->temptype == T_TRGEOMETRY)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Transformation to another SRID is not supported for type %s",
      meostype_name(temp->temptype));
    return false;
  }
  return true;
}
#endif /* RGEO */

/**
 * @ingroup meos_geo_srid
 * @brief Return a spatiotemporal value transformed to another SRID
 * @param[in] temp Spatiotemporal value
 * @param[in] srid_to Target SRID
 * @csqlfn #Tspatial_transform()
 */
Temporal *
tspatial_transform(const Temporal *temp, int32_t srid_to)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL);
#if RGEO
  if (! ensure_not_trgeometry(temp))
    return NULL;
#endif /* RGEO */
  int32_t srid_from = tspatial_srid(temp);
  if (! ensure_srid_known(srid_from) || ! ensure_srid_known(srid_to))
    return NULL;

  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return temporal_copy(temp);

  /* Get the structure with information about the projection */
  LWPROJ *pj;
  if (lwproj_lookup(srid_from, srid_to, &pj) == LW_FAILURE)
    return NULL;

  /* Function lwproj_get does not set pj->source_is_latlong */
  if (tgeodetic_type(temp->temptype))
    pj->source_is_latlong = true;

  /* Transform the spatiotemporal value */
  return tspatial_transf_pj(temp, srid_to, pj);
}

/**
 * @ingroup meos_geo_srid
 * @brief Return a spatiotemporal value transformed to another SRID using a
 * pipeline
 * @param[in] temp Spatiotemporal value
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 * @csqlfn #Tspatial_transform_pipeline()
 */
Temporal *
tspatial_transform_pipeline(const Temporal *temp, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSPATIAL(temp, NULL); VALIDATE_NOT_NULL(pipeline, NULL);
#if RGEO
  if (! ensure_not_trgeometry(temp))
    return NULL;
#endif /* RGEO */
  /* srid_to may legitimately be SRID_UNKNOWN for pipeline transformations:
   * the pipeline string itself encodes the destination CRS. So unlike the
   * sibling tspatial_transform path, we do NOT call ensure_srid_known here. */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_from_str_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the spatiotemporal type */
  Temporal *result = tspatial_transf_pj(temp, srid_to, pj);

  /* Clean up and return */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/*****************************************************************************/


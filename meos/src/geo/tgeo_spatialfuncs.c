/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Spatial functions for temporal geos
 */

#include "geo/geo_cluster.h"
#include "geo/tgeo_spatialfuncs.h"

/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
#include <utils/float.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeom_log.h>
#include <lwgeodetic.h>
#include <lwgeom_geos.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/stbox.h"
#include "geo/tgeo.h"
#include "geo/tgeo_distance.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
#endif
#if POSE
  #include "pose/pose.h"
  #include "pose/posechain.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
#endif
#if POINTCLOUD
  #include "pointcloud/pcpoint.h"
  #include "pointcloud/pcpatch.h"
  #include "pointcloud/meos_schema_hook.h"
#endif

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * @brief Return a 4D point from a datum
 * @note The M dimension is ignored
 */
void
datum_point4d(Datum value, POINT4D *p)
{
  const GSERIALIZED *gs = DatumGetGserializedP(value);
  memset(p, 0, sizeof(POINT4D));
  if (FLAGS_GET_Z(gs->gflags))
  {
    const POINT3DZ *point = (POINT3DZ *) GS_POINT_PTR(gs);
    p->x = point->x;
    p->y = point->y;
    p->z = point->z;
  }
  else
  {
    const POINT2D *point = (POINT2D *) GS_POINT_PTR(gs);
    p->x = point->x;
    p->y = point->y;
  }
  return;
}


/*****************************************************************************/



/**
 * @brief Return true if the points are equal
 */
bool
datum_point_eq(Datum point1, Datum point2)
{
  const GSERIALIZED *gs1 = DatumGetGserializedP(point1);
  const GSERIALIZED *gs2 = DatumGetGserializedP(point2);
  if (gserialized_get_srid(gs1) != gserialized_get_srid(gs2) ||
      FLAGS_GET_Z(gs1->gflags) != FLAGS_GET_Z(gs2->gflags) ||
      FLAGS_GET_GEODETIC(gs1->gflags) != FLAGS_GET_GEODETIC(gs2->gflags))
    return false;
  return geopoint_eq(gs1, gs2);
}


/**
 * @brief Return true if the points are equal, the caller having established
 * that they share a reference system, a dimensionality and a geodetic flag
 * @details The internal twin of #datum_point_eq, for the walks that compare one
 * instant after another. The entry of such a walk establishes the condition
 * once -- `ensure_valid_tgeo_tgeo` refuses two temporal points whose SRIDs
 * differ -- so reading the SRID out of both serializations per element pays
 * again for a question already answered. The condition set is the external
 * form's, asserted rather than tested, and `NDEBUG` removes it from a release
 * build while a debug build checks it
 */
bool
datum_point_eq_intl(Datum point1, Datum point2)
{
  const GSERIALIZED *gs1 = DatumGetGserializedP(point1);
  const GSERIALIZED *gs2 = DatumGetGserializedP(point2);
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));
  assert(FLAGS_GET_Z(gs1->gflags) == FLAGS_GET_Z(gs2->gflags));
  assert(FLAGS_GET_GEODETIC(gs1->gflags) == FLAGS_GET_GEODETIC(gs2->gflags));
  return geopoint_eq(gs1, gs2);
}

/**
 * @brief Return true if the points are equal taking into account floating 
 * point imprecision
 */
bool
datum_point_same(Datum point1, Datum point2)
{
  const GSERIALIZED *gs1 = DatumGetGserializedP(point1);
  const GSERIALIZED *gs2 = DatumGetGserializedP(point2);
  if (gserialized_get_srid(gs1) != gserialized_get_srid(gs2) ||
      FLAGS_GET_Z(gs1->gflags) != FLAGS_GET_Z(gs2->gflags) ||
      FLAGS_GET_GEODETIC(gs1->gflags) != FLAGS_GET_GEODETIC(gs2->gflags))
    return false;
  return geopoint_same(gs1, gs2);
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_eq(Datum point1, Datum point2)
{
  return BoolGetDatum(datum_point_eq(point1, point2));
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_ne(Datum point1, Datum point2)
{
  return BoolGetDatum(! datum_point_eq(point1, point2));
}

/**
 * @brief Return the centroid of a geometry
 */
Datum
datum2_geom_centroid(Datum geo)
{
  return GserializedPGetDatum(geom_centroid(DatumGetGserializedP(geo)));
}

/**
 * @brief Return the centroid of a geography on the spheroid or on the sphere
 * @details The geography twin of #datum2_geom_centroid, which takes the model
 * of the earth as the parameter of the lift
 */
Datum
datum2_geog_centroid(Datum geo, Datum spheroid)
{
  return GserializedPGetDatum(geog_centroid(DatumGetGserializedP(geo),
    DatumGetBool(spheroid)));
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Select the appropriate distance function
 * @details The distance of two geographies takes the model of the earth as a
 * parameter, which the distance of two geometries does not, so the function
 * is applied through #geo_distance_lfinfo
 */
varfunc
geo_distance_fn(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    return (varfunc) &datum_geog_distance;
  else
    return MEOS_FLAGS_GET_Z(flags) ?
      (varfunc) &datum_geom_distance3d : (varfunc) &datum_geom_distance2d;
}

/**
 * @brief Select the appropriate distance function for two points
 * @details As #geo_distance_fn, applied through #pt_distance_lfinfo
 */
varfunc
pt_distance_fn(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    return (varfunc) &datum_geog_distance;
  else
    return MEOS_FLAGS_GET_Z(flags) ?
      (varfunc) &datum_pt_distance3d : (varfunc) &datum_pt_distance2d;
}

/**
 * @brief Set in a lifted structure the distance function selected by
 * #geo_distance_fn and its parameters: none for two geometries, the model of
 * the earth for two geographies
 * @param[in] flags Flags of the spatial values
 * @param[in] spheroid True when measuring on the spheroid, false on the
 * sphere; read for geographies only
 * @param[out] lfinfo Lifted structure
 */
void
geo_distance_lfinfo(int16 flags, bool spheroid, LiftedFunctionInfo *lfinfo)
{
  lfinfo->func = geo_distance_fn(flags);
  lfinfo->numparam = MEOS_FLAGS_GET_GEODETIC(flags) ? 1 : 0;
  lfinfo->param[0] = BoolGetDatum(spheroid);
}

/**
 * @brief Set in a lifted structure the distance function selected by
 * #pt_distance_fn and its parameters, as #geo_distance_lfinfo does
 */
void
pt_distance_lfinfo(int16 flags, bool spheroid, LiftedFunctionInfo *lfinfo)
{
  lfinfo->func = pt_distance_fn(flags);
  lfinfo->numparam = MEOS_FLAGS_GET_GEODETIC(flags) ? 1 : 0;
  lfinfo->param[0] = BoolGetDatum(spheroid);
}

/**
 * @brief Return the distance between two spatial values with the function
 * #geo_distance_lfinfo sets and its parameters, for a caller applying it
 * directly rather than through a lift
 * @param[in] value1,value2 Spatial values
 * @param[in] flags Flags of the spatial values
 * @param[in] spheroid True when measuring on the spheroid, false on the
 * sphere; read for geographies only
 */
Datum
datum_geo_distance(Datum value1, Datum value2, int16 flags, bool spheroid)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  geo_distance_lfinfo(flags, spheroid, &lfinfo);
  return tfunc_base_base(value1, value2, &lfinfo);
}

/**
 * @brief Return the distance between two points with the function
 * #pt_distance_lfinfo sets and its parameters, as #datum_geo_distance does
 */
Datum
datum_pt_distance(Datum value1, Datum value2, int16 flags, bool spheroid)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  pt_distance_lfinfo(flags, spheroid, &lfinfo);
  return tfunc_base_base(value1, value2, &lfinfo);
}

/**
 * @brief Return the 2D distance between the two geometries
 * @pre For PostGIS version > 3 the geometries are NOT toasted
 */
Datum
datum_geom_distance2d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(geom_distance2d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return the 3D distance between the two geometries
 */
Datum
datum_geom_distance3d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(geom_distance3d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return the distance between the two geographies on the spheroid or
 * on the sphere
 * @details The geography twin of #datum_geom_distance2d, which takes the
 * model of the earth as the parameter of the lift
 */
Datum
datum_geog_distance(Datum geog1, Datum geog2, Datum spheroid)
{
  return Float8GetDatum(geog_distance(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), DatumGetBool(spheroid)));
}

/**
 * @brief Return the 2D distance between the two geometry points
 * @details The distance of two points is a question about their coordinates,
 * answered by #point_distance_exact as the double nearest the exact distance
 */
Datum
datum_pt_distance2d(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(geom1);
  const POINT2D *p2 = DATUM_POINT2D_P(geom2);
  const double a[2] = {p1->x, p1->y}, b[2] = {p2->x, p2->y};
  return Float8GetDatum(point_distance_exact(a, b, 2));
}

/**
 * @brief Return the 3D distance between the two geometry points
 * @details Answered by #point_distance_exact, as #datum_pt_distance2d is
 */
Datum
datum_pt_distance3d(Datum geom1, Datum geom2)
{
  const POINT3DZ *p1 = DATUM_POINT3DZ_P(geom1);
  const POINT3DZ *p2 = DATUM_POINT3DZ_P(geom2);
  const double a[3] = {p1->x, p1->y, p1->z}, b[3] = {p2->x, p2->y, p2->z};
  return Float8GetDatum(point_distance_exact(a, b, 3));
}

/*****************************************************************************/

/**
 * @brief Get the MEOS flags from a geo value
 */
static int16
gserialized_flags(const GSERIALIZED *gs)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  MEOS_FLAGS_SET_Z(result, FLAGS_GET_Z(gs->gflags));
  MEOS_FLAGS_SET_GEODETIC(result, FLAGS_GET_GEODETIC(gs->gflags));
  return result;
}

#if CBUFFER
/**
 * @brief Get the MEOS flags from a circular buffer
 */
static int16
cbuffer_flags(void)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  return result;
}
#endif /* CBUFFER */ 

#if NPOINT
/**
 * @brief Get the MEOS flags from a network point
 */
static int16
npoint_flags(void)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  return result;
}
#endif /* NPOINT */ 

#if POSE 
/**
 * @brief Get the MEOS flags from a pose
 */
static int16
pose_flags(Pose *pose)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  MEOS_FLAGS_SET_Z(result, MEOS_FLAGS_GET_Z(pose->flags));
  MEOS_FLAGS_SET_GEODETIC(result, MEOS_FLAGS_GET_GEODETIC(pose->flags));
  return result;
}
#endif /* POSE */

#if POSE
/**
 * @brief Get the MEOS flags from a pose chain
 */
static int16
posechain_flags(const PoseChain *pc)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  MEOS_FLAGS_SET_Z(result, MEOS_FLAGS_GET_Z(pc->flags));
  MEOS_FLAGS_SET_GEODETIC(result, MEOS_FLAGS_GET_GEODETIC(pc->flags));
  return result;
}
#endif /* POSE */

#if H3
/**
 * @brief Get the MEOS flags from an H3 cell index
 */
static int16
h3index_flags(void)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  MEOS_FLAGS_SET_GEODETIC(result, true);
  return result;
}
#endif /* H3 */

#if QUADBIN
/**
 * @brief Get the MEOS flags from a quadbin cell index
 */
static int16
quadbin_flags(void)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  /* Quadbin cells are planar (Web-Mercator slippy tiles), not geodetic */
  return result;
}
#endif
#if S2CELL
/**
 * @brief Return the flags of an S2 cell
 */
int16
s2cell_flags(void)
{
  int16 result = 0; /* Set all flags to false */
  MEOS_FLAGS_SET_X(result, true);
  /* An S2 cell is defined on the sphere, so it is geodetic */
  MEOS_FLAGS_SET_GEODETIC(result, true);
  return result;
}
#endif /* QUADBIN */

#if POINTCLOUD
/**
 * @brief Get the MEOS flags from a point cloud point
 * @note A TPCBox begins with a complete STBox (see the static_asserts in
 * meos_pointcloud.h), so its flags byte IS the value's MEOS spatial flags;
 * this reads the flags off the pcpoint's bounding box rather than deriving
 * them independently
 * @note Which dimensions a pcpoint holds is stated by the schema its pcid
 * names, so where none resolves the value declares the one property that does
 * not depend on it: that it has coordinates. Every question that must decode
 * one asks through @ref meos_pc_schema, which states the miss there
 */
static int16
pcpoint_flags(const Pcpoint *pt)
{
  PCSCHEMA *schema = meos_pc_schema_lookup(pcpoint_get_pcid(pt));
  if (! schema)
  {
    int16 result = 0;
    MEOS_FLAGS_SET_X(result, true);
    return result;
  }
  TPCBox *box = pcpoint_to_tpcbox(pt, schema);
  if (! box)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not compute the bounding box of a pcpoint");
    return -1;
  }
  int16 result = box->flags;
  pfree(box);
  return result;
}

/**
 * @brief Get the MEOS flags from a point cloud patch
 * @note A TPCBox begins with a complete STBox (see the static_asserts in
 * meos_pointcloud.h), so its flags byte IS the value's MEOS spatial flags;
 * this reads the flags off the pcpatch's bounding box rather than deriving
 * them independently
 */
static int16
pcpatch_flags(const Pcpatch *pa)
{
  int32_t srid = meos_pc_schema_srid(pcpatch_get_pcid(pa));
  TPCBox *box = pcpatch_to_tpcbox(pa, srid);
  if (! box)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Could not compute the bounding box of a pcpatch");
    return -1;
  }
  int16 result = box->flags;
  pfree(box);
  return result;
}
#endif /* POINTCLOUD */

/**
 * @brief Get the MEOS flags from a spatial value
 */
int16
spatial_flags(Datum d, MeosType basetype)
{
  /* The pgpointcloud base types are deliberately outside spatial_basetype():
   * their flags are read from their TPCBox rather than derived like the
   * other spatial_* dispatchers, so the catalog-wide predicate stays
   * unchanged and only this single dispatcher admits them, through the
   * catalog's own pointcloud_basetype() predicate — the base-type sibling
   * of tpointcloud_temptype(), which the boxops dispatch already uses the
   * same way to admit tpcpoint/tpcpatch beside tspatial_type(). */
#if POINTCLOUD
  assert(spatial_basetype(basetype) || pointcloud_basetype(basetype));
#else
  assert(spatial_basetype(basetype));
#endif
  switch (basetype)
  {
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return gserialized_flags(DatumGetGserializedP(d));
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_flags();
#endif
#if NPOINT
    case T_NPOINT:
      return npoint_flags();
#endif
#if POSE
    case T_POSE:
      return pose_flags(DatumGetPoseP(d));
    case T_POSECHAIN:
      return posechain_flags(DatumGetPoseChainP(d));
#endif
#if QUADBIN
    case T_QUADBIN:
      (void) d;
      return quadbin_flags();
#endif
#if S2CELL
    case T_S2CELL:
      (void) d;
      return s2cell_flags();
#endif
#if H3
    case T_H3INDEX:
      (void) d;
      return h3index_flags();
#endif
#if POINTCLOUD
    case T_PCPOINT:
      return pcpoint_flags(DatumGetPcpointP(d));
    case T_PCPATCH:
      return pcpatch_flags(DatumGetPcpatchP(d));
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown spatial flags function for type: %s", meostype_name(basetype));
    return -1;
  }
}

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure that the spatial constraints required for operating on two
 * temporal geometries are satisfied
 */
bool
ensure_spatial_validity(const Temporal *temp1, const Temporal *temp2)
{
  if (tspatial_type(temp1->temptype) && tspatial_type(temp2->temptype) &&
      (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
       ! ensure_same_dimensionality(temp1->flags, temp2->flags)))
    return false;
  return true;
}





/**
 * @brief Ensure that the spatiotemporal argument and the geometry/geography
 * have the same type of coordinates, either planar or geodetic
 */
bool
ensure_same_geodetic_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (MEOS_FLAGS_GET_GEODETIC(temp->flags) != FLAGS_GET_GEODETIC(gs->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed planar and geodetic coordinates");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a spatial set and a geometry/geography are both planar
 * or both geodetic
 * @param[in] s Spatial set
 * @param[in] gs Geometry/geography
 */
bool
ensure_same_geodetic_set_geo(const Set *s, const GSERIALIZED *gs)
{
  if (MEOS_FLAGS_GET_GEODETIC(s->flags) != FLAGS_GET_GEODETIC(gs->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed planar and geodetic coordinates");
    return false;
  }
  return true;
}

/**
 * @brief Return true if a set and a geometry/geography are valid for set
 * operations
 * @param[in] s Set
 * @param[in] gs Value
 */
bool
ensure_valid_geoset_geo(const Set *s, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_GEOSET(s, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_not_empty(gs) ||
      ! ensure_same_srid(spatialset_srid(s), geo_srid(gs)) ||
      ! ensure_same_geodetic_set_geo(s, gs))
    return false;
  return true;
}







/**
 * @brief Return true if a spatiotemporal value and a geometry/geography have
 * thesame dimensionality
 */
bool
same_dimensionality_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (MEOS_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->gflags))
    return false;
  return true;
}

/**
 * @brief Ensure that a spatiotemporal value and a geometry/geography have 
 * the same dimensionality
 */
bool
ensure_same_dimensionality_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs)
{
  if (same_dimensionality_tspatial_geo(temp, gs))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Operation on mixed 2D/3D dimensions");
  return false;
}

/**
 * @brief Ensure that a spatiotemporal box and a geometry/geography have the
 * same spatial dimensionality
 */
bool
ensure_same_spatial_dimensionality_stbox_geo(const STBox *box,
  const GSERIALIZED *gs)
{
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      (MEOS_FLAGS_GET_Z(box->flags) != FLAGS_GET_Z(gs->gflags)))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Operation on mixed 2D/3D dimensions");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a spatiotemporal box and a geometry/geography have both
 * planar or geodetic coordinates
 */
bool
ensure_same_geodetic_stbox_geo(const STBox *box, const GSERIALIZED *gs)
{
  if (! MEOS_FLAGS_GET_X(box->flags) || 
      (MEOS_FLAGS_GET_GEODETIC(box->flags) != FLAGS_GET_GEODETIC(gs->gflags)))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Operation on mixed planar and geodetic coordinates");
    return false;
  }
  return true;
}











/*****************************************************************************/

/**
 * @brief Ensure the validity of a spatiotemporal box and a geometry
 */
bool
ensure_valid_stbox_geo(const STBox *box, const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(box, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_has_X(T_STBOX, box->flags) || gserialized_is_empty(gs) ||
      ! ensure_same_srid(box->srid, gserialized_get_srid(gs)) ||
      ! ensure_same_geodetic_stbox_geo(box, gs))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a spatiotemporal value and a 
 * geometry/geography
 * @note The geometry can be empty since some functions such atGeometry or
 * minusGeometry return different result on empty geometries.
 */
bool
ensure_valid_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TSPATIAL(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs))
    return false;
  return true;
}


/**
 * @brief Ensure the validity of a temporal geo and a spatiotemporal box
 */
bool
ensure_valid_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TGEO(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal geo and a spatiotemporal box
 */
bool
ensure_valid_tgeo_stbox(const Temporal *temp, const STBox *box)
{
  VALIDATE_TGEO(temp, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_same_srid(tspatial_srid(temp), stbox_srid(box)) ||
      ! ensure_same_geodetic(temp->flags, box->flags))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two temporal geos
 */
bool
ensure_valid_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TGEO(temp1, false); VALIDATE_TGEO(temp2, false); 
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return false;
  return true;
}

/**
 * @brief Ensure that two temporal numbers have the same span type
 * @param[in] temp1,temp2 Temporal values
 */
bool
ensure_valid_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TSPATIAL(temp1, false); VALIDATE_TSPATIAL(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return false;
  return true;
}


/*****************************************************************************
 * Conversion functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box
 *****************************************************************************/

/**
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geometry/geography transformed from/to a temporal
 * geometry/geography
 * @param[in] inst Temporal geo instant
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @errval NULL
 * @sqlop @p ::
 */
TInstant *
tgeominst_tgeoginst(const TInstant *inst, bool oper)
{
  assert(inst); assert(tgeo_type_all(inst->temptype));
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
  GSERIALIZED *res;
  if (oper == TGEOMP_TO_TGEOGP)
    res = geom_to_geog(gs);
  else
    res = geog_to_geom(gs);
  /* The conversion fails, for example, when the coordinate system of a
   * geometry is not a lon/lat one, which is required for a geography */
  if (! res)
    return NULL;
  MeosType temptype;
  if (oper == TGEOMP_TO_TGEOGP)
    temptype = (inst->temptype == T_TGEOMPOINT) ? T_TGEOGPOINT : T_TGEOGRAPHY;
  else
    temptype = (inst->temptype == T_TGEOGPOINT) ? T_TGEOMPOINT : T_TGEOMETRY;
  return tinstant_make_free(PointerGetDatum(res), temptype, inst->t);
}

/**
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geometry/geography transformed from/to a temporal
 * geometry/geography
 * @param[in] seq Temporal geo sequence
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @errval NULL
 * @sqlop @p ::
 */
TSequence *
tgeomseq_tgeogseq(const TSequence *seq, bool oper)
{
  assert(seq); assert(tgeo_type_all(seq->temptype));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    instants[i] = tgeominst_tgeoginst(TSEQUENCE_INST_N(seq, i), oper);
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
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geometry/geography transformed from/to a temporal
 * geometry/geography
 * @param[in] ss Temporal point sequence set
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @errval NULL
 * @sqlop @p ::
 */
TSequenceSet *
tgeomseqset_tgeogseqset(const TSequenceSet *ss, bool oper)
{
  assert(ss); assert(tgeo_type_all(ss->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    sequences[i] = tgeomseq_tgeogseq(TSEQUENCESET_SEQ_N(ss, i), oper);
    if (! sequences[i])
    {
      pfree_array((void **) sequences, i);
      return NULL;
    }
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geometry/geography transformed from/to a temporal
 * geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @errval NULL
 * @see #tgeominst_tgeoginst
 * @see #tgeomseq_tgeogseq
 * @see #tgeomseqset_tgeogseqset
 * @sqlop @p ::
 */
Temporal *
tgeom_tgeog(const Temporal *temp, bool oper)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeominst_tgeoginst((TInstant *) temp,
        oper);
    case TSEQUENCE:
      return (Temporal *) tgeomseq_tgeogseq((TSequence *) temp,
        oper);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeomseqset_tgeogseqset(
        (TSequenceSet *) temp, oper);
  }
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geography from a temporal geometry
 * @param[in] temp Temporal geo
 * @errval NULL
 * @csqlfn #Tgeometry_to_tgeography()
 */
Temporal *
tgeometry_to_tgeography(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOM(temp, NULL);
  return tgeom_tgeog(temp, TGEOM_TO_TGEOG);
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geometry from to a temporal geography
 * @param[in] temp Temporal point
 * @errval NULL
 * @csqlfn #Tgeography_to_tgeometry()
 */
Temporal *
tgeography_to_tgeometry(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOG(temp, NULL);
  return tgeom_tgeog(temp, TGEOG_TO_TGEOM);
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geography point from a temporal geometry point
 * @param[in] temp Temporal point
 * @errval NULL
 * @csqlfn #Tgeompoint_to_tgeogpoint()
 */
Temporal *
tgeompoint_to_tgeogpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);
  return tgeom_tgeog(temp, TGEOM_TO_TGEOG);
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geometry point from a temporal geography point
 * @param[in] temp Temporal point
 * @errval NULL
 * @csqlfn #Tgeogpoint_to_tgeompoint()
 */
Temporal *
tgeogpoint_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL);
  return tgeom_tgeog(temp, TGEOG_TO_TGEOM);
}

/*****************************************************************************/

/**
 * @brief Ensure that all geometries composing a temporal geo are points
 * @param[in] inst Temporal instant
 */
bool
ensure_tgeoinst_point_type(const TInstant *inst)
{
  assert(inst);
  if (! ensure_point_type(DatumGetGserializedP(tinstant_value_p(inst))))
    return false;
  return true;
}

/**
 * @brief Ensure that all geometries composing a temporal geo are points
 * @param[in] seq Temporal sequence 

 */
bool
ensure_tgeoseq_point_type(const TSequence *seq)
{
  assert(seq);
  for (int i = 0; i < seq->count; i++)
    if (! ensure_tgeoinst_point_type(TSEQUENCE_INST_N(seq, i)))
      return false;
  return true;
}

/**
 * @brief Ensure that all geometries composing a temporal geo are points
 * @param[in] ss Temporal sequence set

 */
bool
ensure_tgeoseqset_point_type(const TSequenceSet *ss)
{
  assert(ss);
  for (int i = 0; i < ss->count; i++)
    if (! ensure_tgeoseq_point_type(TSEQUENCESET_SEQ_N(ss, i)))
      return false;
  return true;
}

/**
 * @brief Ensure that all geometries composing a temporal geo are points
 * @param[in] temp Temporal geo
 */
bool
ensure_tgeo_point_type(const Temporal *temp)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return ensure_tgeoinst_point_type((TInstant *) temp);
    case TSEQUENCE:
      return ensure_tgeoseq_point_type((TSequence *) temp);
    default: /* TSEQUENCESET */
      return ensure_tgeoseqset_point_type((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geo transformed from/to a temporal point
 * @param[in] inst Temporal instant
 * @param[in] oper True when transforming from temporal geo to temporal point,
 * false otherwise
 * @sqlop @p ::
 */
TInstant *
tgeoinst_tpointinst(const TInstant *inst, bool oper)
{
  assert(inst);
  if (oper == TGEO_TO_TPOINT)
    assert(tgeo_type_all(inst->temptype));
  else /* oper == TPOINT_TO_TGEO */
    assert(tpoint_type(inst->temptype));

  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
  if (oper == TGEO_TO_TPOINT && ! ensure_point_type(gs))
    return NULL;

  MeosType temptype;
  if (oper == TGEO_TO_TPOINT)
    temptype = (inst->temptype == T_TGEOMETRY) ? T_TGEOMPOINT : T_TGEOGPOINT;
  else
    temptype = (inst->temptype == T_TGEOMPOINT) ? T_TGEOMETRY : T_TGEOGRAPHY;
  return tinstant_make(PointerGetDatum(gs), temptype, inst->t);
}

/**
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geo transformed from/to a temporal point
 * @param[in] seq Temporal sequence 
 * @param[in] oper True when transforming from temporal geo to temporal point,
 * false otherwise
 * @sqlop @p ::
 */
TSequence *
tgeoseq_tpointseq(const TSequence *seq, bool oper)
{
  assert(seq);
  if (oper == TGEO_TO_TPOINT)
    assert(tgeo_type_all(seq->temptype));
  else /* oper == TPOINT_TO_TGEO */
    assert(tpoint_type(seq->temptype));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tgeoinst_tpointinst(TSEQUENCE_INST_N(seq, i), oper);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geo transformed from/to a temporal point
 * @param[in] ss Temporal sequence set
 * @param[in] oper True when transforming from temporal geo to temporal point,
 * false otherwise
 * @sqlop @p ::
 */
TSequenceSet *
tgeoseqset_tpointseqset(const TSequenceSet *ss, bool oper)
{
  assert(ss);
  if (oper == TGEO_TO_TPOINT)
    assert(tgeo_type_all(ss->temptype));
  else /* oper == TPOINT_TO_TGEO */
    assert(tpoint_type(ss->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tgeoseq_tpointseq(TSEQUENCESET_SEQ_N(ss, i), oper);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_geo_conversion
 * @brief Return a temporal geo transformed from/to a temporal point
 * @param[in] temp Temporal value
 * @param[in] oper True when transforming from temporal geo to temporal point,
 * false otherwise
 * @errval NULL
 * @see #tgeoinst_tpointinst
 * @see #tgeoseq_tpointseq
 * @see #tgeoseqset_tpointseqset
 * @sqlop @p ::
 */
Temporal *
tgeo_tpoint(const Temporal *temp, bool oper)
{
  /* Ensure the validity of the arguments */
  if ((oper == TGEO_TO_TPOINT && (! ensure_tgeo_type_all(temp->temptype) ||
         ! ensure_tgeo_point_type(temp))) ||
      (oper == TPOINT_TO_TGEO && ! ensure_tpoint_type(temp->temptype)))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeoinst_tpointinst((TInstant *) temp, oper);
    case TSEQUENCE:
      return (Temporal *) tgeoseq_tpointseq((TSequence *) temp, oper);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeoseqset_tpointseqset((TSequenceSet *) temp, oper);
  }
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geometry point from a temporal geometry
 * @param[in] temp Temporal geometry
 * @csqlfn #Tgeo_to_tpoint()
 */
Temporal *
tgeometry_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMETRY(temp, NULL);
  return tgeo_tpoint(temp, TGEO_TO_TPOINT);
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geography point from a temporal geography
 * @param[in] temp Temporal geography
 * @csqlfn #Tgeo_to_tpoint()
 */
Temporal *
tgeography_to_tgeogpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGRAPHY(temp, NULL);
  return tgeo_tpoint(temp, TGEO_TO_TPOINT);
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geometry from a temporal geometry point
 * @param[in] temp Temporal geometry point
 * @csqlfn #Tpoint_to_tgeo()
 */
Temporal *
tgeompoint_to_tgeometry(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL);
  return tgeo_tpoint(temp, TPOINT_TO_TGEO);
}

/**
 * @ingroup meos_geo_conversion
 * @brief Return a temporal geography from a temporal geography point
 * @param[in] temp Temporal geography point
 * @csqlfn #Tpoint_to_tgeo()
 */
Temporal *
tgeogpoint_to_tgeography(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL);
  return tgeo_tpoint(temp, TPOINT_TO_TGEO);
}

/*****************************************************************************
 * Affine functions
 *****************************************************************************/

/**
 * @brief Set the coefficients of an affine transformation
 * @details The coefficients follow PostGIS ST_Affine
 */
static void
affine_set(AFFINE *aff, double a, double b, double c, double d, double e,
  double f, double g, double h, double i, double xoff, double yoff,
  double zoff)
{
  aff->afac = a; aff->bfac = b; aff->cfac = c;
  aff->dfac = d; aff->efac = e; aff->ffac = f;
  aff->gfac = g; aff->hfac = h; aff->ifac = i;
  aff->xoff = xoff; aff->yoff = yoff; aff->zoff = zoff;
  return;
}

/**
 * @brief Return true if an affine transformation moves a point of the plane
 * z = 0 out of it
 * @details The height of the image of a point (x, y, 0) is g x + h y + zoff,
 * so the transformation keeps the plane when the three are zero
 */
static bool
affine_leaves_plane(const AFFINE *a)
{
  return a->gfac != 0.0 || a->hfac != 0.0 || a->zoff != 0.0;
}

/**
 * @brief Return a geometry moved by an affine transformation, read in three
 * dimensions when the transformation is
 * @details A two-dimensional geometry moved by a transformation leaving the
 * plane is placed at z = 0 and moved in three dimensions, as a pose moved by
 * a three-dimensional frame is (#pose_motion), so the height the
 * transformation gives it is kept rather than dropped. The geometry read
 * from a serialization shares its coordinates, which lwgeom_affine rewrites
 * in place, so it is read from a copy, as in #geo_transform
 * @param[in] gs Geometry
 * @param[in] a Affine transformation
 * @param[in] lift True when the transformation is three-dimensional
 */
static GSERIALIZED *
geo_affine_lift(const GSERIALIZED *gs, const AFFINE *a, bool lift)
{
  GSERIALIZED *copy = geo_copy(gs);
  LWGEOM *geo = lwgeom_from_gserialized(copy);
  if (lift && ! FLAGS_GET_Z(gs->gflags))
  {
    LWGEOM *geo3d = lwgeom_force_3dz(geo, 0.0);
    lwgeom_free(geo);
    geo = geo3d;
  }
  lwgeom_affine(geo, a);
  GSERIALIZED *result = geo_serialize(geo);
  lwgeom_free(geo);
  pfree(copy);
  return result;
}

/**
 * @brief Return the affine transformation of a temporal geo instant
 * (iterator function)
 * @param[in] inst Temporal geo
 * @param[in] a Affine transformation
 * @param[in] lift True when the transformation is three-dimensional
 * @param[out] result Result
 */
static void
tgeoinst_affine_iter(const TInstant *inst, const AFFINE *a, bool lift,
  TInstant **result)
{
  assert(inst); assert(a); assert(tgeo_type_all(inst->temptype));
  GSERIALIZED *gs = geo_affine_lift(
    DatumGetGserializedP(tinstant_value_p(inst)), a, lift);
  *result = tinstant_make_free(PointerGetDatum(gs), inst->temptype, inst->t);
  return;
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return the affine transformation of a temporal geo instant
 * @param[in] inst Temporal geo
 * @param[in] a Affine transformation
 */
static TInstant *
tgeoinst_affine(TInstant *inst, const AFFINE *a, bool lift)
{
  assert(inst); assert(a); assert(tgeo_type_all(inst->temptype));
  TInstant *result;
  tgeoinst_affine_iter(inst, a, lift, &result);
  return result;
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return the affine transform a temporal geo sequence
 * @param[in] seq Temporal geo
 * @param[in] a Affine transformation
 */
static TSequence *
tgeoseq_affine(const TSequence *seq, const AFFINE *a, bool lift)
{
  assert(seq); assert(a); assert(tgeo_type_all(seq->temptype));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tgeoinst_affine_iter(TSEQUENCE_INST_N(seq, i), a, lift, &instants[i]);
  /* Construct the result */
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return the affine transformation of a temporal geo sequence set
 * @param[in] ss Temporal geo
 * @param[in] a Affine transformation
 */
static TSequenceSet *
tgeoseqset_affine(const TSequenceSet *ss, const AFFINE *a, bool lift)
{
  assert(ss); assert(a); assert(tgeo_type_all(ss->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tgeoseq_affine(TSEQUENCESET_SEQ_N(ss, i), a, lift);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return the affine transformation of a temporal geo, read in three
 * dimensions when the transformation is
 * @param[in] temp Temporal geo
 * @param[in] a Affine transformation
 * @param[in] lift True when the transformation is three-dimensional
 */
static Temporal *
tgeo_affine_lift(const Temporal *temp, const AFFINE *a, bool lift)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeoinst_affine((TInstant *) temp, a, lift);
    case TSEQUENCE:
      return (Temporal *) tgeoseq_affine((TSequence *) temp, a, lift);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeoseqset_affine((TSequenceSet *) temp, a, lift);
  }
}

/*****************************************************************************/

/**
 * @brief Return a geometry scaled by given factors
 * @details The geometry read from a serialization shares its coordinates,
 * which lwgeom_scale rewrites in place, so it is read from a copy, as in
 * #geo_transform
 * @param[in] gs Geometry
 * @param[in] factors Scale factors
 */
static GSERIALIZED *
geo_scale_factors(const GSERIALIZED *gs, const POINT4D *factors)
{
  GSERIALIZED *copy = geo_copy(gs);
  LWGEOM *geom = lwgeom_from_gserialized(copy);
  lwgeom_scale(geom, factors);
  GSERIALIZED *result = geo_serialize(geom);
  lwgeom_free(geom);
  pfree(copy);
  return result;
}

/**
 * @brief Return the scale transformation of a temporal geo instant
 * (iterator function)
 * @param[in] inst Temporal geo
 * @param[in] factors Scale factors
 * @param[out] result Result
 */
static void
tgeoinst_scale_iter(const TInstant *inst, const POINT4D *factors,
  TInstant **result)
{
  GSERIALIZED *gs = geo_scale_factors(
    DatumGetGserializedP(tinstant_value_p(inst)), factors);
  *result = tinstant_make_free(PointerGetDatum(gs), inst->temptype, inst->t);
  return;
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return a temporal geo instant scaled by given factors
 * @param[in] inst Temporal geo
 * @param[in] factors Scale factors
 */
static TInstant *
tgeoinst_scale(const TInstant *inst, const POINT4D *factors)
{
  TInstant *result;
  tgeoinst_scale_iter(inst, factors, &result);
  return result;
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return a temporal geo sequence scaled by given factors
 * @param[in] seq Temporal geo
 * @param[in] factors Scale factors
 */
static TSequence *
tgeoseq_scale(const TSequence *seq, const POINT4D *factors)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tgeoinst_scale_iter(TSEQUENCE_INST_N(seq, i), factors, &instants[i]);
  /* Construct the result */
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return a temporal geo sequence scaled by given factors
 * @param[in] ss Temporal geo
 * @param[in] factors Scale factors
 */
static TSequenceSet *
tgeoseqset_scale(const TSequenceSet *ss, const POINT4D *factors)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tgeoseq_scale(TSEQUENCESET_SEQ_N(ss, i), factors);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return a temporal geo scaled by given factors, the scaling
 * #tgeo_scale_geo and #tgeo_scale share
 * @param[in] temp Temporal geo
 * @param[in] factors Scale factors
 */
static Temporal *
tgeo_scale_factors(const Temporal *temp, const POINT4D *factors)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeoinst_scale((TInstant *) temp, factors);
    case TSEQUENCE:
      return (Temporal *) tgeoseq_scale((TSequence *) temp, factors);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeoseqset_scale((TSequenceSet *) temp, factors);
  }
}

/**
 * @ingroup meos_geo_transf
 * @brief Scale a temporal geo by given factors
 * @param[in] temp Temporal geo
 * @param[in] scale Geometry for the scale factors
 * @param[in] sorigin Point geometry for the origin, may be `NULL`
 * @csqlfn #Tgeo_scale_geo()
 */
Temporal *
tgeo_scale_geo(const Temporal *temp, const GSERIALIZED *scale,
  const GSERIALIZED *sorigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(scale, NULL);
  if (! ensure_not_geodetic(temp->flags) ||
      ! ensure_point_type(scale) || gserialized_is_empty(scale) ||
      (sorigin && 
        (gserialized_is_empty(sorigin) || ! ensure_point_type(sorigin))))
    return NULL;

  bool translate = false;
  AFFINE aff;

  /* Transform the scale input */
  POINT4D factors;
  datum_point4d(PointerGetDatum(scale), &factors);
  if (! FLAGS_GET_Z(scale->gflags))
    factors.z = 1.0;
  /* We don't use the M value */
  factors.m = 1.0;

  /* Do we have the optional false origin? */
  POINT4D origin;
  if (sorigin)
  {
    datum_point4d(PointerGetDatum(sorigin), &origin);
    translate = true;
  }

  /* A two-dimensional value scaled about an origin off its plane leaves
   * the plane when the vertical factor moves it, so it is read in three
   * dimensions, as #geo_affine_lift reads it */
  bool lift = translate && FLAGS_GET_Z(sorigin->gflags) && factors.z != 1.0;

  /* If we have false origin, translate to it before scaling */
  Temporal *temp1;
  if (translate)
  {
    /* Initialize affine */
    memset(&aff, 0, sizeof(AFFINE));
    /* Set rotation/scale/sheer matrix to no-op */
    aff.afac = aff.efac = aff.ifac = 1.0;
    /* Strip false origin from all coordinates */
    aff.xoff = -1 * origin.x;
    aff.yoff = -1 * origin.y;
    aff.zoff = -1 * origin.z;
    temp1 = tgeo_affine_lift(temp, &aff, lift);
  }
  else
    temp1 = (Temporal *) temp;

  /* Scale the temporal geo, moved to the origin when there is one */
  Temporal *temp2 = tgeo_scale_factors(temp1, &factors);

  /* Return to original origin after scaling */
  Temporal *temp3;
  if (translate)
  {
    aff.xoff *= -1;
    aff.yoff *= -1;
    aff.zoff *= -1;
    temp3 = tgeo_affine_lift(temp2, &aff, lift);
  }
  else
    temp3 = temp2;

  /* Cleanup and return */
  if (translate)
  {
    pfree(temp1);
    pfree(temp2);
  }
  return temp3;
}

/**
 * @ingroup meos_geo_transf
 * @brief Scale a temporal geo by the given factors along the x, y, and z axes
 * @param[in] temp Temporal geo
 * @param[in] xfactor,yfactor,zfactor Scale factors
 * @csqlfn #Tgeo_scale()
 */
Temporal *
tgeo_scale(const Temporal *temp, double xfactor, double yfactor,
  double zfactor)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  if (! ensure_not_geodetic(temp->flags))
    return NULL;
  /* The factors of a 3D point, as scale(temp, ST_MakePoint(x, y, z)) reads
   * them */
  POINT4D factors = { .x = xfactor, .y = yfactor, .z = zfactor, .m = 1.0 };
  return tgeo_scale_factors(temp, &factors);
}

/**
 * @brief Return the affine transformation of a temporal geo given the
 * coefficients of its matrix
 * @details The coefficients follow PostGIS ST_Affine, as #tgeo_affine takes
 * them. A geodetic value is refused, as #tgeo_affine refuses it
 * @param[in] temp Temporal geo
 * @param[in] lift True when the transformation is three-dimensional
 */
static Temporal *
tgeo_affine_coefs(const Temporal *temp, bool lift,
  double a, double b, double c, double d, double e, double f,
  double g, double h, double i, double xoff, double yoff, double zoff)
{
  if (! ensure_not_geodetic(temp->flags))
    return NULL;
  AFFINE aff;
  affine_set(&aff, a, b, c, d, e, f, g, h, i, xoff, yoff, zoff);
  return tgeo_affine_lift(temp, &aff, lift);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return the affine transformation of a temporal geo, which translates,
 * rotates and scales it in one step
 * @details The coefficients follow PostGIS ST_Affine. A two-dimensional value
 * moved by a transformation leaving the plane becomes three-dimensional
 * (#geo_affine_lift). An affine map of longitude and latitude is no
 * transformation of the sphere, so a geodetic value is refused
 * @param[in] temp Temporal geo
 * @param[in] a,b,c,d,e,f,g,h,i Coefficients of the 3x3 matrix
 * @param[in] xoff,yoff,zoff Translation
 * @csqlfn #Tgeo_affine(), #Tgeo_affine_2d()
 */
Temporal *
tgeo_affine(const Temporal *temp, double a, double b, double c,
  double d, double e, double f, double g, double h, double i, double xoff,
  double yoff, double zoff)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  if (! ensure_not_geodetic(temp->flags))
    return NULL;
  AFFINE aff;
  affine_set(&aff, a, b, c, d, e, f, g, h, i, xoff, yoff, zoff);
  return tgeo_affine_lift(temp, &aff, affine_leaves_plane(&aff));
}

/**
 * @ingroup meos_geo_transf
 * @brief Return a temporal geo translated by the given offsets
 * @param[in] temp Temporal geo
 * @param[in] deltax,deltay,deltaz Offsets
 * @csqlfn #Tgeo_translate()
 */
Temporal *
tgeo_translate(const Temporal *temp, double deltax, double deltay,
  double deltaz)
{
  VALIDATE_TGEO(temp, NULL);
  return tgeo_affine_coefs(temp, deltaz != 0.0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
    deltax, deltay, deltaz);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return a temporal geo rotated counter-clockwise around a point
 * @param[in] temp Temporal geo
 * @param[in] angle Rotation angle in radians
 * @param[in] x0,y0 Center of the rotation
 * @csqlfn #Tgeo_rotate()
 */
Temporal *
tgeo_rotate(const Temporal *temp, double angle, double x0, double y0)
{
  VALIDATE_TGEO(temp, NULL);
  double c = cos(angle), s = sin(angle);
  return tgeo_affine_coefs(temp, false, c, -s, 0, s, c, 0, 0, 0, 1,
    x0 - c * x0 + s * y0, y0 - s * x0 - c * y0, 0);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return a temporal geo rotated counter-clockwise around a point
 * geometry
 * @details The rotation of #tgeo_rotate about the coordinates of the point,
 * whose origin #tgeo_scale_geo reads the same way: the point is not empty, and it
 * states the SRID of the value or none
 * @param[in] temp Temporal geo
 * @param[in] angle Rotation angle in radians
 * @param[in] origin Center of the rotation
 * @csqlfn #Tgeo_rotate_geo()
 */
Temporal *
tgeo_rotate_geo(const Temporal *temp, double angle, const GSERIALIZED *origin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(origin, NULL);
  if (! ensure_point_type(origin) || ! ensure_not_empty(origin))
    return NULL;
  int32_t srid = gserialized_get_srid(origin);
  if (srid != SRID_UNKNOWN && ! ensure_same_srid(tspatial_srid(temp), srid))
    return NULL;
  POINT4D p;
  datum_point4d(PointerGetDatum(origin), &p);
  return tgeo_rotate(temp, angle, p.x, p.y);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return a temporal geo rotated counter-clockwise around the x axis
 * @param[in] temp Temporal geo
 * @param[in] angle Rotation angle in radians
 * @csqlfn #Tgeo_rotate_x()
 */
Temporal *
tgeo_rotate_x(const Temporal *temp, double angle)
{
  VALIDATE_TGEO(temp, NULL);
  double c = cos(angle), s = sin(angle);
  return tgeo_affine_coefs(temp, true, 1, 0, 0, 0, c, -s, 0, s, c, 0, 0, 0);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return a temporal geo rotated counter-clockwise around the y axis
 * @param[in] temp Temporal geo
 * @param[in] angle Rotation angle in radians
 * @csqlfn #Tgeo_rotate_y()
 */
Temporal *
tgeo_rotate_y(const Temporal *temp, double angle)
{
  VALIDATE_TGEO(temp, NULL);
  double c = cos(angle), s = sin(angle);
  return tgeo_affine_coefs(temp, true, c, 0, s, 0, 1, 0, -s, 0, c, 0, 0, 0);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return a temporal geo rotated counter-clockwise around the z axis
 * @param[in] temp Temporal geo
 * @param[in] angle Rotation angle in radians
 * @csqlfn #Tgeo_rotate_z()
 */
Temporal *
tgeo_rotate_z(const Temporal *temp, double angle)
{
  VALIDATE_TGEO(temp, NULL);
  double c = cos(angle), s = sin(angle);
  return tgeo_affine_coefs(temp, false, c, -s, 0, s, c, 0, 0, 0, 1, 0, 0, 0);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return a temporal geo translated and then scaled
 * @param[in] temp Temporal geo
 * @param[in] deltax,deltay Offsets
 * @param[in] xfactor,yfactor Scale factors
 * @csqlfn #Tgeo_transscale()
 */
Temporal *
tgeo_transscale(const Temporal *temp, double deltax, double deltay,
  double xfactor, double yfactor)
{
  VALIDATE_TGEO(temp, NULL);
  return tgeo_affine_coefs(temp, false, xfactor, 0, 0, 0, yfactor, 0, 0, 0, 1,
    deltax * xfactor, deltay * yfactor, 0);
}

/*****************************************************************************
 * Affine functions over a geometry
 *
 * The family of PostGIS ST_Affine over a geometry, computed by the kernels of
 * the temporal one: a two-dimensional geometry moved by a transformation
 * leaving the plane becomes three-dimensional (#geo_affine_lift), and a
 * geography is refused, an affine map of longitude and latitude being no
 * transformation of the sphere.
 *****************************************************************************/

/**
 * @brief Return the affine transformation of a geometry given the
 * coefficients of its matrix
 * @details The coefficients follow PostGIS ST_Affine, as #geo_affine takes
 * them
 * @param[in] gs Geometry
 * @param[in] lift True when the transformation is three-dimensional
 */
static GSERIALIZED *
geo_affine_coefs(const GSERIALIZED *gs, bool lift,
  double a, double b, double c, double d, double e, double f,
  double g, double h, double i, double xoff, double yoff, double zoff)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;
  AFFINE aff;
  affine_set(&aff, a, b, c, d, e, f, g, h, i, xoff, yoff, zoff);
  return geo_affine_lift(gs, &aff, lift);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return the affine transformation of a geometry, which translates,
 * rotates and scales it in one step
 * @details The coefficients follow PostGIS ST_Affine
 * @param[in] gs Geometry
 * @param[in] a,b,c,d,e,f,g,h,i Coefficients of the 3x3 matrix
 * @param[in] xoff,yoff,zoff Translation
 * @csqlfn #Geo_affine(), #Geo_affine_2d()
 */
GSERIALIZED *
geo_affine(const GSERIALIZED *gs, double a, double b, double c,
  double d, double e, double f, double g, double h, double i, double xoff,
  double yoff, double zoff)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;
  AFFINE aff;
  affine_set(&aff, a, b, c, d, e, f, g, h, i, xoff, yoff, zoff);
  return geo_affine_lift(gs, &aff, affine_leaves_plane(&aff));
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry translated by the given offsets
 * @param[in] gs Geometry
 * @param[in] deltax,deltay,deltaz Offsets
 * @csqlfn #Geo_translate()
 */
GSERIALIZED *
geo_translate(const GSERIALIZED *gs, double deltax, double deltay,
  double deltaz)
{
  return geo_affine_coefs(gs, deltaz != 0.0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
    deltax, deltay, deltaz);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry rotated counter-clockwise around a point
 * @param[in] gs Geometry
 * @param[in] angle Rotation angle in radians
 * @param[in] x0,y0 Center of the rotation
 * @csqlfn #Geo_rotate()
 */
GSERIALIZED *
geo_rotate(const GSERIALIZED *gs, double angle, double x0, double y0)
{
  double c = cos(angle), s = sin(angle);
  return geo_affine_coefs(gs, false, c, -s, 0, s, c, 0, 0, 0, 1,
    x0 - c * x0 + s * y0, y0 - s * x0 - c * y0, 0);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry rotated counter-clockwise around a point geometry
 * @details The rotation of #geo_rotate about the coordinates of the point,
 * read as #tgeo_rotate_geo reads it: the point is not empty, and it states
 * the SRID of the geometry or none
 * @param[in] gs Geometry
 * @param[in] angle Rotation angle in radians
 * @param[in] origin Center of the rotation
 * @csqlfn #Geo_rotate_geo()
 */
GSERIALIZED *
geo_rotate_geo(const GSERIALIZED *gs, double angle, const GSERIALIZED *origin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_NOT_NULL(origin, NULL);
  if (! ensure_point_type(origin) || ! ensure_not_empty(origin))
    return NULL;
  int32_t srid = gserialized_get_srid(origin);
  if (srid != SRID_UNKNOWN &&
      ! ensure_same_srid(gserialized_get_srid(gs), srid))
    return NULL;
  POINT4D p;
  datum_point4d(PointerGetDatum(origin), &p);
  return geo_rotate(gs, angle, p.x, p.y);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry rotated counter-clockwise around the x axis
 * @param[in] gs Geometry
 * @param[in] angle Rotation angle in radians
 * @csqlfn #Geo_rotate_x()
 */
GSERIALIZED *
geo_rotate_x(const GSERIALIZED *gs, double angle)
{
  double c = cos(angle), s = sin(angle);
  return geo_affine_coefs(gs, true, 1, 0, 0, 0, c, -s, 0, s, c, 0, 0, 0);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry rotated counter-clockwise around the y axis
 * @param[in] gs Geometry
 * @param[in] angle Rotation angle in radians
 * @csqlfn #Geo_rotate_y()
 */
GSERIALIZED *
geo_rotate_y(const GSERIALIZED *gs, double angle)
{
  double c = cos(angle), s = sin(angle);
  return geo_affine_coefs(gs, true, c, 0, s, 0, 1, 0, -s, 0, c, 0, 0, 0);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry rotated counter-clockwise around the z axis
 * @param[in] gs Geometry
 * @param[in] angle Rotation angle in radians
 * @csqlfn #Geo_rotate_z()
 */
GSERIALIZED *
geo_rotate_z(const GSERIALIZED *gs, double angle)
{
  double c = cos(angle), s = sin(angle);
  return geo_affine_coefs(gs, false, c, -s, 0, s, c, 0, 0, 0, 1, 0, 0, 0);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Scale a geometry by the factors a point states, about an origin
 * @details The scaling of #tgeo_scale_geo: the factors and the origin are
 * non-empty points, the factor of a point without Z is 1 along z, and a
 * two-dimensional geometry scaled about an origin off its plane leaves the
 * plane when the vertical factor moves it
 * @param[in] gs Geometry
 * @param[in] scale Point geometry stating the scale factors
 * @param[in] sorigin Point geometry for the origin, may be `NULL`
 * @csqlfn #Geo_scale_geo()
 */
GSERIALIZED *
geo_scale_geo(const GSERIALIZED *gs, const GSERIALIZED *scale,
  const GSERIALIZED *sorigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_NOT_NULL(scale, NULL);
  if (! ensure_not_geodetic_geo(gs) ||
      ! ensure_point_type(scale) || gserialized_is_empty(scale) ||
      (sorigin &&
        (gserialized_is_empty(sorigin) || ! ensure_point_type(sorigin))))
    return NULL;

  POINT4D factors;
  datum_point4d(PointerGetDatum(scale), &factors);
  if (! FLAGS_GET_Z(scale->gflags))
    factors.z = 1.0;
  factors.m = 1.0;
  if (! sorigin)
    return geo_scale_factors(gs, &factors);

  POINT4D origin;
  datum_point4d(PointerGetDatum(sorigin), &origin);
  bool lift = FLAGS_GET_Z(sorigin->gflags) && factors.z != 1.0;
  AFFINE aff;
  affine_set(&aff, 1, 0, 0, 0, 1, 0, 0, 0, 1, -origin.x, -origin.y,
    -origin.z);
  GSERIALIZED *gs1 = geo_affine_lift(gs, &aff, lift);
  GSERIALIZED *gs2 = geo_scale_factors(gs1, &factors);
  affine_set(&aff, 1, 0, 0, 0, 1, 0, 0, 0, 1, origin.x, origin.y, origin.z);
  GSERIALIZED *result = geo_affine_lift(gs2, &aff, lift);
  pfree(gs1); pfree(gs2);
  return result;
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Scale a geometry by the given factors along the x, y, and z axes
 * @param[in] gs Geometry
 * @param[in] xfactor,yfactor,zfactor Scale factors
 * @csqlfn #Geo_scale()
 */
GSERIALIZED *
geo_scale(const GSERIALIZED *gs, double xfactor, double yfactor,
  double zfactor)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;
  POINT4D factors = { .x = xfactor, .y = yfactor, .z = zfactor, .m = 1.0 };
  return geo_scale_factors(gs, &factors);
}

/**
 * @ingroup meos_geo_base_transf
 * @brief Return a geometry translated and then scaled
 * @param[in] gs Geometry
 * @param[in] deltax,deltay Offsets
 * @param[in] xfactor,yfactor Scale factors
 * @csqlfn #Geo_transscale()
 */
GSERIALIZED *
geo_transscale(const GSERIALIZED *gs, double deltax, double deltay,
  double xfactor, double yfactor)
{
  return geo_affine_coefs(gs, false, xfactor, 0, 0, 0, yfactor, 0, 0, 0, 1,
    deltax * xfactor, deltay * yfactor, 0);
}

/*****************************************************************************
 * Convex hull functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_accessor
 * @brief Return the convex hull of a temporal geo
 * @param[in] temp Temporal geo
 * @errval NULL
 * @csqlfn #Tgeo_convex_hull()
 */
GSERIALIZED *
tgeo_convex_hull(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  GSERIALIZED *traj = tpoint_type(temp->temptype) ?
    tpoint_trajectory(temp, UNARY_UNION_NO) :
    tgeo_traversed_area(temp, UNARY_UNION_NO);
  GSERIALIZED *result = geom_convex_hull(traj);
  pfree(traj);
  return result;
}

/*****************************************************************************
 * Traversed area functions
 *****************************************************************************/

/**
 * @brief Return the distinct values a temporal value takes, gathered into one
 * geometry
 * @details A value that does not interpolate holds each of its values for the
 * whole of its own interval and covers nothing between them, so the set of
 * values it takes IS the set of points it occupies. That answers the traversed
 * area of a temporal geometry and the trajectory of a temporal point alike,
 * which is why both entries read it rather than one borrowing the other
 * @param[in] temp Temporal value
 * @param[in] unary_union True to dissolve the values into one geometry
 */
GSERIALIZED *
geo_values_collect(const Temporal *temp, bool unary_union)
{
  assert(temp);
  /* Get the array of pointers to the component values */
  int count;
  Datum *values = temporal_values_p(temp, &count);
  /* The array arrives sorted and free of duplicates: that is the contract
   * #temporal_values_p states and each of its three subtype functions
   * establishes, so the values are collected in the order they are given */
  GSERIALIZED **gsarr = palloc(sizeof(GSERIALIZED *) * count);
  for (int i = 0; i < count; i++)
    gsarr[i] = DatumGetGserializedP(values[i]);
  GSERIALIZED *res = geoarr_collect(gsarr, count);
  pfree(values); pfree(gsarr);
  if (! unary_union)
    return res;
  GSERIALIZED *result = geom_unary_union(res, -1);
  pfree(res); 
  return result;
}

/**
 * @ingroup meos_geo_accessor
 * @brief Return the traversed area of a temporal geo that is not a point
 * @details The values a temporal geo takes are its placements, and it holds
 * each for the whole of its own interval, so they are also the area it
 * traverses. A temporal point has no area and answers #tpoint_trajectory()
 * instead
 * @note The geodetic case reaches the same planar dissolve as the planar one,
 * which is a property of this entry that predates the point split and is not
 * settled by it
 * @param[in] temp Temporal geo
 * @param[in] unary_union True when the PostGIS ST_UnaryUnion function is
 * applied to the result
 * @csqlfn #Tgeo_traversed_area()
 */
GSERIALIZED *
tgeo_traversed_area(const Temporal *temp, bool unary_union)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  /* A temporal point has no area: #tpoint_trajectory() is what answers it,
   * and it reads #geo_values_collect() directly rather than through here */
  if (tpoint_type(temp->temptype))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be a temporal geometry or geography");
    return NULL;
  }
  if (! ensure_nonlinear_interp(temp->flags))
    return NULL;
  return geo_values_collect(temp, unary_union);
}

/**
 * @ingroup meos_geo_accessor
 * @brief Return the centroid of a temporal geo as a temporal point
 * @param[in] temp Temporal geo
 * @param[in] spheroid True when computing the centroid of a temporal
 * geography on the spheroid, false on the sphere, as #geog_centroid reads it
 * @csqlfn #Tgeo_centroid()
 */
Temporal *
tgeo_centroid(const Temporal *temp, bool spheroid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);

  bool geodetic = MEOS_FLAGS_GET_GEODETIC(temp->flags);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  /* The centroid of a geography takes the model of the earth as the
   * parameter of the lift, that of a geometry none */
  if (geodetic)
  {
    lfinfo.func = (varfunc) &datum2_geog_centroid;
    lfinfo.numparam = 1;
    lfinfo.param[0] = BoolGetDatum(spheroid);
  }
  else
    lfinfo.func = (varfunc) &datum2_geom_centroid;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = geodetic ? T_TGEOGPOINT : T_TGEOMPOINT;
  /* Centroid is affine in vertex positions: linear input -> linear output */
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return an array of integers specifying the cluster number assigned to
 * the input geometries using the k-means algorithm
 * @param[in] geoms Geometries
 * @param[in] n Number of elements in the input array
 * @param[in] k Number of clusters
 * @param[out] count Number of elements in the output array
 * @note PostGIS function: @p ST_ClusterKMeans(PG_FUNCTION_ARGS)
 */
int *
geo_cluster_kmeans(const GSERIALIZED **geoms, uint32_t n, uint32_t k,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geoms, NULL);
  if (! ensure_positive(n) || ! ensure_positive(k))
    return NULL;
  if (n < k)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "K (%d) must be smaller than the number of input geometries (%d)",
      k, n);
    return NULL;
  }

  /* The members of the array carry one SRID */
  if (! ensure_same_srid_geoarr(geoms, (int) n))
    return NULL;

  /* Read all the input geometries into a list */
  LWGEOM **lwgeoms = palloc(sizeof(LWGEOM *) * n);
  for (uint32_t i = 0; i < n; i++)
    lwgeoms[i] = lwgeom_from_gserialized(geoms[i]);

  /* Calculate k-means on the list */
  int *result = lwgeom_cluster_kmeans((const LWGEOM **) lwgeoms, n, k, 0.0);

  /* Clean up and return */
  for (uint32_t i = 0; i < n; i++)
    if (lwgeoms[i])
      lwgeom_free(lwgeoms[i]);
  pfree(lwgeoms);
  *count = (int) n;
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return an array of integers specifying the cluster number assigned to
 * the input geometries using the DBSCAN algorithm
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of elements in the input array
 * @param[in] tolerance Tolerance
 * @param[in] minpoints Minimum number of points
 * @param[out] count Number of elements in the output array
 * @note PostGIS function: @p ST_ClusterDBSCAN(PG_FUNCTION_ARGS)
 */
uint32_t *
geo_cluster_dbscan(const GSERIALIZED **geoms, uint32_t ngeoms,
  double tolerance, int minpoints, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL); 
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geoms, NULL);
  if (! ensure_positive(ngeoms))
    return NULL;
  if (tolerance < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Tolerance must be a positive number, got %g", tolerance);
    return NULL;
  }
  if (minpoints < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Minpoints must be a positive number, got %d", minpoints);
    return NULL;
  }

  /* The members of the array carry one SRID */
  if (! ensure_same_srid_geoarr(geoms, (int) ngeoms))
    return NULL;

  uint32_t i;
  char *is_in_cluster = NULL;
  LWGEOM **lwgeoms = lwalloc(ngeoms * sizeof(LWGEOM *));
  UNIONFIND *uf = UF_create(ngeoms);
  for (i = 0; i < ngeoms; i++)
    lwgeoms[i] = lwgeom_from_gserialized(geoms[i]);

  /* The clustering of liblwgeom reaches GEOS only for the index narrowing
   * the pairs whose distance it computes, which the bounding boxes answer */
  bool success = geo_union_dbscan(lwgeoms, ngeoms, uf, tolerance,
    (uint32_t) minpoints, minpoints > 1 ? &is_in_cluster : NULL);

  for (i = 0; i < ngeoms; i++)
    lwgeom_free(lwgeoms[i]);
  lwfree(lwgeoms);

  if (! success)
  {
    UF_destroy(uf);
    if (is_in_cluster)
      lwfree(is_in_cluster);
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR, "Error during clustering");
    return NULL;
  }

  uint32_t *result_ids = UF_get_collapsed_cluster_ids(uf, is_in_cluster);
  *count = uf->N;
  UF_destroy(uf);
  if (is_in_cluster)
    lwfree(is_in_cluster);
  return result_ids;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return an array of GeometryCollections partitioning the input
 * geometries into connected clusters that are disjoint
 * @details Each geometry in a cluster intersects at least one other geometry
 * in the cluster, and does not intersect any geometry in other clusters
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of elements in the input array
 * @param[out] count Number of elements in the output array
 * @note PostGIS function: @p ST_ClusterIntersectingWin(PG_FUNCTION_ARGS)
 */
GSERIALIZED ** 
geo_cluster_intersecting(const GSERIALIZED **geoms, uint32_t ngeoms,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL); 
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geoms, NULL);
  if (! ensure_positive(ngeoms))
    return NULL;

  uint32_t i;

  /* TODO short-circuit for one element? */

  /* The clustering of liblwgeom reaches GEOS for the index narrowing the pairs
   * whose intersection it asks about, and for the predicate itself. The
   * bounding boxes answer the narrowing and the native engine answers the
   * predicate */
  if (! ensure_same_srid_geoarr(geoms, (int) ngeoms))
    return NULL;

  LWGEOM **lwgeoms = palloc(ngeoms * sizeof(LWGEOM *));
  for (i = 0; i < ngeoms; i++)
    lwgeoms[i] = lwgeom_from_gserialized(geoms[i]);
  LWGEOM **lw_results;
  uint32_t nclusters;
  bool ok = geo_cluster_intersecting_geoms(lwgeoms, ngeoms, &lw_results,
    &nclusters);
  /* The collections take ownership of the geometries they are built from */
  pfree(lwgeoms);
  if (! ok)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR, "Error during clustering");
    return NULL;
  }
  GSERIALIZED **result = palloc(nclusters * sizeof(GSERIALIZED *));
  for (i = 0; i < nclusters; i++)
  {
    result[i] = geo_serialize(lw_results[i]);
    lwgeom_free(lw_results[i]);
  }
  lwfree(lw_results);
  *count = (int) nclusters;
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return an array of GeometryCollections clustering the input
 * geometries by distance
 * @details Each geometry of a cluster is within the specified distance of at
 * least one other geometry of the same cluster.
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of elements in the input array
 * @param[in] tolerance Tolerance
 * @param[out] count Number of elements in the output array
 * @note PostGIS function: @p ST_ClusterWithin(PG_FUNCTION_ARGS)
 */
GSERIALIZED **
geo_cluster_within(const GSERIALIZED **geoms, uint32_t ngeoms,
  double tolerance, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL); 
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geoms, NULL);
  if (! ensure_positive(ngeoms))
    return NULL;
  if (tolerance < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Tolerance must be a positive number, got %g", tolerance);
    return NULL;
  }

  /* The members of the array carry one SRID */
  if (! ensure_same_srid_geoarr(geoms, (int) ngeoms))
    return NULL;

  uint32_t i;
  LWGEOM **lwgeoms = lwalloc(ngeoms * sizeof(LWGEOM *));
  for (i = 0; i < ngeoms; i++)
    lwgeoms[i] = lwgeom_from_gserialized(geoms[i]);

  LWGEOM **lw_results;
  uint32_t nclusters;
  bool success =
    geo_cluster_within_distance(lwgeoms, ngeoms, tolerance, &lw_results,
      &nclusters);
  /* don't need to destroy items because GeometryCollections have taken ownership */
  pfree(lwgeoms);

  if (! success)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR, "Error during clustering");
    return NULL;
  }
  if (!lw_results)
    return NULL;

  GSERIALIZED **result = palloc(nclusters * sizeof(GSERIALIZED *));
  for (i = 0; i < nclusters; ++i)
  {
    result[i] = geo_serialize(lw_results[i]);
    lwgeom_free(lw_results[i]);
  }
  lwfree(lw_results);
  *count = nclusters;
  return result;
}

/*****************************************************************************/

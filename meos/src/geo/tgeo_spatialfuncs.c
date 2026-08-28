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
#endif
#if POSECHAIN
  #include "posechain/posechain.h"
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

#if CBUFFER || RGEO
/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_ne(Datum point1, Datum point2)
{
  return BoolGetDatum(! datum_point_eq(point1, point2));
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_same(Datum point1, Datum point2)
{
  return BoolGetDatum(datum_point_same(point1, point2));
}

/**
 * @brief Return true if the points are equal
 */
Datum
datum2_point_nsame(Datum point1, Datum point2)
{
  return BoolGetDatum(! datum_point_same(point1, point2));
}
#endif /* CBUFFER || RGEO */

/**
 * @brief Return the centroid of a geometry
 */
Datum
datum2_geom_centroid(Datum geo)
{
  return GserializedPGetDatum(geom_centroid(DatumGetGserializedP(geo)));
}

/**
 * @brief Return the centroid of a geography
 */
Datum
datum2_geog_centroid(Datum geo)
{
  return GserializedPGetDatum(geog_centroid(DatumGetGserializedP(geo),
    BoolGetDatum(false)));
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Select the appropriate distance function
 */
datum_func2
geo_distance_fn(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    return &datum_geog_distance;
  else
    return MEOS_FLAGS_GET_Z(flags) ?
      &datum_geom_distance3d : &datum_geom_distance2d;
}

/**
 * @brief Select the appropriate distance function
 */
datum_func2
pt_distance_fn(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    return &datum_geog_distance;
  else
    return MEOS_FLAGS_GET_Z(flags) ?
      &datum_pt_distance3d : &datum_pt_distance2d;
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
 * @brief Return the distance between the two geographies
 */
Datum
datum_geog_distance(Datum geog1, Datum geog2)
{
  return Float8GetDatum(geog_distance(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2)));
}

/**
 * @brief Return the 2D distance between the two geometry points
 */
Datum
datum_pt_distance2d(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(geom1);
  const POINT2D *p2 = DATUM_POINT2D_P(geom2);
  return Float8GetDatum(distance2d_pt_pt(p1, p2));
}

/**
 * @brief Return the 3D distance between the two geometry points
 */
Datum
datum_pt_distance3d(Datum geom1, Datum geom2)
{
  const POINT3DZ *p1 = DATUM_POINT3DZ_P(geom1);
  const POINT3DZ *p2 = DATUM_POINT3DZ_P(geom2);
  return Float8GetDatum(distance3d_pt_pt((POINT3D *) p1, (POINT3D *) p2));
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

#if POSE || RGEO 
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
#endif /* POSE || RGEO */

#if POSECHAIN
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
#endif /* POSECHAIN */

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
#if POSE || RGEO
    case T_POSE:
      return pose_flags(DatumGetPoseP(d));
#endif
#if POSECHAIN
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
 * @brief Ensure the validity of a spatiotemporal boxes
 */
bool
ensure_valid_spatial_stbox_stbox(const STBox *box1, const STBox *box2)
{
  VALIDATE_NOT_NULL(box1, false); VALIDATE_NOT_NULL(box2, false);
  if (! ensure_has_X(T_STBOX, box1->flags) ||
      ! ensure_has_X(T_STBOX, box2->flags) ||
      ! ensure_same_srid(stbox_srid(box1), stbox_srid(box2)) ||
      ! ensure_same_geodetic(box1->flags, box2->flags))
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
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) &&
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
 * @return On error return `NULL`
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
 * @return On error return `NULL`
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
 * @return On error return `NULL`
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
 * @return On error return `NULL`
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
 * @return On error return `NULL`
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
 * @return On error return `NULL`
 * @csqlfn #Tgeography_to_tgeometry()
 */
Temporal *
tgeography_to_tgeometry(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOG(temp, NULL);
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
 * @return On error return `NULL`
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
 * @brief Return the affine transformation of a temporal geo instant
 * (iterator function)
 * @param[in] inst Temporal geo
 * @param[in] a Affine transformation
 * @param[out] result Result
 */
static void
tgeoinst_affine_iter(const TInstant *inst, const AFFINE *a, TInstant **result)
{
  assert(inst); assert(a); assert(tgeo_type_all(inst->temptype));
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
  LWGEOM *geo = lwgeom_from_gserialized(gs);
  lwgeom_affine(geo, a);
  GSERIALIZED *gs1 = geo_serialize(geo);
  *result = tinstant_make_free(PointerGetDatum(gs1), inst->temptype, inst->t);
  lwgeom_free(geo);
  return;
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return the affine transformation of a temporal geo instant
 * @param[in] inst Temporal geo
 * @param[in] a Affine transformation
 */
static TInstant *
tgeoinst_affine(TInstant *inst, const AFFINE *a)
{
  assert(inst); assert(a); assert(tgeo_type_all(inst->temptype));
  TInstant *result;
  tgeoinst_affine_iter(inst, a, &result);
  return result;
}

/**
 * @ingroup meos_internal_geo_transf
 * @brief Return the affine transform a temporal geo sequence
 * @param[in] seq Temporal geo
 * @param[in] a Affine transformation
 */
static TSequence *
tgeoseq_affine(const TSequence *seq, const AFFINE *a)
{
  assert(seq); assert(a); assert(tgeo_type_all(seq->temptype));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tgeoinst_affine_iter(TSEQUENCE_INST_N(seq, i), a, &instants[i]);
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
tgeoseqset_affine(const TSequenceSet *ss, const AFFINE *a)
{
  assert(ss); assert(a); assert(tgeo_type_all(ss->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tgeoseq_affine(TSEQUENCESET_SEQ_N(ss, i), a);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup meos_geo_transf
 * @brief Return the 3D affine transform of a temporal geo to do things like
 * translate, rotate, scale in one step
 * @param[in] temp Temporal geo
 * @param[in] a Matrix specifying the transformation
 * @csqlfn #Tgeo_affine()
 */
Temporal *
tgeo_affine(const Temporal *temp, const AFFINE *a)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(a, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeoinst_affine((TInstant *) temp, a);
    case TSEQUENCE:
      return (Temporal *) tgeoseq_affine((TSequence *) temp, a);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeoseqset_affine((TSequenceSet *) temp, a);
  }
}

/*****************************************************************************/

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
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  lwgeom_scale(geom, factors);
  GSERIALIZED *gs1 = geo_serialize(geom);
  lwgeom_free(geom);
  *result = tinstant_make_free(PointerGetDatum(gs1), inst->temptype, inst->t);
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
 * @ingroup meos_geo_transf
 * @brief Scale a temporal geo by given factors
 * @param[in] temp Temporal geo
 * @param[in] scale Geometry for the scale factors
 * @param[in] sorigin Point geometry for the origin, may be `NULL`
 * @csqlfn #Tgeo_scale()
 */
Temporal *
tgeo_scale(const Temporal *temp, const GSERIALIZED *scale, 
  const GSERIALIZED *sorigin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(scale, NULL);
  if (! ensure_point_type(scale) || gserialized_is_empty(scale) ||
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
    temp1 = tgeo_affine(temp, &aff);
  }
  else
    temp1 = (Temporal *) temp;

  /* Scale the temporal point */
  Temporal *temp2;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      temp2 = (Temporal *) tgeoinst_scale((TInstant *) temp, &factors);
      break;
    case TSEQUENCE:
      temp2 = (Temporal *) tgeoseq_scale((TSequence *) temp, &factors);
      break;
    default: /* TSEQUENCESET */
      temp2 = (Temporal *) tgeoseqset_scale((TSequenceSet *) temp, &factors);
  }
  
  /* Return to original origin after scaling */
  Temporal *temp3;
  if (translate)
  {
    aff.xoff *= -1;
    aff.yoff *= -1;
    aff.zoff *= -1;
    temp3 = tgeo_affine(temp2, &aff);
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

/*****************************************************************************
 * Convex hull functions
 *****************************************************************************/

/**
 * @ingroup meos_geo_accessor
 * @brief Return the convex hull of a temporal geo
 * @param[in] temp Temporal geo
 * @return On error return `NULL`
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
  MeosType basetype = temptype_basetype(temp->temptype);
  datumarr_sort(values, count, basetype);
  int newcount = datumarr_remove_duplicates(values, count, basetype);
  GSERIALIZED **gsarr = palloc(sizeof(GSERIALIZED *) * newcount);
  for (int i = 0; i < newcount; i++)
    gsarr[i] = DatumGetGserializedP(values[i]);
  GSERIALIZED *res = geo_collect_garray(gsarr, newcount);
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
 * @csqlfn #Tgeo_centroid()
 */
Temporal *
tgeo_centroid(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);

  bool geodetic = MEOS_FLAGS_GET_GEODETIC(temp->flags);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) 
    (geodetic ? &datum2_geog_centroid : &datum2_geom_centroid);
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
 * contains a geometry
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
 * contains a geometry
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
  * @brief Return an array of GeometryCollections partitioning the input
  * geometries into clusters in which each geometry is within the specified
  * distance of at least one other geometry in the same cluster.
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

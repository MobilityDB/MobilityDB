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
 * @brief Spatial functions for temporal points
 */

#include "point/tpoint_spatialfuncs.h"

/* PostgreSQL */
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeodetic.h>
#include <lwgeom_geos.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/lifting.h"
#include "general/temporal_compops.h"
#include "general/tnumber_mathfuncs.h"
#include "general/tsequence.h"
#include "general/type_util.h"
#include "point/pgis_types.h"
#include "point/stbox.h"
#include "point/tpoint.h"
#include "point/tpoint_distance.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
#endif

/* Timestamps in PostgreSQL are encoded as MICROseconds since '2000-01-01'
 * while Unix epoch are encoded as MILLIseconds since '1970-01-01'.
 * Therefore the value used for conversions is computed as follows
 * select date_part('epoch', timestamp '2000-01-01' - timestamp '1970-01-01')
 * which results in 946684800 */
#define DELTA_UNIX_POSTGRES_EPOCH 946684800

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
    POINT3DZ *point = (POINT3DZ *) GS_POINT_PTR(gs);
    p->x = point->x;
    p->y = point->y;
    p->z = point->z;
  }
  else
  {
    POINT2D *point = (POINT2D *) GS_POINT_PTR(gs);
    p->x = point->x;
    p->y = point->y;
  }
  return;
}

/*****************************************************************************/

/**
 * @brief Return true if the points are equal
 * @note This function is called in the iterations over sequences where we
 * are sure that their SRID, Z, and GEODETIC are equal
 */
bool
geopoint_eq(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    return float8_eq(point1->x, point2->x) &&
      float8_eq(point1->y, point2->y) && float8_eq(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    return float8_eq(point1->x, point2->x) && float8_eq(point1->y, point2->y);
  }
}

/**
 * @brief Return true if the points are equal up to the floating point tolerance
 * @note This function is called in the iterations over sequences where we
 * are sure that their SRID, Z, and GEODETIC are equal
 */
bool
geopoint_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    return MEOS_FP_EQ(point1->x, point2->x) &&
      MEOS_FP_EQ(point1->y, point2->y) && MEOS_FP_EQ(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    return MEOS_FP_EQ(point1->x, point2->x) &&
      MEOS_FP_EQ(point1->y, point2->y);
  }
}

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
 * @brief Return true if the points are equal
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

/**
 * @brief Serialize a geometry/geography
 * @pre It is supposed that the flags such as Z and geodetic have been
 * set up before by the calling function
 */
GSERIALIZED *
geo_serialize(const LWGEOM *geom)
{
  size_t size;
  GSERIALIZED *result = gserialized_from_lwgeom((LWGEOM *) geom, &size);
  SET_VARSIZE(result, size);
  return result;
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Select the appropriate distance function
 */
datum_func2
distance_fn(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    return &geog_distance;
  else
    return MEOS_FLAGS_GET_Z(flags) ?
      &geom_distance3d : &geom_distance2d;
}

/**
 * @brief Select the appropriate distance function
 */
datum_func2
pt_distance_fn(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
    return &geog_distance;
  else
    return MEOS_FLAGS_GET_Z(flags) ?
      &pt_distance3d : &pt_distance2d;
}

/**
 * @brief Return the 2D distance between the two geometries
 * @pre For PostGIS version > 3 the geometries are NOT toasted
 */
Datum
geom_distance2d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(geo_distance(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return the 3D distance between the two geometries
 */
Datum
geom_distance3d(Datum geom1, Datum geom2)
{
  return Float8GetDatum(geometry_3Ddistance(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return the distance between the two geographies
 */
Datum
geog_distance(Datum geog1, Datum geog2)
{
  return Float8GetDatum(pgis_geography_distance(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2)));
}

/**
 * @brief Return the 2D distance between the two geometry points
 */
Datum
pt_distance2d(Datum geom1, Datum geom2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(geom1);
  const POINT2D *p2 = DATUM_POINT2D_P(geom2);
  return Float8GetDatum(distance2d_pt_pt(p1, p2));
}

/**
 * @brief Return the 3D distance between the two geometry points
 */
Datum
pt_distance3d(Datum geom1, Datum geom2)
{
  const POINT3DZ *p1 = DATUM_POINT3DZ_P(geom1);
  const POINT3DZ *p2 = DATUM_POINT3DZ_P(geom2);
  return Float8GetDatum(distance3d_pt_pt((POINT3D *) p1, (POINT3D *) p2));
}

/**
 * @brief Return the 2D intersection between the two geometries
 */
Datum
geom_intersection2d(Datum geom1, Datum geom2)
{
  return PointerGetDatum(geometry_intersection(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that the spatial constraints required for operating on two
 * temporal geometries are satisfied
 */
bool
ensure_spatial_validity(const Temporal *temp1, const Temporal *temp2)
{
  if (tgeo_type(temp1->temptype) &&
      (! ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2)) ||
       ! ensure_same_dimensionality(temp1->flags, temp2->flags)))
    return false;
  return true;
}

/**
 * @brief Ensure that the spatiotemporal argument has planar coordinates
 */
bool
ensure_not_geodetic(int16 flags)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only planar coordinates supported");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the spatiotemporal argument have the same type of
 * coordinates, either planar or geodetic
 */
bool
ensure_same_geodetic(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) && MEOS_FLAGS_GET_X(flags2) &&
    MEOS_FLAGS_GET_GEODETIC(flags1) != MEOS_FLAGS_GET_GEODETIC(flags2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed planar and geodetic coordinates");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the two spatial "objects" have the same SRID
 */
bool
ensure_srid_known(int32_t srid)
{
  if (srid == SRID_UNKNOWN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The SRID cannot be unknown");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the two spatial "objects" have the same SRID
 */
bool
ensure_same_srid(int32_t srid1, int32_t srid2)
{
  if (srid1 != srid2)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed SRID");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal point and a geometry/geography have the same
 * SRID
 */
bool
ensure_same_srid_stbox(const STBox *box1, const STBox *box2)
{
  if (MEOS_FLAGS_GET_X(box1->flags) && MEOS_FLAGS_GET_X(box2->flags) &&
      box1->srid != box2->srid)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed SRID");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal point and a geometry/geography have the same
 * SRID
 */
bool
ensure_same_srid_stbox_gs(const STBox *box, const GSERIALIZED *gs)
{
  if (box->srid != gserialized_get_srid(gs))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed SRID");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that two temporal points have the same dimensionality as given
 * by their flags
 */
bool
ensure_same_dimensionality(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) ||
      MEOS_FLAGS_GET_Z(flags1) != MEOS_FLAGS_GET_Z(flags2) ||
      MEOS_FLAGS_GET_T(flags1) != MEOS_FLAGS_GET_T(flags2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The arguments must be of the same dimensionality");
    return false;
  }
  return true;
}

/**
 * @brief Return true if the two temporal points have the same spatial
 * dimensionality as given by their flags
 */
bool
same_spatial_dimensionality(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) ||
      MEOS_FLAGS_GET_Z(flags1) != MEOS_FLAGS_GET_Z(flags2))
    return false;
  return true;
}

/**
 * @brief Ensure that two temporal points have the same spatial dimensionality
 * as given by their flags
 */
bool
ensure_same_spatial_dimensionality(int16 flags1, int16 flags2)
{
  if (! same_spatial_dimensionality(flags1, flags2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed 2D/3D dimensions");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal point and a spatiotemporal box have the same
 * spatial dimensionality as given by their flags
 */
bool
ensure_same_spatial_dimensionality_temp_box(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) ||
      /* Geodetic boxes are always in 3D */
      (! MEOS_FLAGS_GET_GEODETIC(flags2) &&
      MEOS_FLAGS_GET_Z(flags1) != MEOS_FLAGS_GET_Z(flags2)))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed 2D/3D dimensions");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that two geometries/geographies have the same dimensionality
 */
bool
ensure_same_dimensionality_gs(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags) != FLAGS_GET_Z(gs2->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed 2D/3D dimensions");
    return false;
  }
  return true;
}

/**
 * @brief Return true if a temporal point and a geometry/geography have the
 * same dimensionality
 */
bool
same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (MEOS_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->gflags))
    return false;
  return true;
}

/**
 * @brief Ensure that a temporal point and a geometry/geography have the same
 * dimensionality
 */
bool
ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs)
{
  if (! same_dimensionality_tpoint_gs(temp, gs))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed 2D/3D dimensions");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the spatiotemporal boxes have the same spatial
 * dimensionality
 */
bool
ensure_same_spatial_dimensionality_stbox_gs(const STBox *box, const GSERIALIZED *gs)
{
  if (! MEOS_FLAGS_GET_X(box->flags) ||
      /* Geodetic boxes are always in 3D */
     (! MEOS_FLAGS_GET_GEODETIC(box->flags) &&
        MEOS_FLAGS_GET_Z(box->flags) != FLAGS_GET_Z(gs->gflags)))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The spatiotemporal box and the geometry must be of the same dimensionality");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal point has Z dimension
 */
bool
ensure_has_Z(int16 flags)
{
  if (! MEOS_FLAGS_GET_Z(flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal point must have Z dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal point has not Z dimension
 */
bool
ensure_has_not_Z(int16 flags)
{
  if (MEOS_FLAGS_GET_Z(flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal point cannot have Z dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the geometry/geography has not Z dimension
 */
bool
ensure_has_Z_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_Z(gs->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The geometry must have Z dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the geometry/geography has not Z dimension
 */
bool
ensure_has_not_Z_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_Z(gs->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The geometry cannot have Z dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the geometry/geography has M dimension
 */
bool
ensure_has_M_gs(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_M(gs->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only geometries with M dimension accepted");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the geometry/geography has not M dimension
 */
bool
ensure_has_not_M_gs(const GSERIALIZED *gs)
{
  if (FLAGS_GET_M(gs->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only geometries without M dimension accepted");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the geometry/geography is a point
 */
bool
ensure_point_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) != POINTTYPE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only point geometries accepted");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the geometry/geography is not empty
 */
bool
ensure_not_empty(const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only non-empty geometries accepted");
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
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) gs) ||
      gserialized_is_empty(gs) || ! ensure_has_X_stbox(box) ||
      ! ensure_same_srid_stbox_gs(box, gs))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal point and a geometry
 * @note The geometry can be empty since some functions such atGeometry or
 * minusGeometry return different result on empty geometries.
 */
bool
ensure_valid_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  meosType tgeotype =
    FLAGS_GET_GEODETIC(gs->gflags) ? T_TGEOGPOINT : T_TGEOMPOINT;
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_tgeo_type(temp->temptype) ||
      ! ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_temporal_isof_type(temp, tgeotype))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a spatiotemporal boxes
 */
bool
ensure_valid_spatial_stbox_stbox(const STBox *box1, const STBox *box2)
{
  if (! ensure_not_null((void *) box1) || ! ensure_not_null((void *) box2) ||
      ! ensure_has_X_stbox(box1) || ! ensure_has_X_stbox(box2) ||
      ! ensure_same_geodetic(box1->flags, box2->flags) ||
      ! ensure_same_srid(stbox_srid(box1), stbox_srid(box2)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal point and a spatial box
 */
bool
ensure_valid_tpoint_box(const Temporal *temp, const STBox *box)
{
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_tgeo_type(temp->temptype) || ! ensure_has_X_stbox(box) ||
      ! ensure_same_geodetic(temp->flags, box->flags) ||
      ! ensure_same_srid(tpoint_srid(temp), stbox_srid(box)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two temporal points
 */
bool
ensure_valid_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_tgeo_type(temp1->temptype) ||
      ! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2)))
    return false;
  return true;
}

/*****************************************************************************
 * Functions for extracting coordinates
 *****************************************************************************/

/**
 * @brief Get the X coordinates of a temporal point
 */
static Datum
point_get_x(Datum point)
{
  POINT4D p;
  datum_point4d(point, &p);
  return Float8GetDatum(p.x);
}

/**
 * @brief Get the Y coordinates of a temporal point
 */
static Datum
point_get_y(Datum point)
{
  POINT4D p;
  datum_point4d(point, &p);
  return Float8GetDatum(p.y);
}

/**
 * @brief Get the Z coordinates of a temporal point
 */
static Datum
point_get_z(Datum point)
{
  POINT4D p;
  datum_point4d(point, &p);
  return Float8GetDatum(p.z);
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return one of the coordinates of a temporal point as a temporal float
 * @param[in] temp Temporal point
 * @param[in] coord Coordinate number where 0 = X, 1 = Y, 2 = Z
 */
Temporal *
tpoint_get_coord(const Temporal *temp, int coord)
{
  assert(coord >= 0 && coord <= 2);
  /* Ensure validity of the arguments */
  if (coord == 2 && ! ensure_has_Z(temp->flags))
     return NULL;

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  if (coord == 0)
    lfinfo.func = (varfunc) &point_get_x;
  else if (coord == 1)
    lfinfo.func = (varfunc) &point_get_y;
  else /* coord == 2 */
    lfinfo.func = (varfunc) &point_get_z;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TFLOAT;
  return tfunc_temporal(temp, &lfinfo);
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the X coordinates of a temporal point as a temporal float
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_get_x()
 */
Temporal *
tpoint_get_x(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
     return NULL;
  return tpoint_get_coord(temp, 0);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the Y coordinates of a temporal point as a temporal float
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_get_y()
 */
Temporal *
tpoint_get_y(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
     return NULL;
  return tpoint_get_coord(temp, 1);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the Z coordinates of a temporal point as a temporal float
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_get_z()
 */
Temporal *
tpoint_get_z(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
     return NULL;
  return tpoint_get_coord(temp, 2);
}
#endif /* MEOS */

/*****************************************************************************
 * Return true if a point is in a segment (2D, 3D, or geodetic).
 * For 2D/3D points we proceed as follows.
 * If the cross product of (B-A) and (p-A) is 0, then the points A, B,
 * and p are aligned. To know if p is between A and B, we also have to
 * check that the dot product of (B-A) and (p-A) is positive and is less
 * than the square of the distance between A and B.
 * https://stackoverflow.com/questions/328107/how-can-you-determine-a-point-is-between-two-other-points-on-a-line-segment
 *****************************************************************************/

#if 0 /* not used */
/**
 * @brief Return true if point @p p is in the 2D segment defined by @p A and
 * @p B
 * @note The test of `p = A` or `p = B` *must be* done in the calling function
 * to take care of the inclusive/exclusive bounds for temporal sequences
 */
static bool
point2d_on_segment(const POINT2D *p, const POINT2D *A, const POINT2D *B)
{
  double crossproduct = (p->y - A->y) * (B->x - A->x) -
    (p->x - A->x) * (B->y - A->y);
  if (fabs(crossproduct) >= MEOS_EPSILON)
    return false;
  double dotproduct = (p->x - A->x) * (B->x - A->x) +
    (p->y - A->y) * (B->y - A->y);
  return (dotproduct >= 0);
}

/**
 * @brief Return true if a point @p p is in the 3D segment defined by @p A and
 * @p B
 * @note The test of `p = A` or `p = B` *must be* done in the calling function
 * to take care of the inclusive/exclusive bounds for temporal sequences
 */
static bool
point3dz_on_segment(const POINT3DZ *p, const POINT3DZ *A, const POINT3DZ *B)
{
  /* Find the collinearity of the points using the cross product
   * http://www.ambrsoft.com/TrigoCalc/Line3D/LineColinear.htm */
  double i = (p->y - A->y) * (B->z - A->z) - (p->z - A->z) * (B->y - A->y);
  double j = (p->z - A->z) * (B->x - A->x) - (p->x - A->x) * (B->z - A->z);
  double k = (p->x - A->x) * (B->y - A->y) - (p->y - A->y) * (B->x - A->x);
  if (fabs(i) >= MEOS_EPSILON || fabs(j) >= MEOS_EPSILON ||
      fabs(k) >= MEOS_EPSILON)
    return false;
  double dotproduct = (p->x - A->x) * (B->x - A->x) +
    (p->y - A->y) * (B->y - A->y) + (p->z - A->z) * (B->z - A->z);
  return (dotproduct >= 0);
}

/**
 * @brief Return true if point @p p is in the geodetic segment defined by @p A
 * and @p B
 */
static bool
point_on_segment_sphere(const POINT4D *p, const POINT4D *A, const POINT4D *B)
{
  POINT4D closest;
  double dist;
  closest_point_on_segment_sphere(p, A, B, &closest, &dist);
  return (dist > MEOS_EPSILON) && (FP_EQUALS(p->z, closest.z));
}

/**
 * @brief Determine if a point is in a segment
 * @param[in] start,end Points defining the segment
 * @param[in] point Point
 */
static bool
point_on_segment(Datum start, Datum end, Datum point)
{
  GSERIALIZED *gs = DatumGetGserializedP(start);
  if (FLAGS_GET_GEODETIC(gs->gflags))
  {
    POINT4D p1, p2, p;
    datum_point4d(start, &p1);
    datum_point4d(end, &p2);
    datum_point4d(point, &p);
    return point_on_segment_sphere(&p, &p1, &p2);
  }
  if (FLAGS_GET_Z(gs->gflags))
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(start);
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(end);
    const POINT3DZ *p = DATUM_POINT3DZ_P(point);
    return point3dz_on_segment(p, p1, p2);
  }
  /* 2D */
  const POINT2D *p1 = DATUM_POINT2D_P(start);
  const POINT2D *p2 = DATUM_POINT2D_P(end);
  const POINT2D *p = DATUM_POINT2D_P(point);
  return point2d_on_segment(p, p1, p2);
}
#endif /* not used */

/*****************************************************************************
 * Functions derived from PostGIS to increase floating-point precision
 *****************************************************************************/

/**
 * @brief Return a long double between 0 and 1 representing the location of the
 * closest point on the 2D segment to the given point, as a fraction of total
 * segment length
 * @note Function derived from the PostGIS function @p closest_point_on_segment
 */
long double
closest_point2d_on_segment_ratio(const POINT2D *p, const POINT2D *A,
  const POINT2D *B, POINT2D *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y))
  {
    if (closest)
      *closest = *A;
    return 0.0;
  }

  /*
   * We use comp.graphics.algorithms Frequently Asked Questions method
   *
   * (1)          AC dot AB
   *         r = ----------
   *              ||AB||^2
   *  r has the following meaning:
   *  r=0 P = A
   *  r=1 P = B
   *  r<0 P is on the backward extension of AB
   *  r>1 P is on the forward extension of AB
   *  0<r<1 P is interior to AB
   *
   */
  long double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

  if (r < 0)
  {
    if (closest)
      *closest = *A;
    return 0.0;
  }
  if (r > 1)
  {
    if (closest)
      *closest = *B;
    return 1.0;
  }

  if (closest)
  {
    closest->x = (double) (A->x + ( (B->x - A->x) * r ));
    closest->y = (double) (A->y + ( (B->y - A->y) * r ));
  }
  return r;
}

/**
 * @brief Return a long double between 0 and 1 representing the location of the
 * closest point on the 3D segment to the given point, as a fraction of total
 * segment length
 * @note Function derived from the PostGIS function @p closest_point_on_segment
 */
long double
closest_point3dz_on_segment_ratio(const POINT3DZ *p, const POINT3DZ *A,
  const POINT3DZ *B, POINT3DZ *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y) && FP_EQUALS(A->z, B->z))
  {
    *closest = *A;
    return 0.0;
  }

  /*
   * Function #closest_point2d_on_segment_ratio explains how r is computed
   */
  long double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) +
      (p->z-A->z) * (B->z-A->z) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) +
      (B->z-A->z) * (B->z-A->z) );

  if (r < 0)
  {
    *closest = *A;
    return 0.0;
  }
  if (r > 1)
  {
    *closest = *B;
    return 1.0;
  }

  closest->x = (double) (A->x + ( (B->x - A->x) * r ));
  closest->y = (double) (A->y + ( (B->y - A->y) * r ));
  closest->z = (double) (A->z + ( (B->z - A->z) * r ));
  return r;
}

/**
 * @brief Return a float between 0 and 1 representing the location of the
 * closest point on the geography segment to the given point, as a fraction of
 * total segment length
 * @param[in] p Reference point
 * @param[in] A,B Points defining the segment
 * @param[out] closest Closest point in the segment
 * @param[out] dist Distance between the closest point and the reference point
 */
long double
closest_point_on_segment_sphere(const POINT4D *p, const POINT4D *A,
  const POINT4D *B, POINT4D *closest, double *dist)
{
  GEOGRAPHIC_EDGE e;
  GEOGRAPHIC_POINT gp, proj;
  long double length, /* length from A to the closest point */
    seglength; /* length of the segment AB */
  long double result; /* ratio */

  /* Initialize target point */
  geographic_point_init(p->x, p->y, &gp);

  /* Initialize edge */
  geographic_point_init(A->x, A->y, &(e.start));
  geographic_point_init(B->x, B->y, &(e.end));

  /* Get the spherical distance between point and edge */
  *dist = edge_distance_to_point(&e, &gp, &proj);

  /* Compute distance from beginning of the segment to closest point */
  seglength = (long double) sphere_distance(&(e.start), &(e.end));
  length = (long double) sphere_distance(&(e.start), &proj);
  result = length / seglength;

  if (closest)
  {
    /* Copy nearest into returning argument */
    closest->x = rad2deg(proj.lon);
    closest->y = rad2deg(proj.lat);

    /* Compute Z and M values for closest point */
    closest->z = (double) (A->z + ((B->z - A->z) * result));
    closest->m = (double) (A->m + ((B->m - A->m) * result));
  }
  return result;
}

/**
 * @brief Find the interpolation point @p p between geography points @p p1 and
 * @p p2 so that `len(p1,p) == len(p1,p2) * f` and @p p falls on the @p p1,p2
 * segment
 * @param[in] p1,p2 geography points we are interpolating between
 * @param[in] s Spheroid used for during the intepolation, may be @p NULL when
 * using sphere interpolation
 * @param[in] f Fraction
 * @param[out] p Result
 */
void
interpolate_point4d_spheroid(const POINT4D *p1, const POINT4D *p2,
  POINT4D *p, const SPHEROID *s, double f)
{
  GEOGRAPHIC_POINT g, g1, g2;
  geographic_point_init(p1->x, p1->y, &g1);
  geographic_point_init(p2->x, p2->y, &g2);
  int success;
  double dist, dir;

  /* Special sphere case */
  if ( s == NULL || s->a == s->b )
  {
    /* Calculate distance and direction between g1 and g2 */
    dist = sphere_distance(&g1, &g2);
    dir = sphere_direction(&g1, &g2, dist);
    /* Compute interpolation point */
    success = sphere_project(&g1, dist*f, dir, &g);
  }
  /* Spheroid case */
  else
  {
    /* Calculate distance and direction between g1 and g2 */
    dist = spheroid_distance(&g1, &g2, s);
    dir = spheroid_direction(&g1, &g2, s);
    /* Compute interpolation point */
    success = spheroid_project(&g1, s, dist*f, dir, &g);
  }

  /* Compute Cartesian interpolation and precompute z/m values */
  interpolate_point4d(p1, p2, p, f);

  /* If success, use newly computed lat and lon,
   * otherwise return precomputed cartesian result */
  if (success == LW_SUCCESS)
  {
    p->x = rad2deg(longitude_radians_normalize(g.lon));
    p->y = rad2deg(latitude_radians_normalize(g.lat));
  }
  return;
}

/*****************************************************************************
 * Functions specializing the PostGIS functions ST_LineInterpolatePoint and
 * ST_LineLocatePoint
 *****************************************************************************/

/**
 * @brief Return a point created from the arguments
 */
GSERIALIZED *
geopoint_make(double x, double y, double z, bool hasz, bool geodetic,
  int32 srid)
{
  LWPOINT *point = hasz ?
    lwpoint_make3dz(srid, x, y, z) : lwpoint_make2d(srid, x, y);
  FLAGS_SET_GEODETIC(point->flags, geodetic);
  GSERIALIZED *result = geo_serialize((LWGEOM *) point);
  // We CANNOT lwpoint_free(point);
  return result;
}

/**
 * @brief Return a point interpolated from the geometry/geography segment with
 * respect to the fraction of its total length
 * @param[in] start,end Points defining the segment
 * @param[in] ratio Float between 0 and 1 representing the fraction of the
 * total length of the segment where the point must be located
 */
Datum
geosegm_interpolate_point(Datum start, Datum end, long double ratio)
{
  GSERIALIZED *gs = DatumGetGserializedP(start);
  int srid = gserialized_get_srid(gs);
  POINT4D p1, p2, p;
  datum_point4d(start, &p1);
  datum_point4d(end, &p2);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(gs->gflags);
  if (geodetic)
    interpolate_point4d_spheroid(&p1, &p2, &p, NULL, (double) ratio);
  else
  {
    /* We cannot call the PostGIS function
     * interpolate_point4d(&p1, &p2, &p, ratio);
     * since it uses a double and not a long double for the interpolation */
    p.x = p1.x + (double) ((long double) (p2.x - p1.x) * ratio);
    p.y = p1.y + (double) ((long double) (p2.y - p1.y) * ratio);
    p.z = p1.z + (double) ((long double) (p2.z - p1.z) * ratio);
    p.m = 0.0;
  }

  Datum result = PointerGetDatum(geopoint_make(p.x, p.y, p.z, hasz, geodetic,
    srid));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(start));
  return result;
}

/**
 * @brief Return a float between 0 and 1 representing the location of the
 * closest point on the geometry segment to the given point, as a fraction of
 * total segment length
 *@param[in] start,end Points defining the segment
 *@param[in] point Reference point
 *@param[out] dist Distance
 */
long double
geosegm_locate_point(Datum start, Datum end, Datum point, double *dist)
{
  GSERIALIZED *gs = DatumGetGserializedP(start);
  long double result;
  if (FLAGS_GET_GEODETIC(gs->gflags))
  {
    POINT4D p1, p2, p, closest;
    datum_point4d(start, &p1);
    datum_point4d(end, &p2);
    datum_point4d(point, &p);
    double d;
    /* Get the closest point and the distance */
    result = closest_point_on_segment_sphere(&p, &p1, &p2, &closest, &d);
    /* For robustness, force 0/1 when closest point == start/endpoint */
    if (p4d_same(&p1, &closest))
      result = 0.0;
    else if (p4d_same(&p2, &closest))
      result = 1.0;
    /* Return the distance between the closest point and the point if requested */
    if (dist)
    {
      d = WGS84_RADIUS * d;
      /* Add to the distance the vertical displacement if we're in 3D */
      if (FLAGS_GET_Z(gs->gflags))
        d = sqrt( (closest.z - p.z) * (closest.z - p.z) + d*d );
      *dist = d;
    }
  }
  else
  {
    if (FLAGS_GET_Z(gs->gflags))
    {
      const POINT3DZ *p1 = DATUM_POINT3DZ_P(start);
      const POINT3DZ *p2 = DATUM_POINT3DZ_P(end);
      const POINT3DZ *p = DATUM_POINT3DZ_P(point);
      POINT3DZ proj;
      result = closest_point3dz_on_segment_ratio(p, p1, p2, &proj);
      /* For robustness, force 0/1 when closest point == start/endpoint */
      if (p3d_same((POINT3D *) p1, (POINT3D *) &proj))
        result = 0.0;
      else if (p3d_same((POINT3D *) p2, (POINT3D *) &proj))
        result = 1.0;
      if (dist)
        *dist = distance3d_pt_pt((POINT3D *) p, (POINT3D *) &proj);
    }
    else
    {
      const POINT2D *p1 = DATUM_POINT2D_P(start);
      const POINT2D *p2 = DATUM_POINT2D_P(end);
      const POINT2D *p = DATUM_POINT2D_P(point);
      POINT2D proj;
      result = closest_point2d_on_segment_ratio(p, p1, p2, &proj);
      if (p2d_same(p1, &proj))
        result = 0.0;
      else if (p2d_same(p2, &proj))
        result = 1.0;
      if (dist)
        *dist = distance2d_pt_pt((POINT2D *) p, &proj);
    }
  }
  return result;
}

/*****************************************************************************
 * Interpolation functions defining functionality required by tsequence.c
 * that must be implemented by each temporal type
 *****************************************************************************/

/**
 * @brief Return true if a segment of a temporal point value intersects a point
 * at the timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 * @pre The geometry is not empty
 */
bool
tpointsegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  assert(! gserialized_is_empty(DatumGetGserializedP(value)));

  /* We are sure that the trajectory is a line */
  Datum start = tinstant_val(inst1);
  Datum end = tinstant_val(inst2);
  double dist;
  double fraction = (double) geosegm_locate_point(start, end, value, &dist);
  if (fabs(dist) >= MEOS_EPSILON)
    return false;
  if (t != NULL)
  {
    double duration = (double) (inst2->t - inst1->t);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return true;
}

/**
 * @brief Return true if two segments of a temporal geometry points intersect
 * at a timestamptz
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and
 * end1->t = end2->t
 */
bool
tgeompointsegm_intersection(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  double x1, y1, z1 = 0.0, x2, y2, z2 = 0.0, x3, y3, z3 = 0.0, x4, y4, z4 = 0.0;
  bool hasz = MEOS_FLAGS_GET_Z(start1->flags);
  if (hasz)
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(tinstant_val(start1));
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(tinstant_val(end1));
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(tinstant_val(start2));
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(tinstant_val(end2));
    x1 = p1->x; y1 = p1->y; z1 = p1->z;
    x2 = p2->x; y2 = p2->y; z2 = p2->z;
    x3 = p3->x; y3 = p3->y; z3 = p3->z;
    x4 = p4->x; y4 = p4->y; z4 = p4->z;
    /* Segments intersecting in the boundaries */
    if ((float8_eq(x1, x3) && float8_eq(y1, y3) && float8_eq(z1, z3)) ||
        (float8_eq(x2, x4) && float8_eq(y2, y4) && float8_eq(z2, z4)))
      return false;
  }
  else
  {
    const POINT2D *p1 = DATUM_POINT2D_P(tinstant_val(start1));
    const POINT2D *p2 = DATUM_POINT2D_P(tinstant_val(end1));
    const POINT2D *p3 = DATUM_POINT2D_P(tinstant_val(start2));
    const POINT2D *p4 = DATUM_POINT2D_P(tinstant_val(end2));
    x1 = p1->x; y1 = p1->y;
    x2 = p2->x; y2 = p2->y;
    x3 = p3->x; y3 = p3->y;
    x4 = p4->x; y4 = p4->y;
    /* Segments intersecting in the boundaries */
    if ((float8_eq(x1, x3) && float8_eq(y1, y3)) ||
        (float8_eq(x2, x4) && float8_eq(y2, y4)))
      return false;
  }

  long double xdenom = x2 - x1 - x4 + x3;
  long double ydenom = y2 - y1 - y4 + y3;
  long double zdenom = 0.0;
  if (hasz)
    zdenom = z2 - z1 - z4 + z3;
  if (xdenom == 0 && ydenom == 0 && zdenom == 0)
    /* Parallel segments */
    return false;

  /* Potentially avoid the division based on
   * Franklin Antonio, Faster Line Segment Intersection, Graphic Gems III
   * https://github.com/erich666/GraphicsGems/blob/master/gemsiii/insectc.c */
  long double fraction, xfraction = 0, yfraction = 0, zfraction = 0;
  if (xdenom != 0)
  {
    long double xnum = x3 - x1;
    if ((xdenom > 0 && (xnum < 0 || xnum > xdenom)) ||
        (xdenom < 0 && (xnum > 0 || xnum < xdenom)))
      return false;
    xfraction = xnum / xdenom;
    if (xfraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < xfraction)
      /* Intersection occurs out of the period */
      return false;
  }
  if (ydenom != 0)
  {
    long double ynum = y3 - y1;
    if ((ydenom > 0 && (ynum < 0 || ynum > ydenom)) ||
        (ydenom < 0 && (ynum > 0 || ynum < ydenom)))
      return false;
    yfraction = ynum / ydenom;
    if (yfraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < yfraction)
      /* Intersection occurs out of the period */
      return false;
  }
  if (hasz && zdenom != 0)
  {
    long double znum = z3 - z1;
    if ((zdenom > 0 && (znum < 0 || znum > zdenom)) ||
        (zdenom < 0 && (znum > 0 || znum < zdenom)))
      return false;
    zfraction = znum / zdenom;
    if (zfraction < -1 * MEOS_EPSILON || 1.0 + MEOS_EPSILON < zfraction)
      /* Intersection occurs out of the period */
      return false;
  }
  if (hasz)
  {
    /* If intersection occurs at different timestamps on each dimension */
    if ((xdenom != 0 && ydenom != 0 && zdenom != 0 &&
        fabsl(xfraction - yfraction) > MEOS_EPSILON &&
        fabsl(xfraction - zfraction) > MEOS_EPSILON) ||
      (xdenom == 0 && ydenom != 0 && zdenom != 0 &&
        fabsl(yfraction - zfraction) > MEOS_EPSILON) ||
      (xdenom != 0 && ydenom == 0 && zdenom != 0 &&
        fabsl(xfraction - zfraction) > MEOS_EPSILON) ||
      (xdenom != 0 && ydenom != 0 && zdenom == 0 &&
        fabsl(xfraction - yfraction) > MEOS_EPSILON))
      return false;
    if (xdenom != 0)
      fraction = xfraction;
    else if (ydenom != 0)
      fraction = yfraction;
    else
      fraction = zfraction;
  }
  else /* 2D */
  {
    /* If intersection occurs at different timestamps on each dimension */
    if (xdenom != 0 && ydenom != 0 &&
        fabsl(xfraction - yfraction) > MEOS_EPSILON)
      return false;
    fraction = xdenom != 0 ? xfraction : yfraction;
  }
  double duration = (double) (end1->t - start1->t);
  *t = start1->t + (TimestampTz) (duration * fraction);
  /* Note that due to roundoff errors it may be the case that the
   * resulting timestamp t may be equal to inst1->t or to inst2->t */
  if (*t <= start1->t || *t >= end1->t)
    return false;
  return true;
}

/**
 * @brief Return true if two temporal geography point segments
 * intersect at a timestamptz
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and
 * end1->t = end2->t
 */
bool
tgeogpointsegm_intersection(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  Datum mindist;
  bool found = tgeogpoint_min_dist_at_timestamptz(start1, end1, start2, end2,
    &mindist, t);
  if (! found || DatumGetFloat8(mindist) > MEOS_EPSILON)
    return false;
  return true;
}

/**
 * @brief Return true if the three values are collinear
 * @param[in] value1,value2,value3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `value1` and `value2` divided by the duration
 * of the timestamps associated to `value1` and `value3`
 * @param[in] hasz True if the points have Z coordinates
 * @param[in] geodetic True for geography, false for geometry
 */
bool
geopoint_collinear(Datum value1, Datum value2, Datum value3,
  double ratio, bool hasz, bool geodetic)
{
  POINT4D p1, p2, p3, p;
  datum_point4d(value1, &p1);
  datum_point4d(value2, &p2);
  datum_point4d(value3, &p3);
  if (geodetic)
    interpolate_point4d_spheroid(&p1, &p3, &p, NULL, ratio);
  else
    interpolate_point4d(&p1, &p3, &p, ratio);

  bool result = hasz ?
    fabs(p2.x - p.x) <= MEOS_EPSILON && fabs(p2.y - p.y) <= MEOS_EPSILON &&
      fabs(p2.z - p.z) <= MEOS_EPSILON :
    fabs(p2.x - p.x) <= MEOS_EPSILON && fabs(p2.y - p.y) <= MEOS_EPSILON;
  return result;
}

/*****************************************************************************
 * Trajectory functions
 *****************************************************************************/

/**
 * @brief Return -1, 0, or 1 depending on whether the first @p LWPOINT
 * is less than, equal to, or greater than the second one
 * @pre The points are not empty and are of the same dimensionality
 */
static int
lwpoint_cmp(const LWPOINT *p, const LWPOINT *q)
{
  assert(FLAGS_GET_ZM(p->flags) == FLAGS_GET_ZM(q->flags));
  POINT4D p4d, q4d;
  /* We are sure the points are not empty */
  lwpoint_getPoint4d_p(p, &p4d);
  lwpoint_getPoint4d_p(q, &q4d);
  int cmp = float8_cmp_internal(p4d.x, q4d.x);
  if (cmp != 0)
    return cmp;
  cmp = float8_cmp_internal(p4d.y, q4d.y);
  if (cmp != 0)
    return cmp;
  if (FLAGS_GET_Z(p->flags))
  {
    cmp = float8_cmp_internal(p4d.z, q4d.z);
    if (cmp != 0)
      return cmp;
  }
  if (FLAGS_GET_M(p->flags))
  {
    cmp = float8_cmp_internal(p4d.m, q4d.m);
    if (cmp != 0)
      return cmp;
  }
  return 0;
}

/**
 * @brief Comparator function for lwpoints
 */
static int
lwpoint_sort_cmp(const LWPOINT **l, const LWPOINT **r)
{
  return lwpoint_cmp(*l, *r);
}

/**
 * @brief Sort function for lwpoints
 */
void
lwpointarr_sort(LWPOINT **points, int count)
{
  qsort(points, (size_t) count, sizeof(LWPOINT *),
    (qsort_comparator) &lwpoint_sort_cmp);
  return;
}

/**
 * @brief Remove duplicates from an array of LWGEOM points
 */
LWGEOM **
lwpointarr_remove_duplicates(LWGEOM **points, int count, int *newcount)
{
  assert (count > 0);
  LWGEOM **newpoints = palloc(sizeof(LWGEOM *) * count);
  memcpy(newpoints, points, sizeof(LWGEOM *) * count);
  lwpointarr_sort((LWPOINT **) newpoints, count);
  int count1 = 0;
  for (int i = 1; i < count; i++)
    if (! lwpoint_same((LWPOINT *) newpoints[count1], (LWPOINT *) newpoints[i]))
      newpoints[++ count1] = newpoints[i];
  *newcount = count1 + 1;
  return newpoints;
}

/**
 * @brief Return a trajectory from a set of points
 * @details The result is either a linestring or a multipoint depending on
 * whether the interpolation is step/discrete or linear.
 * @param[in] points Array of points
 * @param[in] count Number of elements in the input array
 * @param[in] interp Interpolation
 * @note The function does not remove duplicate points, that is, repeated
 * points in a multipoint or consecutive equal points in a line string
 */
LWGEOM *
lwpointarr_make_trajectory(LWGEOM **points, int count, interpType interp)
{
  if (count == 1)
    return lwpoint_as_lwgeom(lwpoint_clone(lwgeom_as_lwpoint(points[0])));

  LWGEOM *result = (interp == LINEAR) ?
    (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, (uint32_t) count,
      points) :
    (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, points[0]->srid,
      NULL, (uint32_t) count, points);
  FLAGS_SET_Z(result->flags, FLAGS_GET_Z(points[0]->flags));
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(points[0]->flags));
  return result;
}

/**
 * @brief Return the line connecting two geometry points
 * @param[in] value1,value2 Points
 */
LWLINE *
lwline_make(Datum value1, Datum value2)
{
  /* Obtain the flags and the SRID from the first value */
  GSERIALIZED *gs = DatumGetGserializedP(value1);
  int srid = gserialized_get_srid(gs);
  int hasz = FLAGS_GET_Z(gs->gflags);
  int geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  /* Since there is no M value a 0 value is passed */
  POINTARRAY *pa = ptarray_construct_empty((char) hasz, 0, 2);
  POINT4D pt;
  datum_point4d(value1, &pt);
  ptarray_append_point(pa, &pt, LW_TRUE);
  datum_point4d(value2, &pt);
  ptarray_append_point(pa, &pt, LW_TRUE);
  LWLINE *result = lwline_construct(srid, NULL, pa);
  FLAGS_SET_Z(result->flags, hasz);
  FLAGS_SET_GEODETIC(result->flags, geodetic);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the trajectory of a temporal discrete sequence point
 * @param[in] seq Temporal value
 * @note Notice that this function does not remove duplicate points
 * @pre The sequence is not instantaneous
 */
GSERIALIZED *
tpointdiscseq_trajectory(const TSequence *seq)
{
  assert(seq->count > 1);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    Datum value = tinstant_val(TSEQUENCE_INST_N(seq, i));
    GSERIALIZED *gsvalue = DatumGetGserializedP(value);
    points[i] = lwgeom_from_gserialized(gsvalue);
  }
  LWGEOM *lwresult = lwpointarr_make_trajectory(points, seq->count, STEP);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Return the trajectory of a temporal point sequence
 * @param[in] seq Temporal sequence
 * @pre The sequence is not instantaneous
 * @note Since the sequence has been already validated there is no verification
 * of the input in this function, in particular for geographies it is supposed
 * that the composing points are geodetic
 */
GSERIALIZED *
tpointcontseq_trajectory(const TSequence *seq)
{
  assert(seq->count > 1);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  /* Remove two consecutive points if they are equal */
  Datum value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
  GSERIALIZED *gs = DatumGetGserializedP(value);
  points[0] = lwgeom_from_gserialized(gs);
  int npoints = 1;
  for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_val(TSEQUENCE_INST_N(seq, i));
    gs = DatumGetGserializedP(value);
    LWPOINT *point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
    /* Remove two consecutive points if they are equal */
    if (! lwpoint_same(point, (LWPOINT *) points[npoints - 1]))
      points[npoints++] = (LWGEOM *) point;
  }
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  LWGEOM *lwresult = lwpointarr_make_trajectory(points, npoints, interp);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  if (interp == LINEAR)
  {
    for (int i = 0; i < npoints; i++)
      lwgeom_free(points[i]);
    pfree(points);
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point sequence
 * @param[in] seq Temporal sequence
 * @note Since the sequence has been already validated there is no verification
 * of the input in this function, in particular for geographies it is supposed
 * that the composing points are geodetic
 * @csqlfn #Tpoint_trajectory()
 */
GSERIALIZED *
tpointseq_trajectory(const TSequence *seq)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  /* Instantaneous sequence */
  if (seq->count == 1)
    return DatumGetGserializedP(tinstant_value(TSEQUENCE_INST_N(seq, 0)));

  return MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ?
    tpointdiscseq_trajectory(seq) :  tpointcontseq_trajectory(seq);
}

/**
 * @brief Return a geometry from an array of points and lines
 * @pre There is at least one geometry in both arrays
 */
LWGEOM *
lwcoll_from_points_lines(LWGEOM **points, LWGEOM **lines, int npoints,
  int nlines)
{
  assert(npoints > 0 || nlines > 0);
  LWGEOM *result, *respoints = NULL, *reslines = NULL;
  if (npoints > 0)
  {
    if (npoints == 1)
      respoints = points[0];
    else
    {
      /* There may be less points than the size of the array */
      LWGEOM **points1 = palloc(sizeof(LWGEOM *) * npoints);
      memcpy(points1, points, sizeof(LWGEOM *) * npoints);
      // TODO add the bounding box instead of ask PostGIS to compute it again
      respoints = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE,
        points[0]->srid, NULL, (uint32_t) npoints, points1);
      FLAGS_SET_Z(respoints->flags, FLAGS_GET_Z(points[0]->flags));
      FLAGS_SET_GEODETIC(respoints->flags, FLAGS_GET_GEODETIC(points[0]->flags));
    }
  }
  if (nlines > 0)
  {
    if (nlines == 1)
      reslines = (LWGEOM *) lines[0];
    else
    {
      /* There may be less lines than the size of the array */
      LWGEOM **lines1 = palloc(sizeof(LWGEOM *) * nlines);
      memcpy(lines1, lines, sizeof(LWGEOM *) * nlines);
      // TODO add the bounding box instead of ask PostGIS to compute it again
      reslines = (LWGEOM *) lwcollection_construct(MULTILINETYPE,
        lines[0]->srid, NULL, (uint32_t) nlines, lines1);
      FLAGS_SET_Z(reslines->flags, FLAGS_GET_Z(lines[0]->flags));
      FLAGS_SET_GEODETIC(reslines->flags, FLAGS_GET_GEODETIC(lines[0]->flags));
    }
  }
  /* If both points and lines */
  if (npoints > 0 && nlines > 0)
  {
    LWGEOM **geoms = palloc(sizeof(LWGEOM *) * 2);
    geoms[0] = respoints;
    geoms[1] = reslines;
    // TODO add the bounding box instead of ask PostGIS to compute it again
    result = (LWGEOM *) lwcollection_construct(COLLECTIONTYPE, respoints->srid,
      NULL, (uint32_t) 2, geoms);
    FLAGS_SET_Z(result->flags, FLAGS_GET_Z(respoints->flags));
    FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(respoints->flags));
  }
  /* If only points */
  else if (nlines == 0)
    result = respoints;
  /* If only lines */
  else /* npoints == 0 */
    result = reslines;
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_trajectory()
 */
GSERIALIZED *
tpointseqset_trajectory(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_trajectory(TSEQUENCESET_SEQ_N(ss, 0));

  int32 srid = tpointseqset_srid(ss);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss->flags);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(ss->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * ss->count);
  int npoints = 0, nlines = 0;
  /* Iterate as in #tpointcontseq_trajectory accumulating the results */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    Datum value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    GSERIALIZED *gs = DatumGetGserializedP(value);
    /* npoints is the current number of points so far, k is the number of
     * additional points from the current sequence */
    LWGEOM *point1 = lwgeom_from_gserialized(gs);
    points[npoints] = point1;
    int k = 1;
    for (int j = 1; j < seq->count; j++)
    {
      value = tinstant_val(TSEQUENCE_INST_N(seq, j));
      gs = DatumGetGserializedP(value);
      /* Do not add the point if it is equal to the previous ones */
      LWGEOM *point2 = lwgeom_from_gserialized(gs);
      if (! lwpoint_same((LWPOINT *) point1, (LWPOINT *) point2))
      {
        points[npoints + k++] = point2;
        point1 = point2;
      }
      else
        lwgeom_free(point2);
    }
    if (linear && k > 1)
    {
      lines[nlines] = (LWGEOM *) lwline_from_lwgeom_array(srid, (uint32_t) k,
        &points[npoints]);
      FLAGS_SET_Z(lines[nlines]->flags, hasz);
      FLAGS_SET_GEODETIC(lines[nlines]->flags, geodetic);
      nlines++;
      for (int j = 0; j < k; j++)
        lwgeom_free(points[npoints + j]);
    }
    else
      npoints += k;
  }
  LWGEOM *lwresult = lwcoll_from_points_lines(points, lines, npoints, nlines);
  FLAGS_SET_Z(lwresult->flags, hasz);
  FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult); pfree(points); pfree(lines);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_trajectory()
 */
GSERIALIZED *
tpoint_trajectory(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return DatumGetGserializedP(tinstant_value((TInstant *) temp));
    case TSEQUENCE:
      return tpointseq_trajectory((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tpointseqset_trajectory((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Conversion functions
 * Notice that a geometry point and a geography point are of different size
 * since the geography point keeps a bounding box
 *****************************************************************************/

/**
 * @brief Coerce coordinate values into range [-180 -90, 180 90] for geography
 * @note Transposed from PostGIS function @p ptarray_force_geodetic() in file
 * @p lwgeodetic.c. We do not issue a warning.
 */
void
pt_force_geodetic(LWPOINT *point)
{
  POINT4D pt;
  getPoint4d_p(point->point, 0, &pt);
  if ( pt.x < -180.0 || pt.x > 180.0 || pt.y < -90.0 || pt.y > 90.0 )
  {
    pt.x = longitude_degrees_normalize(pt.x);
    pt.y = latitude_degrees_normalize(pt.y);
    ptarray_set_point4d(point->point, 0, &pt);
  }
  FLAGS_SET_GEODETIC(point->flags, true);
  return;
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal point instant transformed from/to a temporal
 * geometry/geography point
 * @param[in] inst Temporal point instant
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TInstant *
tgeompointinst_tgeogpointinst(const TInstant *inst, bool oper)
{
  assert(inst); assert(tgeo_type(inst->temptype));
  GSERIALIZED *gs = DatumGetGserializedP(tinstant_val(inst));
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  /* Short circuit functions geography_from_geometry and
     geometry_from_geography since we know it is a point */
  if (oper == GEOM_TO_GEOG)
  {
    /* We cannot test the following without access to PROJ */
    // srid_check_latlong(geom->srid);
    /* Coerce the coordinate values into [-180 -90, 180 90] for GEOGRAPHY */
    pt_force_geodetic((LWPOINT *) geom);
    geom->srid = SRID_DEFAULT;
    lwgeom_set_geodetic(geom, true);
  }
  else
  {
    geom->srid = 0;
    lwgeom_set_geodetic(geom, false);
  }
  GSERIALIZED *gs1 = geo_serialize(geom);
  return tinstant_make_free(PointerGetDatum(gs1),
    (oper == GEOM_TO_GEOG) ? T_TGEOGPOINT : T_TGEOMPOINT, inst->t);
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal point sequence transformed from/to a temporal
 * geometry/geography point
 * @param[in] seq Temporal sequence point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TSequence *
tgeompointseq_tgeogpointseq(const TSequence *seq, bool oper)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tgeompointinst_tgeogpointinst(TSEQUENCE_INST_N(seq, i), oper);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal point sequence set transformed from/to a temporal
 * geometry/geography point
 * @param[in] ss Temporal point sequence set
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @sqlop ::
 */
TSequenceSet *
tgeompointseqset_tgeogpointseqset(const TSequenceSet *ss, bool oper)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tgeompointseq_tgeogpointseq(TSEQUENCESET_SEQ_N(ss, i),
      oper);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal point transformed from/to a temporal
 * geometry/geography point
 * @param[in] temp Temporal point
 * @param[in] oper True when transforming from geometry to geography,
 * false otherwise
 * @return On error return @p NULL
 * @see #tgeompointinst_tgeogpointinst
 * @see #tgeompointseq_tgeogpointseq
 * @see #tgeompointseqset_tgeogpointseqset
 * @sqlop ::
 */
Temporal *
tgeompoint_tgeogpoint(const Temporal *temp, bool oper)
{
  assert(temp); assert(tgeo_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeompointinst_tgeogpointinst((TInstant *) temp,
        oper);
    case TSEQUENCE:
      return (Temporal *) tgeompointseq_tgeogpointseq((TSequence *) temp,
        oper);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeompointseqset_tgeogpointseqset(
        (TSequenceSet *) temp, oper);
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal geometry point transformed to a temporal geography
 * point
 * @param[in] temp Temporal point
 * @csqlfn #Tgeompoint_to_tgeogpoint()
 */
Temporal *
tgeompoint_to_tgeogpoint(const Temporal *temp)
{
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TGEOMPOINT))
    return NULL;
  return tgeompoint_tgeogpoint(temp, GEOM_TO_GEOG);
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal geography point transformed to a temporal geometry
 * point
 * @param[in] temp Temporal point
 * @csqlfn #Tgeogpoint_to_tgeompoint()
 */
Temporal *
tgeogpoint_to_tgeompoint(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TGEOGPOINT))
    return NULL;
  return tgeompoint_tgeogpoint(temp, GEOG_TO_GEOM);
}
#endif /* MEOS */

/*****************************************************************************
 * Convert a temporal point into a PostGIS geometry/geography where the M
 * coordinates are
 * - either given in the second parameter
 * - or encode the timestamps of the temporal point in Unix epoch
 *
 * Notice that the original subtype is lost in the translation since when
 * converting back and forth a temporal point and a geometry/geography,
 * the minimal subtype is used. Therefore,
 * - an instantaneous sequence converted back and forth will result into an
 *   instant
 * - a singleton sequence set converted back and forth will result into a
 *   sequence
 * This does not affect equality since in MobilityDB equality of temporal types
 * does not take into account the subtype but the temporal semantics. However,
 * this may be an issue when the column of a table is restricted to a given
 * temporal subtype using a type modifier or typmod. We refer to the MobilityDB
 * manual for understanding how to restrict columns of tables using typmod.
 *
 * Notice also that the step interpolation is lost in the translation.
 * Therefore, when converting back and forth a temporal sequence (set) with
 * step interpolation to a geometry/geography the result will be a temporal
 * sequence with step interpolation.

 * Notice also that the temporal bounds are lost in the translation.
 * By default, the temporal bounds are set to true when converting back from a
 * geometry/geography to a temporal point.

 * Therefore, the equivalence
 * temp == (temp::geometry/geography)::tgeompoint/tgeogpoint
 * is true ONLY IF all temporal bounds are true and for temporal points with
 * linear interpolation
 *****************************************************************************/

/**
 * @brief Return a geometry/geography point and a measure transformed into a
 * PostGIS point with M coordinates
 */
static LWGEOM *
point_meas_to_lwpoint(Datum point, Datum meas)
{
  GSERIALIZED *gs = DatumGetGserializedP(point);
  int32 srid = gserialized_get_srid(gs);
  int hasz = FLAGS_GET_Z(gs->gflags);
  int geodetic = FLAGS_GET_GEODETIC(gs->gflags);
  double d = DatumGetFloat8(meas);
  LWPOINT *lwresult;
  if (hasz)
  {
    const POINT3DZ *pt = GSERIALIZED_POINT3DZ_P(gs);
    lwresult = lwpoint_make4d(srid, pt->x, pt->y, pt->z, d);
  }
  else
  {
    const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
    lwresult = lwpoint_make3dm(srid, pt->x, pt->y, d);
  }
  FLAGS_SET_Z(lwresult->flags, hasz);
  FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  return (LWGEOM *) lwresult;
}

/**
 * @brief Return a geometry/geography with M measure from the temporal
 * point instant and optionally a temporal float (iterator function)
 * @param[in] inst Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 * @pre The temporal point and the measure are synchronized
 */
static LWGEOM *
tpointinst_to_geomeas_iter(const TInstant *inst, const TInstant *meas)
{
  Datum m;
  if (meas)
    m = tinstant_val(meas);
  else
  {
    double epoch = ((double) inst->t / 1e6) + DELTA_UNIX_POSTGRES_EPOCH;
    m = Float8GetDatum(epoch);
  }
  return point_meas_to_lwpoint(tinstant_val(inst), m);
}

/**
 * @brief Return a geometry/geography with M measure from a temporal point
 * instant and optionally a temporal float
 * @param[in] inst Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 * @pre The temporal point and the measure are synchronized
 */
static GSERIALIZED *
tpointinst_to_geomeas(const TInstant *inst, const TInstant *meas)
{
  LWGEOM *lwresult = tpointinst_to_geomeas_iter(inst, meas);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Return a geometry/geography with M measure from the temporal
 * point discrete sequence and optionally a temporal float
 * @param[in] seq Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 * @pre The temporal point and the measure are synchronized
 */
static GSERIALIZED *
tpointseq_disc_to_geomeas(const TSequence *seq, const TSequence *meas)
{
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const TInstant *m = meas ? TSEQUENCE_INST_N(meas, 0) : NULL;

  /* Instantaneous sequence */
  if (seq->count == 1)
    return tpointinst_to_geomeas(inst, m);

  /* General case */
  int32 srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    m = meas ? TSEQUENCE_INST_N(meas, i) : NULL;
    points[i] = tpointinst_to_geomeas_iter(inst, m);
  }
  GSERIALIZED *result;
  if (seq->count == 1)
  {
    result = geo_serialize(points[0]);
    lwgeom_free(points[0]); pfree(points);
  }
  else
  {
    LWGEOM *lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, srid,
      NULL, (uint32_t) seq->count, points);
    FLAGS_SET_Z(lwresult->flags, hasz);
    FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
    result = geo_serialize(lwresult);
    lwgeom_free(lwresult);
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a geometry/geography with M measure from the temporal
 * point sequence and optionally a temporal float
 * @param[in] seq Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 * @pre The temporal point and the measure are synchronized
 * @note The function does not add a point if is equal to the previous one.
 */
static GSERIALIZED *
tpointseq_cont_to_geomeas(const TSequence *seq, const TSequence *meas)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return tpointinst_to_geomeas(TSEQUENCE_INST_N(seq, 0),
      meas ? TSEQUENCE_INST_N(meas, 0) : NULL);

  /* General case */
  int32 srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  LWGEOM **points = palloc(sizeof(LWPOINT *) * seq->count);
  /* Keep the first point */
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const TInstant *m = meas ? TSEQUENCE_INST_N(meas, 0) : NULL;
  LWGEOM *value1 = tpointinst_to_geomeas_iter(inst, m);
  points[0] = value1;
  int npoints = 1;
  for (int i = 1; i < seq->count; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i);
    m = meas ? TSEQUENCE_INST_N(meas, i) : NULL;
    LWGEOM *value2 = tpointinst_to_geomeas_iter(inst, m);
    /* Do not add a point if it is equal to the previous one */
    if (lwpoint_same((LWPOINT *) value1, (LWPOINT *) value2) != LW_TRUE)
    {
      points[npoints++] = value2;
      value1 = value2;
    }
    else
      lwgeom_free(value2);
  }
  LWGEOM *lwresult;
  if (npoints == 1)
  {
    lwresult = points[0];
    pfree(points);
  }
  else
  {
    if (linear)
    {
      lwresult = (LWGEOM *) lwline_from_lwgeom_array(srid, (uint32_t) npoints,
        points);
      for (int i = 0; i < npoints; i++)
        lwgeom_free(points[i]);
      pfree(points);
    }
    else
    {
      lwresult = (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, srid, NULL,
        (uint32_t) npoints, points);
    }
  }
  FLAGS_SET_Z(lwresult->flags, hasz);
  FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Return a geometry/geography with M measure from a temporal
 * point sequence set and optionally a temporal float
 * @param[in] ss Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 * @pre The temporal point and the measure are synchronized
 * @note This function has a similar algorithm as #tpointseqset_trajectory
 */
static GSERIALIZED *
tpointseqset_to_geomeas(const TSequenceSet *ss, const TSequenceSet *meas)
{
  const TSequence *seq1, *seq2;

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss, 0);
    seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, 0) : NULL;
    return tpointseq_cont_to_geomeas(seq1, seq2);
  }

  int32 srid = tpointseqset_srid(ss);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(ss->flags);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * ss->count);
  int npoints = 0, nlines = 0;
  /* Iterate as in #tpointseq_to_geomeas accumulating the results */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    const TSequence *seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, i) : NULL;
    /* Keep the first point */
    const TInstant *inst = TSEQUENCE_INST_N(seq1, 0);
    const TInstant *m = meas ? TSEQUENCE_INST_N(seq2, 0) : NULL;
    LWGEOM *value1 = tpointinst_to_geomeas_iter(inst, m);
    /* npoints is the current number of points so far, k is the number of
     * additional points from the current sequence */
    points[npoints] = value1;
    int k = 1;
    for (int j = 1; j < seq1->count; j++)
    {
      inst = TSEQUENCE_INST_N(seq1, j);
      m = meas ? TSEQUENCE_INST_N(seq2, j) : NULL;
      LWGEOM *value2 = tpointinst_to_geomeas_iter(inst, m);
      /* Do not add a point if it is equal to the previous one */
      if (lwpoint_same((LWPOINT *) value1, (LWPOINT *) value2) != LW_TRUE)
      {
        points[npoints + k++] = value2;
        value1 = value2;
      }
      else
        lwgeom_free(value2);
    }
    if (linear && k > 1)
    {
      lines[nlines] = (LWGEOM *) lwline_from_lwgeom_array(srid, (uint32_t) k,
        &points[npoints]);
      FLAGS_SET_Z(lines[nlines]->flags, hasz);
      FLAGS_SET_GEODETIC(lines[nlines]->flags, geodetic);
      nlines++;
      for (int j = 0; j < k; j++)
        lwgeom_free(points[npoints + j]);
    }
    else
      npoints += k;
  }
  LWGEOM *lwresult = lwcoll_from_points_lines(points, lines, npoints, nlines);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult); pfree(points); pfree(lines);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a geometry/geography with M measure from a temporal
 * sequence point and optionally a temporal float
 * @param[in] seq Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 * @result The result is a (Multi)Point when there are only instantaneous
 * sequences or a (Multi)linestring when each composing linestring corresponds
 * to a segment of a sequence of the temporal point
 */
static GSERIALIZED *
tpointseq_cont_to_geomeas_segm(const TSequence *seq, const TSequence *meas)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    /* Result is a point */
    return tpointinst_to_geomeas(TSEQUENCE_INST_N(seq, 0),
      meas ? TSEQUENCE_INST_N(meas, 0) : NULL);

  /* General case */
  int32 srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  const TInstant *m = meas ? TSEQUENCE_INST_N(meas, 0) : NULL;
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * (seq->count - 1));
  LWGEOM *points[2];
  points[0] = tpointinst_to_geomeas_iter(inst, m);
  for (int i = 0; i < seq->count - 1; i++)
  {
    inst = TSEQUENCE_INST_N(seq, i + 1);
    m = meas ? TSEQUENCE_INST_N(meas, i + 1) : NULL;
    points[1] = tpointinst_to_geomeas_iter(inst, m);
    lines[i] = (LWGEOM *) lwline_from_lwgeom_array(srid, 2, points);
    FLAGS_SET_Z(lines[i]->flags, hasz);
    FLAGS_SET_GEODETIC(lines[i]->flags, geodetic);
    lwgeom_free(points[0]);
    points[0] = points[1];
  }
  lwgeom_free(points[0]);
  LWGEOM *lwresult;
  if (seq->count == 2)
  {
    /* Result is a linestring */
    lwresult = lines[0];
    pfree(lines);
  }
  else
  {
    /* Result is a multilinestring */
    lwresult = (LWGEOM *) lwcollection_construct(MULTILINETYPE, srid, NULL,
      (uint32_t) seq->count - 1, lines);
    FLAGS_SET_Z(lwresult->flags, hasz);
    FLAGS_SET_GEODETIC(lwresult->flags, geodetic);
  }
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult);
  return result;
}

/**
 * @brief Return a geometry/geography with M measure from a temporal
 * point sequence set and optionally a temporal float
 * @return The result is a (Multi)Point when there are only
 * instantaneous sequences or a (Multi)linestring when each composing
 * linestring corresponds to a segment of a sequence of the temporal point
 * @param[in] ss Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 */
static GSERIALIZED *
tpointseqset_to_geomeas_segm(const TSequenceSet *ss, const TSequenceSet *meas)
{
  const TSequence *seq1, *seq2;

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss, 0);
    seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, 0) : NULL;
    return tpointseq_cont_to_geomeas_segm(seq1, seq2);
  }

  int32 srid = tpointseqset_srid(ss);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(ss->flags);
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  LWGEOM **lines = palloc(sizeof(LWGEOM *) * ss->totalcount);
  int npoints = 0, nlines = 0;
  /* Iterate as in #tpointseq_to_geomeas_segm accumulating the results */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    const TSequence *seq2 = (meas) ? TSEQUENCESET_SEQ_N(meas, i) : NULL;
    /* Keep the first point */
    const TInstant *inst = TSEQUENCE_INST_N(seq1, 0);
    const TInstant *m = meas ? TSEQUENCE_INST_N(seq2, 0) : NULL;
    /* npoints is the current number of points so far, k is the number of
     * additional points from the current sequence */
    points[npoints] = tpointinst_to_geomeas_iter(inst, m);
    if (seq1->count == 1)
    {
      /* Add a point for the current sequence */
      npoints++;
      continue;
    }
    /* Add lines for each segment of the current sequence */
    for (int j = 1; j < seq1->count; j++)
    {
      inst = TSEQUENCE_INST_N(seq1, j);
      m = meas ? TSEQUENCE_INST_N(seq2, j) : NULL;
      points[npoints + 1] = tpointinst_to_geomeas_iter(inst, m);
      lines[nlines] = (LWGEOM *) lwline_from_lwgeom_array(srid, 2,
        &points[npoints]);
      FLAGS_SET_Z(lines[nlines]->flags, hasz);
      FLAGS_SET_GEODETIC(lines[nlines]->flags, geodetic);
      nlines++;
      lwgeom_free(points[npoints]);
      points[npoints] = points[npoints + 1];
    }
    lwgeom_free(points[npoints]);
  }
  LWGEOM *lwresult = lwcoll_from_points_lines(points, lines, npoints, nlines);
  GSERIALIZED *result = geo_serialize(lwresult);
  lwgeom_free(lwresult); pfree(points); pfree(lines);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a geometry/geography with M measure from a temporal point
 * point and optionally a temporal float
 * @details If a temporal float is given the M coordinates of the result encode
 * the float values. Otherwise, the M coordinates encode the time information
 * of the temporal point in Unix time, that is, the number of seconds since
 * '1970-01-01'
 * @param[in] tpoint Temporal point
 * @param[in] meas Temporal float, may be @p NULL
 * @param[in] segmentize When true, in the general case the resulting geometry
 * will be a MultiLineString composed of one Linestring per segment of the
 * temporal sequence (set)
 * @param[out] result Resulting geometry array
 * @csqlfn #Tpoint_to_geomeas(), #Tpoint_tfloat_to_geomeas()
 */
bool
tpoint_tfloat_to_geomeas(const Temporal *tpoint, const Temporal *meas,
  bool segmentize, GSERIALIZED **result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) tpoint) ||
      ! ensure_not_null((void *) result) ||
      ! ensure_tgeo_type(tpoint->temptype) ||
      (meas && ! ensure_tnumber_type(meas->temptype)))
    return false;

  Temporal *sync1, *sync2;
  if (meas)
  {
    /* Return false if the temporal values do not intersect in time
     * The operation is synchronization without adding crossings */
    if (! intersection_temporal_temporal(tpoint, meas, SYNCHRONIZE_NOCROSS,
        &sync1, &sync2))
      return false;
  }
  else
  {
    sync1 = (Temporal *) tpoint;
    sync2 = NULL;
  }

  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
      *result = tpointinst_to_geomeas((TInstant *) sync1, (TInstant *) sync2);
      break;
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_DISCRETE_INTERP(sync1->flags))
        *result = tpointseq_disc_to_geomeas((TSequence *) sync1,
          (TSequence *) sync2);
      else
        *result = segmentize ?
          tpointseq_cont_to_geomeas_segm(
            (TSequence *) sync1, (TSequence *) sync2) :
          tpointseq_cont_to_geomeas(
            (TSequence *) sync1, (TSequence *) sync2);
      break;
    }
    default: /* TSEQUENCESET */
      *result = segmentize ?
        tpointseqset_to_geomeas_segm(
          (TSequenceSet *) sync1, (TSequenceSet *) sync2) :
        tpointseqset_to_geomeas(
          (TSequenceSet *) sync1, (TSequenceSet *) sync2);
  }
  if (meas)
  {
    pfree(sync1); pfree(sync2);
  }
  return true;
}

/*****************************************************************************
 * Convert trajectory geometry/geography where the M coordinates encode the
 * timestamps in Unix epoch into a temporal point.
 *****************************************************************************/

/**
 * @brief Return a geometry/geography where the M coordinates encode the
 * timestamps in Unix epoch transformed into a temporal point instant
 */
static TInstant *
geomeas_to_tpointinst_iter(LWPOINT *lwpoint)
{
  bool hasz = (bool) FLAGS_GET_Z(lwpoint->flags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(lwpoint->flags);
  LWPOINT *lwpoint1;
  TimestampTz t;
  if (hasz)
  {
    POINT4D point = getPoint4d(lwpoint->point, 0);
    t = (TimestampTz) ((point.m - DELTA_UNIX_POSTGRES_EPOCH) * 1e6);
    lwpoint1 = lwpoint_make3dz(lwpoint->srid, point.x, point.y, point.z);
  }
  else
  {
    POINT3DM point = getPoint3dm(lwpoint->point, 0);
    t = (TimestampTz) ((point.m - DELTA_UNIX_POSTGRES_EPOCH) * 1e6);
    lwpoint1 = lwpoint_make2d(lwpoint->srid, point.x, point.y);
  }
  FLAGS_SET_Z(lwpoint1->flags, hasz);
  FLAGS_SET_GEODETIC(lwpoint1->flags, geodetic);
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint1);
  lwpoint_free(lwpoint1);
  return tinstant_make_free(PointerGetDatum(gs),
    geodetic ? T_TGEOGPOINT : T_TGEOMPOINT, t);
}

/**
 * @brief Return the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch transformed into a temporal
 * point instant
 */
static TInstant *
geomeas_to_tpointinst(const LWGEOM *geom)
{
  /* Geometry is a POINT */
  return geomeas_to_tpointinst_iter((LWPOINT *) geom);
}

/**
 * @brief Ensure that a PostGIS trajectory has increasing timestamps
 * @note The verification is made in this function since calling the PostGIS
 * function lwgeom_is_trajectory causes discrepancies with regression tests
 * due to the error message that varies across PostGIS versions.
 */
static bool
ensure_valid_trajectory(const LWGEOM *geom, bool hasz, bool discrete)
{
  assert(geom->type != MULTIPOINTTYPE || geom->type != MULTILINETYPE);
  LWCOLLECTION *coll = NULL;
  LWLINE *line = NULL;
  uint32_t npoints;
  if (discrete)
  {
    coll = lwgeom_as_lwcollection(geom);
    npoints = coll->ngeoms;
  }
  else
  {
    line = lwgeom_as_lwline(geom);
    npoints = line->points->npoints;
  }
  double m1 = -1 * DBL_MAX, m2;
  for (uint32_t i = 0; i < npoints; i++)
  {
    const POINTARRAY *pa = discrete ?
      ((LWPOINT *) coll->geoms[i])->point : line->points;
    uint32_t where = discrete ? 0 : i;
    if (hasz)
    {
      POINT4D point = getPoint4d(pa, where);
      m2 = point.m;
    }
    else
    {
      POINT3DM point = getPoint3dm(pa, where);
      m2 = point.m;
    }
    if (m1 >= m2)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Trajectory must be valid");
      return false;
    }
    m1 = m2;
  }
  return true;
}

/**
 * @brief Return the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps transformed into Unix epoch into a
 * temporal point discrete sequence
 */
static TSequence *
geomeas_to_tpointseq_disc(const LWGEOM *geom, bool hasz)
{
  /* Verify that the trajectory is valid */
  if (! ensure_valid_trajectory(geom, hasz, true))
    return NULL;

  /* Geometry is a MULTIPOINT */
  LWCOLLECTION *coll = lwgeom_as_lwcollection(geom);
  uint32_t npoints = coll->ngeoms;
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (uint32_t i = 0; i < npoints; i++)
    instants[i] = geomeas_to_tpointinst_iter((LWPOINT *) coll->geoms[i]);
  return tsequence_make_free(instants, npoints, true, true, DISCRETE,
    NORMALIZE);
}

/**
 * @brief Return the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch transformed into a temporal
 * point sequence
 * @note Notice that it is not possible to encode step interpolation in
 * PostGIS and thus sequences obtained will be either discrete or linear.
 */
static TSequence *
geomeas_to_tpointseq_linear(const LWGEOM *geom, bool hasz, bool geodetic)
{
  /* Verify that the trajectory is valid */
  if (! ensure_valid_trajectory(geom, hasz, false))
    return NULL;
  /* Geometry is a LINESTRING */
  LWLINE *lwline = lwgeom_as_lwline(geom);
  uint32_t npoints = lwline->points->npoints;
  TInstant **instants = palloc(sizeof(TInstant *) * npoints);
  for (uint32_t i = 0; i < npoints; i++)
  {
    /* Return freshly allocated LWPOINT */
    LWPOINT *lwpoint = lwline_get_lwpoint(lwline, i);
    /* Function lwline_get_lwpoint lose the geodetic flag if any */
    FLAGS_SET_Z(lwpoint->flags, hasz);
    FLAGS_SET_GEODETIC(lwpoint->flags, geodetic);
    instants[i] = geomeas_to_tpointinst_iter(lwpoint);
    lwpoint_free(lwpoint);
  }
  /* The resulting sequence assumes linear interpolation */
  return tsequence_make_free(instants, npoints, true, true, LINEAR, NORMALIZE);
}

/**
 * @brief Return the PostGIS trajectory geometry/geography where the M
 * coordinates encode the timestamps in Unix epoch transformed into a temporal
 * point sequence set
 * @note With respect to functions #geomeas_to_tpointseq_disc and
 * #geomeas_to_tpointseq_linear, there is no validation of the trajectory since
 * it is more elaborated to be done. Nevertheless, erroneous geometries where
 * the timestamps are not increasing will be detected by the constructor of
 * the sequence set.
 */
static TSequenceSet *
geomeas_to_tpointseqset(const LWGEOM *geom, bool hasz, bool geodetic)
{
  /* Geometry is a MULTILINESTRING or a COLLECTION composed of (MULTI)POINT and
   * (MULTI)LINESTRING */
  LWCOLLECTION *coll = lwgeom_as_lwcollection(geom);
  int ngeoms = coll->ngeoms;
  int totalgeoms = 0;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *geom1 = coll->geoms[i];
    if (geom1->type != POINTTYPE && geom1->type != MULTIPOINTTYPE &&
        geom1->type != LINETYPE && geom1->type != MULTILINETYPE)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
        "Component geometry/geography must be of type (Multi)Point(Z)M or (Multi)Linestring(Z)M");
      return NULL;
    }
    if (geom1->type == POINTTYPE || geom1->type == LINETYPE)
      totalgeoms++;
    else /* geom1->type == MULTIPOINTTYPE || geom1->type == MULTILINETYPE */
      totalgeoms += lwgeom_as_lwcollection(geom1)->ngeoms;
  }

  TSequence **sequences = palloc(sizeof(TSequence *) * totalgeoms);
  int nseqs = 0;
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *geom1 = coll->geoms[i];
    if (geom1->type == POINTTYPE)
    {
      TInstant *inst1 = geomeas_to_tpointinst(geom1);
      /* The resulting sequence assumes linear interpolation */
      sequences[nseqs++] = tinstant_to_tsequence_free(inst1, LINEAR);
    }
    else if (geom1->type == LINETYPE)
      sequences[nseqs++] = geomeas_to_tpointseq_linear(geom1, hasz, geodetic);
    else /* geom1->type == MULTIPOINTTYPE || geom1->type == MULTILINETYPE */
    {
      LWCOLLECTION *coll1 = lwgeom_as_lwcollection(geom1);
      int ngeoms1 = coll1->ngeoms;
      for (int j = 0; j < ngeoms1; j++)
      {
        LWGEOM *geom2 = coll1->geoms[j];
        if (geom2->type == POINTTYPE)
        {
          TInstant *inst2 = geomeas_to_tpointinst(geom2);
          /* The resulting sequence assumes linear interpolation */
          sequences[nseqs++] = tinstant_to_tsequence_free(inst2, LINEAR);
        }
        else /* geom2->type == LINETYPE */
          sequences[nseqs++] = geomeas_to_tpointseq_linear(geom2, hasz, geodetic);
      }
    }
  }
  /* It is necessary to sort the sequences */
  tseqarr_sort(sequences, nseqs);
  /* The resulting sequence set assumes linear interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE_NO);
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a geometry/geography with M measure encoding timestamps
 * transformed to a temporal point
 * @param[in] gs Geometry
 * @csqlfn #Geomeas_to_tpoint()
 */
Temporal *
geomeas_to_tpoint(const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_empty(gs) ||
      ! ensure_has_M_gs(gs))
    return NULL;

  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool geodetic = (bool) FLAGS_GET_GEODETIC(gs->gflags);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  Temporal *result = NULL;
  if (geom->type == POINTTYPE)
    result = (Temporal *) geomeas_to_tpointinst(geom);
  else if (geom->type == MULTIPOINTTYPE)
    result = (Temporal *) geomeas_to_tpointseq_disc(geom, hasz);
  else if (geom->type == LINETYPE)
    result = (Temporal *) geomeas_to_tpointseq_linear(geom, hasz, geodetic);
  else if (geom->type == MULTILINETYPE || geom->type == COLLECTIONTYPE)
    result = (Temporal *) geomeas_to_tpointseqset(geom, hasz, geodetic);
  else
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Invalid geometry type for trajectory");
  lwgeom_free(geom);
  return result;
}

/*****************************************************************************
 * Mapbox Vector Tile functions for temporal points.
 *****************************************************************************/

/**
 * @brief Return a temporal point with consecutive equal points removed
 * @note The equality test is done only on the X and Y dimensions
 */
static TSequence *
tpointseq_remove_repeated_points(const TSequence *seq, double tolerance,
  int min_points)
{
  /* No-op on short inputs */
  if (seq->count <= min_points)
    return tsequence_copy(seq);

  double tolsq = tolerance * tolerance;
  double dsq = FLT_MAX;

  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  instants[0] = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *last = DATUM_POINT2D_P(tinstant_val(instants[0]));
  int npoints = 1;
  for (int i = 1; i < seq->count; i++)
  {
    bool last_point = (i == seq->count - 1);
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const POINT2D *pt = DATUM_POINT2D_P(tinstant_val(inst));

    /* Don't drop points if we are running short of points */
    if (seq->count - i > min_points - npoints)
    {
      if (tolerance > 0.0)
      {
        /* Only drop points that are within our tolerance */
        dsq = distance2d_sqr_pt_pt(last, pt);
        /* Allow any point but the last one to be dropped */
        if (! last_point && dsq <= tolsq)
          continue;
      }
      else
      {
        /* At tolerance zero, only skip exact dupes */
        if (FP_EQUALS(pt->x, last->x) && FP_EQUALS(pt->y, last->y))
          continue;
      }

      /* Got to last point, and it's not very different from
       * the point that preceded it. We want to keep the last
       * point, not the second-to-last one, so we pull our write
       * index back one value */
      if (last_point && npoints > 1 && tolerance > 0.0 && dsq <= tolsq)
      {
        npoints--;
      }
    }

    /* Save the point */
    instants[npoints++] = inst;
    last = pt;
  }
  /* Construct the result */
  TSequence *result = tsequence_make(instants, npoints, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  pfree(instants);
  return result;
}

/**
 * @brief Return a temporal point with consecutive equal points removed
 * @note The equality test is done only on the X and Y dimensions
 */
static TSequenceSet *
tpointseqset_remove_repeated_points(const TSequenceSet *ss, double tolerance,
  int min_points)
{
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    TSequence *seq1 = tpointseq_remove_repeated_points(
      TSEQUENCESET_SEQ_N(ss, 0), tolerance, min_points);
    return tsequence_to_tsequenceset_free(seq1);
  }

  /* No-op on short inputs */
  if (ss->totalcount <= min_points)
    return tsequenceset_copy(ss);

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int npoints = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Don't drop sequences if we are running short of points */
    if (ss->totalcount - npoints > min_points)
    {
      /* Minimum number of points set to 2 */
      sequences[i] = tpointseq_remove_repeated_points(seq, tolerance, 2);
      npoints += sequences[i]->count;
    }
    else
    {
      /* Save the sequence */
      sequences[i] = tsequence_copy(seq);
    }
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return a temporal point with consecutive equal points removed
 * @note The equality test is done only on the X and Y dimensions
 */
static Temporal *
tpoint_remove_repeated_points(const Temporal *temp, double tolerance,
  int min_points)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tpointseq_remove_repeated_points(
        (TSequence *) temp, tolerance, min_points);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_remove_repeated_points(
        (TSequenceSet *) temp, tolerance, min_points);
  }
}

/*****************************************************************************
 * Affine functions
 *****************************************************************************/

/**
 * @brief Return the affine transformation of a temporal point instant
 * (iterator function)
 */
static void
tpointinst_affine_iter(const TInstant *inst, const AFFINE *a, int srid,
  bool hasz, TInstant **result)
{
  Datum value = tinstant_val(inst);
  double x, y;
  LWPOINT *lwpoint;
  if (hasz)
  {
    const POINT3DZ *pt = DATUM_POINT3DZ_P(value);
    POINT3DZ p3d;
    x = pt->x;
    y = pt->y;
    double z = pt->z;
    p3d.x = a->afac * x + a->bfac * y + a->cfac * z + a->xoff;
    p3d.y = a->dfac * x + a->efac * y + a->ffac * z + a->yoff;
    p3d.z = a->gfac * x + a->hfac * y + a->ifac * z + a->zoff;
    lwpoint = lwpoint_make3dz(srid, p3d.x, p3d.y, p3d.z);
  }
  else
  {
    const POINT2D *pt = DATUM_POINT2D_P(value);
    POINT2D p2d;
    x = pt->x;
    y = pt->y;
    p2d.x = a->afac * x + a->bfac * y + a->xoff;
    p2d.y = a->dfac * x + a->efac * y + a->yoff;
    lwpoint = lwpoint_make2d(srid, p2d.x, p2d.y);
  }
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
  lwpoint_free(lwpoint);
  *result = tinstant_make_free(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
  return;
}

/**
 * @brief Return the affine transformation of a temporal point instant
 */
static TInstant *
tpointinst_affine(const TInstant *inst, const AFFINE *a)
{
  int srid = tpointinst_srid(inst);
  bool hasz = MEOS_FLAGS_GET_Z(inst->flags);
  TInstant *result;
  tpointinst_affine_iter(inst, a, srid, hasz, &result);
  return result;
}

/**
 * @brief Return the affine transform a temporal point sequence
 */
static TSequence *
tpointseq_affine(const TSequence *seq, const AFFINE *a)
{
  int srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    tpointinst_affine_iter(TSEQUENCE_INST_N(seq, i), a, srid, hasz,
      &instants[i]);
  /* Construct the result */
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Return the affine transformation of a temporal point sequence set
 * @param[in] ss Temporal point
 * @param[in] a Affine transformation
 */
static TSequenceSet *
tpointseqset_affine(const TSequenceSet *ss, const AFFINE *a)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tpointseq_affine(TSEQUENCESET_SEQ_N(ss, i), a);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Return the affine transform a temporal point
 */
static Temporal *
tpoint_affine(const Temporal *temp, const AFFINE *a)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpointinst_affine((TInstant *) temp, a);
    case TSEQUENCE:
      return (Temporal *) tpointseq_affine((TSequence *) temp, a);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_affine((TSequenceSet *) temp, a);
  }
}

/*****************************************************************************
 * Grid functions
 *****************************************************************************/

/**
 * @brief Return the last argument initialized with a temporal point snapped to
 * a grid specification
 */
static void
point_grid(Datum value, bool hasz, const gridspec *grid, POINT4D *p)
{
  /* Read and round point */
  datum_point4d(value, p);
  if (grid->xsize > 0)
    p->x = rint((p->x - grid->ipx) / grid->xsize) * grid->xsize + grid->ipx;
  if (grid->ysize > 0)
    p->y = rint((p->y - grid->ipy) / grid->ysize) * grid->ysize + grid->ipy;
  if (hasz && grid->zsize > 0)
    p->z = rint((p->z - grid->ipz) / grid->zsize) * grid->zsize + grid->ipz;
  return;
}

/**
 * @brief Return a temporal point instant snapped to a grid specification
 */
static TInstant *
tpointinst_grid(const TInstant *inst, const gridspec *grid)
{
  bool hasz = MEOS_FLAGS_GET_Z(inst->flags);
  if (grid->xsize == 0 && grid->ysize == 0 && (hasz ? grid->zsize == 0 : 1))
    return tinstant_copy(inst);

  int srid = tpointinst_srid(inst);
  Datum value = tinstant_val(inst);
  POINT4D p;
  point_grid(value, hasz, grid, &p);
  /* Write rounded values into the next instant */
  LWPOINT *lwpoint = hasz ?
    lwpoint_make3dz(srid, p.x, p.y, p.z) : lwpoint_make2d(srid, p.x, p.y);
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
  lwpoint_free(lwpoint);
  /* Construct the result */
  return tinstant_make_free(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
}

/**
 * @brief Return a temporal point sequence snapped to a grid specification
 */
static TSequence *
tpointseq_grid(const TSequence *seq, const gridspec *grid, bool filter_pts)
{
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  int srid = tpointseq_srid(seq);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int ninsts = 0;
  for (int i = 0; i < seq->count; i++)
  {
    POINT4D p, prev_p = { 0 }; /* make compiler quiet */
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    Datum value = tinstant_val(inst);
    point_grid(value, hasz, grid, &p);
    /* Skip duplicates */
    if (i > 1 && prev_p.x == p.x && prev_p.y == p.y &&
      (hasz ? prev_p.z == p.z : 1))
      continue;

    /* Write rounded values into the next instant */
    LWPOINT *lwpoint = hasz ?
      lwpoint_make3dz(srid, p.x, p.y, p.z) : lwpoint_make2d(srid, p.x, p.y);
    GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
    lwpoint_free(lwpoint);
    instants[ninsts++] = tinstant_make_free(PointerGetDatum(gs), T_TGEOMPOINT, inst->t);
    memcpy(&prev_p, &p, sizeof(POINT4D));
  }
  /* We are sure that ninsts > 0 */
  if (filter_pts && ninsts == 1)
  {
    pfree_array((void **) instants, 1);
    return NULL;
  }

  /* Construct the result */
  return tsequence_make_free(instants, ninsts, ninsts > 1 ?
    seq->period.lower_inc : true, ninsts > 1 ? seq->period.upper_inc : true,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Return a temporal point sequence set snapped to a grid specification
 */
static TSequenceSet *
tpointseqset_grid(const TSequenceSet *ss, const gridspec *grid, bool filter_pts)
{
  int nseqs = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tpointseq_grid(TSEQUENCESET_SEQ_N(ss, i), grid, filter_pts);
    if (seq != NULL)
      sequences[nseqs++] = seq;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Return a temporal point snapped to a grid specification
 * @details Only the X, Y, and possible Z dimensions are gridded, the timestamp
 * is kept unmodified. Two consecutive instants falling on the same grid cell
 * are collapsed into one single instant.
 */
static Temporal *
tpoint_grid(const Temporal *temp, const gridspec *grid, bool filter_pts)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tpointinst_grid((TInstant *) temp, grid);
    case TSEQUENCE:
      return (Temporal *) tpointseq_grid((TSequence *) temp, grid, filter_pts);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_grid((TSequenceSet *) temp, grid,
        filter_pts);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal point transformed into vector tile coordinate
 * space
 * @param[in] tpoint Temporal point
 * @param[in] box Geometric bounds of the tile contents without buffer
 * @param[in] extent Tile extent in tile coordinate space
 * @param[in] buffer Buffer distance in tile coordinate space
 * @param[in] clip_geom True if temporal point should be clipped
 */
static Temporal *
tpoint_mvt(const Temporal *tpoint, const STBox *box, uint32_t extent,
  uint32_t buffer, bool clip_geom)
{
  AFFINE affine = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  gridspec grid = {0, 0, 0, 0, 1, 1, 0, 0};
  double width = box->xmax - box->xmin;
  double height = box->ymax - box->ymin;
  double resx, resy, res, fx, fy;

  resx = width / extent;
  resy = height / extent;
  res = (resx < resy ? resx : resy) / 2;
  fx = extent / width;
  fy = -(extent / height);

  /* Remove all non-essential points (under the output resolution) */
  Temporal *tpoint1 = tpoint_remove_repeated_points(tpoint, res, 2);

  /* Euclidean (not synchronized) distance, i.e., parameter set to false */
  Temporal *tpoint2 = temporal_simplify_dp(tpoint1, res, false);
  pfree(tpoint1);

  /* Transform to tile coordinate space */
  affine.afac = fx;
  affine.efac = fy;
  affine.ifac = 1;
  affine.xoff = -box->xmin * fx;
  affine.yoff = -box->ymax * fy;
  Temporal *tpoint3 = tpoint_affine(tpoint2, &affine);
  pfree(tpoint2);

  /* Snap to integer precision, removing duplicate and single points */
  Temporal *tpoint4 = tpoint_grid(tpoint3, &grid, true);
  pfree(tpoint3);
  if (tpoint4 == NULL || ! clip_geom)
    return tpoint4;

  /* Clip temporal point taking into account the buffer */
  double max = (double) extent + (double) buffer;
  double min = -(double) buffer;
  int srid = tpoint_srid(tpoint);
  STBox clip_box;
  stbox_set(true, false, false, srid, min, max, min, max, 0, 0, NULL,
    &clip_box);
  Temporal *tpoint5 = tpoint_restrict_stbox(tpoint4, &clip_box, false,
    REST_AT);
  pfree(tpoint4);
  if (tpoint5 == NULL)
    return NULL;
  /* We need to grid again the result of the clipping */
  Temporal *result = tpoint_grid(tpoint5, &grid, true);
  pfree(tpoint5);
  return result;
}

/*****************************************************************************
 * Decouple the points and the timestamps of a temporal point.
 * With respect to the trajectory functions, e.g., #tpoint_trajectory,
 * the resulting geometry is not optimized in order to maintain the
 * composing points of the geometry and the associated timestamps synchronized
 *****************************************************************************/

/**
 * @brief Decouple the points and the timestamptz values of a temporal point
 * instant
 * @param[in] inst Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 * @note The function does not remove consecutive points/instants that are equal
 * @note The timestamps are returned in Unix epoch
 */
static GSERIALIZED *
tpointinst_decouple(const TInstant *inst, int64 **timesarr, int *count)
{
  int64 *times = palloc(sizeof(int64));
  times[0] = (inst->t / 1000000) + DELTA_UNIX_POSTGRES_EPOCH;
  *timesarr = times;
  *count = 1;
  return DatumGetGserializedP(tinstant_value(inst));
}

/**
 * @brief Decouple the points and the timestamptz values of a temporal point
 * sequence (iterator function)
 * @param[in] seq Temporal point
 * @param[out] times Array of timestamps
 * @note The function does not remove consecutive points/instants that are equal
 * @note The timestamps are returned in Unix epoch
 */
static LWGEOM *
tpointseq_decouple_iter(const TSequence *seq, int64 *times)
{
  /* General case */
  LWGEOM **points = palloc(sizeof(LWGEOM *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    Datum value = tinstant_val(inst);
    GSERIALIZED *gs = DatumGetGserializedP(value);
    points[i] = lwgeom_from_gserialized(gs);
    times[i] = (inst->t / 1000000) + DELTA_UNIX_POSTGRES_EPOCH;
  }
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  LWGEOM *result = lwpointarr_make_trajectory(points, seq->count, interp);
  if (interp == LINEAR)
  {
    for (int i = 0; i < seq->count; i++)
      lwpoint_free((LWPOINT *) points[i]);
    pfree(points);
  }
  return result;
}

/**
 * @brief Decouple the points and the timestamptz values of a temporal point
 * sequence
 * @param[in] seq Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 * @note The function does not remove consecutive points/instants that are equal
 * @note The timestamps are returned in Unix epoch
 */
static GSERIALIZED *
tpointseq_decouple(const TSequence *seq, int64 **timesarr, int *count)
{
  int64 *times = palloc(sizeof(int64) * seq->count);
  LWGEOM *geom = tpointseq_decouple_iter(seq, times);
  GSERIALIZED *result = geo_serialize(geom);
  *timesarr = times;
  *count = seq->count;
  pfree(geom);
  return result;
}

/**
 * @brief Decouple the points and the timestamptz values of a temporal point
 * sequence set
 * @param[in] ss Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 * @note The function does not remove consecutive points/instants that are equal
 * @note The timestamps are returned in Unix epoch
 */
static GSERIALIZED *
tpointseqset_decouple(const TSequenceSet *ss, int64 **timesarr, int *count)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_decouple(TSEQUENCESET_SEQ_N(ss, 0), timesarr, count);

  /* General case */
  uint32_t colltype = 0;
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * ss->count);
  int64 *times = palloc(sizeof(int64) * ss->totalcount);
  int ntimes = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    geoms[i] = tpointseq_decouple_iter(seq, &times[ntimes]);
    ntimes += seq->count;
    /* If output type not initialized make geom type as output type */
    if (! colltype)
      colltype = lwtype_get_collectiontype(geoms[i]->type);
    /* If geom type is not compatible with current output type
     * make output type a collection */
    else if (colltype != COLLECTIONTYPE &&
      lwtype_get_collectiontype(geoms[i]->type) != colltype)
      colltype = COLLECTIONTYPE;
  }
  LWGEOM *coll = (LWGEOM *) lwcollection_construct((uint8_t) colltype,
    geoms[0]->srid, NULL, (uint32_t) ss->count, geoms);
  GSERIALIZED *result = geo_serialize(coll);
  *timesarr = times;
  *count = ss->totalcount;
  /* We cannot lwgeom_free(geoms[i]) or pfree(geoms) */
  lwgeom_free(coll);
  return result;
}

/**
 * @brief Decouple the points and the timestamptz values of a temporal point
 * @param[in] temp Temporal point
 * @param[out] timesarr Array of timestamps encoded in Unix epoch
 * @param[out] count Number of elements in the output array
 * @note The function does not remove consecutive points/instants that are equal
 * @note The timestamps are returned in Unix epoch
 */
static GSERIALIZED *
tpoint_decouple(const Temporal *temp, int64 **timesarr, int *count)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tpointinst_decouple((TInstant *) temp, timesarr, count);
    case TSEQUENCE:
      return tpointseq_decouple((TSequence *) temp, timesarr, count);
    default: /* TSEQUENCESET */
      return tpointseqset_decouple((TSequenceSet *) temp, timesarr, count);
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal point transformed to Mapbox Vector Tile format
 * @param[in] temp Temporal point
 * @param[in] bounds Bounds
 * @param[in] extent Extent
 * @param[in] buffer Buffer
 * @param[in] clip_geom True when the geometry is clipped
 * @param[out] gsarr Array of geometries
 * @param[out] timesarr Array of timestamps
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tpoint_AsMVTGeom()
 */
bool
tpoint_AsMVTGeom(const Temporal *temp, const STBox *bounds, int32_t extent,
  int32_t buffer, bool clip_geom, GSERIALIZED **gsarr, int64 **timesarr,
  int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) bounds) ||
      ! ensure_not_null((void *) gsarr) ||
      ! ensure_not_null((void *) timesarr) ||
      ! ensure_not_null((void *) count) || ! ensure_tgeo_type(temp->temptype))
    return false;

  if (bounds->xmax - bounds->xmin <= 0 || bounds->ymax - bounds->ymin <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "%s: Geometric bounds are too small", __func__);
    return false;
  }
  if (extent <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "%s: Extent must be greater than 0", __func__);
    return false;
  }

  /* Contrary to what is done in PostGIS we do not use the following filter
   * to enable the visualization of temporal points with instant subtype.
   * PostGIS filtering adapted to MobilityDB would be as follows.

  / * Bounding box test to drop geometries smaller than the resolution * /
  STBox box;
  temporal_set_bbox(temp, &box);
  double tpoint_width = box.xmax - box.xmin;
  double tpoint_height = box.ymax - box.ymin;
  / * We use half of the square height and width as limit: We use this
   * and not area so it works properly with lines * /
  double bounds_width = ((bounds->xmax - bounds->xmin) / extent) / 2.0;
  double bounds_height = ((bounds->ymax - bounds->ymin) / extent) / 2.0;
  if (tpoint_width < bounds_width && tpoint_height < bounds_height)
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_RETURN_NULL();
  }
  */

  Temporal *temp1 = tpoint_mvt(temp, bounds, extent, buffer, clip_geom);
  if (temp1 == NULL)
    return false;

  /* Decouple the geometry and the timestamps */
  *gsarr = tpoint_decouple(temp1, timesarr, count);

  pfree(temp1);
  return true;
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

/**
 * @brief Return the length traversed by a temporal geometry point sequence
 * @pre The temporal point has linear interpolation
 */
static double
tpointseq_length_2d(const TSequence *seq)
{
  double result = 0;
  Datum start = tinstant_val(TSEQUENCE_INST_N(seq, 0));
  const POINT2D *p1 = DATUM_POINT2D_P(start);
  for (int i = 1; i < seq->count; i++)
  {
    Datum end = tinstant_val(TSEQUENCE_INST_N(seq, i));
    const POINT2D *p2 = DATUM_POINT2D_P(end);
    result += sqrt( ((p1->x - p2->x) * (p1->x - p2->x)) +
      ((p1->y - p2->y) * (p1->y - p2->y)) );
    p1 = p2;
  }
  return result;
}

/**
 * @brief Return the length traversed by a temporal geometry point sequence
 * @pre The temporal point has linear interpolation
 */
static double
tpointseq_length_3d(const TSequence *seq)
{
  double result = 0;
  Datum start = tinstant_val(TSEQUENCE_INST_N(seq, 0));
  const POINT3DZ *p1 = DATUM_POINT3DZ_P(start);
  for (int i = 1; i < seq->count; i++)
  {
    Datum end = tinstant_val(TSEQUENCE_INST_N(seq, i));
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(end);
    result += sqrt( ((p1->x - p2->x)*(p1->x - p2->x)) +
      ((p1->y - p2->y)*(p1->y - p2->y)) +
      ((p1->z - p2->z)*(p1->z - p2->z)) );
    p1 = p2;
  }
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal point sequence
 * @param[in] seq Temporal sequence
 * @csqlfn #Tpoint_length()
 */
double
tpointseq_length(const TSequence *seq)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  if (seq->count == 1)
    return 0;

  if (! MEOS_FLAGS_GET_GEODETIC(seq->flags))
  {
    return MEOS_FLAGS_GET_Z(seq->flags) ?
      tpointseq_length_3d(seq) : tpointseq_length_2d(seq);
  }
  else
  {
    /* We are sure that the trajectory is a line */
    GSERIALIZED *traj = tpointcontseq_trajectory(seq);
    double result = pgis_geography_length(traj, true);
    pfree(traj);
    return result;
  }
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_length()
 */
double
tpointseqset_length(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));
  double result = 0;
  for (int i = 0; i < ss->count; i++)
    result += tpointseq_length(TSEQUENCESET_SEQ_N(ss, i));
  return result;
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal point sequence (set)
 * @param[in] temp Temporal point
 * @return On error return -1.0
 * @csqlfn #Tpoint_length()
 */
double
tpoint_length(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return -1.0;

  assert(temptype_subtype(temp->subtype));
  if (! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return 0.0;
  else if (temp->subtype == TSEQUENCE)
    return tpointseq_length((TSequence *) temp);
  else /* TSEQUENCESET */
    return tpointseqset_length((TSequenceSet *) temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point sequence
 * @param[in] seq Temporal sequence
 * @param[in] prevlength Previous length to be added to the current sequence
 * @pre The sequence has linear interpolation
 * @csqlfn #Tpoint_cumulative_length()
 */
TSequence *
tpointseq_cumulative_length(const TSequence *seq, double prevlength)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(Float8GetDatum(prevlength), T_TFLOAT,
      TSEQUENCE_INST_N(seq, 0)->t);
    return tinstant_to_tsequence_free(inst, LINEAR);
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  datum_func2 func = pt_distance_fn(seq->flags);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_val(inst1);
  double length = prevlength;
  instants[0] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_val(inst2);
    if (! datum_point_eq(value1, value2))
      length += DatumGetFloat8(func(value1, value2));
    instants[i] = tinstant_make(Float8GetDatum(length), T_TFLOAT, inst2->t);
    value1 = value2;
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point sequence
 * set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_cumulative_length()
 */
TSequenceSet *
tpointseqset_cumulative_length(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  double length = 0;
  for (int i = 0; i < ss->count; i++)
  {
    sequences[i] = tpointseq_cumulative_length(TSEQUENCESET_SEQ_N(ss, i),
      length);
    /* sequences[i] may have less sequences than composing sequence due to
     * normalization */
    const TInstant *end = TSEQUENCE_INST_N(sequences[i], sequences[i]->count - 1);
    length = DatumGetFloat8(tinstant_val(end));
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point
 * @param[in] temp Temporal point
 * @return On error return @p NULL
 * @csqlfn #Tpoint_cumulative_length()
 */
Temporal *
tpoint_cumulative_length(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  if (! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return temporal_from_base_temp(Float8GetDatum(0.0), T_TFLOAT, temp);
  else if (temp->subtype == TSEQUENCE)
    return (Temporal *) tpointseq_cumulative_length((TSequence *) temp, 0);
  else /* TSEQUENCESET */
    return (Temporal *) tpointseqset_cumulative_length((TSequenceSet *) temp);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the convex hull of a temporal point
 * @param[in] temp Temporal point
 * @return On error return @p NULL
 * @csqlfn #Tpoint_convex_hull()
 */
GSERIALIZED *
tpoint_convex_hull(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return NULL;

  GSERIALIZED *traj = tpoint_trajectory(temp);
  GSERIALIZED *result = geometry_convex_hull(traj);
  pfree(traj);
  return result;
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the speed of a temporal point sequence
 * @param[in] seq Temporal sequence
 * @pre The temporal point has linear interpolation
 * @csqlfn #Tpoint_speed()
 */
TSequence *
tpointseq_speed(const TSequence *seq)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  datum_func2 func = pt_distance_fn(seq->flags);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_val(inst1);
  double speed = 0.0; /* make compiler quiet */
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    Datum value2 = tinstant_val(inst2);
    speed = datum_point_eq(value1, value2) ? 0.0 :
      DatumGetFloat8(func(value1, value2)) /
        ((double)(inst2->t - inst1->t) / 1000000.0);
    instants[i] = tinstant_make(Float8GetDatum(speed), T_TFLOAT, inst1->t);
    inst1 = inst2;
    value1 = value2;
  }
  instants[seq->count - 1] = tinstant_make(Float8GetDatum(speed), T_TFLOAT,
    seq->period.upper);
  /* The resulting sequence has step interpolation */
  TSequence *result = tsequence_make((const TInstant **) instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  pfree_array((void **) instants, seq->count - 1);
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the speed of a temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_speed()
 */
TSequenceSet *
tpointseqset_speed(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  assert(MEOS_FLAGS_LINEAR_INTERP(ss->flags));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (seq->count > 1)
      sequences[nseqs++] = tpointseq_speed(seq);
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the speed of a temporal point
 * @param[in] temp Temporal point
 * @return On error return @p NULL
 * @csqlfn #Tpoint_speed()
 */
Temporal *
tpoint_speed(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype) ||
      ! ensure_linear_interp(temp->flags))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return NULL;
    case TSEQUENCE:
      return (Temporal *) tpointseq_speed((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tpointseqset_speed((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

/**
 * @brief Return a temporal point sequence split into temporal float sequences
 * for each of its coordinates (iterator function)
 */
void
tpointseq_twcentroid_iter(const TSequence *seq, bool hasz, interpType interp,
  TSequence **seqx, TSequence **seqy, TSequence **seqz)
{
  TInstant **instantsx = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsy = palloc(sizeof(TInstant *) * seq->count);
  TInstant **instantsz = hasz ?
    palloc(sizeof(TInstant *) * seq->count) : NULL;

  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    POINT4D p;
    datum_point4d(tinstant_val(inst), &p);
    instantsx[i] = tinstant_make(Float8GetDatum(p.x), T_TFLOAT, inst->t);
    instantsy[i] = tinstant_make(Float8GetDatum(p.y), T_TFLOAT, inst->t);
    if (hasz)
      instantsz[i] = tinstant_make(Float8GetDatum(p.z), T_TFLOAT, inst->t);
  }
  *seqx = tsequence_make_free(instantsx, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
  *seqy = tsequence_make_free(instantsy, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
  if (hasz)
    *seqz = tsequence_make_free(instantsz, seq->count, seq->period.lower_inc,
      seq->period.upper_inc, interp, NORMALIZE);
  return;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the time-weighed centroid of a temporal geometry point
 * sequence
 * @param[in] seq Temporal sequence
 * @csqlfn #Tpoint_twcentroid()
 */
GSERIALIZED *
tpointseq_twcentroid(const TSequence *seq)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  int srid = tpointseq_srid(seq);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence *seqx, *seqy, *seqz;
  tpointseq_twcentroid_iter(seq, hasz, interp, &seqx, &seqy, &seqz);
  double twavgx = tnumberseq_twavg(seqx);
  double twavgy = tnumberseq_twavg(seqy);
  double twavgz = (hasz) ? tnumberseq_twavg(seqz) : 0.0;
  GSERIALIZED *result = geopoint_make(twavgx, twavgy, twavgz, hasz, false, srid);
  pfree(seqx); pfree(seqy);
  if (hasz)
    pfree(seqz);
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the time-weighed centroid of a temporal geometry point
 * sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_twcentroid()
 */
GSERIALIZED *
tpointseqset_twcentroid(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  int srid = tpointseqset_srid(ss);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  interpType interp = MEOS_FLAGS_GET_INTERP(ss->flags);
  TSequence **sequencesx = palloc(sizeof(TSequence *) * ss->count);
  TSequence **sequencesy = palloc(sizeof(TSequence *) * ss->count);
  TSequence **sequencesz = hasz ?
    palloc(sizeof(TSequence *) * ss->count) : NULL;
  for (int i = 0; i < ss->count; i++)
    tpointseq_twcentroid_iter(TSEQUENCESET_SEQ_N(ss, i), hasz, interp,
      &sequencesx[i], &sequencesy[i], &sequencesz[i]);
  TSequenceSet *ssx = tsequenceset_make_free(sequencesx, ss->count, NORMALIZE);
  TSequenceSet *ssy = tsequenceset_make_free(sequencesy, ss->count, NORMALIZE);
  TSequenceSet *ssz = hasz ?
    tsequenceset_make_free(sequencesz, ss->count, NORMALIZE) : NULL;

  double twavgx = tnumberseqset_twavg(ssx);
  double twavgy = tnumberseqset_twavg(ssy);
  double twavgz = hasz ? tnumberseqset_twavg(ssz) : 0;
  GSERIALIZED *result = geopoint_make(twavgx, twavgy, twavgz, hasz, false, srid);
  pfree(ssx); pfree(ssy);
  if (hasz)
    pfree(ssz);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the time-weighed centroid of a temporal geometry point
 * @param[in] temp Temporal point
 * @return On error return @p NULL
 * @csqlfn #Tpoint_twcentroid()
 */
GSERIALIZED *
tpoint_twcentroid(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return DatumGetGserializedP(tinstant_value((TInstant *) temp));
    case TSEQUENCE:
      return tpointseq_twcentroid((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tpointseqset_twcentroid((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

/**
 * @brief Return the azimuth of two geometry points
 */
static Datum
geom_azimuth(Datum geom1, Datum geom2)
{
  double result;
  azimuth_pt_pt(DATUM_POINT2D_P(geom1), DATUM_POINT2D_P(geom2), &result);
  return Float8GetDatum(result);
}

/**
 * @brief Return the azimuth two geography points
 */
static Datum
geog_azimuth(Datum geog1, Datum geog2)
{
  const GSERIALIZED *g1 = DatumGetGserializedP(geog1);
  const GSERIALIZED *g2 = DatumGetGserializedP(geog2);
  const LWGEOM *geom1 = lwgeom_from_gserialized(g1);
  const LWGEOM *geom2 = lwgeom_from_gserialized(g2);

  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);
  double result = lwgeom_azumith_spheroid(lwgeom_as_lwpoint(geom1),
    lwgeom_as_lwpoint(geom2), &s);
  return Float8GetDatum(result);
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the direction of a temporal point sequence
 * @param[in] seq Temporal sequence
 * @param[out] result Azimuth between the first and last point
 * @result True when it is possible to determine the azimuth, i.e., when there
 * are at least two points that are not equal; false, otherwise.
 * @csqlfn #Tpoint_direction()
 */
bool
tpointseq_direction(const TSequence *seq, double *result)
{
  assert(seq); assert(result); assert(tgeo_type(seq->temptype));
  /* Instantaneous sequence */
  if (seq->count == 1)
    return false;

  /* Determine the PostGIS function to call */
  datum_func2 func = MEOS_FLAGS_GET_GEODETIC(seq->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  Datum value1 = tinstant_val(TSEQUENCE_INST_N(seq, 0));
  Datum value2 = tinstant_val(TSEQUENCE_INST_N(seq, seq->count - 1));
  if (datum_point_eq(value1, value2))
    return false;

  *result = DatumGetFloat8(func(value1, value2));
  return true;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the direction of a temporal point sequence set
 * @param[in] ss Temporal sequence set
 * @param[out] result Azimuth between the first and last point
 * @result True when it is possible to determine the azimuth, i.e., when there
 * are at least two points that are not equal; false, otherwise.
 * @csqlfn #Tpoint_direction()
 */
bool
tpointseqset_direction(const TSequenceSet *ss, double *result)
{
  assert(ss); assert(result); assert(tgeo_type(ss->temptype));
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_direction(TSEQUENCESET_SEQ_N(ss, 0), result);

  /* Determine the PostGIS function to call */
  datum_func2 func = MEOS_FLAGS_GET_GEODETIC(ss->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, 0);
  const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss, ss->count - 1);
  Datum value1 = tinstant_val(TSEQUENCE_INST_N(seq1, 0));
  Datum value2 = tinstant_val(TSEQUENCE_INST_N(seq2, seq2->count - 1));
  if (datum_point_eq(value1, value2))
    return false;

  *result = DatumGetFloat8(func(value1, value2));
  return true;
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the direction of a temporal point
 * @param[in] temp Temporal point
 * @param[out] result Azimuth between the first and last point
 * @csqlfn #Tpoint_direction()
 */
bool
tpoint_direction(const Temporal *temp, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) result) ||
      ! ensure_tgeo_type(temp->temptype))
    return false;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return false;
    case TSEQUENCE:
      return tpointseq_direction((TSequence *) temp, result);
    default: /* TSEQUENCESET */
      return tpointseqset_direction((TSequenceSet *) temp, result);
  }
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

/**
 * @brief Return the temporal azimuth of a temporal geometry point sequence
 * (iterator function)
 * @param[in] seq Temporal value
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tpointseq_azimuth_iter(const TSequence *seq, TSequence **result)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* Determine the PostGIS function to call */
  datum_func2 func = MEOS_FLAGS_GET_GEODETIC(seq->flags) ?
    &geog_azimuth : &geom_azimuth;

  /* We are sure that there are at least 2 instants */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_val(inst1);
  int ninsts = 0, nseqs = 0;
  Datum azimuth = 0; /* Make the compiler quiet */
  bool lower_inc = seq->period.lower_inc;
  bool upper_inc = false; /* make compiler quiet */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_val(inst2);
    upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    if (! datum_point_eq(value1, value2))
    {
      azimuth = func(value1, value2);
      instants[ninsts++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
    }
    else
    {
      if (ninsts != 0)
      {
        instants[ninsts++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
        upper_inc = true;
        /* Resulting sequence has step interpolation */
        result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
          lower_inc, upper_inc, STEP, NORMALIZE);
        for (int j = 0; j < ninsts; j++)
          pfree(instants[j]);
        ninsts = 0;
      }
      lower_inc = true;
    }
    inst1 = inst2;
    value1 = value2;
  }
  if (ninsts != 0)
  {
    instants[ninsts++] = tinstant_make(azimuth, T_TFLOAT, inst1->t);
    /* Resulting sequence has step interpolation */
    result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      lower_inc, upper_inc, STEP, NORMALIZE);
  }

  pfree(instants);
  return nseqs;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point sequence
 * @param[in] seq Temporal sequence
 * @csqlfn #Tpoint_azimuth()
 */
TSequenceSet *
tpointseq_azimuth(const TSequence *seq)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int count = tpointseq_azimuth_iter(seq, sequences);
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_azimuth()
 */
TSequenceSet *
tpointseqset_azimuth(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_azimuth(TSEQUENCESET_SEQ_N(ss, 0));

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
    nseqs += tpointseq_azimuth_iter(TSEQUENCESET_SEQ_N(ss, i), &sequences[nseqs]);
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point
 * @param[in] temp Temporal point
 * @return On error return @p NULL
 * @csqlfn #Tpoint_azimuth()
 */
Temporal *
tpoint_azimuth(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  if (! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    return NULL;
  else if (temp->subtype == TSEQUENCE)
    return (Temporal *) tpointseq_azimuth((TSequence *) temp);
  else /* TSEQUENCESET */
    return (Temporal *) tpointseqset_azimuth((TSequenceSet *) temp);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the temporal angular difference of a temporal geometry point
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_angular_difference()
 */
Temporal *
tpoint_angular_difference(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return NULL;

  Temporal *tazimuth = tpoint_azimuth(temp);
  Temporal *result = NULL;
  if (tazimuth)
  {
    Temporal *tazimuth_deg = tfloat_degrees(tazimuth, false);
    result = tnumber_angular_difference(tazimuth_deg);
    pfree(tazimuth_deg);
  }
  return result;
}

/*****************************************************************************
 * Temporal bearing
 *****************************************************************************/

/**
 * @brief Normalize the bearing from -180° to + 180° (in radians) to
 * 0° to 360° (in radians)
 */
static double
alpha(const POINT2D *p1, const POINT2D *p2)
{
  if (p1->x <= p2->x && p1->y <= p2->y)
    return 0.0;
  if ((p1->x < p2->x && p1->y > p2->y) ||
      (p1->x >= p2->x && p1->y > p2->y))
    return M_PI;
  else /* p1->x > p2->x && p1->y <= p2->y */
    return M_PI * 2.0;
}

/**
 * @brief Return the bearing between two geometry points
 */
static Datum
geom_bearing(Datum point1, Datum point2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(point1);
  const POINT2D *p2 = DATUM_POINT2D_P(point2);
  if ((fabs(p1->x - p2->x) <= MEOS_EPSILON) &&
      (fabs(p1->y - p2->y) <= MEOS_EPSILON))
    return Float8GetDatum(0.0);
  if (fabs(p1->y - p2->y) > MEOS_EPSILON)
  {
    double bearing = pg_datan((p1->x - p2->x) / (p1->y - p2->y)) +
      alpha(p1, p2);
    if (fabs(bearing) <= MEOS_EPSILON)
      bearing = 0.0;
    return Float8GetDatum(bearing);
  }
  if (p1->x < p2->x)
    return Float8GetDatum(M_PI / 2.0);
  else
    return Float8GetDatum(M_PI * 3.0 / 2.0);
}

/**
 * @brief Return the bearing between two geography points
 * @details The formulae used are the following:
 * - lat  = sin(Δlong).cos(lat2)
 * - long = cos(lat1).sin(lat2) - sin(lat1).cos(lat2).cos(Δlong)
 * - θ    = atan2(lat, long)
 * @note Derived from https://gist.github.com/jeromer/2005586
 * @note In PostGIS, for geodetic coordinates, X is longitude and Y is latitude
 */
static Datum
geog_bearing(Datum point1, Datum point2)
{
  const POINT2D *p1 = DATUM_POINT2D_P(point1);
  const POINT2D *p2 = DATUM_POINT2D_P(point2);
  if ((fabs(p1->x - p2->x) <= MEOS_EPSILON) &&
      (fabs(p1->y - p2->y) <= MEOS_EPSILON))
    return Float8GetDatum(0.0);

  double lat1 = float8_mul(p1->y, RADIANS_PER_DEGREE);
  double lat2 = float8_mul(p2->y, RADIANS_PER_DEGREE);
  double diffLong = float8_mul(p2->x - p1->x, RADIANS_PER_DEGREE);
  double lat = pg_dsin(diffLong) * pg_dcos(lat2);
  double lgt = ( pg_dcos(lat1) * pg_dsin(lat2) ) -
    ( pg_dsin(lat1) * pg_dcos(lat2) * pg_dcos(diffLong) );
  /* Notice that the arguments are inverted, e.g., wrt the atan2 in Python */
  double initial_bearing = pg_datan2(lat, lgt);
  /* Normalize the bearing from -180° to + 180° (in radians) to
   * 0° to 360° (in radians) */
  double bearing = fmod(initial_bearing + M_PI * 2.0, M_PI * 2.0);
  return Float8GetDatum(bearing);
}

/**
 * @brief Select the appropriate bearing function
 */
static datum_func2
get_bearing_fn(int16 flags)
{
  return MEOS_FLAGS_GET_GEODETIC(flags) ? &geog_bearing : &geom_bearing;
}

/**
 * @brief Return the value and timestamptz at which the a temporal point
 * segment and a point are at the minimum bearing
 * @param[in] start,end Instants defining the segment
 * @param[in] point Geometric/geography point
 * @param[in] basetypid Base type
 * @param[out] value Value
 * @param[out] t Timestamp
 * @pre The segment is not constant and has linear interpolation
 * @note The parameter @p basetype is not needed for temporal points
 */
static bool
tpoint_geo_min_bearing_at_timestamptz(const TInstant *start,
  const TInstant *end, Datum point, meosType basetypid __attribute__((unused)),
  Datum *value, TimestampTz *t)
{
  Datum dstart = tinstant_val(start);
  Datum dend = tinstant_val(end);
  const POINT2D *p1 = DATUM_POINT2D_P(dstart);
  const POINT2D *p2 = DATUM_POINT2D_P(dend);
  const POINT2D *p = DATUM_POINT2D_P(point);
  const POINT2D *q;
  long double fraction;
  Datum proj = 0; /* make compiler quiet */
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(start->flags);
  if (geodetic)
  {
    GEOGRAPHIC_EDGE e, e1;
    GEOGRAPHIC_POINT gp, inter;
    geographic_point_init(p->x, p->y, &gp);
    geographic_point_init(p1->x, p1->y, &(e.start));
    geographic_point_init(p2->x, p2->y, &(e.end));
    if (! edge_contains_coplanar_point(&e, &gp))
      return false;
    /* Create an edge in the same meridian as p */
    geographic_point_init(p->x, 89.999999, &(e1.start));
    geographic_point_init(p->x, -89.999999, &(e1.end));
    edge_intersection(&e, &e1, &inter);
    proj = PointerGetDatum(geopoint_make(rad2deg(inter.lon),
      rad2deg(inter.lat), 0, false, true, tpointinst_srid(start)));
    fraction = geosegm_locate_point(dstart, dend, proj, NULL);
  }
  else
  {
    bool ds = (p1->x - p->x) > 0;
    bool de = (p2->x - p->x) > 0;
    /* If there is not a North passage */
    if (ds == de)
      return false;
    fraction = (long double)(p->x - p1->x) / (long double)(p2->x - p1->x);
  }
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return false;
  long double duration = (long double) (end->t - start->t);
  *t = start->t + (TimestampTz) (duration * fraction);
  *value = (Datum) 0;
  /* Compute the projected value only for geometries */
  if (! geodetic)
    proj = tsegment_value_at_timestamptz(start, end, LINEAR, *t);
  q = DATUM_POINT2D_P(proj);
  /* We add a turning point only if p is to the North of q */
  bool result = FP_GTEQ(p->y, q->y) ? true : false;
  pfree(DatumGetPointer(proj));
  return result;
}

/**
 * @brief Return the value and timestamptz at which two temporal point
 * segments are at the minimum bearing
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] value Value
 * @param[out] t Timestamp
 * @pre The segments are not both constants and are both linear
 * @note This function is currently not available for two temporal geographic
 * points
 */
static bool
tpointsegm_min_bearing_at_timestamptz(const TInstant *start1,
  const TInstant *end1, const TInstant *start2,
  const TInstant *end2, Datum *value, TimestampTz *t)
{
  assert(! MEOS_FLAGS_GET_GEODETIC(start1->flags));
  const POINT2D *sp1 = DATUM_POINT2D_P(tinstant_val(start1));
  const POINT2D *ep1 = DATUM_POINT2D_P(tinstant_val(end1));
  const POINT2D *sp2 = DATUM_POINT2D_P(tinstant_val(start2));
  const POINT2D *ep2 = DATUM_POINT2D_P(tinstant_val(end2));
  /* It there is a North passage we call the function
    tgeompoint_min_dist_at_timestamp */
  bool ds = (sp1->x - sp2->x) > 0;
  bool de = (ep1->x - ep2->x) > 0;
  if (ds == de)
    return false;

  /*
   * Compute the instants t1 and t2 at which the linear functions of the two
   * segments take the value 0: at1 + b = 0, ct2 + d = 0. There is a
   * minimum/maximum exactly at the middle between t1 and t2.
   * To reduce problems related to floating point arithmetic, t1 and t2
   * are shifted, respectively, to 0 and 1 before the computation
   * N.B. The code that follows is adapted from the function
   * #tnumber_arithop_tp_at_timestamp1 in file tnumber_mathfuncs.c
   */
  if ((ep1->x - sp1->x) == 0.0 || (ep2->x - sp2->x) == 0.0)
    return false;

  long double d1 = (-1 * sp1->x) / (ep1->x - sp1->x);
  long double d2 = (-1 * sp2->x) / (ep2->x - sp2->x);
  long double min = Min(d1, d2);
  long double max = Max(d1, d2);
  long double fraction = min + (max - min)/2;
  long double duration = (long double) (end1->t - start1->t);
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    /* Minimum/maximum occurs out of the period */
    return false;

  *t = start1->t + (TimestampTz) (duration * fraction);
  /* We need to verify that at timestamp t the first segment is to the
   * North of the second */
  Datum value1 = tsegment_value_at_timestamptz(start1, end1, LINEAR, *t);
  Datum value2 = tsegment_value_at_timestamptz(start2, end2, LINEAR, *t);
  sp1 = DATUM_POINT2D_P(value1);
  sp2 = DATUM_POINT2D_P(value2);
  if (sp1->y > sp2->y) // TODO Use MEOS_EPSILON
    return false;
 /* We know that the bearing is 0 */
  if (value)
    *value = Float8GetDatum(0.0);
  return true;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the temporal bearing between two geometry/geography points
 * @param[in] gs1,gs2 Geometries
 * @param[out] result Result
 * @note The following function could be included in PostGIS one day
 * @csqlfn #Bearing_point_point()
 */
bool
bearing_point_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs1) || ! ensure_not_null((void *) gs2) ||
      ! ensure_point_type(gs1) || ! ensure_point_type(gs2) ||
      ! ensure_same_srid(gserialized_get_srid(gs1), gserialized_get_srid(gs2)) ||
      ! ensure_same_dimensionality_gs(gs1, gs2))
    return false;

  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;
  *result = FLAGS_GET_GEODETIC(gs1->gflags) ?
    DatumGetFloat8(geog_bearing(PointerGetDatum(gs1), PointerGetDatum(gs2))) :
    DatumGetFloat8(geom_bearing(PointerGetDatum(gs1), PointerGetDatum(gs2)));
  return true;
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the temporal bearing between a temporal point and a point
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[out] invert True when the result should be inverted
 * @return On empty geometry or on error return NULL
 * @csqlfn #Bearing_tpoint_point()
 */
Temporal *
bearing_tpoint_point(const Temporal *temp, const GSERIALIZED *gs, bool invert)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_point_type(gs) ||
      ! ensure_same_dimensionality_tpoint_gs(temp, gs))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) get_bearing_fn(temp->flags);
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = &tpoint_geo_min_bearing_at_timestamptz;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return the temporal bearing between two temporal points
 * @param[in] temp1,temp2 Temporal points
 * @return On error return @p NULL
 * @csqlfn #Bearing_tpoint_tpoint()
 */
Temporal *
bearing_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_tpoint(temp1, temp2) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) )
    return NULL;

  datum_func2 func = get_bearing_fn(temp1->flags);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = lfinfo.reslinear ?
    &tpointsegm_min_bearing_at_timestamptz : NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/

/* Defined in liblwgeom_internal.h */
#define PGIS_FP_TOLERANCE 1e-12

/**
 * @brief Calculate the distance between two geography points given as GEOS
 * geometries
 */
static double
geog_distance_geos(const GEOSGeometry *pt1, const GEOSGeometry *pt2)
{
  /* Skip PostGIS function calls */
  double x1, y1, x2, y2;
  GEOSGeomGetX(pt1, &x1);
  GEOSGeomGetY(pt1, &y1);
  GEOSGeomGetX(pt2, &x2);
  GEOSGeomGetY(pt2, &y2);

  /* Code taken from ptarray_distance_spheroid function in lwgeodetic.c */

  GEOGRAPHIC_POINT g1, g2;
  geographic_point_init(x1, y1, &g1);
  geographic_point_init(x2, y2, &g2);

  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* Sphere special case, axes equal */
  double distance = s.radius * sphere_distance(&g1, &g2);
  if ( s.a == s.b )
    return distance;
  /* Below tolerance, actual distance isn't of interest */
  else if ( distance < 0.95 * PGIS_FP_TOLERANCE )
    return distance;
  /* Close or greater than tolerance, get the real answer to be sure */
  else
    return spheroid_distance(&g1, &g2, &s);
}

/**
 * @brief Calculate the length of the diagonal of the minimum rotated rectangle
 * of the input GEOS geometry
 * @return On error return -1.0
 * @note The computation is always done in 2D
 */
static double
mrr_distance_geos(GEOSGeometry *geom, bool geodetic)
{

  double result = 0;
  int numGeoms = GEOSGetNumGeometries(geom);
  if (numGeoms == 2)
  {
    const GEOSGeometry *pt1 = GEOSGetGeometryN(geom, 0);
    const GEOSGeometry *pt2 = GEOSGetGeometryN(geom, 1);
    if (geodetic)
      result = geog_distance_geos(pt1, pt2);
    else
      GEOSDistance(pt1, pt2, &result);
  }
  else if (numGeoms > 2)
  {
    GEOSGeometry *mrr_geom = GEOSMinimumRotatedRectangle(geom);
    GEOSGeometry *pt1, *pt2;
    switch (GEOSGeomTypeId(mrr_geom))
    {
      case GEOS_POINT:
        result = 0;
        break;
      case GEOS_LINESTRING: /* compute length of linestring */
        if (geodetic)
        {
          pt1 = GEOSGeomGetStartPoint(mrr_geom);
          pt2 = GEOSGeomGetEndPoint(mrr_geom);
          result = geog_distance_geos(pt1, pt2);
          GEOSGeom_destroy(pt1);
          GEOSGeom_destroy(pt2);
        }
        else
          GEOSGeomGetLength(mrr_geom, &result);
        break;
      case GEOS_POLYGON: /* compute length of diagonal */
        pt1 = GEOSGeomGetPointN(GEOSGetExteriorRing(mrr_geom), 0);
        pt2 = GEOSGeomGetPointN(GEOSGetExteriorRing(mrr_geom), 2);
        if (geodetic)
          result = geog_distance_geos(pt1, pt2);
        else
          GEOSDistance(pt1, pt2, &result);
        GEOSGeom_destroy(pt1);
        GEOSGeom_destroy(pt2);
        break;
      default:
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "Invalid geometry type for Minimum Rotated Rectangle");
        return -1.0;
    }
  }
  return result;
}

/**
 * @brief Create a GEOS Multipoint geometry from a part (defined by start and
 * end) of a temporal point sequence
 */
static GEOSGeometry *
multipoint_make(const TSequence *seq, int start, int end)
{
  GSERIALIZED *gs = NULL; /* make compiler quiet */
  GEOSGeometry **geoms = palloc(sizeof(GEOSGeometry *) * (end - start + 1));
  for (int i = 0; i < end - start + 1; ++i)
  {
    if (tgeo_type(seq->temptype))
      gs = DatumGetGserializedP(
        tinstant_val(TSEQUENCE_INST_N(seq, start + i)));
#if NPOINT
    else if (seq->temptype == T_TNPOINT)
      gs = npoint_geom(DatumGetNpointP(
        tinstant_val(TSEQUENCE_INST_N(seq, start + i))));
#endif
    else
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Sequence must have a spatial base type");
      return NULL;
    }
    const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
    geoms[i] = GEOSGeom_createPointFromXY(pt->x, pt->y);
  }
  GEOSGeometry *result = GEOSGeom_createCollection(GEOS_MULTIPOINT, geoms, end - start + 1);
  pfree(geoms);
  return result;
}

/**
 * @brief Add the point stored in the given instant to a GEOS multipoint
 * geometry
 */
static GEOSGeometry *
multipoint_add_inst_free(GEOSGeometry *geom, const TInstant *inst)
{
  GSERIALIZED *gs = NULL; /* make compiler quiet */
  if (tgeo_type(inst->temptype))
    gs = DatumGetGserializedP(tinstant_val(inst));
#if NPOINT
  else if (inst->temptype == T_TNPOINT)
    gs = npoint_geom(DatumGetNpointP(tinstant_val(inst)));
#endif
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Instant must have a spatial base type");
    return NULL;
  }
  const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
  GEOSGeometry *geom1 = GEOSGeom_createPointFromXY(pt->x, pt->y);
  GEOSGeometry *result = GEOSUnion(geom, geom1);
  GEOSGeom_destroy(geom1); GEOSGeom_destroy(geom);
  return result;
}

/**
 * @brief Return the subsequences where the temporal value stays within an area
 * with a given maximum size for at least the specified duration
 * (iterator function)
 * @param[in] seq Temporal sequence
 * @param[in] maxdist Maximum distance
 * @param[in] mintunits Minimum duration
 * @param[out] result Resulting sequences
 * @pre The temporal sequence is not instantaneous
 */
int
tpointseq_stops_iter(const TSequence *seq, double maxdist, int64 mintunits,
  TSequence **result)
{
  assert(seq); assert(seq->count > 1);
  assert(tgeo_type(seq->temptype) || seq->temptype == T_TNPOINT);

  /* Use GEOS only for non-scalar input */
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);
  const TInstant *inst1 = NULL, *inst2 = NULL; /* make compiler quiet */
  GEOSGeometry *geom = NULL;
  initGEOS(lwnotice, lwgeom_geos_error);
  geom = GEOSGeom_createEmptyCollection(GEOS_MULTIPOINT);

  int end, start = 0, nseqs = 0;
  bool  is_stopped = false,
        previously_stopped = false,
        rebuild_geom = false;

  for (end = 0; end < seq->count; ++end)
  {
    inst1 = TSEQUENCE_INST_N(seq, start);
    inst2 = TSEQUENCE_INST_N(seq, end);

    while (! is_stopped && end - start > 1
      && (int64)(inst2->t - inst1->t) >= mintunits)
    {
      inst1 = TSEQUENCE_INST_N(seq, ++start);
      rebuild_geom = true;
    }

    if (rebuild_geom)
    {
      GEOSGeom_destroy(geom);
      geom = multipoint_make(seq, start, end);
      rebuild_geom = false;
    }
    else
      geom = multipoint_add_inst_free(geom, inst2);

    if (end - start == 0)
      continue;

    is_stopped = mrr_distance_geos(geom, geodetic) <= maxdist;
    inst2 = TSEQUENCE_INST_N(seq, end - 1);
    if (! is_stopped && previously_stopped
      && (int64)(inst2->t - inst1->t) >= mintunits) // Found a stop
    {
      const TInstant **insts = palloc(sizeof(TInstant *) * (end - start));
      for (int i = 0; i < end - start; ++i)
          insts[i] = TSEQUENCE_INST_N(seq, start + i);
      result[nseqs++] = tsequence_make(insts, end - start,
        true, true, LINEAR, NORMALIZE_NO);
      start = end;
      rebuild_geom = true;
    }
    previously_stopped = is_stopped;
  }
  GEOSGeom_destroy(geom);

  inst2 = TSEQUENCE_INST_N(seq, end - 1);
  if (is_stopped && (int64)(inst2->t - inst1->t) >= mintunits)
  {
    const TInstant **insts = palloc(sizeof(TInstant *) * (end - start));
    for (int i = 0; i < end - start; ++i)
        insts[i] = TSEQUENCE_INST_N(seq, start + i);
    result[nseqs++] = tsequence_make(insts, end - start,
      true, true, LINEAR, NORMALIZE_NO);
  }

  return nseqs;
}

/*****************************************************************************
 * Functions computing the intersection of two segments derived from PostGIS
 * The seg2d_intersection function is a modified version of the PostGIS
 * lw_segment_intersects function and also returns the intersection point
 * in case the two segments intersect at equal endpoints.
 * The intersection point is required in tpointseq_linear_find_splits
 * only for this intersection type (MEOS_SEG_TOUCH_END).
 *****************************************************************************/

/*
 * The possible ways a pair of segments can interact.
 * Returned by the function seg2d_intersection
 */
enum
{
  MEOS_SEG_NO_INTERSECTION,  /* Segments do not intersect */
  MEOS_SEG_OVERLAP,          /* Segments overlap */
  MEOS_SEG_CROSS,            /* Segments cross */
  MEOS_SEG_TOUCH_END,        /* Segments touch in two equal enpoints */
  MEOS_SEG_TOUCH,            /* Segments touch without equal enpoints */
} MEOS_SEG_INTER_TYPE;

/**
 * @brief Find the *unique* intersection point @p p between two closed
 * collinear segments @p ab and @p cd
 * @details Return @p p and a @p MEOS_SEG_INTER_TYPE value.
 * @note If the segments overlap no point is returned since they
 * can be an infinite number of them.
 * @pre This function is called after verifying that the points are
 * collinear and that their bounding boxes intersect.
 */
static int
parseg2d_intersection(const POINT2D *a, const POINT2D *b, const POINT2D *c,
  const POINT2D *d, POINT2D *p)
{
  /* Compute the intersection of the bounding boxes */
  double xmin = Max(Min(a->x, b->x), Min(c->x, d->x));
  double xmax = Min(Max(a->x, b->x), Max(c->x, d->x));
  double ymin = Max(Min(a->y, b->y), Min(c->y, d->y));
  double ymax = Min(Max(a->y, b->y), Max(c->y, d->y));
  /* If the intersection of the bounding boxes is not a point */
  if (xmin < xmax || ymin < ymax )
    return MEOS_SEG_OVERLAP;
  /* We are sure that the segments touch each other */
  if ((b->x == c->x && b->y == c->y) ||
      (b->x == d->x && b->y == d->y))
  {
    p->x = b->x;
    p->y = b->y;
    return MEOS_SEG_TOUCH_END;
  }
  if ((a->x == c->x && a->y == c->y) ||
      (a->x == d->x && a->y == d->y))
  {
    p->x = a->x;
    p->y = a->y;
    return MEOS_SEG_TOUCH_END;
  }
  /* We should never arrive here since this function is called after verifying
   * that the bounding boxes of the segments intersect */
  return MEOS_SEG_NO_INTERSECTION;
}

/**
 * @brief Determines the side of segment P where Q lies
 * @details
 * - Return -1  if point Q is left of segment P
 * - Return  1  if point Q is right of segment P
 * - Return  0  if point Q in on segment P
 * @note Function adapted from @p lw_segment_side() to take into account
 * precision errors
 */
static int
seg2d_side(const POINT2D *p1, const POINT2D *p2, const POINT2D *q)
{
  double side = ( (q->x - p1->x) * (p2->y - p1->y) -
    (p2->x - p1->x) * (q->y - p1->y) );
  if (fabs(side) < MEOS_EPSILON)
    return 0;
  else
    return SIGNUM(side);
}

/**
 * @brief Function derived from file @p lwalgorithm.c since it is declared
 * static
 */
static bool
lw_seg_interact(const POINT2D *p1, const POINT2D *p2, const POINT2D *q1,
  const POINT2D *q2)
{
  double minq = FP_MIN(q1->x, q2->x);
  double maxq = FP_MAX(q1->x, q2->x);
  double minp = FP_MIN(p1->x, p2->x);
  double maxp = FP_MAX(p1->x, p2->x);

  if (FP_GT(minp, maxq) || FP_LT(maxp, minq))
    return false;

  minq = FP_MIN(q1->y, q2->y);
  maxq = FP_MAX(q1->y, q2->y);
  minp = FP_MIN(p1->y, p2->y);
  maxp = FP_MAX(p1->y, p2->y);

  if (FP_GT(minp,maxq) || FP_LT(maxp,minq))
    return false;

  return true;
}

/**
 * @brief Find the *unique* intersection point @p p between two closed segments
 * @p ab and @p cd
 * @details Return @p p and a @p MEOS_SEG_INTER_TYPE value.
 * @note Currently, the function only computes @p p if the result value is
 * @p MEOS_SEG_TOUCH_END, since the return value is never used in other cases.
 * @note If the segments overlap no point is returned since they can be an
 * infinite number of them.
 */
static int
seg2d_intersection(const POINT2D *a, const POINT2D *b, const POINT2D *c,
  const POINT2D *d, POINT2D *p)
{
  /* assume the following names: p = Segment(a, b), q = Segment(c, d) */
  int pq1, pq2, qp1, qp2;

  /* No envelope interaction => we are done. */
  if (! lw_seg_interact(a, b, c, d))
    return MEOS_SEG_NO_INTERSECTION;

  /* Are the start and end points of q on the same side of p? */
  pq1 = seg2d_side(a, b, c);
  pq2 = seg2d_side(a, b, d);
  if ((pq1 > 0 && pq2 > 0) || (pq1 < 0 && pq2 < 0))
    return MEOS_SEG_NO_INTERSECTION;

  /* Are the start and end points of p on the same side of q? */
  qp1 = seg2d_side(c, d, a);
  qp2 = seg2d_side(c, d, b);
  if ((qp1 > 0 && qp2 > 0) || (qp1 < 0 && qp2 < 0))
    return MEOS_SEG_NO_INTERSECTION;

  /* Nobody is on one side or another? Must be colinear. */
  if (pq1 == 0 && pq2 == 0 && qp1 == 0 && qp2 == 0)
    return parseg2d_intersection(a, b, c, d, p);

  /* Check if the intersection is an endpoint */
  if (pq1 == 0 || pq2 == 0 || qp1 == 0 || qp2 == 0)
  {
    /* Check for two equal endpoints */
    if ((b->x == c->x && b->y == c->y) ||
        (b->x == d->x && b->y == d->y))
    {
      p->x = b->x;
      p->y = b->y;
      return MEOS_SEG_TOUCH_END;
    }
    if ((a->x == c->x && a->y == c->y) ||
        (a->x == d->x && a->y == d->y))
    {
      p->x = a->x;
      p->y = a->y;
      return MEOS_SEG_TOUCH_END;
    }

    /* The intersection is inside one of the segments
     * note: p is not compute for this type of intersection */
    return MEOS_SEG_TOUCH;
  }

  /* Crossing
   * note: p is not compute for this type of intersection */
  return MEOS_SEG_CROSS;
}

/*****************************************************************************
 * Non self-intersecting (a.k.a. simple) functions
 *****************************************************************************/

/**
 * @brief Return a temporal point sequence with discrete or step
 * interpolation split into an array of non self-intersecting fragments
 * @param[in] seq Temporal point
 * @param[out] count Number of elements in the resulting array
 * @result Boolean array determining the instant numbers at which the
 * discrete sequence must be split
 * @pre The temporal point has at least 2 instants
 */
static bool *
tpointseq_discstep_find_splits(const TSequence *seq, int *count)
{
  assert(! MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  assert(seq->count > 1);
  /* bitarr is an array of bool for collecting the splits */
  bool *bitarr = palloc0(sizeof(bool) * seq->count);
  int numsplits = 0;
  int start = 0, end = seq->count - 1;
  while (start < end)
  {
    /* Find intersections in the piece defined by start and end in a
     * breadth-first search */
    int j = start, k = start + 1;
    Datum value1 = tinstant_val(TSEQUENCE_INST_N(seq, j));
    Datum value2 = tinstant_val(TSEQUENCE_INST_N(seq, k));
    while (true)
    {
      if (datum_point_eq(value1, value2))
      {
        /* Set the new start */
        bitarr[k] = true;
        numsplits++;
        start = k;
        break;
      }
      if (j < k - 1)
      {
        j++;
        value1 = tinstant_val(TSEQUENCE_INST_N(seq, j));
      }
      else
      {
        k++;
        if (k > end)
          break;
        j = start;
        value1 = tinstant_val(TSEQUENCE_INST_N(seq, j));
        value2 = tinstant_val(TSEQUENCE_INST_N(seq, k));
      }
    }
    if (k > end)
      break;
  }
  *count = numsplits;
  return bitarr;
}

/**
 * @brief Return a temporal point sequence with linear interpolation split into
 * an array of non self-intersecting fragments
 * @note The function works only on 2D even if the input points are in 3D
 * @param[in] seq Temporal point
 * @param[out] count Number of elements in the resulting array
 * @result Boolean array determining the instant numbers at which the
 * sequence must be split
 * @pre The input sequence has at least 3 instants
 */
static bool *
tpointseq_linear_find_splits(const TSequence *seq, int *count)
{
  assert(seq->count >= 2);
 /* points is an array of points in the sequence */
  const POINT2D **points = palloc0(sizeof(POINT2D *) * seq->count);
  /* bitarr is an array of bool for collecting the splits */
  bool *bitarr = palloc0(sizeof(bool) * seq->count);
  points[0] = DATUM_POINT2D_P(tinstant_val(TSEQUENCE_INST_N(seq, 0)));
  int numsplits = 0;
  for (int i = 1; i < seq->count; i++)
  {
    points[i] = DATUM_POINT2D_P(tinstant_val(TSEQUENCE_INST_N(seq, i)));
    /* If stationary segment we need to split the sequence */
    if (points[i - 1]->x == points[i]->x && points[i - 1]->y == points[i]->y)
    {
      if (i > 1 && ! bitarr[i - 1])
      {
        bitarr[i - 1] = true;
        numsplits++;
      }
      if (i < seq->count - 1)
      {
        bitarr[i] = true;
        numsplits++;
      }
    }
  }

  /* Loop for every split due to stationary segments while adding
   * additional splits due to intersecting segments */
  int start = 0;
  while (start < seq->count - 2)
  {
    int end = start + 1;
    while (end < seq->count - 1 && ! bitarr[end])
      end++;
    if (end == start + 1)
    {
      start = end;
      continue;
    }
    /* Find intersections in the piece defined by start and end in a
     * breadth-first search */
    int i = start, j = start + 1;
    while (j < end)
    {
      /* If the bounding boxes of the segments intersect */
      if (lw_seg_interact(points[i], points[i + 1], points[j],
        points[j + 1]))
      {
        /* Candidate for intersection */
        POINT2D p = { 0 }; /* make compiler quiet */
        int intertype = seg2d_intersection(points[i], points[i + 1],
          points[j], points[j + 1], &p);
        if (intertype > 0 &&
          /* Exclude the case when two consecutive segments that
           * necessarily touch each other in their common point */
          (intertype != MEOS_SEG_TOUCH_END || j != i + 1 ||
           p.x != points[j]->x || p.y != points[j]->y))
        {
          /* Set the new end */
          end = j;
          bitarr[end] = true;
          numsplits++;
          break;
        }
      }
      if (i < j - 1)
        i++;
      else
      {
        j++;
        i = start;
      }
    }
    /* Process the next split */
    start = end;
  }
  pfree(points);
  *count = numsplits;
  return bitarr;
}

/*****************************************************************************
 * Functions for testing whether a temporal point is simple and for spliting
 * a temporal point into an array of temporal points that are simple.
 * A temporal point is simple if all its components are non self-intersecting.
 * - a temporal instant point is simple
 * - a temporal discrete sequence point is simple if it is non self-intersecting
 * - a temporal sequence point is simple if it is non self-intersecting and
 *   do not have stationary segments
 * - a temporal sequence set point is simple if every composing sequence is
 *   simple even if two composing sequences intersect
 *****************************************************************************/

/**
 * @brief Return true if a temporal point does not self-intersect
 * @param[in] seq Temporal point
 * @pre The temporal point sequence has discrete or step interpolation
 */
static bool
tpointseq_discstep_is_simple(const TSequence *seq)
{
  assert(seq->count > 1);
  Datum *points = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
    points[i] = tinstant_val(TSEQUENCE_INST_N(seq, i));
  datumarr_sort(points, seq->count, temptype_basetype(seq->temptype));
  bool found = false;
  for (int i = 1; i < seq->count; i++)
  {
    if (datum_point_eq(points[i - 1], points[i]))
    {
      found = true;
      break;
    }
  }
  pfree(points);
  return ! found;
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect
 * @param[in] seq Temporal point
 * @csqlfn #Tpoint_is_simple()
 */
bool
tpointseq_is_simple(const TSequence *seq)
{
  assert(seq); assert(tgeo_type(seq->temptype));
  /* Instantaneous sequence */
  if (seq->count == 1)
    return true;

  if (! MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    return tpointseq_discstep_is_simple(seq);

  int numsplits;
  bool *splits = tpointseq_linear_find_splits(seq, &numsplits);
  pfree(splits);
  return (numsplits == 0);
}

/**
 * @ingroup meos_internal_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tpoint_is_simple()
 */
bool
tpointseqset_is_simple(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type(ss->temptype));
  bool result = true;
  for (int i = 0; i < ss->count; i++)
  {
    result &= tpointseq_is_simple(TSEQUENCESET_SEQ_N(ss, i));
    if (! result)
      break;
  }
  return result;
}

/**
 * @ingroup meos_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect
 * @param[in] temp Temporal point
 * @csqlfn #Tpoint_is_simple()
 */
bool
tpoint_is_simple(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tgeo_type(temp->temptype))
    return false;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return true;
    case TSEQUENCE:
      return tpointseq_is_simple((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tpointseqset_is_simple((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal discrete sequence point split into an array of non
 * self-intersecting fragments
 * @param[in] seq Temporal point
 * @param[in] splits Bool array stating the splits
 * @param[in] count Number of elements in the resulting array
 * @pre The sequence has at least two instants
 */
static TSequence **
tpointdiscseq_split(const TSequence *seq, bool *splits, int count)
{
  assert(seq->count > 1);
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TSequence **result = palloc(sizeof(TSequence *) * count);
  /* Create the splits */
  int start = 0, nseqs = 0;
  while (start < seq->count)
  {
    int end = start + 1;
    while (end < seq->count && ! splits[end])
      end++;
    /* Construct piece from start to end */
    for (int j = 0; j < end - start; j++)
      instants[j] = TSEQUENCE_INST_N(seq, j + start);
    result[nseqs++] = tsequence_make(instants, end - start, true, true,
      DISCRETE, NORMALIZE_NO);
    /* Continue with the next split */
    start = end;
  }
  pfree(instants);
  return result;
}

/**
 * @brief Return a temporal point split into an array of non self-intersecting
 * fragments
 * @param[in] seq Temporal sequence point
 * @param[in] splits Bool array stating the splits
 * @param[in] count Number of elements in the resulting array
 * @note This function is called for each sequence of a sequence set
 */
static TSequence **
tpointcontseq_split(const TSequence *seq, bool *splits, int count)
{
  assert(seq->count > 2);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TSequence **result = palloc(sizeof(TSequence *) * count);
  /* Create the splits */
  int start = 0, nseqs = 0;
  while (start < seq->count - 1)
  {
    int end = start + 1;
    while (end < seq->count - 1 && ! splits[end])
      end++;
    /* Construct fragment from start to end inclusive */
    for (int j = 0; j <= end - start; j++)
      instants[j] = (TInstant *) TSEQUENCE_INST_N(seq, j + start);
    bool lower_inc1 = (start == 0) ? seq->period.lower_inc : true;
    bool upper_inc1 = (end == seq->count - 1) ?
      seq->period.upper_inc && ! splits[seq->count - 1] : false;
    /* The last two values of sequences with step interpolation and
     * exclusive upper bound must be equal */
    bool tofree = false;
    if (! linear && ! upper_inc1 &&
      ! datum_point_eq(tinstant_val(instants[end - start - 1]),
      tinstant_val(instants[end - start])))
    {
      Datum value = tinstant_val(instants[end - start - 1]);
      TimestampTz t = (instants[end - start])->t;
      instants[end - start] = tinstant_make(value, seq->temptype, t);
      tofree = true;
      upper_inc1 = false;
    }
    result[nseqs++] = tsequence_make((const TInstant **) instants, end - start + 1,
      lower_inc1, upper_inc1, linear ? LINEAR : STEP, NORMALIZE_NO);
    if (tofree)
      /* Free the last instant created for the step interpolation */
      pfree(instants[end - start]);
    /* Continue with the next split */
    start = end;
  }
  if (nseqs < count)
  {
    /* Construct last fragment containing the last instant of sequence */
    instants[0] = (TInstant *) TSEQUENCE_INST_N(seq, seq->count - 1);
    result[nseqs++] = tsequence_make((const TInstant **) instants,
      seq->count - start, true, seq->period.upper_inc,
      linear, NORMALIZE_NO);
  }
  pfree(instants);
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal sequence point split into an array of non
 * self-intersecting fragments
 * @param[in] seq Temporal sequence point
 * @param[out] count Number of elements in the resulting array
 * @note This function is called for each sequence of a sequence set
 * @csqlfn #Tpoint_make_simple()
 */
TSequence **
tpointseq_make_simple(const TSequence *seq, int *count)
{
  assert(seq); assert(count); assert(tgeo_type(seq->temptype));
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TSequence **result;
  /* Special cases when the input sequence has 1 or 2 instants */
  if ((interp == DISCRETE && seq->count == 1) ||
      (interp != DISCRETE && seq->count <= 2))
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    return result;
  }

  int numsplits;
  bool *splits = (interp == LINEAR) ?
    tpointseq_linear_find_splits(seq, &numsplits) :
    tpointseq_discstep_find_splits(seq, &numsplits);
  if (numsplits == 0)
  {
    result = palloc(sizeof(TSequence *));
    result[0] = tsequence_copy(seq);
    *count = 1;
    pfree(splits);
    return result;
  }

  result = (interp == DISCRETE) ?
    tpointdiscseq_split(seq, splits, numsplits + 1) :
    tpointcontseq_split(seq, splits, numsplits + 1);
  *count = numsplits + 1;
  pfree(splits);
  return result;
}

/**
 * @ingroup meos_internal_temporal_spatial_transf
 * @brief Return a temporal sequence set point split into an array of non
 * self-intersecting fragments
 * @param[in] ss Temporal sequence set point
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tpoint_make_simple()
 */
TSequence **
tpointseqset_make_simple(const TSequenceSet *ss, int *count)
{
  assert(ss); assert(count); assert(tgeo_type(ss->temptype));
  /* Singleton sequence set */
  if (ss->count == 1)
    return tpointseq_make_simple(TSEQUENCESET_SEQ_N(ss, 0), count);

  /* General case */
  TSequence ***sequences = palloc0(sizeof(TSequence **) * ss->count);
  int *nseqs = palloc0(sizeof(int) * ss->count);
  int totalseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    sequences[i] = tpointseq_make_simple(TSEQUENCESET_SEQ_N(ss, i), &nseqs[i]);
    totalseqs += nseqs[i];
  }
  assert(totalseqs > 0);
  *count = totalseqs;
  return tseqarr2_to_tseqarr(sequences, nseqs, ss->count, totalseqs);
}

/**
 * @ingroup meos_temporal_spatial_transf
 * @brief Return a temporal point split into an array of non self-intersecting
 * fragments
 * @param[in] temp Temporal point
 * @param[out] count Number of elements in the output array
 * @see #tpointseq_make_simple
 * @see #tpointseqset_make_simple
 * @csqlfn #Tpoint_make_simple()
 */
Temporal **
tpoint_make_simple(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      Temporal **result = palloc0(sizeof(TInstant *));
      result[0] = (Temporal *) tinstant_copy((TInstant *) temp);
      *count = 1;
      return result;
    }
    case TSEQUENCE:
      return (Temporal **) tpointseq_make_simple((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return (Temporal **) tpointseqset_make_simple((TSequenceSet *) temp, count);
  }
}

/*****************************************************************************/

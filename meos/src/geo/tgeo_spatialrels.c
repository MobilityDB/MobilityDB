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
 * @brief Ever/always spatial relationships for temporal geos
 * @details These relationships compute the ever/always spatial relationship
 * between the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding temporal spatial
 * relationship.
 *
 * The following relationships are supported for geometries: `contains`,
 * `disjoint`, `intersects`, `touches`, and `dwithin`.
 *
 * The following relationships are supported for geographies: `disjoint`,
 * `intersects`, `dwithin`.
 *
 * Only `disjoint`, `dwithin`, and `intersects` are supported for 3D geometries.
 */

#include "geo/tgeo_spatialrels.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_tempspatialrels.h"

/*****************************************************************************
 * Spatial relationship functions
 * disjoint and intersects are inverse to each other
 *****************************************************************************/

/**
 * @brief Return a Datum true if the first geometry covers the second one
 */
Datum
datum_geom_covers(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geom_spatialrel(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), COVERS));
}

/**
 * @brief Return a Datum true if two geometries are disjoint in 2D
 */
Datum
datum_geom_disjoint2d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(! geom_spatialrel(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), INTERSECTS));
}

/**
 * @brief Return a Datum true if two geometries are disjoint in 3D
 */
Datum
datum_geom_disjoint3d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(! geom_intersects3d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return a Datum true if two geographies are disjoint
 */
Datum
datum_geog_disjoint(Datum geog1, Datum geog2)
{
  return BoolGetDatum(! geog_dwithin(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), 0.00001, true));
}

/**
 * @brief Return a Datum true if two geometries intersect in 2D
 */
Datum
datum_geom_intersects2d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geom_spatialrel(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), INTERSECTS));
}

/**
 * @brief Return a Datum true if two geometries intersect in 3D
 */
Datum
datum_geom_intersects3d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geom_intersects3d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return a Datum true if two geographies intersect
 */
Datum
datum_geog_intersects(Datum geog1, Datum geog2)
{
  return BoolGetDatum(geog_dwithin(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), 0.00001, true));
}

/**
 * @brief Return a Datum true if two 2D geometries are within a distance
 */
Datum
datum_geom_dwithin2d(Datum geom1, Datum geom2, Datum dist)
{
  return BoolGetDatum(geom_dwithin2d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), DatumGetFloat8(dist)));
}

/**
 * @brief Return a Datum true if two 3D geometries are within a distance
 */
Datum
datum_geom_dwithin3d(Datum geom1, Datum geom2, Datum dist)
{
  return BoolGetDatum(geom_dwithin3d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), DatumGetFloat8(dist)));
}

/**
 * @brief Return a Datum true if two geographies are within a distance
 */
Datum
datum_geog_dwithin(Datum geog1, Datum geog2, Datum dist)
{
  return BoolGetDatum(geog_dwithin(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), DatumGetFloat8(dist), true));
}

/*****************************************************************************/

/**
 * @brief Select the appropriate intersect function
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func2
get_intersects_fn_geo(int16 flags1, uint8_t flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_intersects;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &datum_geom_intersects3d : &datum_geom_intersects2d;
}

/**
 * @brief Select the appropriate dwithin function
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func3
get_dwithin_fn(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && MEOS_FLAGS_GET_Z(flags2) ?
      &datum_geom_dwithin3d : &datum_geom_dwithin2d;
}

/**
 * @brief Select the appropriate dwithin function
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func3
get_dwithin_fn_geo(int16 flags1, uint8_t flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &datum_geom_dwithin3d : &datum_geom_dwithin2d;
}

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Generic spatial relationship for the trajectory of a temporal geo
 * and a geometry
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 */
static int
spatialrel_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum param, varfunc func, int numparam, bool invert)
{
  /* Ensure the validity of the arguments */
  assert(temp); assert(gs); assert(tgeo_type_all(temp->temptype));
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return -1;

  assert(numparam == 2 || numparam == 3);
  Datum geo = PointerGetDatum(gs);
  Datum traj = tpoint_type(temp->temptype) ? 
    PointerGetDatum(tpoint_trajectory(temp)) :
    PointerGetDatum(tgeo_traversed_area(temp));
  Datum result;
  if (numparam == 2)
  {
    datum_func2 func2 = (datum_func2) func;
    result = invert ? func2(geo, traj) : func2(traj, geo);
  }
  else /* numparam == 3 */
  {
    datum_func3 func3 = (datum_func3) func;
    result = invert ? func3(geo, traj, param) : func3(traj, geo, param);
  }
  pfree(DatumGetPointer(traj));
  return result ? 1 : 0;
}

/*****************************************************************************/

/**
 * @brief Return true if two temporal geos ever/always satisfy a spatial
 * relationship
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] func Spatial relationship
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_spatialrel_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial(temp1, temp2) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return -1;

  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.ever = ever;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return eafunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry ever contains a temporal geo, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @note The function does not accept 3D or geography since it is based on the
 * PostGIS ST_Relate function. The function tests whether the trajectory
 * intersects the interior of the geometry. Please refer to the documentation
 * of the ST_Contains and ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_geo_tgeo()
 */
int
econtains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_has_not_Z_geo(gs) || 
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1;
  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp) : tgeo_traversed_area(temp);
  bool result = geom_relate_pattern(gs, traj, "T********");
  pfree(traj);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry always contains a temporal geo,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @note The function tests whether the trajectory is contained in the geometry.
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_geo_tgeo()
 */
int
acontains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_has_not_Z_geo(gs) || 
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1;
  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp) : tgeo_traversed_area(temp);
  bool result = geom_contains(gs, traj);
  pfree(traj);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always disjoint (only always works for both geometry and geography)
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geo and a geometry are ever disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @note eDisjoint(tpoint, geo) is equivalent to NOT covers(geo, traj(tpoint))
 * @note The function does not accept geography since it is based on the
 * PostGIS ST_Covers function provided by GEOS
 * @csqlfn #Edisjoint_tgeo_geo()
 */
int
edisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || ! ensure_not_geodetic(temp->flags))
    return -1;
  datum_func2 func = &datum_geom_covers;
  int result = spatialrel_tgeo_geo(temp, gs, (Datum) NULL, (varfunc) func, 2,
    INVERT);
  return INVERT_RESULT(result);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geo and a geometry are always disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @note aDisjoint(a, b) is equivalent to NOT eIntersects(a, b)
 * @csqlfn #Adisjoint_tgeo_geo()
 */
inline int
adisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return INVERT_RESULT(eintersects_tgeo_geo(temp, gs));
}

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if the temporal geos ever/always intersect, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tgeo_tgeo(), #Aintersects_tgeo_tgeo()
 */
int
ea_disjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  if (! ensure_valid_tgeo_tgeo(temp1, temp2))
    return -1;
  datum_func2 func;
  if (MEOS_FLAGS_GET_GEODETIC(temp1->flags))
    func = &datum_geog_disjoint;
  else 
    func = MEOS_FLAGS_GET_Z(temp1->flags) && MEOS_FLAGS_GET_Z(temp2->flags) ?
      &datum_geom_disjoint3d : &datum_geom_disjoint2d;
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, func, ever);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if the temporal geos are ever disjoint, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Edisjoint_tgeo_tgeo()
 */
inline int
edisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_disjoint_tgeo_tgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if the temporal geos are always disjoint, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Adisjoint_tgeo_tgeo()
 */
inline int
adisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_disjoint_tgeo_tgeo(temp1, temp2, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always intersects (for both geometry and geography)
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry and a temporal geo ever intersect,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Eintersects_tgeo_geo()
 */
int
eintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs))
    return -1;
  datum_func2 func = get_intersects_fn_geo(temp->flags, gs->gflags);
  return spatialrel_tgeo_geo(temp, gs, (Datum) NULL, (varfunc) func, 2,
    INVERT_NO);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry and a temporal geo always intersect,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @note aIntersects(tpoint, geo) is equivalent to NOT eDisjoint(tpoint, geo)
 * @note The function does not accept geography since the eDisjoint function
 * is based on the PostGIS ST_Covers function provided by GEOS
 * @csqlfn #Aintersects_tgeo_geo()
 */
inline int
aintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return INVERT_RESULT(edisjoint_tgeo_geo(temp, gs));
}

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if the temporal geos ever/always intersect, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tgeo_tgeo(), #Aintersects_tgeo_tgeo()
 */
int
ea_intersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  if (! ensure_valid_tgeo_tgeo(temp1, temp2))
    return -1;
  datum_func2 func = get_intersects_fn_geo(temp1->flags, temp1->flags);
  return ea_spatialrel_tspatial_tspatial(temp1, temp2, func, ever);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if the temporal geos ever intersect, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Eintersects_tgeo_tgeo()
 */
inline int
eintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_intersects_tgeo_tgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if the temporal geos always intersect, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Aintersects_tgeo_tgeo()
 */
inline int
aintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_intersects_tgeo_tgeo(temp1, temp2, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geo and a geometry ever touch, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Etouches_tgeo_geo()
 */
int
etouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) ||
      ! ensure_not_geodetic(temp->flags) || gserialized_is_empty(gs) ||
      ! ensure_valid_tspatial_geo(temp, gs))
    return -1;

  /* There is no need to do a bounding box test since this is done in
   * the SQL function definition */
  datum_func2 func = get_intersects_fn_geo(temp->flags, gs->gflags);
  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp) : tgeo_traversed_area(temp);
  GSERIALIZED *gsbound = geom_boundary(gs);
  bool result = false;
  if (gsbound && ! gserialized_is_empty(gsbound))
    result = func(GserializedPGetDatum(gsbound), GserializedPGetDatum(traj));
  /* TODO */
  // else if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  // {
    // /* The geometry is a point or a multipoint -> the boundary is empty */
    // GSERIALIZED *tempbound = geom_boundary(traj);
    // if (tempbound)
    // {
      // result = func(GserializedPGetDatum(tempbound), GserializedPGetDatum(gs));
      // pfree(tempbound);
    // }
  // }
  pfree(traj); pfree(gsbound);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geo and a geometry always touch, 0 if not,
 * and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tgeo_geo()
 */
int
atouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  /* There is no need to do a bounding box test since this is done in
   * the SQL function definition */
  GSERIALIZED *gsbound = geom_boundary(gs);
  bool result = false;
  if (gsbound && ! gserialized_is_empty(gsbound))
  {
    Temporal *temp1 = tgeo_restrict_geom(temp, gsbound, NULL, REST_MINUS);
    result = (temp1 == NULL);
    if (temp1)
      pfree(temp1);
  }
  pfree(gsbound);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always dwithin (for both geometry and geography)
 * The function only accepts points and not arbitrary geometries/geographies
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry and a temporal geo are ever within the
 * given distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tgeo_geo()
 */
int
edwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || 
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;
  datum_func3 func = get_dwithin_fn_geo(temp->flags, gs->gflags);
  return spatialrel_tgeo_geo(temp, gs, Float8GetDatum(dist),
    (varfunc) func, 3, INVERT_NO);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry and a temporal geo are always within a
 * distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @note The function is not available for 3D or geograhies since it is based
 * on thePostGIS ST_Buffer() function which is performed by GEOS
 * @csqlfn #Adwithin_tgeo_geo()
 */
int
adwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || 
      ! ensure_not_geodetic(temp->flags) || 
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  GSERIALIZED *buffer = geom_buffer(gs, dist, "");
  datum_func2 func = &datum_geom_covers;
  int result = spatialrel_tgeo_geo(temp, buffer, (Datum) NULL,
    (varfunc) func, 2, INVERT);
  pfree(buffer);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal geos are ever within a distance
 * @param[in] inst1,inst2 Temporal geos
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal geos are synchronized
 */
static bool
ea_dwithin_tgeoinst_tgeoinst(const TInstant *inst1, const TInstant *inst2,
  double dist, datum_func3 func)
{
  assert(inst1); assert(inst2);
  /* Result is the same for both EVER and ALWAYS */
  return DatumGetBool(func(tinstant_value_p(inst1), tinstant_value_p(inst2), 
    Float8GetDatum(dist)));
}

/**
 * @brief Return true if two temporal geos are ever within a distance
 * @param[in] seq1,seq2 Temporal geos
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal geos are synchronized
 */
static bool
ea_dwithin_tgeoseq_tgeoseq_discstep(const TSequence *seq1,
  const TSequence *seq2, double dist, datum_func3 func, bool ever)
{
  assert(seq1); assert(seq2);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < seq1->count; i++)
  {
    bool res = ea_dwithin_tgeoinst_tgeoinst(TSEQUENCE_INST_N(seq1, i),
      TSEQUENCE_INST_N(seq2, i), dist, func);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/**
 * @brief Return true if two temporal geos are ever within a distance
 * @param[in] seq1,seq2 Temporal geos
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal geos are synchronized
 */
static bool
ea_dwithin_tpointseq_tpointseq_cont(const TSequence *seq1,
  const TSequence *seq2, double dist, datum_func3 func, bool ever)
{
  assert(seq1); assert(seq2);

  if (seq1->count == 1)
  {
    return ea_dwithin_tgeoinst_tgeoinst(TSEQUENCE_INST_N(seq1, 0), 
      TSEQUENCE_INST_N(seq2, 0), dist, func);
  }

  const TInstant *start1 = TSEQUENCE_INST_N(seq1, 0);
  const TInstant *start2 = TSEQUENCE_INST_N(seq2, 0);
  Datum sv1 = tinstant_value_p(start1);
  Datum sv2 = tinstant_value_p(start2);

  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  bool hasz = MEOS_FLAGS_GET_Z(seq1->flags);
  TimestampTz lower = start1->t;
  bool ret_loop = ever ? true : false;
  for (int i = 1; i < seq1->count; i++)
  {
    const TInstant *end1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *end2 = TSEQUENCE_INST_N(seq2, i);
    Datum ev1 = tinstant_value_p(end1);
    Datum ev2 = tinstant_value_p(end2);
    TimestampTz upper = end1->t;

    /* Both segments are constant */
    if (datum_point_eq(sv1, ev1) && datum_point_eq(sv2, ev2))
    {
      bool res = DatumGetBool(func(sv1, sv2, Float8GetDatum(dist)));
      if ((ever && res) || (! ever && ! res))
        return ret_loop;
    }

    /* General case */
    TimestampTz t1, t2;
    Datum sev1 = linear1 ? ev1 : sv1;
    Datum sev2 = linear2 ? ev2 : sv2;
    /* Find the instants t1 and t2 (if any) during which the dwithin function
     * is true */
    int solutions = tdwithin_tpointsegm_tpointsegm(sv1, sev1, sv2, sev2,
      lower, upper, dist, hasz, func, &t1, &t2);
    /* Determine whether the segment is always/ever within the distance */
    bool res;
    if (ever)
      res = solutions > 0;
    else
      res = (solutions == 2 && t1 == lower && t2 == upper) ||
        (solutions == 1 && t1 == lower && t1 == upper);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;

    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
  }
  return ! ret_loop;
}

/**
 * @brief Return true if two temporal geos are ever within a distance
 * @param[in] ss1,ss2 Temporal geos
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal geos are synchronized
 */
static bool
ea_dwithin_tgeoseqset_tgeoseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, double dist, datum_func3 func, bool ever)
{
  assert(ss1); assert(ss2);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(ss2->flags);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < ss1->count; i++)
  {
    bool res = linear ?
      ea_dwithin_tpointseq_tpointseq_cont(TSEQUENCESET_SEQ_N(ss1, i), 
        TSEQUENCESET_SEQ_N(ss2, i), dist, func, ever) :
      ea_dwithin_tgeoseq_tgeoseq_discstep(TSEQUENCESET_SEQ_N(ss1, i), 
        TSEQUENCESET_SEQ_N(ss2, i), dist, func, ever);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/*****************************************************************************/

/**
 * @brief Return 1 if two temporal geos are ever within a distance,
 * 0 if not, -1 if the temporal geos do not intersect on time
 * @pre The temporal geos are synchronized
 */
int
ea_dwithin_tgeo_tgeo_sync(const Temporal *sync1, const Temporal *sync2,
  double dist, bool ever)
{
  datum_func3 func = get_dwithin_fn(sync1->flags, sync2->flags);
  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
      return ea_dwithin_tgeoinst_tgeoinst((TInstant *) sync1,
        (TInstant *) sync2, dist, func);
    case TSEQUENCE:
      return MEOS_FLAGS_LINEAR_INTERP(sync1->flags) ||
          MEOS_FLAGS_LINEAR_INTERP(sync2->flags) ?
        ea_dwithin_tpointseq_tpointseq_cont((TSequence *) sync1,
          (TSequence *) sync2, dist, func, ever) :
        ea_dwithin_tgeoseq_tgeoseq_discstep((TSequence *) sync1,
          (TSequence *) sync2, dist, func, ever);
    default: /* TSEQUENCESET */
      return ea_dwithin_tgeoseqset_tgeoseqset((TSequenceSet *) sync1,
        (TSequenceSet *) sync2, dist, func, ever);
  }
}

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if two temporal geos are ever within a distance,
 * 0 if not, -1 on error or if the temporal geos do not intersect on time
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edwithin_tgeo_tgeo()
 */
int
ea_dwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_tgeo(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  Temporal *sync1, *sync2;
  /* Return NULL if the temporal geos do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return -1;

  bool result = ea_dwithin_tgeo_tgeo_sync(sync1, sync2, dist, ever);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if two temporal geos are ever within a distance,
 * 0 if not, -1 on error or if the temporal geos do not intersect on time
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tgeo_tgeo()
 */
inline int
edwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist)
{
  return ea_dwithin_tgeo_tgeo(temp1, temp2, dist, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if two temporal geos are always within a distance,
 * 0 if not, -1 on error or if the temporal geos do not intersect on time
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tgeo_tgeo()
 */
inline int
adwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist)
{
  return ea_dwithin_tgeo_tgeo(temp1, temp2, dist, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

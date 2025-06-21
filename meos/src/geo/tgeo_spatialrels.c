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
 * filtering purposes before applying the corresponding spatiotemporal
 * relationship.
 *
 * The following relationships are supported for geometries: `contains`,
 * `covers`, `disjoint`, `intersects`, `touches`, and `dwithin`.
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
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/tsequence.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
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
datum_geom_contains(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geom_spatialrel(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), CONTAINS));
}

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

/**
 * @brief Return a Datum true if two 2D geometries are within a distance
 */
Datum
datum_geom_relate_pattern(Datum geom1, Datum geom2, Datum p)
{
  return BoolGetDatum(geom_relate_pattern(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), (char *) DatumGetPointer(p)));
}

/**
 * @brief Return a Datum true if the first geometry covers the second one
 */
Datum
datum_geom_touches(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geom_touches(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/*****************************************************************************/

/**
 * @brief Select the appropriate disjoint function depending on the flags
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func2
geo_disjoint_fn(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_disjoint;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && MEOS_FLAGS_GET_Z(flags2) ?
      &datum_geom_disjoint3d : &datum_geom_disjoint2d;
}

/**
 * @brief Select the appropriate disjoint function depending on the flags
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func2
geo_disjoint_fn_geo(int16 flags1, uint8_t flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_disjoint;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &datum_geom_disjoint3d : &datum_geom_disjoint2d;
}

/**
 * @brief Select the appropriate intersects function depending on the flags
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func2
geo_intersects_fn(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_intersects;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && MEOS_FLAGS_GET_Z(flags2) ?
      &datum_geom_intersects3d : &datum_geom_intersects2d;
}

/**
 * @brief Select the appropriate intersects function depending on the flags
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func2
geo_intersects_fn_geo(int16 flags1, uint8_t flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_intersects;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &datum_geom_intersects3d : &datum_geom_intersects2d;
}

/**
 * @brief Select the appropriate dwithin function depending on the flags
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func3
geo_dwithin_fn(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &datum_geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && MEOS_FLAGS_GET_Z(flags2) ?
      &datum_geom_dwithin3d : &datum_geom_dwithin2d;
}

/**
 * @brief Select the appropriate dwithin function depending on the flags
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func3
geo_dwithin_fn_geo(int16 flags1, uint8_t flags2)
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
 * @brief Generic spatial relationship for the trajectory or traversed area
 * of a temporal geo and a geometry
 * @details The function computes the function passed as parameter with the
 * trajectory or the traversed area of the temporal geometry and the geometry
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the function
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 * @note Since some GEOS versions do not support geometry collections, the
 * function iterates for each geometry of the collection and returns when the
 * function is true for one of them.
 */
static int
spatialrel_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, Datum param,
  varfunc func, int numparam, bool invert)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  assert(numparam == 2 || numparam == 3);
  Datum geo = PointerGetDatum(gs);
  GSERIALIZED *trav = tpoint_type(temp->temptype) ?
    tpoint_trajectory(temp) : tgeo_traversed_area(temp);
  Datum dtrav, result;

  /* Call the GEOS function if the traversed area is not a collection */
  if (gserialized_get_type(trav) != COLLECTIONTYPE)
  {
    dtrav = PointerGetDatum(trav);
    if (numparam == 2)
    {
      datum_func2 func2 = (datum_func2) func;
      result = invert ? func2(geo, dtrav) : func2(dtrav, geo);
    }
    else /* numparam == 3 */
    {
      datum_func3 func3 = (datum_func3) func;
      result = invert ? func3(geo, dtrav, param) : func3(dtrav, geo, param);
    }
    pfree(DatumGetPointer(dtrav));
    return result ? 1 : 0;
  }

  /* Call the GEOS function for each element in the collection */
  LWCOLLECTION *coll = lwgeom_as_lwcollection(lwgeom_from_gserialized(trav));
  for (uint32_t i = 0; i < coll->ngeoms; i++)
  {
    const LWGEOM *elem = lwcollection_getsubgeom((LWCOLLECTION *) coll, i);
    dtrav = PointerGetDatum(geo_serialize(elem));
    if (numparam == 2)
    {
      datum_func2 func2 = (datum_func2) func;
      result = invert ? func2(geo, dtrav) : func2(dtrav, geo);
    }
    else /* numparam == 3 */
    {
      datum_func3 func3 = (datum_func3) func;
      result = invert ? func3(geo, dtrav, param) : func3(dtrav, geo, param);
    }
    /* We cannot lwgeom_free((LWGEOM *) coll); */
    if (result)
      return result ? 1 : 0;
  }
  pfree(trav);
  return result ? 1 : 0;
}

/*****************************************************************************/

/**
 * @brief Generic spatial relationship for the trajectory or traversed area
 * of two temporal geometries
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the function
 * @return On error return -1
 * @note Since some GEOS versions do not support geometry collections, the
 * function iterates for each geometry of the collection and returns when the
 * function is true for one of them.
 */
int
spatialrel_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  Datum param, varfunc func, int numparam)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tspatial_tspatial(temp1, temp2))
    return -1;

  assert(numparam == 2 || numparam == 3);
  GSERIALIZED *trav1 = tpoint_type(temp1->temptype) ?
    tpoint_trajectory(temp1) : tgeo_traversed_area(temp1);
  GSERIALIZED *trav2 = tpoint_type(temp2->temptype) ?
    tpoint_trajectory(temp2) : tgeo_traversed_area(temp2);
  Datum dtrav1 = PointerGetDatum(trav1);
  Datum dtrav2 = PointerGetDatum(trav2);
  Datum result;

  /* Call the GEOS function if the traversed area is not a collection */
  if (gserialized_get_type(trav1) != COLLECTIONTYPE)
  {
    dtrav1 = PointerGetDatum(trav1);
    if (numparam == 2)
    {
      datum_func2 func2 = (datum_func2) func;
      result = func2(dtrav1, dtrav2);
    }
    else /* numparam == 3 */
    {
      datum_func3 func3 = (datum_func3) func;
      result = func3(dtrav1, dtrav2, param);
    }
    pfree(DatumGetPointer(dtrav1));
    return result ? 1 : 0;
  }

  /* Call the GEOS function for each trapezoid in the collection */
  LWCOLLECTION *coll = lwgeom_as_lwcollection(lwgeom_from_gserialized(trav1));
  for (uint32_t i = 0; i < coll->ngeoms; i++)
  {
    const LWGEOM *elem = lwcollection_getsubgeom((LWCOLLECTION *) coll, i);
    dtrav1 = PointerGetDatum(geo_serialize(elem));
    if (numparam == 2)
    {
      datum_func2 func2 = (datum_func2) func;
      result = func2(dtrav1, dtrav2);
    }
    else /* numparam == 3 */
    {
      datum_func3 func3 = (datum_func3) func;
      result = func3(dtrav1, dtrav2, param);
    }
    /* We cannot lwgeom_free((LWGEOM *) coll); */
    if (result)
      return result ? 1 : 0;
  }
  pfree(trav1); pfree(trav2);
  return result ? 1 : 0;
}

/*****************************************************************************/

/**
 * @brief Return true if two temporal geos ever/always satisfy a spatial
 * relationship
 * @details The function applies function `func` to every composing geometry
 * of the temporal geometry
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] func Spatial relationship
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True if the arguments should be inverted
 * @note Mixed 2D/3D allowed
 * @note The function assumes that all validity tests have been previously done
 */
int
ea_spatialrel_tspatial_geo(const Temporal *temp, const GSERIALIZED *gs,
  datum_func2 func, bool ever, bool invert)
{
  /* Ensure the validity of the arguments */
  assert(temp); assert(gs); assert(! gserialized_is_empty(gs)); assert(func);
  int count;
  Datum *datumarr = temporal_values_p(temp, &count);
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    result = invert ?  func(PointerGetDatum(gs), datumarr[i]) :
      func(datumarr[i], PointerGetDatum(gs));
    if (result)
    {
      if (ever)
        break;
    }
    else
    {
      if (! ever)
        break;
    }
  }
  pfree(datumarr);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if two temporal geometries ever/always satisfy a spatial
 * relationship
 * @details The function uses the lifting infrastructure to account for
 * synchronization and interpolation and applies function `func` to every
 * instant of of the temporal geometries
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
  return eafunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if two temporal geometries ever/always contains satisfy a
 * spatial relationship, 0 if not, and -1 on error
 * @details
 * - A temporal geometry *ever* contains another one if there is an instant in
 *   which the two temporal geometries satisfy the relationship.
 * - A temporal geometry *always* contains another one if the traversed area
 *   of the first temporal geometry contains the traversed area of the second
 *   one
 * @param[in] temp1,temp2 Temporal geometries
 * @param[in] func Spatial relationship function to apply
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_spatialrel_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func, bool ever)
{
  VALIDATE_TGEO(temp1, -1); VALIDATE_TGEO(temp2, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_tgeo(temp1, temp2) ||
      ! ensure_not_geodetic(temp1->flags) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return -1;
  /* Ever */
  if (ever)
    return ea_spatialrel_tspatial_tspatial(temp1, temp2, func, EVER);
  /* Always */
  int result = spatialrel_tgeo_tgeo(temp1, temp2, (Datum) NULL,
    (varfunc) func, 2);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal geometry ever/always contains a geo, 0 if not,
 * and -1 on error or if the geometry is empty
 * @details
 * - A temporal geometry *ever* contains a geometry if the traversed area of
 *   the temporal geometry and the geometry intersect only in their interior,
 *   that is, they satisfy the relate pattern `"T********"`
 * - A temporal geometry *always* contains a geometry if the traversed area of
 *   the temporal geometry contains the geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True if the arguments should be inverted
 * @note Please refer to the documentation of the `ST_Contains` and `ST_Covers`
 * functions
 * https://postgis.net/docs/ST_Contains.html
 * https://postgis.net/docs/ST_Convers.html
 * for detailed explanations about the difference between both functions.
 */
int
ea_contains_tgeo_geo_int(const Temporal *temp, const GSERIALIZED *gs, bool ever,
  bool invert)
{
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1;
  char p[10] = "T********";
  int result = ever ?
    spatialrel_tgeo_geo(temp, gs, PointerGetDatum(&p),
      (varfunc) &datum_geom_relate_pattern, 3, invert) :
    spatialrel_tgeo_geo(temp, gs, (Datum) NULL,
      (varfunc) &datum_geom_contains, 2, invert);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever/always contains a geo, 0 if not,
 * and -1 on error or if the geometry is empty
 */
inline int
ea_contains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_contains_tgeo_geo_int(temp, gs, ever, INVERT);
}

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever/always contains a geo, 0 if not,
 * and -1 on error or if the geometry is empty
 */
inline int
ea_contains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_contains_tgeo_geo_int(temp, gs, ever, INVERT_NO);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry ever contains a temporal geo, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @csqlfn #Econtains_geo_tgeo()
 */
int
econtains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_tgeo_geo_int(temp, gs, EVER, INVERT);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry always contains a temporal geo,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @csqlfn #Acontains_geo_tgeo()
 */
int
acontains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_tgeo_geo_int(temp, gs, ALWAYS, INVERT);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever contains a geo, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Econtains_tgeo_geo()
 */
int
econtains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_contains_tgeo_geo_int(temp, gs, EVER, INVERT_NO);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry always contains a temporal geo,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Acontains_tgeo_geo()
 */
int
acontains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_contains_tgeo_geo_int(temp, gs, ALWAYS, INVERT_NO);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever contains another temporal
 * geometry, 0 if not, and -1 on error
 * @details
 * - A temporal geometry *ever* contains another one if there is an instant in
 *   which the two temporal geometries satisfy the relationship.
 * - A temporal geometry *always* contains another one if the traversed area
 *   of the first temporal geometry contains the traversed area of the second
 *   one
 * @param[in] temp1,temp2 Temporal geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note Please refer to the documentation of the `ST_Contains` and `ST_Covers`
 * functions
 * https://postgis.net/docs/ST_Contains.html
 * https://postgis.net/docs/ST_Convers.html
 * for detailed explanations about the difference between both functions.
 */
int
ea_contains_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  return ea_spatialrel_tgeo_tgeo(temp1, temp2, &datum_geom_contains, ever);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever contains another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Econtains_tgeo_tgeo()
 */
inline int
econtains_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_contains_tgeo_tgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever contains another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Acontains_tgeo_tgeo()
 */
inline int
acontains_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_contains_tgeo_tgeo(temp1, temp2, ALWAYS);
}
#endif /* MEOS */


/*****************************************************************************
 * Ever/always covers
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal geometry ever/always covers a geo, 0 if not,
 * and -1 on error or if the geometry is empty
 * @details
 * - A temporal geometry *ever* covers a geometry if there is an instant in
 *   which the temporal geometry and the geometry satisfy the relationship.
 * - A temporal geometry *always* covers a geometry if the traversed area of
 *   the temporal geometry covers the geometry
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] invert True if the arguments should be inverted
 * @note Please refer to the documentation of the `ST_Covers` and `ST_Covers`
 * functions
 * https://postgis.net/docs/ST_Covers.html
 * https://postgis.net/docs/ST_Convers.html
 * for detailed explanations about the difference between both functions.
 */
int
ea_covers_tgeo_geo_int(const Temporal *temp, const GSERIALIZED *gs, bool ever,
  bool invert)
{
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1;
  int result = ever ?
    /* Iterate for each composing geometry */
    ea_spatialrel_tspatial_geo(temp, gs, &datum_geom_covers, EVER, invert) :
    /* Compute the result from the traversed area and the geometry */
    spatialrel_tgeo_geo(temp, gs, (Datum) NULL, (varfunc) &datum_geom_covers,
      2, invert);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever/always covers a geo, 0 if not,
 * and -1 on error or if the geometry is empty
 */
inline int
ea_covers_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_covers_tgeo_geo_int(temp, gs, ever, INVERT);
}

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever/always covers a geo, 0 if not,
 * and -1 on error or if the geometry is empty
 */
inline int
ea_covers_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  return ea_covers_tgeo_geo_int(temp, gs, ever, INVERT_NO);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry ever covers a temporal geo, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @csqlfn #Ecovers_geo_tgeo()
 */
int
ecovers_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_covers_tgeo_geo_int(temp, gs, EVER, INVERT);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry always covers a temporal geo,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal geo
 * @csqlfn #Acovers_geo_tgeo()
 */
int
acovers_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_covers_tgeo_geo_int(temp, gs, ALWAYS, INVERT);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever covers a geo, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Ecovers_tgeo_geo()
 */
int
ecovers_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_tgeo_geo_int(temp, gs, EVER, INVERT_NO);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry always covers a temporal geo,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Acovers_tgeo_geo()
 */
int
acovers_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_covers_tgeo_geo_int(temp, gs, ALWAYS, INVERT_NO);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever covers another one, 0 if not,
 * and -1 on error
 * @details
 * - A temporal geometry *ever* covers another one if there is an instant in
 *   which the two temporal geometries satisfy the relationship.
 * - A temporal geometry *always* covers another one if the traversed
 *   area of the first temporal geometry covers the traversed area of the
 *   second one
 * @param[in] temp1,temp2 Temporal geometries
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @note Please refer to the documentation of the `ST_Covers` and `ST_Covers`
 * functions
 * https://postgis.net/docs/ST_Covers.html
 * https://postgis.net/docs/ST_Convers.html
 * for detailed explanations about the difference between both functions.
 */
int
ea_covers_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  return ea_spatialrel_tgeo_tgeo(temp1, temp2, &datum_geom_covers, ever);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever covers another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ecovers_tgeo_tgeo()
 */
inline int
ecovers_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_covers_tgeo_tgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever covers another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Acovers_tgeo_tgeo()
 */
inline int
acovers_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_covers_tgeo_tgeo(temp1, temp2, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always disjoint (work for both geometry and geography)
 *****************************************************************************/

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry and a geometry are ever disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @details
 * - A temporal point is *ever* disjoint with a geometry if the trajectory
 *   of the temporal point is NOT covered by the geometry
 * - A temporal geometry is *ever* disjoint with a geometry if one of its
 *   component geometries is disjoint with the geometry
 * - A temporal geometry and a geometry are *always* disjoint if the traversed
 *   area of the temporal geometry DO NOT ever intersects the geometry,
 *   that is, aDisjoint(a, b) is equivalent to NOT eIntersects(a, b)
 * @param[in] temp Temporal geometry
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edisjoint_tgeo_geo()
 */
int
ea_disjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;
  int result;

  /* ALWAYS */
  if (! ever)
  {
    return INVERT_RESULT(ea_intersects_tgeo_geo(temp, gs, EVER));
  }

  /* EVER */

  /* Temporal point case */
  if (tpoint_type(temp->temptype))
  {
    datum_func2 func = &datum_geom_covers;
    result = spatialrel_tgeo_geo(temp, gs, (Datum) NULL, (varfunc) func, 2,
      INVERT);
    return INVERT_RESULT(result);
  }

  /* Temporal geometry case */
  int count;
  Datum *datumarr = temporal_values_p(temp, &count);
  datum_func2 func = geo_disjoint_fn_geo(temp->flags, gs->gflags);
  result = 0;
  for (int i = 0; i < count; i++)
  {
    if (func(datumarr[i], PointerGetDatum(gs)))
    {
      result = 1;
      break;
    }
  }
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry and a geometry are ever disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 */
inline int
ea_disjoint_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_disjoint_tgeo_geo(temp, gs, ever);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry and a geometry are ever disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Edisjoint_tgeo_geo()
 */
inline int
edisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_tgeo_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry and a geometry are always disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Adisjoint_tgeo_geo()
 */
inline int
adisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_tgeo_geo(temp, gs, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if the temporal geos ever/always intersect, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @details Two temporal geometries are ever/always disjoint if there is an
 * instant in which the two geometries satisfy the relationship
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tgeo_tgeo(), #Aintersects_tgeo_tgeo()
 */
int
ea_disjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  VALIDATE_TGEO(temp1, -1); VALIDATE_TGEO(temp2, -1);
  if (! ensure_valid_tgeo_tgeo(temp1, temp2))
    return -1;
  datum_func2 func = geo_disjoint_fn(temp1->flags, temp2->flags);
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
 * @brief Return 1 if a temporal geometry ever/always intersects a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @details
 * - A temporal geometry *ever* intersects a geometry if the traversed area or
 *   the trajectory of the temporal geometry and the geometry intersect
 * - A temporal geometry *always* intersects a geometry if the traversed area
 *   or the trajectory of the temporal geometry and the geometry are NOT ever
 *   disjoint
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tgeo_geo()
 */
int
ea_intersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  /* ALWAYS */
  if (! ever)
    return INVERT_RESULT(ea_disjoint_tgeo_geo(temp, gs, EVER));

  /* EVER */
  datum_func2 func = geo_intersects_fn_geo(temp->flags, gs->gflags);
  return spatialrel_tgeo_geo(temp, gs, (Datum) NULL, (varfunc) func, 2,
    INVERT_NO);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry intersects a temporal geometry, 0 if not,
 * and -1 on error or if the geometry is empty
 */
inline int
ea_intersects_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  return ea_intersects_tgeo_geo(temp, gs, ever);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever intersects a temporal
 * geometry, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Aintersects_tgeo_geo()
 */
inline int
eintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_intersects_tgeo_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry always intersects a temporal
 * geometry, 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Aintersects_tgeo_geo()
 */
inline int
aintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_intersects_tgeo_geo(temp, gs, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if the temporal geos ever/always intersect, 0 if not, and
 * -1 on error or if the temporal geos do not intersect in time
 * @details Two temporal geometries ever/always intersect if there is an
 * instant in which the two geometries satisfy the relationship
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Eintersects_tgeo_tgeo(), #Aintersects_tgeo_tgeo()
 */
int
ea_intersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  VALIDATE_TGEO(temp1, -1); VALIDATE_TGEO(temp2, -1);
  if (! ensure_valid_tgeo_tgeo(temp1, temp2))
    return -1;
  datum_func2 func = geo_intersects_fn(temp1->flags, temp2->flags);
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
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point ever/always touches a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @details
 * - A temporal point *ever* touches a geometry if (a) the trajectory of the
 *   temporal point intersects the boundary of the geometry when the latter
 *   is not empty, or (b) the boundary of the trajectory of the temporal point
 *   intersects the geometry when the former is not empty
 * - A temporal point *always* touches a geometry if the restriction of the
 *   temporal point to the complement of the boundary of the geometry (that is,
 *   MINUS) is empty.
 * Note that when the two boundaries are empty, the trajectory of the point and
 * the geometry are both (multi)points and the result is false.
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tpoint_geo()
 */
int
ea_touches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic(temp->flags) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1;

  /* Bounding box test */
  STBox *box1 = tspatial_to_stbox(temp);
  STBox *box2 = geo_stbox(gs);
  if (! overlaps_stbox_stbox(box1, box2))
    return 0;

  /* EVER */
  if (ever)
  {
    datum_func2 func = geo_intersects_fn_geo(temp->flags, gs->gflags);
    GSERIALIZED *traj = tpoint_trajectory(temp);
    GSERIALIZED *gsbound = geom_boundary(gs);
    bool result = false;
    if (gsbound && ! gserialized_is_empty(gsbound))
      result = func(GserializedPGetDatum(gsbound), GserializedPGetDatum(traj));
    else if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
    {
      /* The geometry is a point or a multipoint -> the boundary is empty */
      GSERIALIZED *tempbound = geom_boundary(traj);
      if (tempbound)
      {
        result = func(GserializedPGetDatum(tempbound), GserializedPGetDatum(gs));
        pfree(tempbound);
      }
    }
    pfree(traj); pfree(gsbound);
    return result ? 1 : 0;
  }

  /* ALWAYS */
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

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point ever touches a geometry, 0 if not,
 * and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tpoint_geo()
 */
int
etouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tpoint_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point always touches a geometry, 0 if not,
 * and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tpoint_geo()
 */
int
atouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tpoint_geo(temp, gs, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever/always touches a geometry,
 * 0 if not, and -1 on error or if the geometry is empty
 * @details
 * - A temporal geometry *ever* touches a geometry if the there is an instant
 *   in which they satisfy the relationship
 * - A temporal geometry *always* touches  a geometry if they satisfy the
 *   relationship in every instant
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tgeo_geo()
 */
int
ea_touches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic(temp->flags) || ! ensure_has_not_Z_geo(gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1;

  /* Bounding box test */
  STBox *box1 = tspatial_to_stbox(temp);
  STBox *box2 = geo_stbox(gs);
  if (! overlaps_stbox_stbox(box1, box2))
    return 0;

  return ea_spatialrel_tspatial_geo(temp, gs, &datum_geom_touches, ever,
    INVERT_NO);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever touches a geometry, 0 if not,
 * and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Etouches_tgeo_geo()
 */
int
etouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tgeo_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry always touches a geometry, 0 if not,
 * and -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tgeo_geo()
 */
int
atouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tgeo_geo(temp, gs, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever/always touches another one,
 * 0 if not, and -1 on error
 * @details
 * - A temporal geometry *ever* touches another one if there is an instant
 *  in which they satisfy the relationship.
 * - A temporal geometry *always* touches another one if there is an instant
 *   in which the boundary of one of the two geometries intersects the
 *   trajectory or traversed area of the other.
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Etouches_tgeo_tgeo()
 */
int
ea_touches_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool ever)
{
  VALIDATE_TGEO(temp1, -1); VALIDATE_TGEO(temp2, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_tgeo(temp1, temp2) ||
      /* The validity function ensures that both have the same geodetic flag */
      ! ensure_not_geodetic(temp1->temptype) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp1->temptype, temp2->flags))
    return -1;

  /* Bounding box test */
  STBox *box1 = tspatial_to_stbox(temp1);
  STBox *box2 = tspatial_to_stbox(temp2);
  if (! overlaps_stbox_stbox(box1, box2))
    return 0;

  return ea_spatialrel_tspatial_tspatial(temp1, temp2, &datum_geom_touches,
    ever);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry ever touches another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Etouches_tgeo_tgeo()
 */
int
etouches_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_touches_tgeo_tgeo(temp1, temp2, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a temporal geometry always touches another one, 0 if not,
 * and -1 on error
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Atouches_tgeo_tgeo()
 */
int
atouches_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  return ea_touches_tgeo_tgeo(temp1, temp2, EVER);
}
#endif /* MEOS */

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
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edwithin_tgeo_geo()
 */
int
ea_dwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist,
  bool ever)
{
  VALIDATE_TGEO(temp, -1); VALIDATE_NOT_NULL(gs, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  /* EVER */
  if (ever)
  {
    datum_func3 func = geo_dwithin_fn_geo(temp->flags, gs->gflags);
    return spatialrel_tgeo_geo(temp, gs, Float8GetDatum(dist),
      (varfunc) func, 3, INVERT_NO);
  }

  /* ALWAYS */
  GSERIALIZED *buffer = geom_buffer(gs, dist, "");
  int result = spatialrel_tgeo_geo(temp, buffer, (Datum) NULL,
    (varfunc) &datum_geom_covers, 2, INVERT);
  pfree(buffer);
  return result;
}

#if MEOS
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
  return ea_dwithin_tgeo_geo(temp, gs, dist, EVER);
}

/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if a geometry and a temporal geo are always within a
 * distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tgeo_geo()
 */
int
adwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  return ea_dwithin_tgeo_geo(temp, gs, dist, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Return 1 or 2 if two temporal point segments are within a distance
 * during the period defined by the output timestamps, return 0 otherwise
 * @param[in] start1,end1 Base values defining the first segment
 * @param[in] start2,end2 Base values defining the second segment
 * @param[in] value Base value
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not both constants.
 */
int
tpointsegm_tdwithin_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum value, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1), assert(t2);
  double dist = DatumGetFloat8(value);
  double duration = (double) (upper - lower);
  const GSERIALIZED *gs1 = DatumGetGserializedP(start1);
  const GSERIALIZED *gs2 = DatumGetGserializedP(start2);
  datum_func3 func = geo_dwithin_fn(gs1->gflags, gs2->gflags);
  bool hasz = FLAGS_GET_Z(gs1->gflags);
  long double a, b, c;
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(start1);
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(end1);
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(start2);
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(end2);

    /* per1 functions
     * x(t) = a1 * t + c1
     * y(t) = a2 * t + c2
     * z(t) = a3 * t + c3 */
    double a1 = (p2->x - p1->x);
    double c1 = p1->x;
    double a2 = (p2->y - p1->y);
    double c2 = p1->y;
    double a3 = (p2->z - p1->z);
    double c3 = p1->z;

    /* per2 functions
     * x(t) = a4 * t + c4
     * y(t) = a5 * t + c5
     * z(t) = a6 * t + c6 */
    double a4 = (p4->x - p3->x);
    double c4 = p3->x;
    double a5 = (p4->y - p3->y);
    double c5 = p3->y;
    double a6 = (p4->z - p3->z);
    double c6 = p3->z;

    /* compute the distance function */
    double a_x = (a1 - a4) * (a1 - a4);
    double a_y = (a2 - a5) * (a2 - a5);
    double a_z = (a3 - a6) * (a3 - a6);
    double b_x = 2 * (a1 - a4) * (c1 - c4);
    double b_y = 2 * (a2 - a5) * (c2 - c5);
    double b_z = 2 * (a3 - a6) * (c3 - c6);
    double c_x = (c1 - c4) * (c1 - c4);
    double c_y = (c2 - c5) * (c2 - c5);
    double c_z = (c3 - c6) * (c3 - c6);
    /* distance function = dist */
    a = a_x + a_y + a_z;
    b = b_x + b_y + b_z;
    c = c_x + c_y + c_z - (dist * dist);
  }
  else /* 2D */
  {
    const POINT2D *p1 = DATUM_POINT2D_P(start1);
    const POINT2D *p2 = DATUM_POINT2D_P(end1);
    const POINT2D *p3 = DATUM_POINT2D_P(start2);
    const POINT2D *p4 = DATUM_POINT2D_P(end2);
    /* per1 functions
     * x(t) = a1 * t + c1
     * y(t) = a2 * t + c2 */
    double a1 = (p2->x - p1->x);
    double c1 = p1->x;
    double a2 = (p2->y - p1->y);
    double c2 = p1->y;
    /* per2 functions
     * x(t) = a3 * t + c3
     * y(t) = a4 * t + c4 */
    double a3 = (p4->x - p3->x);
    double c3 = p3->x;
    double a4 = (p4->y - p3->y);
    double c4 = p3->y;
    /* compute the distance function */
    double a_x = (a1 - a3) * (a1 - a3);
    double a_y = (a2 - a4) * (a2 - a4);
    double b_x = 2 * (a1 - a3) * (c1 - c3);
    double b_y = 2 * (a2 - a4) * (c2 - c4);
    double c_x = (c1 - c3) * (c1 - c3);
    double c_y = (c2 - c4) * (c2 - c4);
    /* distance function = dist */
    a = a_x + a_y;
    b = b_x + b_y;
    c = c_x + c_y - (dist * dist);
  }
  /* They are parallel, moving in the same direction at the same speed */
  if (a == 0)
  {
    if (! func(start1, start2, Float8GetDatum(dist)))
      return 0;
    *t1 = lower;
    *t2 = upper;
    return 2;
  }

  /* Solving the quadratic equation for distance = dist */
  long double discriminant = b * b - 4 * a * c;

  /* One solution */
  if (discriminant == 0)
  {
    long double t5 = (-1 * b) / (2 * a);
    if (t5 < 0.0 || t5 > 1.0)
      return 0;
    *t1 = *t2 = lower + (TimestampTz) (t5 * duration);
    return 1;
  }
  /* No solution */
  if (discriminant < 0)
    return 0;
  else
  /* At most two solutions within the time interval */
  {
    /* Apply a mixture of quadratic formula and Viète formula to improve precision */
    long double t5, t6;
    if (b >= 0)
    {
      t5 = (-1 * b - sqrtl(discriminant)) / (2 * a);
      t6 = (2 * c ) / (-1 * b - sqrtl(discriminant));
    }
    else
    {
      t5 = (2 * c ) / (-1 * b + sqrtl(discriminant));
      t6 = (-1 * b + sqrtl(discriminant)) / (2 * a);
    }

    /* If the two intervals do not intersect */
    if (0.0 > t6 || t5 > 1.0)
      return 0;
    /* Compute the intersection of the two intervals */
    long double t7 = Max(0.0, t5);
    long double t8 = Min(1.0, t6);
    if (fabsl(t7 - t8) < MEOS_EPSILON)
    {
      *t1 = *t2 = lower + (TimestampTz) (t7 * duration);
      return 1;
    }
    else
    {
      *t1 = lower + (TimestampTz) (t7 * duration);
      *t2 = lower + (TimestampTz) (t8 * duration);
      return 2;
    }
  }
}

/**
 * @ingroup meos_internal_geo_spatial_rel_ever
 * @brief Return 1 if two temporal geos are ever/always within a distance,
 * 0 if not, -1 on error or if the temporal geos do not intersect on time
 * @param[in] temp1,temp2 Temporal geos
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edwithin_tgeo_tgeo(), #Adwithin_tgeo_tgeo()
 */
int
ea_dwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist,
  bool ever)
{
  VALIDATE_TGEO(temp1, -1); VALIDATE_TGEO(temp2, -1);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tgeo_tgeo(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  datum_func3 func = geo_dwithin_fn(temp1->flags, temp2->flags);
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Float8GetDatum(dist);
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.ever = ever;
  lfinfo.tpfn_temp = tpoint_type(temp1->temptype) ?
    &tpointsegm_tdwithin_turnpt : NULL;
  return eafunc_temporal_temporal(temp1, temp2, &lfinfo);
}

#if MEOS
/**
 * @ingroup meos_geo_rel_ever
 * @brief Return 1 if two temporal geos are ever within a distance,
 * 0 if not, -1 on error or if they do not intersect on time
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
 * 0 if not, -1 on error or if they do not intersect on time
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

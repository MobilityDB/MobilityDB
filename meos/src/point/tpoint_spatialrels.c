/*****************************************************************************
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
 * @brief Ever/always spatial relationships for temporal points
 *
 * These relationships compute the ever/always spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported for geometries: contains,
 * disjoint, intersects, touches, and dwithin.
 *
 * The following relationships are supported for geographies: disjoint,
 * intersects, dwithin.
 *
 * Only disjoint, dwithin, and intersects are supported for 3D geometries.
 */

#include "point/tpoint_spatialrels.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "point/pgis_types.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_tempspatialrels.h"

/* Function calling PostGIS ST_Intersects to reuse their cache */

extern Datum pgis_intersects2d(Datum geom1, Datum geom2);

/*****************************************************************************
 * Spatial relationship functions
 * disjoint and intersects are inverse to each other
 *****************************************************************************/

/**
 * @brief Return a Datum true if the first geometry contains the second one
 */
Datum
geom_contains(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geometry_spatialrel(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), CONTAINS));
}

/**
 * @brief Return a Datum true if two geometries are disjoint in 2D
 */
Datum
geom_disjoint2d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(! DatumGetBool(geom_intersects2d(geom1, geom2)));
}

/**
 * @brief Return a Datum true if two geometries are disjoint in 3D
 */
Datum
geom_disjoint3d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(! DatumGetBool(geom_intersects3d(geom1, geom2)));
}

/**
 * @brief Return a Datum true if two geographies are disjoint
 */
Datum
geog_disjoint(Datum geog1, Datum geog2)
{
  return BoolGetDatum(! DatumGetBool(geog_intersects(geog1, geog2)));
}

/**
 * @brief Return a Datum true if two geometries intersect in 2D
 */
Datum
geom_intersects2d(Datum geom1, Datum geom2)
{
#if MEOS
  return BoolGetDatum(geometry_spatialrel(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), INTERSECTS));
#else
  return pgis_intersects2d(geom1, geom2);
#endif /* MEOS */
}

/**
 * @brief Return a Datum true if two geometries intersect in 3D
 */
Datum
geom_intersects3d(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geometry_3Dintersects(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2)));
}

/**
 * @brief Return a Datum true if two geographies intersect
 */
Datum
geog_intersects(Datum geog1, Datum geog2)
{
  return BoolGetDatum(pgis_geography_dwithin(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), 0.0, true));
}

#if 0 /* not used */
/**
 * @brief Return a Datum true if two geometries touch
 */
Datum
geom_touches(Datum geom1, Datum geom2)
{
  return BoolGetDatum(geometry_spatialrel(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), TOUCHES));
}
#endif /* not used */

/**
 * @brief Return a Datum true if two 2D geometries are within a distance
 */
Datum
geom_dwithin2d(Datum geom1, Datum geom2, Datum dist)
{
  return BoolGetDatum(geometry_dwithin2d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), DatumGetFloat8(dist)));
}

/**
 * @brief Return a Datum true if two 3D geometries are within a distance
 */
Datum
geom_dwithin3d(Datum geom1, Datum geom2, Datum dist)
{
  return BoolGetDatum(geometry_dwithin3d(DatumGetGserializedP(geom1),
    DatumGetGserializedP(geom2), DatumGetFloat8(dist)));
}

/**
 * @brief Return a Datum true if two geographies are within a distance
 */
Datum
geog_dwithin(Datum geog1, Datum geog2, Datum dist)
{
  return BoolGetDatum(pgis_geography_dwithin(DatumGetGserializedP(geog1),
    DatumGetGserializedP(geog2), DatumGetFloat8(dist), true));
}

/*****************************************************************************/

/**
 * @brief Select the appropriate disjoint function 
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func2
get_disjoint_fn_gs(int16 flags1, uint8_t flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &geog_disjoint;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &geom_disjoint3d : &geom_disjoint2d;
}

/**
 * @brief Select the appropriate intersect function
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func2
get_intersects_fn_gs(int16 flags1, uint8_t flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &geog_intersects;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &geom_intersects3d : &geom_intersects2d;
}

/**
 * @brief Select the appropriate dwithin function
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
datum_func3
get_dwithin_fn(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && MEOS_FLAGS_GET_Z(flags2) ?
      &geom_dwithin3d : &geom_dwithin2d;
}

/**
 * @brief Select the appropriate dwithin function
 * @note We need two parameters to cope with mixed 2D/3D arguments
 */
static datum_func3
get_dwithin_fn_gs(int16 flags1, uint8_t flags2)
{
  if (MEOS_FLAGS_GET_GEODETIC(flags1))
    return &geog_dwithin;
  else
    /* 3D only if both arguments are 3D */
    return MEOS_FLAGS_GET_Z(flags1) && FLAGS_GET_Z(flags2) ?
      &geom_dwithin3d : &geom_dwithin2d;
}

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Generic spatial relationship for the trajectory of a temporal point
 * and a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] numparam Number of parameters of the functions
 * @param[in] invert True if the arguments should be inverted
 * @return On error return -1
 */
static int
spatialrel_tpoint_traj_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum param, Datum (*func)(Datum, ...), int numparam, bool invert)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  assert(numparam == 2 || numparam == 3);
  Datum geo = PointerGetDatum(gs);
  Datum traj = PointerGetDatum(tpoint_trajectory(temp));
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
 * @brief Return true if two temporal points ever/always satisfy a spatial
 * relationship
 * @param[in] temp1,temp2 Temporal points
 * @param[in] func Spatial relationship
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
int
ea_spatialrel_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum), bool ever)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_valid_tpoint_tpoint(temp1, temp2) ||
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
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a geometry ever contains a temporal point, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal point
 * @note The function does not accept 3D or geography since it is based on the
 * PostGIS ST_Relate function. The function tests whether the trajectory
 * intersects the interior of the geometry. Please refer to the documentation
 * of the ST_Contains and ST_Relate functions
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Econtains_geo_tpoint()
 */
int
econtains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_has_not_Z_gs(gs) || ! ensure_has_not_Z(temp->flags))
    return -1;
  GSERIALIZED *traj = tpoint_trajectory(temp);
  bool result = geo_relate_pattern(gs, traj, "T********");
  pfree(traj);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a geometry always contains a temporal point,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] gs Geometry
 * @param[in] temp Temporal point
 * @note The function tests whether the trajectory is contained in the geometry.
 * https://postgis.net/docs/ST_Relate.html
 * https://postgis.net/docs/ST_Contains.html
 * @csqlfn #Acontains_geo_tpoint()
 */
int
acontains_geo_tpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_has_not_Z_gs(gs) || ! ensure_has_not_Z(temp->flags))
    return -1;
  GSERIALIZED *traj = tpoint_trajectory(temp);
  bool result = DatumGetBool(geom_contains(PointerGetDatum(gs),
    PointerGetDatum(traj)));
  pfree(traj);
  return result ? 1 : 0;
}

/*****************************************************************************
 * Ever/always disjoint (for both geometry and geography)
 *****************************************************************************/

/**
 * @brief Return true if a temporal instant point and a geometry are
 * ever/always disjoint
 * @param[in] inst Temporal point
 * @param[in] geo Geometry
 * @param[in] func PostGIS function to be called
 */
bool
ea_disjoint_tpointinst_geo(const TInstant *inst, Datum geo,
  datum_func2 func)
{
  return DatumGetBool(func(tinstant_val(inst), geo));
}

/**
 * @brief Return true if a temporal sequence point and a geometry are
 * ever/always disjoint
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] func PostGIS function to be called
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
bool
ea_disjoint_tpointseq_geo(const TSequence *seq, Datum geo,
  datum_func2 func, bool ever)
{
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < seq->count; i++)
  {
    bool res = DatumGetBool(func(tinstant_val(TSEQUENCE_INST_N(seq, i)), geo));
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/**
 * @brief Return true if a temporal sequence set point and a geometry are ever
 * disjoint
 * @param[in] ss Temporal point
 * @param[in] geo Geometry
 * @param[in] func PostGIS function to be used for instantaneous sequences
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
bool
ea_disjoint_tpointseqset_geo(const TSequenceSet *ss, Datum geo,
  datum_func2 func, bool ever)
{
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < ss->count; i++)
  {
    bool res = ea_disjoint_tpointseq_geo(TSEQUENCESET_SEQ_N(ss, i), geo, func,
      ever);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point and a geometry are ever disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edisjoint_tpoint_geo()
 */
Datum
ea_disjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return Int32GetDatum(-1);

  datum_func2 func = get_disjoint_fn_gs(temp->flags, gs->gflags);
  bool result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = ea_disjoint_tpointinst_geo((TInstant *) temp, PointerGetDatum(gs),
        func);
      break;
    case TSEQUENCE:
      result = ea_disjoint_tpointseq_geo((TSequence *) temp, PointerGetDatum(gs),
        func, ever);
      break;
    default: /* TSEQUENCESET */
      result = ea_disjoint_tpointseqset_geo((TSequenceSet *) temp,
        PointerGetDatum(gs), func, ever);
  }
  return result ? Int32GetDatum(1) : Int32GetDatum(0);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point and a geometry are ever disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Edisjoint_tpoint_geo()
 */
int
edisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return DatumGetInt32(ea_disjoint_tpoint_geo(temp, gs, EVER));
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point and a geometry are always disjoint,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Adisjoint_tpoint_geo()
 */
int
adisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return DatumGetInt32(ea_disjoint_tpoint_geo(temp, gs, ALWAYS));
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if the temporal points are ever disjoint, 0 if not, and
 * -1 on error or if the temporal points do not intersect in time
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Edisjoint_tpoint_tpoint()
 */
int
edisjoint_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_nsame, EVER) :
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_ne, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if the temporal points are always disjoint, 0 if not, and
 * -1 on error or if the temporal points do not intersect in time
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Adisjoint_tpoint_tpoint()
 */
int
adisjoint_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_nsame, ALWAYS) :
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_ne, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always intersects (for both geometry and geography)
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a geometry and a temporal point ever intersect,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Eintersects_tpoint_geo()
 */
int
eintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  datum_func2 func = get_intersects_fn_gs(temp->flags, gs->gflags);
  return spatialrel_tpoint_traj_geo(temp, gs, (Datum) NULL,
    (varfunc) func, 2, INVERT_NO);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a geometry and a temporal point always intersect,
 * 0 if not, and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Aintersects_tpoint_geo()
 */
int
aintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ! edisjoint_tpoint_geo(temp, gs);
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if the temporal points ever intersect, 0 if not, and
 * -1 on error or if the temporal points do not intersect in time
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Eintersects_tpoint_tpoint()
 */
int
eintersects_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_same, EVER) :
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_eq, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if the temporal points always intersect, 0 if not, and
 * -1 on error or if the temporal points do not intersect in time
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Aintersects_tpoint_tpoint()
 */
int
aintersects_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_same, ALWAYS) :
    ea_spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_eq, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point and a geometry ever touch, 0 if not, and
 * -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Etouches_tpoint_geo()
 */
int
etouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  /* There is no need to do a bounding box test since this is done in
   * the SQL function definition */
  varfunc func = (MEOS_FLAGS_GET_Z(temp->flags) && FLAGS_GET_Z(gs->gflags)) ?
    (varfunc) &geom_intersects3d : (varfunc) &geom_intersects2d;
  GSERIALIZED *traj = tpoint_trajectory(temp);
  GSERIALIZED *gsbound = geometry_boundary(gs);
  bool result = false;
  if (gsbound && ! gserialized_is_empty(gsbound))
    result = func(PointerGetDatum(gsbound), PointerGetDatum(traj));
  /* TODO */
  // else if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  // {
    // /* The geometry is a point or a multipoint -> the boundary is empty */
    // GSERIALIZED *tempbound = geometry_boundary(traj);
    // if (tempbound)
    // {
      // result = func(PointerGetDatum(tempbound), PointerGetDatum(gs));
      // pfree(tempbound);
    // }
  // }
  pfree(traj); pfree(gsbound);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a temporal point and a geometry always touch, 0 if not,
 * and -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tpoint_geo()
 */
int
atouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return -1;

  /* There is no need to do a bounding box test since this is done in
   * the SQL function definition */
  GSERIALIZED *gsbound = geometry_boundary(gs);
  bool result = false;
  if (gsbound && ! gserialized_is_empty(gsbound))
  {
    Temporal *temp1 = tpoint_restrict_geom_time(temp, gsbound, NULL, NULL,
      REST_MINUS);
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
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a geometry and a temporal point are ever within the
 * given distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tpoint_geo()
 */
int
edwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  datum_func3 func = get_dwithin_fn_gs(temp->flags, gs->gflags);
  return spatialrel_tpoint_traj_geo(temp, gs, Float8GetDatum(dist),
    (varfunc) func, 3, INVERT_NO);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if a geometry and a temporal point are always within a
 * distance, 0 if not, -1 on error or if the geometry is empty
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @note The function is not available for 3D or geograhies since it is based
 * on thePostGIS ST_Buffer() function which is performed by GEOS
 * @csqlfn #Adwithin_tpoint_geo()
 */
int
adwithin_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_geodetic(temp->flags) || ! ensure_has_not_Z(temp->flags) ||
      ! ensure_valid_tpoint_geo(temp, gs) || gserialized_is_empty(gs) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  GSERIALIZED *buffer = geometry_buffer(gs, dist, "");
  datum_func2 func = get_intersects_fn_gs(temp->flags, gs->gflags);
  int result = spatialrel_tpoint_traj_geo(temp, buffer, Float8GetDatum(dist),
    (varfunc) func, 3, INVERT_NO);
  pfree(buffer);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal points are ever within a distance
 * @param[in] inst1,inst2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points are synchronized
 */
static bool
ea_dwithin_tpointinst_tpointinst(const TInstant *inst1, const TInstant *inst2,
  double dist, datum_func3 func)
{
  assert(inst1); assert(inst2);
  Datum value1 = tinstant_val(inst1);
  Datum value2 = tinstant_val(inst2);
  /* Result is the same for both EVER and ALWAYS */
  return DatumGetBool(func(value1, value2, Float8GetDatum(dist)));
}

/**
 * @brief Return true if two temporal points are ever within a distance
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal points are synchronized
 */
static bool
ea_dwithin_tpointseq_tpointseq_discstep(const TSequence *seq1,
  const TSequence *seq2, double dist, datum_func3 func, bool ever)
{
  assert(seq1); assert(seq2);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < seq1->count; i++)
  {
    bool res = ea_dwithin_tpointinst_tpointinst(TSEQUENCE_INST_N(seq1, i),
      TSEQUENCE_INST_N(seq2, i), dist, func);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/**
 * @brief Return true if two temporal points are ever within a distance
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal points are synchronized
 */
static bool
ea_dwithin_tpointseq_tpointseq_cont(const TSequence *seq1,
  const TSequence *seq2, double dist, datum_func3 func, bool ever)
{
  assert(seq1); assert(seq2);

  const TInstant *start1, *start2;
  if (seq1->count == 1)
  {
    start1 = TSEQUENCE_INST_N(seq1, 0);
    start2 = TSEQUENCE_INST_N(seq2, 0);
    return ea_dwithin_tpointinst_tpointinst(start1, start2, dist, func);
  }

  start1 = TSEQUENCE_INST_N(seq1, 0);
  start2 = TSEQUENCE_INST_N(seq2, 0);
  Datum sv1 = tinstant_val(start1);
  Datum sv2 = tinstant_val(start2);

  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  bool hasz = MEOS_FLAGS_GET_Z(seq1->flags);
  TimestampTz lower = start1->t;
  bool lower_inc = seq1->period.lower_inc;
  bool ret_loop = ever ? true : false;
  for (int i = 1; i < seq1->count; i++)
  {
    const TInstant *end1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *end2 = TSEQUENCE_INST_N(seq2, i);
    Datum ev1 = tinstant_val(end1);
    Datum ev2 = tinstant_val(end2);
    TimestampTz upper = end1->t;
    bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;

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
    bool res = (solutions == 2 ||
      (solutions == 1 && ((t1 != lower || lower_inc) &&
        (t1 != upper || upper_inc))));
    if ((ever && res) || (! ever && ! res))
      return ret_loop;

    sv1 = ev1;
    sv2 = ev2;
    lower = upper;
    lower_inc = true;
  }
  return ! ret_loop;
}

/**
 * @brief Return true if two temporal points are ever within a distance
 * @param[in] ss1,ss2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @pre The temporal points are synchronized
 */
static bool
ea_dwithin_tpointseqset_tpointseqset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, double dist, datum_func3 func, bool ever)
{
  assert(ss1); assert(ss2);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(ss2->flags);
  bool ret_loop = ever ? true : false;
  for (int i = 0; i < ss1->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, i);
    bool res = linear ?
      ea_dwithin_tpointseq_tpointseq_cont(seq1, seq2, dist, func, ever) :
      ea_dwithin_tpointseq_tpointseq_discstep(seq1, seq2, dist, func, ever);
    if ((ever && res) || (! ever && ! res))
      return ret_loop;
  }
  return ! ret_loop;
}

/*****************************************************************************/

/**
 * @brief Return 1 if two temporal points are ever within a distance,
 * 0 if not, -1 if the temporal points do not intersect on time
 * @pre The temporal points are synchronized
 */
int
ea_dwithin_tpoint_tpoint1(const Temporal *sync1, const Temporal *sync2,
  double dist, bool ever)
{
  datum_func3 func = get_dwithin_fn(sync1->flags, sync2->flags);
  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
      return ea_dwithin_tpointinst_tpointinst((TInstant *) sync1,
        (TInstant *) sync2, dist, func);
    case TSEQUENCE:
      return MEOS_FLAGS_LINEAR_INTERP(sync1->flags) ||
          MEOS_FLAGS_LINEAR_INTERP(sync2->flags) ?
        ea_dwithin_tpointseq_tpointseq_cont((TSequence *) sync1,
          (TSequence *) sync2, dist, func, ever) :
        ea_dwithin_tpointseq_tpointseq_discstep((TSequence *) sync1,
          (TSequence *) sync2, dist, func, ever);
    default: /* TSEQUENCESET */
      return ea_dwithin_tpointseqset_tpointseqset((TSequenceSet *) sync1,
        (TSequenceSet *) sync2, dist, func, ever);
  }
}

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return 1 if two temporal points are ever within a distance,
 * 0 if not, -1 on error or if the temporal points do not intersect on time
 * @param[in] temp1,temp2 Temporal points
 * @param[in] dist Distance
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @csqlfn #Edwithin_tpoint_tpoint()
 */
int
ea_dwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_tpoint_tpoint(temp1, temp2) ||
      ! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return -1;

  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return -1;

  bool result = ea_dwithin_tpoint_tpoint1(sync1, sync2, dist, ever);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if two temporal points are ever within a distance,
 * 0 if not, -1 on error or if the temporal points do not intersect on time
 * @param[in] temp1,temp2 Temporal points
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tpoint_tpoint()
 */
int
edwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2))
    return -1;
  return ea_dwithin_tpoint_tpoint(temp1, temp2, dist, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if two temporal points are always within a distance,
 * 0 if not, -1 on error or if the temporal points do not intersect on time
 * @param[in] temp1,temp2 Temporal points
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tpoint_tpoint()
 */
int
adwithin_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2))
    return -1;
  return ea_dwithin_tpoint_tpoint(temp1, temp2, dist, ALWAYS);
}

/*****************************************************************************/

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
 * @brief Ever/always spatial relationships for temporal network points
 *
 * These relationships compute the ever/always spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported:
 * contains, disjoint, intersects, touches, and dwithin
 */

#include "npoint/tnpoint_spatialrels.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic binary functions for two temporal network points
 *****************************************************************************/

/**
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] func PostGIS function to be called
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_spatialrel_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  datum_func2 func, bool ever)
{
  assert(tspatial_srid(temp1) == tspatial_srid(temp2));
  /* Transform the temporal network points */
  Temporal *tpoint1 = tnpoint_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_tgeompoint(temp2);
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = lfinfo.argtype[1] = tpoint1->temptype;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(tpoint1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(tpoint2->flags);
  lfinfo.ever = ever;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  int result = eafunc_temporal_temporal(tpoint1, tpoint2, &lfinfo);
  /* Clean up and return */
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a geometry ever/always contains a temporal network
 * point
 * @param[in] gs Geometry
 * @param[in] temp Temporal network point
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_contains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *temp, bool ever)
{
  assert(gs); assert(temp);
  if (gserialized_is_empty(gs))
    return -1;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  int result = ever ? econtains_geo_tpoint(gs, tempgeom) :
    acontains_geo_tpoint(gs, tempgeom);
  pfree(tempgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Econtains_geo_tnpoint()
 */
int
econtains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_tnpoint(gs, temp, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Acontains_geo_tnpoint()
 */
int
acontains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return ea_contains_geo_tnpoint(gs, temp, ALWAYS);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a network point ever/always contains a temporal
 * network point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_contains_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool ever)
{
  assert(np); assert(temp);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  int result = ever ? econtains_geo_tpoint(gs, tempgeom) :
    acontains_geo_tpoint(gs, tempgeom);
  pfree(tempgeom); pfree(gs);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 */
int
econtains_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_contains_tnpoint_npoint(temp, np, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 */
int
acontains_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_contains_tnpoint_npoint(temp, np, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always disjoint
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network point are
 * ever/always disjoint
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_disjoint_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  assert(temp); assert(gs);
  if (gserialized_is_empty(gs))
    return -1;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  int result = ever ? edisjoint_tpoint_geo(tempgeom, gs) :
    adisjoint_tpoint_geo(tempgeom, gs);
  pfree(tempgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Edisjoint_tnpoint_npoint()
 */
int
edisjoint_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_tnpoint_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Adisjoint_tnpoint_geo()
 */
int
adisjoint_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_disjoint_tnpoint_geo(temp, gs, ALWAYS);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal network point are
 * ever/always disjoint
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_disjoint_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool ever)
{
  assert(temp); assert(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  int result = ever ? edisjoint_tpoint_geo(tempgeom, gs) :
    adisjoint_tpoint_geo(tempgeom, gs);
  pfree(tempgeom); pfree(gs);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Edisjoint_tnpoint_npoint()
 */
int
edisjoint_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_disjoint_tnpoint_npoint(temp, np, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Adisjoint_tnpoint_npoint()
 */
int
adisjoint_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_disjoint_tnpoint_npoint(temp, np, ALWAYS);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if the temporal network points are ever disjoint, 0 if not,
 * and -1 on error or if the temporal points do not intersect in time
 * @param[in] temp1,temp2 Temporal network points
 * @csqlfn #Edisjoint_tnpoint_tnpoint()
 */
int
edisjoint_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tnpoint_tnpoint(temp1, temp2, &datum2_point_ne, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return 1 if the temporal network points are always disjoint, 0 if 
 * not, and -1 on error or if the temporal points do not intersect in time
 * @param[in] temp1,temp2 Temporal network points
 * @csqlfn #Adisjoint_tnpoint_tnpoint()
 */
int
adisjoint_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return ea_spatialrel_tnpoint_tnpoint(temp1, temp2, &datum2_point_ne, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network point ever/always
 * intersect
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_intersects_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool ever)
{
  assert(temp); assert(gs);
  if (gserialized_is_empty(gs))
    return -1;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  int result = ever ? eintersects_tpoint_geo(tempgeom, gs) :
    aintersects_tpoint_geo(tempgeom, gs);
  pfree(tempgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Eintersects_tnpoint_npoint()
 */
int
eintersects_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_intersects_tnpoint_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Aintersects_tnpoint_geo()
 */
int
aintersects_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_intersects_tnpoint_geo(temp, gs, ALWAYS);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal network point
 * ever/always intersect
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_intersects_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool ever)
{
  assert(temp); assert(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  int result = ever ? eintersects_tpoint_geo(tempgeom, gs) :
    aintersects_tpoint_geo(tempgeom, gs);
  pfree(tempgeom); pfree(gs);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Eintersects_tnpoint_npoint()
 */
int
eintersects_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_intersects_tnpoint_npoint(temp, np, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Aintersects_tnpoint_npoint()
 */
int
aintersects_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_intersects_tnpoint_npoint(temp, np, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network point ever/always
 * touch
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_touches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool ever)
{
  assert(temp); assert(gs);
  if (gserialized_is_empty(gs))
    return -1;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  int result = ever ? etouches_tpoint_geo(tempgeom, gs) :
    atouches_tpoint_geo(tempgeom, gs);
  pfree(tempgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Etouches_tnpoint_npoint()
 */
int
etouches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tnpoint_geo(temp, gs, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @csqlfn #Atouches_tnpoint_geo()
 */
int
atouches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return ea_touches_tnpoint_geo(temp, gs, ALWAYS);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a network point
 * ever/always touch
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_touches_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool ever)
{
  assert(temp); assert(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  int result = ever ? etouches_tpoint_geo(tempgeom, gs) :
    atouches_tpoint_geo(tempgeom, gs);
  pfree(tempgeom); pfree(gs);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Etouches_tnpoint_npoint()
 */
int
etouches_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_touches_tnpoint_npoint(temp, np, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Atouches_tnpoint_npoint()
 */
int
atouches_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return ea_touches_tnpoint_npoint(temp, np, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************
 * Ever/always dwithin
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Generic spatial relationships for a temporal network point and a
 * geometry
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_dwithin_tnpoint_geom(const Temporal *temp, const GSERIALIZED *gs,
  double dist, bool ever)
{
  assert(temp); assert(gs);
  if (gserialized_is_empty(gs))
    return -1;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  int result = ever ? edwithin_tpoint_geo(tempgeom, gs, dist) :
    adwithin_tpoint_geo(tempgeom, gs, dist);
  pfree(tempgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tnpoint_tnpoint()
 */
int
edwithin_tnpoint_geom(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  return ea_dwithin_tnpoint_geom(temp, gs, dist, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tnpoint_tnpoint()
 */
int
adwithin_tnpoint_geom(const Temporal *temp, const GSERIALIZED *gs, double dist)
{
  return ea_dwithin_tnpoint_geom(temp, gs, dist, ALWAYS);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Generic spatial relationships for a temporal network point and a
 * geometry
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] dist Distance
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_dwithin_tnpoint_npoint(const Temporal *temp, const Npoint *np, double dist,
  bool ever)
{
  assert(temp); assert(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  int result = ever ? edwithin_tpoint_geo(tempgeom, gs, dist) :
    adwithin_tpoint_geo(tempgeom, gs, dist);
  pfree(tempgeom); pfree(gs);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tnpoint_tnpoint()
 */
int
edwithin_tnpoint_npoint(const Temporal *temp, const Npoint *np, double dist)
{
  return ea_dwithin_tnpoint_npoint(temp, np, dist, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tnpoint_tnpoint()
 */
int
adwithin_tnpoint_npoint(const Temporal *temp, const Npoint *np, double dist)
{
  return ea_dwithin_tnpoint_npoint(temp, np, dist, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] dist Distance
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_dwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return -1;

  Temporal *sync1, *sync2;
  /* Return -1 if the temporal points do not intersect in time */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return -1;

  Temporal *tpoint1 = tnpoint_tgeompoint(sync1);
  Temporal *tpoint2 = tnpoint_tgeompoint(sync2);
  bool result = ea_dwithin_tpoint_tpoint_sync(tpoint1, tpoint2, dist, ever);
  pfree(tpoint1); pfree(tpoint2);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

#if MEOS
/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] dist Distance
 * @csqlfn #Edwithin_tnpoint_tnpoint()
 */
int
edwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  return ea_dwithin_tnpoint_tnpoint(temp1, temp2, dist, EVER);
}

/**
 * @ingroup meos_temporal_spatial_rel_ever
 * @brief Return true if the temporal network points always satisfy the spatial
 * relationship
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] dist Distance
 * @csqlfn #Adwithin_tnpoint_tnpoint()
 */
int
adwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  return ea_dwithin_tnpoint_tnpoint(temp1, temp2, dist, ALWAYS);
}
#endif /* MEOS */

/*****************************************************************************/

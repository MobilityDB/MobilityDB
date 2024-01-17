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
#include <meos_internal.h>
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"
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
  Datum (*func)(Datum, Datum), bool ever)
{
  assert(tnpoint_srid(temp1) == tnpoint_srid(temp2));
  /* Transform the temporal network points */
  Temporal *tpoint1 = tnpoint_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_tgeompoint(temp2);
  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(tpoint1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = lfinfo.argtype[1] = basetype;
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

/**
 * @brief Return true if a network point ever/always contains a temporal
 * network point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @param[in] ever True to compute the ever semantics, false for always
 */
int
ea_contains_npoint_tnpoint(const Npoint *np, const Temporal *temp, bool ever)
{
  assert(np); assert(temp);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  int result = ever ? econtains_geo_tpoint(gs, tempgeom) :
    acontains_geo_tpoint(gs, tempgeom);
  pfree(tempgeom); pfree(gs);
  return result;
}

/*****************************************************************************
 * Ever/always disjoint
 *****************************************************************************/

/**
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

/**
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

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

/**
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

/**
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

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

/**
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

/**
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

/*****************************************************************************
 * Ever/always dwithin
 *****************************************************************************/

/**
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

/**
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

/*****************************************************************************/

/**
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 */
int
ea_dwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  double dist, bool ever)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_srid(tnpoint_srid(temp1), tnpoint_srid(temp2)))
    return -1;

  Temporal *sync1, *sync2;
  /* Return -1 if the temporal points do not intersect in time */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return -1;

  Temporal *tpoint1 = tnpoint_tgeompoint(sync1);
  Temporal *tpoint2 = tnpoint_tgeompoint(sync2);
  bool result = ea_dwithin_tpoint_tpoint1(tpoint1, tpoint2, dist, ever);
  pfree(tpoint1); pfree(tpoint2);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

/*****************************************************************************/

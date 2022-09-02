/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Ever spatial relationships for temporal network points.
 *
 * These relationships compute the ever spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported:
 * contains, disjoint, intersects, touches, and dwithin
 */

#include "npoint/tnpoint_spatialrels.h"

/* MobilityDB */
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic binary functions for tnpoint <rel> (geo | Npoint)
 *****************************************************************************/

/**
 * @brief Generic spatial relationships for a temporal network point and a
 * geometry
 *
 * @param[in] temp Temporal network point
 * @param[in] geom Geometry
 * @param[in] func PostGIS function to be called
 * @param[in] invert True if the arguments should be inverted
 */
Datum
spatialrel_tnpoint_geo(const Temporal *temp, Datum geom,
  Datum (*func)(Datum, Datum), bool invert)
{
  Datum geom1 = PointerGetDatum(tnpoint_geom(temp));
  Datum result = invert ? func(geom, geom1) : func(geom1, geom);
  pfree(DatumGetPointer(geom1));
  return result;
}

/**
 * @brief Generic spatial relationships for a temporal network point and a
 * network point
 */
Datum
spatialrel_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  Datum (*func)(Datum, Datum), bool invert)
{
  Datum geom1 = PointerGetDatum(tnpoint_geom(temp));
  Datum geom2 = PointerGetDatum(npoint_geom(np));
  Datum result = invert ? func(geom2, geom1) : func(geom1, geom2);
  pfree(DatumGetPointer(geom1));
  pfree(DatumGetPointer(geom2));
  return result;
}

/**
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 */
int
spatialrel_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum))
{
  ensure_same_srid(tnpoint_srid(temp1), tnpoint_srid(temp2));
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return -1;

  Temporal *tpoint1 = tnpoint_tgeompoint(sync1);
  Temporal *tpoint2 = tnpoint_tgeompoint(sync2);
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(tpoint1->temptype);
  lfinfo.argtype[1] = temptype_basetype(tpoint2->temptype);
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MOBDB_FLAGS_GET_LINEAR(tpoint1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(tpoint2->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  int result = efunc_temporal_temporal(tpoint1, tpoint2, &lfinfo);
  /* Finish */
  pfree(tpoint1); pfree(tpoint2);
  pfree(sync1); pfree(sync2);
  return result;
}

/*****************************************************************************
 * Generic ternary functions for tnpoint <rel> geo/tnpoint
 *****************************************************************************/

/**
 * @brief Generic spatial relationships for a temporal network point and a
 * geometry
 *
 * @param[in] temp Temporal network point
 * @param[in] geom Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] invert True if the arguments should be inverted
 */
Datum
spatialrel3_tnpoint_geom(const Temporal *temp, Datum geom, Datum param,
  Datum (*func)(Datum, Datum, Datum), bool invert)
{
  Datum geom1 = PointerGetDatum(tnpoint_geom(temp));
  Datum result = invert ? func(geom, geom1, param) : func(geom1, geom, param);
  pfree(DatumGetPointer(geom1));
  return result;
}

/**
 * @brief Generic spatial relationships for a temporal network point and a
 * geometry
 *
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] invert True if the arguments should be inverted
 */
Datum
spatialrel3_tnpoint_npoint(const Temporal *temp, const Npoint *np, Datum param,
  Datum (*func)(Datum, Datum, Datum), bool invert)
{
  Datum geom1 = PointerGetDatum(tnpoint_geom(temp));
  Datum geom2 = PointerGetDatum(npoint_geom(np));
  Datum result = invert ? func(geom2, geom1, param) :
    func(geom1, geom2, param);
  pfree(DatumGetPointer(geom1));
  pfree(DatumGetPointer(geom2));
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 */
int
dwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  ensure_same_srid(tnpoint_srid(temp1), tnpoint_srid(temp2));
  Temporal *sync1, *sync2;
  /* Return -1 if the temporal points do not intersect in time */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return -1;

  Temporal *tpoint1 = tnpoint_tgeompoint(sync1);
  Temporal *tpoint2 = tnpoint_tgeompoint(sync2);
  bool result = dwithin_tpoint_tpoint1(tpoint1, tpoint2, dist);
  pfree(tpoint1); pfree(tpoint2);
  pfree(sync1); pfree(sync2);
  return result ? 1 : 0;
}

/*****************************************************************************/

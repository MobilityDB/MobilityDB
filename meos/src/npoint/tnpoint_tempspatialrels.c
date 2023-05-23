/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Temporal spatial relationships for temporal network points.
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported:
 * tcontains, tdisjoint, tintersects, ttouches, and tdwithin
 */

#include "npoint/tnpoint_tempspatialrels.h"

/* MEOS */
#include <meos.h>
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_tempspatialrels.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the temporal disjoint/intersection relationship between the
 * temporal network point and the network point
 */
Temporal *
tinterrel_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool tinter,
  bool restr, bool atvalue)
{
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *geo = npoint_geom(np);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(tempgeom, geo, tinter, restr,
     atvalue);
  pfree(tempgeom);
  pfree(geo);
  return result;
}

/**
 * @brief Return the temporal disjoint/intersection relationship between the
 * temporal network point and the geometry
 */
Temporal *
tinterrel_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool tinter,
  bool restr, bool atvalue)
{
  if (gserialized_is_empty(geo))
    return NULL;
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(geo));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(tempgeom, geo, tinter, restr,
    atvalue);
  pfree(tempgeom);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the temporal contains relationship between the geometry and
 * the temporal network point
 */
Temporal *
tcontains_geo_tnpoint(GSERIALIZED *geo, Temporal *temp, bool restr,
  bool atvalue)
{
  if (gserialized_is_empty(geo))
    return NULL;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tcontains_geo_tpoint(geo, tempgeom, restr, atvalue);
  pfree(tempgeom);
  return result;
}

/**
 * @brief Return the temporal touches relationship between the temporal network
 * point and the geometry
 */
Temporal *
ttouches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo, bool restr,
  bool atvalue)
{
  if (gserialized_is_empty(geo))
    return NULL;
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(geo));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo(tempgeom, geo, restr, atvalue);
  pfree(tempgeom);
  return result;
}

/**
 * @brief Return the temporal touches relationship between the temporal network
 * point and the geometry
 */
Temporal *
ttouches_geo_tnpoint(const GSERIALIZED *geo, const Temporal *temp, bool restr,
  bool atvalue)
{
  return ttouches_tnpoint_geo(temp, geo, restr, atvalue);
}

/**
 * @brief Return the temporal touches relationship between the temporal network
 * point and the network point
 */
Temporal *
ttouches_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool restr,
  bool atvalue)
{
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *geo = npoint_geom(np);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo(tempgeom, geo, restr, atvalue);
  pfree(tempgeom);
  pfree(geo);
  return result;
}

/**
 * @brief Return the temporal touches relationship between the temporal network
 * point and the network point
 */
Temporal *
ttouches_npoint_tnpoint(const Npoint *np, const Temporal *temp, bool restr,
  bool atvalue)
{
  return ttouches_tnpoint_npoint(temp, np, restr, atvalue);
}

/**
 * @brief Return a temporal Boolean that states whether the geometry and the
 * temporal network point are within the given distance
 */
Temporal *
tdwithin_tnpoint_geo(Temporal *temp, GSERIALIZED *geo, double dist, bool restr,
  bool atvalue)
{
  if (gserialized_is_empty(geo))
    return NULL;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tempgeom, geo, dist, restr, atvalue);
  pfree(tempgeom);
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether the geometry and the
 * temporal network point are within the given distance
 */
Temporal *
tdwithin_geo_tnpoint(GSERIALIZED *geo, Temporal *temp, double dist, bool restr,
  bool atvalue)
{
  return tdwithin_tnpoint_geo(temp, geo, dist, restr, atvalue);
}

/**
 * @brief Return a temporal Boolean that states whether the network point and
 * the temporal network point are within the given distance
 */
Temporal *
tdwithin_tnpoint_npoint(Temporal *temp, Npoint *np, double dist, bool restr,
  bool atvalue)
{
  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tempgeom, geom, dist, restr, atvalue);
  pfree(geom);
  return result;
}

/**
 * @brief Return a temporal Boolean that states whether the network point and
 * the temporal network point are within the given distance
 */
Temporal *
tdwithin_npoint_tnpoint(Npoint *np, Temporal *temp, double dist, bool restr,
  bool atvalue)
{
  return tdwithin_tnpoint_npoint(temp, np, dist, restr, atvalue);
}

/**
 * @brief Return a temporal Boolean that states whether the temporal network
 * points are within the given distance
 */
Temporal *
tdwithin_tnpoint_tnpoint(Temporal *temp1, Temporal *temp2, double dist,
  bool restr, bool atvalue)
{
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  Temporal *tempgeom1 = tnpoint_tgeompoint(sync1);
  Temporal *tempgeom2 = tnpoint_tgeompoint(sync2);
  Temporal *result = tdwithin_tpoint_tpoint1(tempgeom1, tempgeom2, dist, restr,
    atvalue);
  pfree(sync1); pfree(sync2);
  pfree(tempgeom1); pfree(tempgeom2);
  return result;
}

/*****************************************************************************/

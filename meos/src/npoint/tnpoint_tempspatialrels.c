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
 * @brief Temporal spatial relationships for temporal network points
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported:
 * tcontains, tdisjoint, tintersects, ttouches, and tdwithin
 */

#include "npoint/tnpoint_tempspatialrels.h"

/* MEOS */
#include <meos.h>
#include <meos_npoint.h>
#include <meos_internal.h>
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_tempspatialrels.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the temporal disjoint/intersection relationship between a
 * temporal network point and a network point
 */
Temporal *
tinterrel_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool tinter,
  bool restr, bool atvalue)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) np) ||
      ! ensure_same_srid(tspatial_srid(temp), npoint_srid(np)))
    return NULL;

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(tpoint, gs, tinter, restr, atvalue);
  pfree(tpoint); pfree(gs);
  return result;
}

/**
 * @brief Return the temporal disjoint/intersection relationship between a
 * temporal network point and a geometry
 */
Temporal *
tinterrel_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool tinter,
  bool restr, bool atvalue)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      gserialized_is_empty(gs) ||
      ! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return NULL;

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(tpoint, gs, tinter, restr, atvalue);
  pfree(tpoint);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_spatial_rel_temp
 * @brief Return the temporal contains relationship between a geometry and
 * a temporal network point
 * @param[in] gs Geometry
 * @param[in] temp Temporal network point
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tcontains_geo_tnpoint()
 */
Temporal *
tcontains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *temp, bool restr,
  bool atvalue)
{
  if (gserialized_is_empty(gs))
    return NULL;
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *result = tcontains_geo_tpoint(gs, tpoint, restr, atvalue);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_rel_temp
 * @brief Return the temporal touches relationship between a temporal network
 * point and a geometry
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_tnpoint_geo()
 */
Temporal *
ttouches_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr,
  bool atvalue)
{
  /* Ensure validity of the arguments */
  if (gserialized_is_empty(gs) ||
      ! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return NULL;

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo(tpoint, gs, restr, atvalue);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_rel_temp
 * @brief Return the temporal touches relationship between a temporal network
 * point and a network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Ttouches_tnpoint_npoint()
 */
Temporal *
ttouches_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool restr,
  bool atvalue)
{
  /* Ensure validity of the arguments */
  if (! ensure_same_srid(tspatial_srid(temp), npoint_srid(np)))
    return NULL;

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = npoint_geom(np);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo(tpoint, gs, restr, atvalue);
  pfree(tpoint); pfree(gs);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry and a
 * temporal network point are within a distance
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tnpoint_geo()
 */
Temporal *
tdwithin_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs, double dist,
  bool restr, bool atvalue)
{
  if (gserialized_is_empty(gs))
    return NULL;
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tpoint, gs, dist, restr, atvalue);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_rel_temp
 * @brief Return a temporal Boolean that states whether a network point and
 * a temporal network point are within a distance
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tnpoint_npoint()
 */
Temporal *
tdwithin_tnpoint_npoint(const Temporal *temp, const Npoint *np, double dist,
  bool restr, bool atvalue)
{
  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tpoint, geom, dist, restr, atvalue);
  pfree(geom);
  return result;
}

/**
 * @ingroup meos_temporal_spatial_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal network
 * points are within a distance
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] dist Distance
 * @param[in] restr True when the result is restricted to a value
 * @param[in] atvalue Value to restrict
 * @csqlfn #Tdwithin_tnpoint_tnpoint()
 */
Temporal *
tdwithin_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  double dist, bool restr, bool atvalue)
{
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  Temporal *tpoint1 = tnpoint_tgeompoint(sync1);
  Temporal *tpoint2 = tnpoint_tgeompoint(sync2);
  Temporal *result = tdwithin_tpoint_tpoint_sync(tpoint1, tpoint2, dist, restr,
    atvalue);
  pfree(sync1); pfree(sync2); pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************/

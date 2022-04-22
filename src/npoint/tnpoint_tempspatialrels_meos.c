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
 * @file tnpoint_tempspatialrels.c
 * @brief Temporal spatial relationships for temporal network points.
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported:
 * tcontains, tdisjoint, tintersects, ttouches, and tdwithin
 */

#include "npoint/tnpoint_tempspatialrels.h"

/* MobilityDB */
#include "general/temporaltypes.h"
#include "point/tpoint_tempspatialrels.h"

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

/**
 * Return the temporal intersects relationship between the geometry and the
 * temporal network point
 */
Temporal *
tdisjoint_geo_tnpoint(const GSERIALIZED *geo, const Temporal *temp,
  bool restr, Datum atvalue)
{
  return tinterrel_tnpoint_geo(temp, geo, TDISJOINT, restr, atvalue);
}

/**
 * Return the temporal intersects relationship between the network point and
 * the temporal network point
 */
Temporal *
tdisjoint_npoint_tnpoint(const Npoint *np, const Temporal *temp, bool restr,
  Datum atvalue)
{
  return tinterrel_tnpoint_npoint(temp, np, TDISJOINT, restr, atvalue);
}

/**
 * Return the temporal intersects relationship between the temporal network
 * point and the geometry
 */
Temporal *
tdisjoint_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo,
  bool restr, Datum atvalue)
{
  return tinterrel_tnpoint_geo(temp, geo, TDISJOINT, restr, atvalue);
}

/**
 * Return the temporal intersects relationship between the temporal network
 * point and the network point
 */
Temporal *
tdisjoint_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool restr,
  Datum atvalue)
{
  return tinterrel_tnpoint_npoint(temp, np, TDISJOINT, restr, atvalue);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

/**
 * Return the temporal intersects relationship between the geometry and the
 * temporal network point
 */
Temporal *
tintersects_geo_tnpoint(const GSERIALIZED *geo, const Temporal *temp,
  bool restr, Datum atvalue)
{
  return tinterrel_tnpoint_geo(temp, geo, TINTERSECTS, restr, atvalue);
}

/**
 * Return the temporal intersects relationship between the network point and
 * the temporal network point
 */
Temporal *
tintersects_npoint_tnpoint(const Npoint *np, const Temporal *temp, bool restr,
  Datum atvalue)
{
  return tinterrel_tnpoint_npoint(temp, np, TINTERSECTS, restr, atvalue);
}

/**
 * Return the temporal intersects relationship between the temporal network
 * point and the geometry
 */
Temporal *
tintersects_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo,
  bool restr, Datum atvalue)
{
  return tinterrel_tnpoint_geo(temp, geo, TINTERSECTS, restr, atvalue);
}

/**
 * Return the temporal intersects relationship between the temporal network
 * point and the network point
 */
Temporal *
tintersects_tnpoint_npoint(const Temporal *temp, const Npoint *np, bool restr,
  Datum atvalue)
{
  return tinterrel_tnpoint_npoint(temp, np, TINTERSECTS, restr, atvalue);
}

/*****************************************************************************/

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
 * @brief Temporal spatial relationships for temporal points.
 *
 * These relationships are applied at each instant and result in a temporal
 * Boolean.
 *
 * The following relationships are supported for a temporal geometry point
 * and a geometry: `tcontains`, `tdisjoint`, `tintersects`, `ttouches`, and
 * `tdwithin`.
 *
 * The following relationships are supported for two temporal geometry points:
 * `tdwithin`.
 *
 * The following relationships are supported for two temporal geography points:
 * `tdisjoint`, `tintersects`, `tdwithin`.
 *
 * tintersects and tdisjoint for a temporal point and a geometry allow a fast
 * implementation by (1) using bounding box tests, and (2) splitting temporal
 * sequence points into an array of simple (that is, not self-intersecting)
 * fragments and the answer is computed for each fragment without any
 * additional call to PostGIS.
 *
 * The implementation of tcontains and ttouches involving a temporal point
 * and a geometry is derived from the above by computing the boundary of the
 * geometry and
 * (1) tcontains(geo, tpoint) = tintersects(geo, tpoint) &
 *     ~ tintersects(st_boundary(geo), tpoint)
 *     where & and ~ are the temporal boolean operators and and not
 * (2) ttouches(geo, tpoint) = tintersects(st_boundary(geo), tpoint)
 *
 * Notice also that twithin has a custom implementation as follows
 * - In the case of a temporal point and a geometry we (1) call PostGIS to
 *   compute a buffer of the geometry and the distance parameter d, and
 *   (2) compute the result from tpointseq_at_geometry(seq, geo_buffer)
 * - In the case of two temporal points we need to compute the instants
 *   at which two temporal sequences have a distance d between each other,
 *   which amounts to solve the equation distance(seg1(t), seg2(t)) = d.
 */

#include "point/tpoint_tempspatialrels.h"

/* MobilityDB */
#include <meos.h>

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return the temporal intersects relationship between a temporal point
 * and a geometry
 * @sqlfunc tdisjoint()
 */
Temporal *
tdisjoint_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo,
  bool restr, bool atvalue)
{
  return tinterrel_tpoint_geo(temp, geo, TDISJOINT, restr, atvalue);
}

/*****************************************************************************
 * Temporal intersects
 * Available for temporal geography points
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return the temporal intersects relationship between a temporal point
 * and a geometry
 * @sqlfunc tintersects()
 */
Temporal *
tintersects_tpoint_geo(const Temporal *temp, const GSERIALIZED *geo,
  bool restr, bool atvalue)
{
  return tinterrel_tpoint_geo(temp, geo, TINTERSECTS, restr, atvalue);
}

/*****************************************************************************/

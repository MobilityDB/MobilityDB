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
 * @brief MEOS wrappers for temporal spatial relationships for temporal points.
 */

#include "point/tpoint_tempspatialrels.h"

/* MEOS */
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

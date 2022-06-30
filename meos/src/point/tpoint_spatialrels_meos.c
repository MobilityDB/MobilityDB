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
 * @brief Ever spatial relationships for temporal points.
 *
 * These relationships compute the ever spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported for geometries: `contains`,
 * `disjoint`, `intersects`, `touches`, and `dwithin`.
 *
 * The following relationships are supported for geographies: `disjoint`,
 * `intersects`, `dwithin`.
 *
 * Only `disjoint`, `dwithin`, and `intersects` are supported for 3D geometries.
 */

#include "point/tpoint_spatialrels.h"

/* C */
#include <assert.h>
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/temporal_catalog.h"
#include "general/temporal_util.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_distance.h"
#include "point/tpoint_tempspatialrels.h"

/*****************************************************************************
 * Ever disjoint
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if the temporal points are ever disjoint, 0 if not, and
 * -1 if the temporal points do not intersect in time
 * @sqlfunc disjoint()
 */
int
disjoint_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  int result = spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_ne);
  return result;
}

/*****************************************************************************
 * Ever intersects (for both geometry and geography)
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_rel
 * @brief Return 1 if the temporal points ever intersect, 0 if not, and
 * -1 if the temporal points do not intersect in time
 * @sqlfunc intersects()
 */
int
intersects_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return spatialrel_tpoint_tpoint(temp1, temp2, &datum2_point_eq);
}

/*****************************************************************************/

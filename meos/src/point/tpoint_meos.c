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
 * @brief General functions for temporal points.
 */

#include "point/tpoint.h"

/* PostgreSQL */
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/temporal_catalog.h"
#include "general/temporal_util.h"
#include "general/lifting.h"
#include "general/temporal_compops.h"
#include "point/stbox.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a point and a temporal point
 * @sqlop @p #=
 */
Temporal *
teq_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return tcomp_tpoint_point(tpoint, geo, &datum2_eq2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal point and a point
 * @sqlop @p #=
 */
Temporal *
teq_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return tcomp_tpoint_point(tpoint, geo, &datum2_eq2, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal difference of a point and a temporal point
 * @sqlop @p #<>
 */
Temporal *
tne_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return tcomp_tpoint_point(tpoint, geo, &datum2_ne2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal difference of the temporal point and a point
 * @sqlop @p #<>
 */
Temporal *
tne_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return tcomp_tpoint_point(tpoint, geo, &datum2_ne2, INVERT_NO);
}

/*****************************************************************************/

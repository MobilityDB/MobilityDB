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
 * @brief General functions for temporal points.
 */

#include "point/tpoint.h"

/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/lifting.h"
#include "general/temporal_compops.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Copy a GSERIALIZED. This function is not available anymore in PostGIS 3
 */
GSERIALIZED *
gserialized_copy(const GSERIALIZED *g)
{
  GSERIALIZED *result = palloc(VARSIZE(g));
  memcpy(result, g, VARSIZE(g));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_box_cast
 * @brief Return the bounding box of a temporal point
 * @sqlfunc stbox()
 * @sqlop @p ::
 */
STBox *
tpoint_to_stbox(const Temporal *temp)
{
  STBox *result = palloc(sizeof(STBox));
  temporal_set_bbox(temp, result);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Return the bounding box of a temporal point expanded on the
 * spatial dimension
 * @sqlfunc expandSpace()
 */
STBox *
geo_expand_space(const GSERIALIZED *gs, double d)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBox box;
  geo_set_stbox(gs, &box);
  STBox *result = stbox_expand_space(&box, d);
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Return the bounding box of a temporal point expanded on the
 * spatial dimension
 * @sqlfunc expandSpace()
 */
STBox *
tpoint_expand_space(const Temporal *temp, double d)
{
  STBox box;
  temporal_set_bbox(temp, &box);
  STBox *result = stbox_expand_space(&box, d);
  return result;
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a temporal point and a point
 */
Temporal *
tcomp_tpoint_point(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType, meosType), bool invert)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  meosType basetype = temptype_basetype(temp->temptype);
  Temporal *result = tcomp_temporal_base(temp, PointerGetDatum(gs), basetype,
    func, invert);
  return result;
}

/*****************************************************************************/

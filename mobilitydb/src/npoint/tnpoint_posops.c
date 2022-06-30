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
 * @brief Relative position operators for temporal network points.
 *
 * The following operators are defined for the spatial dimension:
 * - left, overleft, right, overright, below, overbelow, above, overabove,
 *   front, overfront, back, overback
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 * The following operators for the temporal dimension:
 * - before, overbefore, after, overafter
 * for both temporal geometry and geography points are "inherited" from the
 * basic temporal types. In this file they are defined when one of the
 * arguments is a stbox.
 */

/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_npoint/tnpoint_boxops.h"

/*****************************************************************************/
/* geo op Temporal */

PG_FUNCTION_INFO_V1(Left_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry is strictly to the left of the temporal network point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry does not extend to the right of the temporal network point
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry is strictly to the right of the temporal network point
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry does not extend to the left of the temporal network point
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry is strictly below the temporal network point
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
PGDLLEXPORT Datum
Below_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry does not extend above the temporal network point
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
PGDLLEXPORT Datum
Overbelow_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry is strictly above the temporal network point
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
PGDLLEXPORT Datum
Above_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_geom_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the geometry does not extend below the temporal network point
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
PGDLLEXPORT Datum
Overabove_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tnpoint_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op geo */

PG_FUNCTION_INFO_V1(Left_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly to the left of
 * the geometry
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend to the
 * right of the geometry
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly to the right
 * of the geometry
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend to the left
 * of the geometry
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly below the geometry
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
PGDLLEXPORT Datum
Below_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend above the geometry
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly above the geometry
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
PGDLLEXPORT Datum
Above_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_geom);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend below the geometry
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
PGDLLEXPORT Datum
Overabove_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_geo_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(Left_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly to the left of the
 * temporal network point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &left_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overleft_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend to the right of the
 * temporal network point
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &overleft_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Right_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly to the right of the
 * temporal network point
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &right_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overright_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend to the left of the
 * temporal network point
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &overright_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Below_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly below the temporal
 * network point
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
PGDLLEXPORT Datum
Below_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &below_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overbelow_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend above the temporal
 * network point
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
PGDLLEXPORT Datum
Overbelow_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &overbelow_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Above_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly above the temporal
 * network point
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
PGDLLEXPORT Datum
Above_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &above_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overabove_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend below the temporal
 * network point
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
PGDLLEXPORT Datum
Overabove_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &overabove_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Before_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly before of the temporal
 * network point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Before_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &before_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overbefore_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend after the temporal
 * network point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Overbefore_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &overbefore_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(After_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly after the temporal
 * network point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
After_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &after_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overafter_stbox_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend before the temporal
 * network point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Overafter_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tnpoint_ext(fcinfo, &overafter_stbox_stbox, false);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(Left_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly to the left of the
 * spatiotemporal box
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &left_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend to the right of
 * the spatiotemporal box
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &overleft_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly to the right of the
 * spatiotemporal box
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &right_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend to the left of
 * the spatiotemporal box
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &overright_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly below the
 * spatiotemporal box
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
PGDLLEXPORT Datum
Below_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &below_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend above the
 * spatiotemporal box
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &overbelow_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly above the
 * spatiotemporal box
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
PGDLLEXPORT Datum
Above_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &above_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend below the
 * spatiotemporal box
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
PGDLLEXPORT Datum
Overabove_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &overabove_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Before_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly before the
 * spatiotemporal box
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Before_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &before_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overbefore_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend after the
 * spatiotemporal box
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Overbefore_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &overbefore_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(After_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point is strictly after the
 * spatiotemporal box
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
After_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &after_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overafter_tnpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal network point does not extend before the
 * spatiotemporal box
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Overafter_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_stbox_ext(fcinfo, &overafter_stbox_stbox, false);
}

/*****************************************************************************/
/* Npoint op Temporal */

PG_FUNCTION_INFO_V1(Left_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point is strictly to the left of the
 * temporal point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point does not extend to the right of the
 * temporal point
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point is strictly to the right of the
 * temporal point
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point does not extend to the left of the
 * temporal point
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point is strictly below the temporal point
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
PGDLLEXPORT Datum
Below_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point does not extend above the temporal point
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
PGDLLEXPORT Datum
Overbelow_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point is strictly above the temporal point
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
PGDLLEXPORT Datum
Above_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the network point does not extend below the temporal point
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
PGDLLEXPORT Datum
Overabove_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_npoint_tnpoint_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Npoint */

PG_FUNCTION_INFO_V1(Left_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point is strictly to the left of the
 * network point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point does not extend to the right of the
 * network point
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point is strictly to the right of the
 * network point
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point does not extend to the left of the
 * network point
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point is strictly below the network point
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
PGDLLEXPORT Datum
Below_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point does not extend above the network point
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point is strictly above the network point
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
PGDLLEXPORT Datum
Above_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal point does not extend below the network point
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
PGDLLEXPORT Datum
Overabove_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_npoint_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(Left_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point is strictly to the left of
 * the second one
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point does not extend to the right
 * of the second one
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point is strictly to the right of
 * the second one
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point does not extend to the left
 * of the second one
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point is strictly below the
 * second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
PGDLLEXPORT Datum
Below_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point does not extend above the
 * second one
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point is strictly above the
 * second one
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
PGDLLEXPORT Datum
Above_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point does not extend below the
 * second one
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
PGDLLEXPORT Datum
Overabove_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point is strictly before the
 * second one
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Before_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point does not extend after the
 * second one
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Overbefore_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point is strictly after the
 * second one
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
After_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal network point does not extend before the
 * second one
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Overafter_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return boxop_tnpoint_tnpoint_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/

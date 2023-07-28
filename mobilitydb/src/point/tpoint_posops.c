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
 * @brief Relative position operators for temporal geometry points.
 *
 * The following operators are defined for the spatial dimension:
 * `left`, `overleft`, `right`, `overright`, `below`, `overbelow`, `above`,
 * `overabove`, `front`, `overfront`, `back`, `overback`.
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 *
 * The following operators are defined for for the time dimension: `before`,
 * `overbefore`,  `after`, `overafter`. For both temporal geometry and
 * geography points the same operators are derived from the basic temporal
 * types. In this file they are defined when one of the arguments is an
 * `STBox`.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_point/tpoint_boxops.h"

/*****************************************************************************/
/* stbox op Temporal */

PGDLLEXPORT Datum Left_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly to the left of a temporal point
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend to the right of a temporal point
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly to the right of a temporal point
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend to the left of a temporal point
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly below a temporal point
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend above a temporal point
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
Datum
Overbelow_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly above a temporal point
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
Datum
Above_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend below a temporal point
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
Datum
Overabove_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Front_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly in front of a temporal point
 * @sqlfunc temporal_front()
 * @sqlop @p <</
 */
Datum
Front_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &front_stbox_stbox);
}

PGDLLEXPORT Datum Overfront_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend to the back of a temporal point
 * @sqlfunc temporal_overfront()
 * @sqlop @p &</
 */
Datum
Overfront_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overfront_stbox_stbox);
}

PGDLLEXPORT Datum Back_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly back of a temporal point
 * @sqlfunc temporal_back()
 * @sqlop @p />>
 */
Datum
Back_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &back_stbox_stbox);
}

PGDLLEXPORT Datum Overback_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend to the front of a temporal point
 * @sqlfunc temporal_overback()
 * @sqlop @p /&>
 */
Datum
Overback_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overback_stbox_stbox);
}

PGDLLEXPORT Datum Before_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly before a temporal point
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend after a temporal point
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box is strictly after a temporal point
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a spatiotemporal box does not extend before a temporal point
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op stbox */

PGDLLEXPORT Datum Left_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly to the left of a spatiotemporal box
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend to the right of a spatiotemporal box
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly to the right of a spatiotemporal box
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend to the left of a spatiotemporal box
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly below a spatiotemporal box
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend above a spatiotemporal box
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
Datum
Overbelow_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly above a spatiotemporal box
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
Datum
Above_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend below a spatiotemporal box
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
Datum
Overabove_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Front_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly in front of a spatiotemporal box
 * @sqlfunc temporal_front()
 * @sqlop @p <</
 */
Datum
Front_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &front_stbox_stbox);
}

PGDLLEXPORT Datum Overfront_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend to the back of a spatiotemporal box
 * @sqlfunc temporal_overfront()
 * @sqlop @p &</
 */
Datum
Overfront_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overfront_stbox_stbox);
}

PGDLLEXPORT Datum Back_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly back of a spatiotemporal box
 * @sqlfunc temporal_back()
 * @sqlop @p />>
 */
Datum
Back_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &back_stbox_stbox);
}

PGDLLEXPORT Datum Overback_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend to the front of a spatiotemporal box
 * @sqlfunc temporal_overback()
 * @sqlop @ /&>
 */
Datum
Overback_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overback_stbox_stbox);
}

PGDLLEXPORT Datum Before_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly before a spatiotemporal box
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend after a spatiotemporal box
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point is strictly after a spatiotemporal box
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal point does not extend before a spatiotemporal box
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Temporal */

PGDLLEXPORT Datum Left_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly to the left of the second one
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend to the right of the second one
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly to the right of the second one
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend to the left of the second one
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly below the second one
 * @sqlfunc temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend above the second one
 * @sqlfunc temporal_overbelow()
 * @sqlop @p &<|
 */
Datum
Overbelow_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly above the second one
 * @sqlfunc temporal_above()
 * @sqlop @p |>>
 */
Datum
Above_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend below the second one
 * @sqlfunc temporal_overabove()
 * @sqlop @p |&>
 */
Datum
Overabove_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Front_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly in front of the second one
 * @sqlfunc temporal_front()
 * @sqlop @p <</
 */
Datum
Front_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &front_stbox_stbox);
}

PGDLLEXPORT Datum Overfront_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend to the back of the second one
 * @sqlfunc temporal_overfront()
 * @sqlop @p &</
 */
Datum
Overfront_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overfront_stbox_stbox);
}

PGDLLEXPORT Datum Back_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly back of the second one
 * @sqlfunc temporal_back()
 * @sqlop @p />>
 */
Datum
Back_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &back_stbox_stbox);
}

PGDLLEXPORT Datum Overback_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend to the front of the second one
 * @sqlfunc temporal_overback()
 * @sqlop @p /&>
 */
Datum
Overback_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overback_stbox_stbox);
}

PGDLLEXPORT Datum Before_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly before the second one
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend after the second one
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point is strictly after the second one
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal point does not extend before the second one
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/

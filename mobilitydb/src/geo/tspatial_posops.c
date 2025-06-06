/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Position operators for spatiotemporal geometries
 * @details The following operators are defined for the spatial dimension:
 * `left`, `overleft`, `right`, `overright`, `below`, `overbelow`, `above`,
 * `overabove`, `front`, `overfront`, `back`, `overback`.
 * There are no equivalent operators for the temporal geographies since
 * PostGIS does not currently provide such functionality for geography.
 *
 * The following operators are defined for for the time dimension: `before`,
 * `overbefore`,  `after`, `overafter`. For both temporal geometry and
 * geographies the same operators are derived from the basic temporal
 * types. In this file they are defined when one of the arguments is an
 * `STBox`.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
/* MobilityDB */
#include "pg_geo/tspatial_boxops.h"

/*****************************************************************************/
/* stbox op Temporal */

PGDLLEXPORT Datum Left_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is to the left of a temporal
 * spatial value
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
inline Datum
Left_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the right of a
 * spatiotemporal value
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
inline Datum
Overleft_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is to the right of a temporal
 * spatial value
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
inline Datum
Right_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the left of a
 * spatiotemporal value
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
inline Datum
Overright_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is below a spatiotemporal value
 * @sqlfn temporal_below()
 * @sqlop @p <<|
 */
inline Datum
Below_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend above a temporal
 * spatial value
 * @sqlfn temporal_overbelow()
 * @sqlop @p &<|
 */
inline Datum
Overbelow_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is above a spatiotemporal value
 * @sqlfn temporal_above()
 * @sqlop @p |>>
 */
inline Datum
Above_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend below a temporal
 * spatial value
 * @sqlfn temporal_overabove()
 * @sqlop @p |&>
 */
inline Datum
Overabove_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Front_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is in front of a spatiotemporal
 * value
 * @sqlfn temporal_front()
 * @sqlop @p <</
 */
inline Datum
Front_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &front_stbox_stbox);
}

PGDLLEXPORT Datum Overfront_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the back of a
 * spatiotemporal value
 * @sqlfn temporal_overfront()
 * @sqlop @p &</
 */
inline Datum
Overfront_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overfront_stbox_stbox);
}

PGDLLEXPORT Datum Back_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is at the back of a temporal
 * spatial value
 * @sqlfn temporal_back()
 * @sqlop @p />>
 */
inline Datum
Back_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &back_stbox_stbox);
}

PGDLLEXPORT Datum Overback_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the front of a
 * spatiotemporal value
 * @sqlfn temporal_overback()
 * @sqlop @p /&>
 */
inline Datum
Overback_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overback_stbox_stbox);
}

PGDLLEXPORT Datum Before_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is before a spatiotemporal
 * value
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
inline Datum
Before_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is not after a spatiotemporal
 * value
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
inline Datum
Overbefore_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is after a spatiotemporal value
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
inline Datum
After_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_stbox_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_stbox_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is not before a spatiotemporal
 * value
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
inline Datum
Overafter_stbox_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tspatial(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op stbox */

PGDLLEXPORT Datum Left_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is to the left of a
 * spatiotemporal box
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
inline Datum
Left_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the right of
 * a spatiotemporal box
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
inline Datum
Overleft_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is to the right of a
 * spatiotemporal box
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
inline Datum
Right_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the left
 * of a spatiotemporal box
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
inline Datum
Overright_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is below a spatiotemporal box
 * @sqlfn temporal_below()
 * @sqlop @p <<|
 */
inline Datum
Below_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend above a
 * spatiotemporal box
 * @sqlfn temporal_overbelow()
 * @sqlop @p &<|
 */
inline Datum
Overbelow_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is above a spatiotemporal box
 * @sqlfn temporal_above()
 * @sqlop @p |>>
 */
inline Datum
Above_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend below a
 * spatiotemporal box
 * @sqlfn temporal_overabove()
 * @sqlop @p |&>
 */
inline Datum
Overabove_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Front_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is in front of a
 * spatiotemporal box
 * @sqlfn temporal_front()
 * @sqlop @p <</
 */
inline Datum
Front_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &front_stbox_stbox);
}

PGDLLEXPORT Datum Overfront_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the back
 * of a spatiotemporal box
 * @sqlfn temporal_overfront()
 * @sqlop @p &</
 */
inline Datum
Overfront_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overfront_stbox_stbox);
}

PGDLLEXPORT Datum Back_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is at the back of a
 * spatiotemporal box
 * @sqlfn temporal_back()
 * @sqlop @p />>
 */
inline Datum
Back_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &back_stbox_stbox);
}

PGDLLEXPORT Datum Overback_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the front
 * of a spatiotemporal box
 * @sqlfn temporal_overback()
 * @sqlop @ /&>
 */
inline Datum
Overback_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overback_stbox_stbox);
}

PGDLLEXPORT Datum Before_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is before a spatiotemporal
 * box
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
inline Datum
Before_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is not after a spatiotemporal
 * box
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
inline Datum
Overbefore_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is after a spatiotemporal box
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
inline Datum
After_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_tspatial_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tspatial_stbox);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is not before a spatiotemporal
 * box
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
inline Datum
Overafter_tspatial_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_stbox(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Temporal */

PGDLLEXPORT Datum Left_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is to the left of the
 * second one
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
inline Datum
Left_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the right of the second one
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
inline Datum
Overleft_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is to the right of
 * the second one
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
inline Datum
Right_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the left of the second one
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
inline Datum
Overright_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is below the second
 * one
 * @sqlfn temporal_below()
 * @sqlop @p <<|
 */
inline Datum
Below_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend above
 * the second one
 * @sqlfn temporal_overbelow()
 * @sqlop @p &<|
 */
inline Datum
Overbelow_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is above the second
 * one
 * @sqlfn temporal_above()
 * @sqlop @p |>>
 */
inline Datum
Above_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend below
 * the second one
 * @sqlfn temporal_overabove()
 * @sqlop @p |&>
 */
inline Datum
Overabove_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Front_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is in front of the
 * second one
 * @sqlfn temporal_front()
 * @sqlop @p <</
 */
inline Datum
Front_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &front_stbox_stbox);
}

PGDLLEXPORT Datum Overfront_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the back of the second one
 * @sqlfn temporal_overfront()
 * @sqlop @p &</
 */
inline Datum
Overfront_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overfront_stbox_stbox);
}

PGDLLEXPORT Datum Back_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is at the back of the
 * second one
 * @sqlfn temporal_back()
 * @sqlop @p />>
 */
inline Datum
Back_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &back_stbox_stbox);
}

PGDLLEXPORT Datum Overback_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the front of the second one
 * @sqlfn temporal_overback()
 * @sqlop @p /&>
 */
inline Datum
Overback_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overback_stbox_stbox);
}

PGDLLEXPORT Datum Before_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is before the second
 * one
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
inline Datum
Before_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is not after the
 * second one
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
inline Datum
Overbefore_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is after the second
 * one
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
inline Datum
After_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_tspatial_tspatial(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tspatial_tspatial);
/**
 * @ingroup mobilitydb_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is not before the
 * second one
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
inline Datum
Overafter_tspatial_tspatial(PG_FUNCTION_ARGS)
{
  return Boxop_tspatial_tspatial(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/

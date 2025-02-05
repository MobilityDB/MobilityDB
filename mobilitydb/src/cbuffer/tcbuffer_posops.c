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
 * @brief Position operators for temporal circular buffers.
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

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_cbuffer/tcbuffer_boxops.h"

/*****************************************************************************/
/* stbox op Temporal */

PGDLLEXPORT Datum Left_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is to the left of a temporal
 * circular buffer
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the right of
 * a temporal circular buffer
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is to the right of a temporal
 * circular buffer
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the left of
 * a temporal circular buffer
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is below a temporal circular buffer
 * @sqlfn temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend above a temporal
 * circular buffer
 * @sqlfn temporal_overbelow()
 * @sqlop @p &<|
 */
Datum
Overbelow_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is above a temporal circular buffer
 * @sqlfn temporal_above()
 * @sqlop @p |>>
 */
Datum
Above_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend below a temporal
 * circular buffer
 * @sqlfn temporal_overabove()
 * @sqlop @p |&>
 */
Datum
Overabove_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Before_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is before of a temporal circular
 * buffer
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Before_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is not after a temporal circular
 * buffer
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Overbefore_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is after a temporal circular buffer
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
After_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_stbox_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_stbox_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is not before a temporal circular
 * buffer
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Overafter_stbox_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_stbox_tcbuffer(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op stbox */

PGDLLEXPORT Datum Left_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is to the left of a
 * spatiotemporal box
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer does not extend to the
 * right of a spatiotemporal box
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is to the right of a
 * spatiotemporal box
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer does not extend to the left
 * of a spatiotemporal box
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is below a spatiotemporal box
 * @sqlfn temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer does not extend above a
 * spatiotemporal box
 * @sqlfn temporal_overbelow()
 * @sqlop @p &<|
 */
Datum
Overbelow_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is above a spatiotemporal box
 * @sqlfn temporal_above()
 * @sqlop @p |>>
 */
Datum
Above_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer does not extend below a
 * spatiotemporal box
 * @sqlfn temporal_overabove()
 * @sqlop @p |&>
 */
Datum
Overabove_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Before_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is before a spatiotemporal
 * box
 * @sqlfn temporal_left()
 * @sqlop @p <<#
 */
Datum
Before_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is not after a spatiotemporal
 * box
 * @sqlfn temporal_left()
 * @sqlop @p <&#
 */
Datum
Overbefore_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is after a spatiotemporal box
 * @sqlfn temporal_left()
 * @sqlop @p >>#
 */
Datum
After_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_tcbuffer_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tcbuffer_stbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal circular buffer is not before a
 * spatiotemporal box
 * @sqlfn temporal_left()
 * @sqlop @p #&>
 */
Datum
Overafter_tcbuffer_stbox(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_stbox(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Temporal */

PGDLLEXPORT Datum Left_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is to the left of
 * the second one
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &left_stbox_stbox);
}

PGDLLEXPORT Datum Overleft_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer does not extend to
 * the right of the second one
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &overleft_stbox_stbox);
}

PGDLLEXPORT Datum Right_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is to the right of
 * the second one
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &right_stbox_stbox);
}

PGDLLEXPORT Datum Overright_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer does not extend to
 * the left of the second one
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &overright_stbox_stbox);
}

PGDLLEXPORT Datum Below_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is below the second
 * one
 * @sqlfn temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &below_stbox_stbox);
}

PGDLLEXPORT Datum Overbelow_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer does not extend above
 * the second one
 * @sqlfn temporal_overbelow()
 * @sqlop @p &<|
 */
Datum
Overbelow_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &overbelow_stbox_stbox);
}

PGDLLEXPORT Datum Above_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is above the second
 * one
 * @sqlfn temporal_above()
 * @sqlop @p |>>
 */
Datum
Above_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &above_stbox_stbox);
}

PGDLLEXPORT Datum Overabove_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer does not extend below
 * the second one
 * @sqlfn temporal_overabove()
 * @sqlop @p |&>
 */
Datum
Overabove_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &overabove_stbox_stbox);
}

PGDLLEXPORT Datum Before_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is before the
 * second one
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Before_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &before_stbox_stbox);
}

PGDLLEXPORT Datum Overbefore_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is not after the
 * second one
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Overbefore_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &overbefore_stbox_stbox);
}

PGDLLEXPORT Datum After_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is after the second
 * one
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
After_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &after_stbox_stbox);
}

PGDLLEXPORT Datum Overafter_tcbuffer_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tcbuffer_tcbuffer);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal circular buffer is not before the
 * second one
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Overafter_tcbuffer_tcbuffer(PG_FUNCTION_ARGS)
{
  return Boxop_tcbuffer_tcbuffer(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/

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
 * @brief Position operators for temporal types
 *
 * The following operators are defined:
 * - `left`, `overleft`, `right`, `overright` for the value dimension
 * - `before`, `overbefore`, `after`, `overafter`for the time dimension
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_general/temporal_boxops.h"

/*****************************************************************************/
/* <Time> op Temporal */

PGDLLEXPORT Datum Before_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a timestamptz span is before a temporal value
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overbefore_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a timestamptz span is not after a temporal value
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum After_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a timestamptz span is after a temporal value
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overafter_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a timestamptz span is not before a temporal value
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Temporal op Period */

PGDLLEXPORT Datum Before_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal value is before a timestamptz span
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overbefore_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal value is not after a timestamptz span
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum After_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal value is after a timestamptz span
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
Datum
After_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overafter_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal value is not before a timestamptz span
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Temporal op Temporal */

PGDLLEXPORT Datum Before_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal value is before the second one
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overbefore_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal value is not after the second one
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum After_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal value is after the second one
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
Datum
After_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overafter_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal value is not before the second one
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Span op Tnumber */

PGDLLEXPORT Datum Left_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a number span is to the left of a temporal number
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overleft_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a number span does not extend to the right of a
 * temporal number
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum Right_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a number span is to the right of a temporal number
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overright_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a number span does not extend to the left of a temporal number
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Tnumber op Span */

PGDLLEXPORT Datum Left_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is to the left of a number span
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overleft_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the right of a number span
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum Right_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is to the right of a number span
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overright_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the left of a
 * number span
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* TBox op Temporal */

PGDLLEXPORT Datum Left_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box is to the left of a temporal number
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &left_tbox_tbox);
}

PGDLLEXPORT Datum Overleft_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box does not extend to the right of a temporal number
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &overleft_tbox_tbox);
}

PGDLLEXPORT Datum Right_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box is to the right of a temporal number
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &right_tbox_tbox);
}

PGDLLEXPORT Datum Overright_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box does not extend to the left of a temporal number
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &overright_tbox_tbox);
}

PGDLLEXPORT Datum Before_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box is before a temporal number
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &before_tbox_tbox);
}

PGDLLEXPORT Datum Overbefore_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box is not after a temporal number
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &overbefore_tbox_tbox);
}

PGDLLEXPORT Datum After_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box is after a temporal number
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &after_tbox_tbox);
}

PGDLLEXPORT Datum Overafter_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal box is not before a temporal number
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/
/* Temporal op TBox */

PGDLLEXPORT Datum Left_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is to the left of a temporal box
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &left_tbox_tbox);
}

PGDLLEXPORT Datum Overleft_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the right of a temporal box
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &overleft_tbox_tbox);
}

PGDLLEXPORT Datum Right_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is to the right of a temporal box
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &right_tbox_tbox);
}

PGDLLEXPORT Datum Overright_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the left of a temporal box
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &overright_tbox_tbox);
}

PGDLLEXPORT Datum Before_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is before a temporal box
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &before_tbox_tbox);
}

PGDLLEXPORT Datum Overbefore_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is not after a temporal box
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &overbefore_tbox_tbox);
}

PGDLLEXPORT Datum After_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is after a temporal box
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &after_tbox_tbox);
}

PGDLLEXPORT Datum Overafter_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if a temporal number is not before a temporal box
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/
/* Tnumber op Tnumber */

PGDLLEXPORT Datum Left_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number is to the left of the
 * second one
 * @sqlfn temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &left_tbox_tbox);
}

PGDLLEXPORT Datum Overleft_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number does not extend to the right of the
 * second one
 * @sqlfn temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &overleft_tbox_tbox);
}

PGDLLEXPORT Datum Right_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number is to the right of the
 * second one
 * @sqlfn temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &right_tbox_tbox);
}

PGDLLEXPORT Datum Overright_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number does not extend to the left of the
 * second one
 * @sqlfn temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &overright_tbox_tbox);
}

PGDLLEXPORT Datum Before_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number is before the second one
 * @sqlfn temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &before_tbox_tbox);
}

PGDLLEXPORT Datum Overbefore_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number is not after the second one
 * @sqlfn temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &overbefore_tbox_tbox);
}

PGDLLEXPORT Datum After_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number is after the second one
 * @sqlfn temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &after_tbox_tbox);
}

PGDLLEXPORT Datum Overafter_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_pos
 * @brief Return true if the first temporal number is not before the second one
 * @sqlfn temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/

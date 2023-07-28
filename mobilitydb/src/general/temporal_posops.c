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
 * @brief Relative position operators for temporal types.
 *
 * The following operators are defined:
 * - `left`, `overleft`, `right`, `overright` for the value dimension
 * - `before`, `overbefore`, `after`, `overafter`for the time dimension
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/palloc.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_general/temporal_boxops.h"

/*****************************************************************************/
/* <Time> op Temporal */

PGDLLEXPORT Datum Before_period_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_period_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the period value is strictly before the temporal value
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overbefore_period_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_period_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the period value is not after the temporal value
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum After_period_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_period_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the period value is strictly after the temporal value
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overafter_period_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_period_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the period value is not before the temporal value
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Temporal op Period */

PGDLLEXPORT Datum Before_temporal_period(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_temporal_period);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal value is strictly before the period value
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overbefore_temporal_period(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_temporal_period);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal value is not after the period value
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum After_temporal_period(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_temporal_period);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal value is strictly after the period value
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overafter_temporal_period(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_temporal_period);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the temporal value is not before the period value
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Temporal op Temporal */

PGDLLEXPORT Datum Before_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal value is strictly before the second one
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overbefore_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal value is not after the second one
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum After_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal value is strictly after the second one
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overafter_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal value is not before the second one
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Span op Tnumber */

PGDLLEXPORT Datum Left_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a number span is strictly to the left of a temporal number
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_numspan_tnumber_ext(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overleft_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a number span is not to the right of a temporal number
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_numspan_tnumber_ext(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum Right_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a number span is strictly to the right of a temporal number
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_numspan_tnumber_ext(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overright_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a number span is not to the left of a temporal number
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_numspan_tnumber_ext(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* Tnumber op Span */

PGDLLEXPORT Datum Left_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is strictly to the left of a number span
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_numspan_ext(fcinfo, &left_span_span);
}

PGDLLEXPORT Datum Overleft_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is not to the right of a number span
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_numspan_ext(fcinfo, &overleft_span_span);
}

PGDLLEXPORT Datum Right_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is strictly to the right of a number span
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_numspan_ext(fcinfo, &right_span_span);
}

PGDLLEXPORT Datum Overright_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is not to the left of a number span
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_numspan_ext(fcinfo, &overright_span_span);
}

/*****************************************************************************/
/* TBox op Temporal */

PGDLLEXPORT Datum Left_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is strictly to the left of a temporal number
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &left_tbox_tbox);
}

PGDLLEXPORT Datum Overleft_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is not to the right of a temporal number
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overleft_tbox_tbox);
}

PGDLLEXPORT Datum Right_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is strictly to the right of a temporal number
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &right_tbox_tbox);
}

PGDLLEXPORT Datum Overright_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is not to the left of a temporal number
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overright_tbox_tbox);
}

PGDLLEXPORT Datum Before_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is strictly before a temporal number
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &before_tbox_tbox);
}

PGDLLEXPORT Datum Overbefore_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is not after a temporal number
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overbefore_tbox_tbox);
}

PGDLLEXPORT Datum After_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is strictly after a temporal number
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &after_tbox_tbox);
}

PGDLLEXPORT Datum Overafter_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal box is not before a temporal number
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/
/* Temporal op TBox */

PGDLLEXPORT Datum Left_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is strictly to the left of a temporal box
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &left_tbox_tbox);
}

PGDLLEXPORT Datum Overleft_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is not to the right of a temporal box
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overleft_tbox_tbox);
}

PGDLLEXPORT Datum Right_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is strictly to the right of a temporal box
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &right_tbox_tbox);
}

PGDLLEXPORT Datum Overright_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is not to the left of a temporal box
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overright_tbox_tbox);
}

PGDLLEXPORT Datum Before_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is strictly before a temporal box
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &before_tbox_tbox);
}

PGDLLEXPORT Datum Overbefore_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is not after a temporal box
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overbefore_tbox_tbox);
}

PGDLLEXPORT Datum After_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is strictly after a temporal box
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &after_tbox_tbox);
}

PGDLLEXPORT Datum Overafter_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if a temporal number is not before a temporal box
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/
/* Tnumber op Tnumber */

PGDLLEXPORT Datum Left_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is strictly to the left of
 * the second one
 * @sqlfunc temporal_left()
 * @sqlop @p <<
 */
Datum
Left_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &left_tbox_tbox);
}

PGDLLEXPORT Datum Overleft_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is not to the right of
 * the second one
 * @sqlfunc temporal_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overleft_tbox_tbox);
}

PGDLLEXPORT Datum Right_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is strictly to the right of
 * the second one
 * @sqlfunc temporal_right()
 * @sqlop @p >>
 */
Datum
Right_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &right_tbox_tbox);
}

PGDLLEXPORT Datum Overright_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is not to the left of
 * the second one
 * @sqlfunc temporal_overright()
 * @sqlop @p &>
 */
Datum
Overright_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overright_tbox_tbox);
}

PGDLLEXPORT Datum Before_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is strictly before
 * the second one
 * @sqlfunc temporal_before()
 * @sqlop @p <<#
 */
Datum
Before_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &before_tbox_tbox);
}

PGDLLEXPORT Datum Overbefore_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is not after the second one
 * @sqlfunc temporal_overbefore()
 * @sqlop @p &<#
 */
Datum
Overbefore_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overbefore_tbox_tbox);
}

PGDLLEXPORT Datum After_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is strictly after
 * the second one
 * @sqlfunc temporal_after()
 * @sqlop @p #>>
 */
Datum
After_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &after_tbox_tbox);
}

PGDLLEXPORT Datum Overafter_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_pos
 * @brief Return true if the first temporal number is not before the second one
 * @sqlfunc temporal_overafter()
 * @sqlop @p #&>
 */
Datum
Overafter_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/

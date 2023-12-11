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
 * @brief Operators for span set types
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/spanset.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/spanset.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains a value
 * @sqlfn span_contains()
 * @sqlop @p \@>
 */
Datum
Contains_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = contains_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contains_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set contains a span
 * @sqlfn span_contains()
 * @sqlop @p \@>
 */
Datum
Contains_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = contains_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contains_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains a span set
 * @sqlfn span_contains()
 * @sqlop @p \@>
 */
Datum
Contains_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = contains_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contains_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span set contains the second one
 * @sqlfn span_contains()
 * @sqlop @p \@>
 */
Datum
Contains_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = contains_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is contained in a span set
 * @sqlfn span_contained()
 * @sqlop @p <@
 */
Datum
Contained_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = contained_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contained_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span is contained in a span set
 * @sqlfn span_contained()
 * @sqlop @p <@
 */
Datum
Contained_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = contained_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contained_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set is contained in a span
 * @sqlfn span_contained()
 * @sqlop @p <@
 */
Datum
Contained_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = contained_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contained_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span set is contained in the second one
 * @sqlfn span_contained()
 * @sqlop @p <@
 */
Datum
Contained_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = contained_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and a span set overlap
 * @sqlfn span_overlaps()
 * @sqlop @p &&
 */
Datum
Overlaps_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overlaps_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overlaps_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and a span overlap
 * @sqlfn span_overlaps()
 * @sqlop @p &&
 */
Datum
Overlaps_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overlaps_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overlaps_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if two span sets overlap
 * @sqlfn span_overlaps()
 * @sqlop @p &&
 */
Datum
Overlaps_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = overlaps_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

PGDLLEXPORT Datum Adjacent_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is adjacent to a span
 * @sqlfn span_adjacent()
 * @sqlop @p span_adjacent
 */
Datum
Adjacent_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = adjacent_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Adjacent_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is adjacent to a span
 * @sqlfn span_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = adjacent_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Adjacent_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and a span set are adjacent
 * @sqlfn span_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = adjacent_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Adjacent_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and a span are adjacent
 * @sqlfn span_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = adjacent_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Adjacent_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if two span sets are adjacent
 * @sqlfn span_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = adjacent_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Strictly left of
 *****************************************************************************/

PGDLLEXPORT Datum Left_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the left of a span
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = left_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Left_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is to the left of a span set
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = left_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Left_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the left of a span
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = left_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Left_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is to the left a span
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = left_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Left_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is to the left of the second one
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = left_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

PGDLLEXPORT Datum Right_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the right of a span
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = right_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Right_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is to the right of a span set
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = right_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Right_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the right of a span
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = right_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Right_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is to the right of a span
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = right_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Right_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is to the right of the second one
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = right_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

PGDLLEXPORT Datum Overleft_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the right of a span
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overleft_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overleft_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the right of a span
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overleft_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overleft_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span does not extend to the right of a span set
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overleft_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overleft_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set does not extend to the right of a span set
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overleft_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overleft_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set does not extend to the right of the
 * second one
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = overleft_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Does not extend to left of
 *****************************************************************************/

PGDLLEXPORT Datum Overright_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the left of a span
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overright_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overright_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the left of a span
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overright_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overright_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span does not extend to the left of a span set
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overright_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overright_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set does not extend to the left of a span
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overright_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overright_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set does not extend to the left of the
 * second one
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = overright_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PGDLLEXPORT Datum Union_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_value_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a value and a span set
 * @sqlfn span_union()
 * @sqlop @p +
 */
Datum
Union_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  SpanSet *result = union_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Union_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_span_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a span and a span set
 * @sqlfn span_union()
 * @sqlop @p +
 */
Datum
Union_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = union_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Union_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_spanset_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a span set and a value
 * @sqlfn span_union()
 * @sqlop @p +
 */
Datum
Union_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  SpanSet *result = union_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Union_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_spanset_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a span set and a span
 * @sqlfn span_union()
 * @sqlop @p +
 */
Datum
Union_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = union_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Union_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_spanset_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of two span sets
 * @sqlfn span_union()
 * @sqlop @p +
 */
Datum
Union_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  SpanSet *result = union_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PGDLLEXPORT Datum Intersection_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_value_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a value and a span set
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Intersection_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  SpanSet *result = intersection_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intersection_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_span_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a span and a span set
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Intersection_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = intersection_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intersection_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_spanset_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a span set and a value
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Intersection_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  SpanSet *result = intersection_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intersection_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_spanset_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a span set and a span
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Intersection_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = intersection_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intersection_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_spanset_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of two span sets
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Intersection_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  SpanSet *result = intersection_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

PGDLLEXPORT Datum Minus_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_value_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a value and a span set
 * @sqlfn span_minus()
 * @sqlop @p -
 */
Datum
Minus_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  SpanSet *result = minus_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Minus_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_span_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a span and a span set
 * @sqlfn span_minus()
 * @sqlop @p -
 */
Datum
Minus_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = minus_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Minus_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_spanset_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a span set and a value
 * @sqlfn span_minus()
 * @sqlop @p -
 */
Datum
Minus_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  SpanSet *result = minus_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Minus_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_spanset_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a span set and a span
 * @sqlfn span_minus()
 * @sqlop @p -
 */
Datum
Minus_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = minus_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Minus_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_spanset_spanset);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of two span sets
 * @sqlfn span_minus()
 * @sqlop @p -
 */
Datum
Minus_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  SpanSet *result = minus_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/******************************************************************************
 * Distance functions returning a double
 ******************************************************************************/

PGDLLEXPORT Datum Distance_value_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_value_spanset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between a value and a span set
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  double result = distance_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Distance_span_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_span_spanset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a span and a span set
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum ss = PG_GETARG_DATUM(1);
  Span sp1;
  spanset_span_slice(ss, &sp1);
  double result = distance_span_span(&sp1, s);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Distance_spanset_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_spanset_value);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between a span set and a value
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  double result = distance_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Distance_spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_spanset_span);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between a span set and a span
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  double result = distance_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Distance_spanset_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between two span sets
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  double result = distance_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_FLOAT8(result);
}

/******************************************************************************/

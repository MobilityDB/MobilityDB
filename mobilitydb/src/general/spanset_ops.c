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
 * @brief Operators for time types.
 */

#include "general/time_ops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/periodset.h"
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains a value
 * @sqlfunc span_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = contains_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set contains an ordered set
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = contains_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set contains a span set
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = contains_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains a span set
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = contains_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span set contains the second one
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Contained_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is contained by a span
 * @sqlfunc span_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = contained_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if an ordered set is contained by a span set
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = contained_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span is contained by a span set
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = contained_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set is contained by a span
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = contained_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span set is contained by the second one
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Overlaps_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if an ordered set and a span overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overlaps_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if an ordered set and a span set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overlaps_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and an ordered set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = overlaps_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and a span set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overlaps_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and an ordered set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = overlaps_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and a span overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overlaps_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the span sets overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Adjacent_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is contained by a span
 * @sqlfunc span_adjacent()
 * @sqlop @p span_adjacent
 */
PGDLLEXPORT Datum
Adjacent_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = adjacent_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if an ordered set and a span set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = adjacent_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is contained by a span
 * @sqlfunc span_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = adjacent_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and an ordered set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = adjacent_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and a span set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = adjacent_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and a span are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = adjacent_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the span sets are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Left_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the left of a span
 * @sqlfunc span_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = left_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is strictly to the left of a span set
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = left_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly to the left of a span set
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = left_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the left of a span
 * @sqlfunc span_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = left_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is strictly to the left an ordered set
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = left_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is strictly to the left a span
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = left_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is strictly to the left of the second one
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Right_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the right of a span
 * @sqlfunc span_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = right_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is strictly after a span set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = right_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly after a span set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = right_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is to the right of a span
 * @sqlfunc span_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = right_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is strictly after an ordered set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = right_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is strictly after a span
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = right_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is strictly after the second one
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Overleft_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the right of a span
 * @sqlfunc span_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overleft_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is not after a span set
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overleft_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the right of a span
 * @sqlfunc span_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overleft_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is not after an ordered set
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = overleft_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not after a span set
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overleft_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is not after a span set
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overleft_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is not after the second one
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Overright_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the left of a span
 * @sqlfunc span_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overright_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is not left a span set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overright_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value does not extend to the left of a span
 * @sqlfunc span_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overright_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not to the left of a span set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overright_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is not left an ordered set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = overright_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is not to the left of a span
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overright_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is not to the left of the second one
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Union_value_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a value and a span set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PeriodSet *result = union_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of an ordered set and a span set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = union_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_span_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a span and a span set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = union_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_spanset_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a span set and a value
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PeriodSet *result = union_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a span set and an ordered set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  SpanSet *result = union_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_spanset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a span set and a span
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = union_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of the span sets
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Intersection_value_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a value and a span set
 * @sqlfunc span_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result;
  bool found = intersection_value_spanset(d, basetype, ss, &result);
  PG_FREE_IF_COPY(ss, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of an ordered set and a span set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  OrderedSet *result = intersection_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_span_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a span and a span set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = intersection_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_spanset_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a span set and a value
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum result;
  bool found = intersection_spanset_value(ss, d, basetype, &result);
  PG_FREE_IF_COPY(ss, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a span set and an ordered set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  TimestampSet *result = intersection_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_spanset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a span set and a span
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Intersection_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of the span sets
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Minus_value_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a value and a span set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result;
  bool found = minus_value_spanset(d, basetype, ss, &result);
  PG_FREE_IF_COPY(ss, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Minus_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of an ordered set and a span set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_orderedset_spanset(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  TimestampSet *result = minus_orderedset_spanset(os, ss);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_span_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span and a span set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Minus_spanset_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span set and a value
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PeriodSet *result = minus_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span set and an ordered set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_spanset_orderedset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  SpanSet *result = minus_spanset_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_spanset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span set and a span
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Minus_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of the span sets
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
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

PG_FUNCTION_INFO_V1(Distance_value_spanset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between a value and a span set
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  double result = distance_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_orderedset_spanset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between an ordered set and a span set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_orderedset_spanset(PG_FUNCTION_ARGS)
{
  Datum os = PG_GETARG_DATUM(0);
  Datum ss = PG_GETARG_DATUM(1);
  Period p1, p2;
  orderedset_span_slice(os, &p1);
  spanset_span_slice(ss, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_span_spanset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a span and a span set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum ss = PG_GETARG_DATUM(1);
  Period p1;
  spanset_span_slice(ss, &p1);
  double result = distance_span_span(&p1, s);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_spanset_value);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between a span set and a value
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  double result = distance_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_spanset_orderedset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a span set and an ordered set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_spanset_orderedset(PG_FUNCTION_ARGS)
{
  Datum ss = PG_GETARG_DATUM(0);
  Datum os = PG_GETARG_DATUM(1);
  Period p1, p2;
  spanset_span_slice(ss, &p1);
  orderedset_span_slice(os, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_spanset_span);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between a span set and a span
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  double result = distance_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance between the span sets
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  double result = distance_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 0);
  PG_RETURN_FLOAT8(result);
}

/******************************************************************************/

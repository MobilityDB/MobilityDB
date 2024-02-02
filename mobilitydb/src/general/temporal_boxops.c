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
 * @brief Bounding box functions for temporal types
 *
 * The bounding box of temporal values are
 * - a `Span` for temporal Boolean and temporal text values
 * - a `TBox` for temporal integers and floats, where the *x* coordinate is for
 *   the value dimension and the *t* coordinate is for the time dimension.
 * The following functions are defined: `overlaps`, `contains`, `contained`,
 * `same`, and `adjacent`.
 *
 * The functions consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 */

#include "general/temporal_boxops.h"

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/tbox.h"
#include "general/temporal.h"

/*****************************************************************************/

/**
 * @brief Return the size in bytes to read from toast to get the basic
 * information from a temporal: Temporal struct (i.e., TInstant, TSequence,
 * or TSequenceSet) and bounding box size
*/
size_t
temporal_max_header_size(void)
{
  size_t sz = Max(Max(sizeof(TInstant), sizeof(TSequence)), 
    sizeof(TSequenceSet));
  return DOUBLE_PAD(sz) + DOUBLE_PAD(sizeof(bboxunion));
}

/*****************************************************************************
 * Bounding box functions for temporal types: Generic functions
 * The inclusive/exclusive bounds are taken into account for the comparisons
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a timestamptz span and a temporal
 * value
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tstzspan_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *))
{
  Span *s = PG_GETARG_SPAN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_temporal_tstzspan(temp, s, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for a temporal value and a timestamptz
 * span
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_temporal_tstzspan(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = boxop_temporal_tstzspan(temp, s, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for two temporal values
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_temporal_temporal(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_temporal_temporal(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Bounding box functions for temporal types
 *****************************************************************************/

PGDLLEXPORT Datum Contains_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a timestamptz span contains the time span of a
 * temporal value
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &contains_span_span);
}

PGDLLEXPORT Datum Contains_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value contains a
 * timestamptz span
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &contains_span_span);
}

PGDLLEXPORT Datum Contains_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time span of the first temporal value
 * contains the one of the second one
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &contains_span_span);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contained_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a timestamptz span is contained the time span of
 * a temporal value
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &contained_span_span);
}

PGDLLEXPORT Datum Contained_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value is contained
 * in a timestamptz span
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &contained_span_span);
}

PGDLLEXPORT Datum Contained_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time span of the first temporal value is
 * contained in the one of the second temporal value
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &contained_span_span);
}

/*****************************************************************************/

PGDLLEXPORT Datum Overlaps_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a
 * temporal value overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &overlaps_span_span);
}

PGDLLEXPORT Datum Overlaps_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a
 * timestamptz span overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &overlaps_span_span);
}

PGDLLEXPORT Datum Overlaps_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &overlaps_span_span);
}

/*****************************************************************************/

PGDLLEXPORT Datum Same_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a
 * temporal value are equal
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &span_eq);
}

PGDLLEXPORT Datum Same_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a
 * timestamptz span are equal
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &span_eq);
}

PGDLLEXPORT Datum Same_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values are equal
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &span_eq);
}

/*****************************************************************************/

PGDLLEXPORT Datum Adjacent_tstzspan_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tstzspan_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a
 * temporal value are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tstzspan_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_tstzspan_temporal(fcinfo, &adjacent_span_span);
}

PGDLLEXPORT Datum Adjacent_temporal_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_temporal_tstzspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a
 * timestamptz span are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_temporal_tstzspan(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_tstzspan(fcinfo, &adjacent_span_span);
}

PGDLLEXPORT Datum Adjacent_temporal_temporal(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_temporal_temporal);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_temporal_temporal(PG_FUNCTION_ARGS)
{
  return Boxop_temporal_temporal(fcinfo, &adjacent_span_span);
}

/*****************************************************************************
 * Generic bounding box functions for temporal number types
 *****************************************************************************/

/**
 * @brief Generic bounding box function for a span and a temporal number
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_numspan_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *))
{
  Span *s = PG_GETARG_SPAN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tnumber_numspan(temp, s, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for a temporal number and a span
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tnumber_numspan(FunctionCallInfo fcinfo,
  bool (*func)(const Span *, const Span *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = boxop_tnumber_numspan(temp, s, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for a temporal box and a temporal
 * number
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tbox_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *))
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TBox box1;
  temporal_set_bbox(temp, &box1);
  bool result = boxop_tnumber_tbox(temp, box, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for a temporal number and a temporal
 * box
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tnumber_tbox(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBox *box = PG_GETARG_TBOX_P(1);
  bool result = boxop_tnumber_tbox(temp, box, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic bounding box function for two temporal numbers
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Bounding box function
 */
Datum
Boxop_tnumber_tnumber(FunctionCallInfo fcinfo,
  bool (*func)(const TBox *, const TBox *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = boxop_tnumber_tnumber(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Bounding box functions for temporal numbers
 *****************************************************************************/

PGDLLEXPORT Datum Contains_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a number span contains the value span of a temporal
 * number
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &contains_span_span);
}

PGDLLEXPORT Datum Contains_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number contains
 * a number span
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &contains_span_span);
}

PGDLLEXPORT Datum Contains_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a temporal box contains the bounding box of a
 * temporal number
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &contains_tbox_tbox);
}

PGDLLEXPORT Datum Contains_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number contains a
 * temporal box
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &contains_tbox_tbox);
}

PGDLLEXPORT Datum Contains_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding box of the first temporal number contains
 * the one of the second temporal number
 * @sqlfn contains_bbox()
 * @sqlop @p \@>
 */
Datum
Contains_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &contains_tbox_tbox);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contained_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a number span is contained in the value span
 * of a temporal number
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &contained_span_span);
}

PGDLLEXPORT Datum Contained_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number is
 * contained in a number span
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &contained_span_span);
}

PGDLLEXPORT Datum Contained_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a temporal box is contained in the bounding box of a
 * temporal number
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &contained_tbox_tbox);
}

PGDLLEXPORT Datum Contained_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number is contained in
 * a temporal box
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &contained_tbox_tbox);
}

PGDLLEXPORT Datum Contained_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding box of the first temporal number is
 * contained in the one of the second temporal number
 * @sqlfn contained_bbox()
 * @sqlop @p <@
 */
Datum
Contained_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &contained_tbox_tbox);
}

/*****************************************************************************/

PGDLLEXPORT Datum Overlaps_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a
 * temporal number overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &overlaps_span_span);
}

PGDLLEXPORT Datum Overlaps_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and the
 * number span overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &overlaps_span_span);
}

PGDLLEXPORT Datum Overlaps_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &overlaps_tbox_tbox);
}

PGDLLEXPORT Datum Overlaps_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &overlaps_tbox_tbox);
}

PGDLLEXPORT Datum Overlaps_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers overlap
 * @sqlfn overlaps_bbox()
 * @sqlop @p &&
 */
Datum
Overlaps_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &overlaps_tbox_tbox);
}

/*****************************************************************************/

PGDLLEXPORT Datum Same_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a
 * temporal number are equal
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &span_eq);
}

PGDLLEXPORT Datum Same_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and a
 * number span are equal
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &span_eq);
}

PGDLLEXPORT Datum Same_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &same_tbox_tbox);
}

PGDLLEXPORT Datum Same_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a
 * temporal box are equal in the common dimensions
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &same_tbox_tbox);
}

PGDLLEXPORT Datum Same_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers are equal
 * @sqlfn same_bbox()
 * @sqlop @p ~=
 */
Datum
Same_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &same_tbox_tbox);
}

/*****************************************************************************/

PGDLLEXPORT Datum Adjacent_numspan_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_numspan_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a
 * temporal number are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_numspan_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_numspan_tnumber(fcinfo, &adjacent_span_span);
}

PGDLLEXPORT Datum Adjacent_tnumber_numspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tnumber_numspan);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and a
 * number span are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tnumber_numspan(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_numspan(fcinfo, &adjacent_span_span);
}

PGDLLEXPORT Datum Adjacent_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tbox_tnumber(fcinfo, &adjacent_tbox_tbox);
}

PGDLLEXPORT Datum Adjacent_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tbox(fcinfo, &adjacent_tbox_tbox);
}

PGDLLEXPORT Datum Adjacent_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers are adjacent
 * @sqlfn adjacent_bbox()
 * @sqlop @p -|-
 */
Datum
Adjacent_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Boxop_tnumber_tnumber(fcinfo, &adjacent_tbox_tbox);
}

/*****************************************************************************/

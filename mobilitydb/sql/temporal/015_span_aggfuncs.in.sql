/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Aggregate functions for types whose bounding box is a span
 */

/*****************************************************************************/
-- span + span

-- The functions are not strict
CREATE FUNCTION extentTransition(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION extentTransition(bigintspan, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(bigintspan, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION extentTransition(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION extentTransition(datespan, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(datespan, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION extentTransition(tstzspan, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(tstzspan, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(intspan) (
  SFUNC = extentTransition,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigintspan) (
  SFUNC = extentTransition,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(floatspan) (
  SFUNC = extentTransition,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(datespan) (
  SFUNC = extentTransition,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tstzspan) (
  SFUNC = extentTransition,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/
-- span + base

-- The functions are not strict
CREATE FUNCTION extentTransition(intspan, integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(bigintspan, bigint)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(floatspan, float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(datespan, date)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(tstzspan, timestamptz)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(integer) (
  SFUNC = extentTransition,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigint) (
  SFUNC = extentTransition,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(float) (
  SFUNC = extentTransition,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(date) (
  SFUNC = extentTransition,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(timestamptz) (
  SFUNC = extentTransition,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/
-- span + <type>

-- The functions are not strict
CREATE FUNCTION extentTransition(intspan, intset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(bigintspan, bigintset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(floatspan, floatset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(datespan, dateset)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(tstzspan, tstzset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(intset) (
  SFUNC = extentTransition,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigintset) (
  SFUNC = extentTransition,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(floatset) (
  SFUNC = extentTransition,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(dateset) (
  SFUNC = extentTransition,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tstzset) (
  SFUNC = extentTransition,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

-- The functions are not strict
CREATE FUNCTION extentTransition(intspan, intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(bigintspan, bigintspanset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(floatspan, floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(datespan, datespanset)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentTransition(tstzspan, tstzspanset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(intspanset) (
  SFUNC = extentTransition,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigintspanset) (
  SFUNC = extentTransition,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(floatspanset) (
  SFUNC = extentTransition,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(datespanset) (
  SFUNC = extentTransition,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tstzspanset) (
  SFUNC = extentTransition,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/

-- The functions are not strict
CREATE FUNCTION spanUnionTransition(internal, intspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanUnionTransition(internal, bigintspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanUnionTransition(internal, floatspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanUnionTransition(internal, datespan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanUnionTransition(internal, tstzspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION spanset_union_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spansetstate_serialize(internal)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spansetstate_serialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spansetstate_deserialize(bytea, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spansetstate_deserialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The functions are not strict
CREATE FUNCTION intspan_union_finalfn(internal)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION bigintspan_union_finalfn(internal)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION floatspan_union_finalfn(internal)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION datespan_union_finalfn(internal)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tstzspan_union_finalfn(internal)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE spanUnion(intspan) (
  SFUNC = spanUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = intspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(bigintspan) (
  SFUNC = spanUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = bigintspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(floatspan) (
  SFUNC = spanUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = floatspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(datespan) (
  SFUNC = spanUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = datespan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(tstzspan) (
  SFUNC = spanUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = tstzspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************/

-- The functions are not strict
CREATE FUNCTION spansetUnionTransition(internal, intspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spansetUnionTransition(internal, bigintspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spansetUnionTransition(internal, floatspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spansetUnionTransition(internal, datespanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spansetUnionTransition(internal, tstzspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE spansetUnion(intspanset) (
  SFUNC = spansetUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = intspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spansetUnion(bigintspanset) (
  SFUNC = spansetUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = bigintspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spansetUnion(floatspanset) (
  SFUNC = spansetUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = floatspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spansetUnion(datespanset) (
  SFUNC = spansetUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = datespan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);
CREATE AGGREGATE spansetUnion(tstzspanset) (
  SFUNC = spansetUnionTransition,
  STYPE = internal,
  COMBINEFUNC = spanset_union_combinefn,
  FINALFUNC = tstzspan_union_finalfn,
  FINALFUNC_MODIFY = READ_WRITE,
  SERIALFUNC = spansetstate_serialize,
  DESERIALFUNC = spansetstate_deserialize,
  PARALLEL = safe
);

/*****************************************************************************/

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
 * @brief Aggregate functions for types whose bounding box is a span
 */

/*****************************************************************************/
-- span + span

CREATE FUNCTION span_extent_transfn(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION span_extent_transfn(bigintspan, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(bigintspan, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION span_extent_transfn(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_transfn(datespan, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(datespan, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_transfn(tstzspan, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_combinefn(tstzspan, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(intspan) (
  SFUNC = span_extent_transfn,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigintspan) (
  SFUNC = span_extent_transfn,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(floatspan) (
  SFUNC = span_extent_transfn,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(datespan) (
  SFUNC = span_extent_transfn,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tstzspan) (
  SFUNC = span_extent_transfn,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/
-- span + base

CREATE FUNCTION span_extent_transfn(intspan, integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_transfn(bigintspan, bigint)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_transfn(floatspan, float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_transfn(datespan, date)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_extent_transfn(tstzspan, timestamptz)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanbase_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(integer) (
  SFUNC = span_extent_transfn,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigint) (
  SFUNC = span_extent_transfn,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(float) (
  SFUNC = span_extent_transfn,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(date) (
  SFUNC = span_extent_transfn,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(timestamptz) (
  SFUNC = span_extent_transfn,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/
-- span + <type>

CREATE FUNCTION set_extent_transfn(intspan, intset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_extent_transfn(bigintspan, bigintset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_extent_transfn(floatspan, floatset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_extent_transfn(datespan, dateset)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_extent_transfn(tstzspan, tstzset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Set_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(intset) (
  SFUNC = set_extent_transfn,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigintset) (
  SFUNC = set_extent_transfn,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(floatset) (
  SFUNC = set_extent_transfn,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(dateset) (
  SFUNC = set_extent_transfn,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tstzset) (
  SFUNC = set_extent_transfn,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

CREATE FUNCTION spanset_extent_transfn(intspan, intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_extent_transfn(bigintspan, bigintspanset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_extent_transfn(floatspan, floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_extent_transfn(datespan, datespanset)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_extent_transfn(tstzspan, tstzspanset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanset_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(intspanset) (
  SFUNC = spanset_extent_transfn,
  STYPE = intspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(bigintspanset) (
  SFUNC = spanset_extent_transfn,
  STYPE = bigintspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(floatspanset) (
  SFUNC = spanset_extent_transfn,
  STYPE = floatspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(datespanset) (
  SFUNC = spanset_extent_transfn,
  STYPE = datespan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);
CREATE AGGREGATE extent(tstzspanset) (
  SFUNC = spanset_extent_transfn,
  STYPE = tstzspan,
  COMBINEFUNC = span_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************/

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
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = intspan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(bigintspan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = bigintspan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(floatspan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = floatspan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(datespan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = datespan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(tstzspan) (
  SFUNC = array_agg_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = tstzspan_union_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

CREATE FUNCTION spanset_union_transfn(internal, intspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_union_transfn(internal, bigintspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_union_transfn(internal, floatspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_union_transfn(internal, datespanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION spanset_union_transfn(internal, tstzspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE spanUnion(intspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = intspan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(bigintspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = bigintspan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(floatspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = floatspan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(datespanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = datespan_union_finalfn,
  PARALLEL = safe
);
CREATE AGGREGATE spanUnion(tstzspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = tstzspan_union_finalfn,
  PARALLEL = safe
);

/*****************************************************************************/

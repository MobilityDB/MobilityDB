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

/*
 * time_aggfuncs.sql
 * Aggregate functions for time types
 */

/*****************************************************************************/

CREATE FUNCTION tagg_serialize(internal)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tagg_serialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tagg_deserialize(bytea, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tagg_deserialize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE TYPE tint;

CREATE FUNCTION tcount_transfn(internal, timestamptz)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Timestamp_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tstzset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tstzset_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tstzspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Period_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tstzspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Periodset_tcount_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION tcount_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_tcount_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tagg_finalfn(internal)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_tagg_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tcount(timestamptz) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tcount(tstzset) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tcount(tstzspan) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tcount(tstzspanset) (
  SFUNC = tcount_transfn,
  STYPE = internal,
  COMBINEFUNC = tcount_combinefn,
  FINALFUNC = tint_tagg_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION timestamp_tunion_transfn(internal, timestamptz)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Timestamp_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tstzset_tunion_transfn(internal, tstzset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tstzset_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION period_tunion_transfn(internal, tstzspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Period_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION periodset_tunion_transfn(internal, tstzspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Periodset_tunion_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION time_tunion_combinefn(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Time_tunion_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION timestamp_tunion_finalfn(internal)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Timestamp_tunion_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_tunion_finalfn(internal)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Period_tunion_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tunion(timestamptz) (
  SFUNC = timestamp_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = timestamp_tunion_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tunion(tstzset) (
  SFUNC = tstzset_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = timestamp_tunion_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tunion(tstzspan) (
  SFUNC = period_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = period_tunion_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

CREATE AGGREGATE tunion(tstzspanset) (
  SFUNC = periodset_tunion_transfn,
  STYPE = internal,
  COMBINEFUNC = time_tunion_combinefn,
  FINALFUNC = period_tunion_finalfn,
  SERIALFUNC = tagg_serialize,
  DESERIALFUNC = tagg_deserialize,
  PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION span_union_transfn(internal, intspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_union_transfn(internal, bigintspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_union_transfn(internal, floatspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION span_union_transfn(internal, tstzspan)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION intspan_union_finalfn(internal)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspan_union_finalfn(internal)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan_union_finalfn(internal)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspan_union_finalfn(internal)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Span_union_finalfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE span_union(intspan) (
  SFUNC = span_union_transfn,
  STYPE = internal,
  FINALFUNC = intspan_union_finalfn
);
CREATE AGGREGATE span_union(bigintspan) (
  SFUNC = span_union_transfn,
  STYPE = internal,
  FINALFUNC = bigintspan_union_finalfn
);
CREATE AGGREGATE span_union(floatspan) (
  SFUNC = span_union_transfn,
  STYPE = internal,
  FINALFUNC = floatspan_union_finalfn
);
CREATE AGGREGATE span_union(tstzspan) (
  SFUNC = span_union_transfn,
  STYPE = internal,
  FINALFUNC = tstzspan_union_finalfn
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
CREATE FUNCTION spanset_union_transfn(internal, tstzspanset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE span_union(intspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
  FINALFUNC = intspan_union_finalfn
);
CREATE AGGREGATE span_union(bigintspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
  FINALFUNC = bigintspan_union_finalfn
);
CREATE AGGREGATE span_union(floatspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
  FINALFUNC = floatspan_union_finalfn
);
CREATE AGGREGATE span_union(tstzspanset) (
  SFUNC = spanset_union_transfn,
  STYPE = internal,
  FINALFUNC = tstzspan_union_finalfn
);

/*****************************************************************************/

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
 * @brief Type plumbing for `tquadbin`, a temporal type carrying CARTO
 * quadbin cell indices as a function of time.
 *
 * On-disk representation is the same Temporal structure used by
 * every other temporal type; the basetype is the dedicated
 * `quadbin`, not `int8`, and the catalog entry
 * `{T_TQUADBIN, T_QUADBIN}` drives dispatch. tquadbin is classified
 * as a planar `tspatial_type` — quadbin cells are Web-Mercator
 * squares whose lon/lat geometry lives in SRID 4326, so the bounding
 * box is a planar `stbox` (X/Y + T dimensions, GEODETIC flag clear),
 * matching `tgeompoint`.
 *
 * Casts to / from `tbigint` are ASSIGNMENT-only — the user must
 * spell out the cast, mistakes surface as a clear binder error
 * rather than silently reinterpreting an arbitrary int64
 * trajectory as a stream of quadbin cells. The two types share the
 * int64 payload but carry different bbox shapes, so the casts go
 * through a real function — see below.
 */

CREATE TYPE tquadbin;

/******************************************************************************
 * Input / Output
 ******************************************************************************/

CREATE FUNCTION tquadbin_in(cstring, oid, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tquadbin_recv(internal, oid, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_out(tquadbin)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_send(tquadbin)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tquadbin (
  internallength = variable,
  input = tquadbin_in,
  output = temporal_out,
  send = temporal_send,
  receive = tquadbin_recv,
  typmod_in = temporal_typmod_in,
  typmod_out = temporal_typmod_out,
  storage = extended,
  alignment = double,
  analyze = temporal_analyze
);

/******************************************************************************
 * Typmod enforcer + self-cast (mirrors tbigint / tint / tfloat / ttext)
 ******************************************************************************/

CREATE FUNCTION tquadbin(tquadbin, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tquadbin AS tquadbin) WITH FUNCTION tquadbin(tquadbin, integer)
  AS IMPLICIT;

/******************************************************************************
 * Explicit assignment casts to / from tbigint
 *
 * The int64 payload is shared, but tquadbin sequences carry a planar
 * STBox bbox while tbigint sequences carry a TBox; a binary coercion
 * would leave the wrong bbox shape in place and the wrong temptype byte
 * inside the Temporal header. The cast therefore goes through a real
 * function that lifts an identity Datum function so the result is
 * rebuilt at the correct shape. The cast is declared `AS ASSIGNMENT`
 * so the user must spell out `::tquadbin` or `::tbigint` — mistakes
 * surface as a clear binder error rather than silently reinterpreting
 * an arbitrary int64 trajectory as a stream of quadbin cells.
 ******************************************************************************/

CREATE FUNCTION tquadbin(tbigint)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tbigint_to_tquadbin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbigint(tquadbin)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Tquadbin_to_tbigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbigint AS tquadbin) WITH FUNCTION tquadbin(tbigint) AS ASSIGNMENT;
CREATE CAST (tquadbin AS tbigint) WITH FUNCTION tbigint(tquadbin) AS ASSIGNMENT;

/******************************************************************************
 * Constructors
 *
 * Inherited Temporal<T> surface wired to the generic constructor symbols
 * (the value Datum carries the quadbin cell, dispatch is on the base oid).
 * `tquadbin` has step interpolation only, so the from-span and array
 * constructors take no interpolation argument / default to `'step'`, and a
 * cell has no scalar-distance metric so `tquadbinSeqSetGaps` exposes only the
 * time gap (the `tbigint` / `ttext` form, not the continuous `tcbuffer` one).
 ******************************************************************************/

CREATE FUNCTION tquadbin(quadbin, timestamptz)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tinstant_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tquadbin(quadbin, tstzset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tquadbin(quadbin, tstzspan)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tquadbin(quadbin, tstzspanset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tsequenceset_from_base_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tquadbinSeq(tquadbin[], text DEFAULT 'step',
    lowerInc boolean DEFAULT true, upperInc boolean DEFAULT true)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tsequence_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tquadbinSeqSet(tquadbin[])
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tquadbinSeqSetGaps(tquadbin[], maxt interval DEFAULT NULL)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor_gaps'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/******************************************************************************
 * Conversions
 ******************************************************************************/

CREATE FUNCTION timeSpan(tquadbin)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tquadbin AS tstzspan) WITH FUNCTION timeSpan(tquadbin);

/******************************************************************************
 * Transformations
 ******************************************************************************/

CREATE FUNCTION tquadbinInst(tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_to_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tquadbinSeq(tquadbin, text DEFAULT NULL)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_to_tsequence'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tquadbinSeqSet(tquadbin, text DEFAULT NULL)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_to_tsequenceset'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION setInterp(tquadbin, text)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_set_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendInstant(tquadbin, tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_append_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendSequence(tquadbin, tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_append_tsequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION merge(tquadbin, tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_merge'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION merge(tquadbin[])
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_merge_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTime(tquadbin, interval)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION scaleTime(tquadbin, interval)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftScaleTime(tquadbin, interval, interval)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tprecision(tquadbin, duration interval,
  origin timestamptz DEFAULT '2000-01-03')
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tsample(tquadbin, duration interval,
  origin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor functions
 *
 * The value accessors that return the `quadbin` cell (`startValue`,
 * `endValue`, `valueN`, `getValues`, `valueAtTimestamp`) and the lifted cell
 * operations live in `355_tquadbin_spatialfuncs.in.sql`.
 ******************************************************************************/

CREATE FUNCTION tempSubtype(tquadbin)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION interp(tquadbin)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(tquadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValue(tquadbin)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Tinstant_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTimestamp(tquadbin)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tinstant_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getTime(tquadbin)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(tquadbin, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lowerInc(tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upperInc(tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(tquadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startInstant(tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_start_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endInstant(tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_end_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instantN(tquadbin, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_instant_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instants(tquadbin)
  RETURNS tquadbin[]
  AS 'MODULE_PATHNAME', 'Temporal_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(tquadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startTimestamp(tquadbin)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_start_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endTimestamp(tquadbin)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_end_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampN(tquadbin, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_timestamptz_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestamps(tquadbin)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Temporal_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(tquadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSequence(tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_start_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSequence(tquadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_end_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequenceN(tquadbin, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_sequence_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequences(tquadbin)
  RETURNS tquadbin[]
  AS 'MODULE_PATHNAME', 'Temporal_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION segments(tquadbin)
  RETURNS tquadbin[]
  AS 'MODULE_PATHNAME', 'Temporal_segments'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Unnest
 ******************************************************************************/

CREATE TYPE quadbin_tstzspanset AS (
  value quadbin,
  time tstzspanset
);

CREATE FUNCTION unnest(tquadbin)
  RETURNS SETOF quadbin_tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Restriction functions
 ******************************************************************************/

CREATE FUNCTION atValue(tquadbin, quadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_at_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValue(tquadbin, quadbin)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_minus_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValues(tquadbin, quadbinset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_at_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValues(tquadbin, quadbinset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_minus_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tquadbin, timestamptz)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(tquadbin, timestamptz)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_minus_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(tquadbin, tstzset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(tquadbin, tstzset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(tquadbin, tstzspan)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(tquadbin, tstzspan)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(tquadbin, tstzspanset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(tquadbin, tstzspanset)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION beforeTimestamp(tquadbin, timestamptz, strict bool DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_before_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION afterTimestamp(tquadbin, timestamptz, strict bool DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_after_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Modification functions
 ******************************************************************************/

CREATE FUNCTION insert(tquadbin, tquadbin, connect boolean DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION update(tquadbin, tquadbin, connect boolean DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(tquadbin, timestamptz, connect boolean DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_delete_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(tquadbin, tstzset, connect boolean DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(tquadbin, tstzspan, connect boolean DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(tquadbin, tstzspanset, connect boolean DEFAULT TRUE)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE time_tquadbin AS (
  time timestamptz,
  temp tquadbin
);

CREATE FUNCTION timeSplit(tquadbin, bin_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/******************************************************************************
 * Comparison functions and B-tree / hash indexing
 *
 * All tquadbin values share the exact same on-disk layout as every
 * other temporal type, so the generic Temporal_* dispatch in the
 * backend is enough — no tquadbin-specific wrappers required.
 ******************************************************************************/

CREATE FUNCTION lt(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eq(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(tquadbin, tquadbin)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tquadbin_btree_ops
  DEFAULT FOR TYPE tquadbin USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 cmp(tquadbin, tquadbin);

/******************************************************************************/

CREATE FUNCTION temporal_hash(tquadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tquadbin_hash_ops
  DEFAULT FOR TYPE tquadbin USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(tquadbin);

/******************************************************************************/

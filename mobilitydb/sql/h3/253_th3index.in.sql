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
 * @brief Type plumbing for `th3index`, a temporal type carrying H3 cell
 * indices as a function of time.
 *
 * On-disk representation is the same Temporal structure used by
 * every other temporal type; the basetype is the dedicated
 * `h3index`, not `int8`, and the catalog entry
 * `{T_TH3INDEX, T_H3INDEX}` drives dispatch. th3index is
 * classified as a `tspatial_type` and `tgeodetic_type` — H3 cells
 * are geographic hexagons on the WGS84 sphere, always geodetic and
 * always lat/lon, so the bounding box is a geodetic `stbox` (X/Y +
 * T dimensions, GEODETIC flag set), matching `tgeogpoint`.
 *
 * Casts to / from `tbigint` are ASSIGNMENT-only — the user must
 * spell out the cast, mistakes surface as a clear binder error
 * rather than silently reinterpreting an arbitrary int64
 * trajectory as a stream of H3 cells. The two types are
 * binary-coercible (the int64 payload is identical) so the casts
 * are `WITHOUT FUNCTION` — see below.
 */

CREATE TYPE th3index;

/******************************************************************************
 * Input / Output
 ******************************************************************************/

CREATE FUNCTION th3index_in(cstring, oid, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3index_recv(internal, oid, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_out(th3index)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_send(th3index)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE th3index (
  internallength = variable,
  input = th3index_in,
  output = temporal_out,
  send = temporal_send,
  receive = th3index_recv,
  typmod_in = temporal_typmod_in,
  typmod_out = temporal_typmod_out,
  storage = extended,
  alignment = double,
  analyze = tspatial_analyze
);

/******************************************************************************
 * Text and (Hex)WKB I/O (mirrors the tcbuffer / tnpoint / tpose plug-in types)
 ******************************************************************************/

CREATE FUNCTION asText(th3index)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(th3index, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(th3index, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(temp th3index, options int4 DEFAULT 0,
    flags int4 DEFAULT 0, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3indexFromBinary(bytea)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3indexFromHexWKB(text)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3indexFromMFJSON(text)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Typmod enforcer + self-cast (mirrors tbigint / tint / tfloat / ttext)
 ******************************************************************************/

CREATE FUNCTION th3index(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (th3index AS th3index) WITH FUNCTION th3index(th3index, integer)
  AS IMPLICIT;

/******************************************************************************
 * Explicit assignment casts to / from tbigint
 *
 * The int64 payload is shared, but th3index sequences carry a geodetic
 * STBox bbox while tbigint sequences carry a TBox; a binary coercion
 * would leave the wrong bbox shape in place and the wrong temptype byte
 * inside the Temporal header. The cast therefore goes through a real
 * function that lifts an identity Datum function so the result is
 * rebuilt at the correct shape. The cast is declared `AS ASSIGNMENT`
 * so the user must spell out `::th3index` or `::tbigint` — mistakes
 * surface as a clear binder error rather than silently reinterpreting
 * an arbitrary int64 trajectory as a stream of H3 cells.
 ******************************************************************************/

CREATE FUNCTION th3index(tbigint)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tbigint_to_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbigint(th3index)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Th3index_to_tbigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbigint AS th3index) WITH FUNCTION th3index(tbigint) AS ASSIGNMENT;
CREATE CAST (th3index AS tbigint) WITH FUNCTION tbigint(th3index) AS ASSIGNMENT;

/******************************************************************************
 * Constructors
 *
 * Inherited Temporal<T> surface wired to the generic constructor symbols
 * (the value Datum carries the h3 cell, dispatch is on the base oid).
 * `th3index` has step interpolation only, so the from-span and array
 * constructors take no interpolation argument / default to `'step'`, and a
 * cell has no scalar-distance metric so `th3indexSeqSetGaps` exposes only the
 * time gap (the `tbigint` / `ttext` form, not the continuous `tcbuffer` one).
 ******************************************************************************/

CREATE FUNCTION th3index(h3index, timestamptz)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tinstant_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION th3index(h3index, tstzset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION th3index(h3index, tstzspan)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION th3index(h3index, tstzspanset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tsequenceset_from_base_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3indexSeq(th3index[], text DEFAULT 'step',
    lowerInc boolean DEFAULT true, upperInc boolean DEFAULT true)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tsequence_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION th3indexSeqSet(th3index[])
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION th3indexSeqSetGaps(th3index[], maxt interval DEFAULT NULL)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor_gaps'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/******************************************************************************
 * Transformations
 ******************************************************************************/

CREATE FUNCTION th3indexInst(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_as_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION th3indexSeq(th3index, text DEFAULT NULL)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequence'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION th3indexSeqSet(th3index, text DEFAULT NULL)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequenceset'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION setInterp(th3index, text)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_set_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendInstant(th3index, th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_append_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendSequence(th3index, th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_append_tsequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION merge(th3index, th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_merge'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION merge(th3index[])
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_merge_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTime(th3index, interval)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION scaleTime(th3index, interval)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftScaleTime(th3index, interval, interval)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tprecision(th3index, duration interval,
  origin timestamptz DEFAULT '2000-01-03')
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tsample(th3index, duration interval,
  origin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor functions
 ******************************************************************************/

-- GENERATED-ACCESSORS-BEGIN h3 — tools/codegen/inherited/generate.py from templates/accessors.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.yaml (accessor_families) and re-run.

CREATE FUNCTION tempSubtype(th3index)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tempBasetype(th3index)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_basetype_name'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interp(th3index)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(th3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- value is a reserved word in SQL
CREATE FUNCTION getValue(th3index)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Tinstant_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(th3index)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tinstant_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- values is a reserved word in SQL
CREATE FUNCTION getValues(th3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(th3index)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timeSpan is the bounding period, the tstzspan extent of the temporal value
CREATE FUNCTION timeSpan(th3index)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(th3index)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(th3index)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(th3index, int)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(th3index, timestamptz)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(th3index, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lowerInc(th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upperInc(th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(th3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_start_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_end_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_instant_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(th3index)
  RETURNS th3index[]
  AS 'MODULE_PATHNAME', 'Temporal_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(th3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(th3index)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_start_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(th3index)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_end_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(th3index, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_timestamptz_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(th3index)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Temporal_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(th3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_start_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_end_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_sequence_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(th3index)
  RETURNS th3index[]
  AS 'MODULE_PATHNAME', 'Temporal_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION segments(th3index)
  RETURNS th3index[]
  AS 'MODULE_PATHNAME', 'Temporal_segments'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- GENERATED-ACCESSORS-END h3

-- The tstzspan cast is backed by the generated timeSpan accessor.
CREATE CAST (th3index AS tstzspan) WITH FUNCTION timeSpan(th3index);

/******************************************************************************
 * Unnest
 ******************************************************************************/

CREATE TYPE h3index_tstzspanset AS (
  value h3index,
  time tstzspanset
);

CREATE FUNCTION unnest(th3index)
  RETURNS SETOF h3index_tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Restriction functions
 ******************************************************************************/

CREATE FUNCTION atValue(th3index, h3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_at_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValue(th3index, h3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_minus_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValues(th3index, h3indexset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_at_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValues(th3index, h3indexset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_minus_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(th3index, timestamptz)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(th3index, timestamptz)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_minus_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(th3index, tstzset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(th3index, tstzset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(th3index, tstzspan)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(th3index, tstzspan)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(th3index, tstzspanset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(th3index, tstzspanset)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION beforeTimestamp(th3index, timestamptz, strict bool DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_before_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION afterTimestamp(th3index, timestamptz, strict bool DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_after_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Modification functions
 ******************************************************************************/

CREATE FUNCTION insert(th3index, th3index, connect boolean DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION update(th3index, th3index, connect boolean DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(th3index, timestamptz, connect boolean DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_delete_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(th3index, tstzset, connect boolean DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(th3index, tstzspan, connect boolean DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(th3index, tstzspanset, connect boolean DEFAULT TRUE)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE time_th3index AS (
  time timestamptz,
  temp th3index
);

CREATE FUNCTION timeSplit(th3index, bin_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_th3index
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/******************************************************************************
 * Comparison functions and B-tree / hash indexing
 *
 * All th3index values share the exact same on-disk layout as every
 * other temporal type, so the generic Temporal_* dispatch in the
 * backend is enough — no th3index-specific wrappers required.
 ******************************************************************************/

CREATE FUNCTION lt(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eq(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(th3index, th3index)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS th3index_btree_ops
  DEFAULT FOR TYPE th3index USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 cmp(th3index, th3index);

/******************************************************************************/

CREATE FUNCTION temporal_hash(th3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS th3index_hash_ops
  DEFAULT FOR TYPE th3index USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(th3index);

/******************************************************************************/

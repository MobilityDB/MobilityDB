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
 * @brief Type plumbing for `ts2cell`, a temporal type carrying Google
 * S2 cell indices as a function of time.
 *
 * On-disk representation is the same Temporal structure used by
 * every other temporal type; the basetype is the dedicated
 * `s2cell`, not `int8`, and the catalog entry
 * `{T_TS2CELL, T_S2CELL}` drives dispatch. ts2cell is classified
 * as a geodetic `tspatial_type` — an S2 cell is a region of the
 * WGS84 sphere, so the bounding box is a geodetic `stbox` (X/Y + T
 * dimensions, GEODETIC flag set, SRID 4326), matching `tgeogpoint`
 * where the Web-Mercator `tquadbin` carries a planar box.
 *
 * Casts to / from `tbigint` are ASSIGNMENT-only — the user must
 * spell out the cast, mistakes surface as a clear binder error
 * rather than silently reinterpreting an arbitrary int64
 * trajectory as a stream of s2cell cells. The two types share the
 * int64 payload but carry different bbox shapes, so the casts go
 * through a real function — see below.
 */

-- GENERATED-IO-BEGIN s2cell — tools/codegen/inherited/generate.py from templates/comparisons.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/io_families.yaml and re-run.
CREATE TYPE ts2cell;

/******************************************************************************
 * Input / Output
 ******************************************************************************/

CREATE FUNCTION ts2cell_in(cstring, oid, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ts2cell_recv(internal, oid, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_out(ts2cell)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_send(ts2cell)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE ts2cell (
  internallength = variable,
  input = ts2cell_in,
  output = temporal_out,
  send = temporal_send,
  receive = ts2cell_recv,
  typmod_in = temporal_typmod_in,
  typmod_out = temporal_typmod_out,
  storage = extended,
  alignment = double,
  analyze = tspatial_analyze
);

-- GENERATED-IO-END s2cell

-- GENERATED-REPRESENTATIONS-BEGIN s2cell — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/******************************************************************************
 * Text and (Hex)WKB I/O (mirrors the tcbuffer / tnpoint / tpose plug-in types)
 ******************************************************************************/

CREATE FUNCTION asText(ts2cell)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(ts2cell, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(ts2cell, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(temp ts2cell, options integer DEFAULT 0,
    flags integer DEFAULT 0, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ts2cellFromBinary(bytea)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ts2cellFromHexWKB(text)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ts2cellFromMFJSON(text)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-END s2cell

/******************************************************************************
 * Typmod enforcer + self-cast (mirrors tbigint / tint / tfloat / ttext)
 ******************************************************************************/

CREATE FUNCTION ts2cell(ts2cell, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (ts2cell AS ts2cell) WITH FUNCTION ts2cell(ts2cell, integer)
  AS IMPLICIT;

/******************************************************************************
 * Conversions
 *
 * The int64 payload is shared, but ts2cell sequences carry a geodetic
 * STBox bbox while tbigint sequences carry a TBox; a binary coercion
 * would leave the wrong bbox shape in place and the wrong temptype byte
 * inside the Temporal header. The cast therefore goes through a real
 * function that lifts an identity Datum function so the result is
 * rebuilt at the correct shape. The cast is declared `AS ASSIGNMENT`
 * so the user must spell out `::ts2cell` or `::tbigint` — mistakes
 * surface as a clear binder error rather than silently reinterpreting
 * an arbitrary int64 trajectory as a stream of s2cell cells.
 ******************************************************************************/

CREATE FUNCTION ts2cell(tbigint)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tbigint_to_ts2cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbigint(ts2cell)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Ts2cell_to_tbigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbigint AS ts2cell) WITH FUNCTION ts2cell(tbigint) AS ASSIGNMENT;
CREATE CAST (ts2cell AS tbigint) WITH FUNCTION tbigint(ts2cell) AS ASSIGNMENT;

/******************************************************************************
 * Constructors
 *
 * Inherited Temporal<T> surface wired to the generic constructor symbols
 * (the value Datum carries the s2cell cell, dispatch is on the base oid).
 * `ts2cell` has step interpolation only, so the from-span and array
 * constructors take no interpolation argument / default to `'step'`, and a
 * cell has no scalar-distance metric so `ts2cellSeqSetGaps` exposes only the
 * time gap (the `tbigint` / `ttext` form, not the continuous `tcbuffer` one).
 ******************************************************************************/

CREATE FUNCTION ts2cell(s2cell, timestamptz)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tinstant_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ts2cell(s2cell, tstzset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ts2cell(s2cell, tstzspan)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ts2cell(s2cell, tstzspanset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tsequenceset_from_base_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The inclusivity parameters are spelled `lowerInc` / `upperInc`, the
-- spelling `022_temporal.in.sql` gives the base temporal type and the one
-- `th3index` and `tquadbin` give the sibling cell indexes. The spatial
-- families spell the same two parameters `lower_inc` / `upper_inc`, so a
-- named-argument call does not carry across families; that split is a
-- property of the published surface, not of this file.
CREATE FUNCTION ts2cellSeq(ts2cell[], text DEFAULT 'step',
    lowerInc boolean DEFAULT true, upperInc boolean DEFAULT true)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tsequence_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ts2cellSeqSet(ts2cell[])
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION ts2cellSeqSetGaps(ts2cell[], maxt interval DEFAULT NULL)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor_gaps'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/******************************************************************************
 * Transformations
 ******************************************************************************/

CREATE FUNCTION ts2cellInst(ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_as_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION ts2cellSeq(ts2cell, text DEFAULT NULL)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequence'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION ts2cellSeqSet(ts2cell, text DEFAULT NULL)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequenceset'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION setInterp(ts2cell, text)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_set_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendInstant(ts2cell, ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_append_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendSequence(ts2cell, ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_append_tsequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION merge(ts2cell, ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_merge'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION merge(ts2cell[])
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_merge_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTime(ts2cell, interval)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION scaleTime(ts2cell, interval)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftScaleTime(ts2cell, interval, interval)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tprecision(ts2cell, duration interval,
  origin timestamptz DEFAULT '2000-01-03')
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tsample(ts2cell, duration interval,
  origin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor functions
 *
 * The lifted cell operations live in `355_ts2cell_spatialfuncs.in.sql`.
 ******************************************************************************/

-- GENERATED-ACCESSORS-BEGIN s2cell — tools/codegen/inherited/generate.py from templates/accessors.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/accessor_families.yaml and re-run.

CREATE FUNCTION tempSubtype(ts2cell)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tempBasetype(ts2cell)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_basetype_name'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interp(ts2cell)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(ts2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- value is a reserved word in SQL
CREATE FUNCTION getValue(ts2cell)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Tinstant_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(ts2cell)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tinstant_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- values is a reserved word in SQL
CREATE FUNCTION getValues(ts2cell)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(ts2cell)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timeSpan is the bounding period, the tstzspan extent of the temporal value
CREATE FUNCTION timeSpan(ts2cell)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(ts2cell)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(ts2cell)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(ts2cell, int)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(ts2cell, timestamptz)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(ts2cell, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lowerInc(ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upperInc(ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(ts2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_start_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_end_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(ts2cell, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_instant_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(ts2cell)
  RETURNS ts2cell[]
  AS 'MODULE_PATHNAME', 'Temporal_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(ts2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(ts2cell)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_start_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(ts2cell)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_end_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(ts2cell, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_timestamptz_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(ts2cell)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Temporal_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(ts2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_start_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(ts2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_end_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(ts2cell, integer)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_sequence_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(ts2cell)
  RETURNS ts2cell[]
  AS 'MODULE_PATHNAME', 'Temporal_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION segments(ts2cell)
  RETURNS ts2cell[]
  AS 'MODULE_PATHNAME', 'Temporal_segments'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- GENERATED-ACCESSORS-END s2cell

-- The tstzspan cast is backed by the generated timeSpan accessor.
CREATE CAST (ts2cell AS tstzspan) WITH FUNCTION timeSpan(ts2cell);

/******************************************************************************
 * Unnest
 ******************************************************************************/

CREATE TYPE s2cell_tstzspanset AS (
  value s2cell,
  time tstzspanset
);

CREATE FUNCTION unnest(ts2cell)
  RETURNS SETOF s2cell_tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Restriction functions
 ******************************************************************************/

CREATE FUNCTION atValue(ts2cell, s2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_at_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValue(ts2cell, s2cell)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_minus_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValues(ts2cell, s2cellset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_at_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValues(ts2cell, s2cellset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_minus_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(ts2cell, timestamptz)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(ts2cell, timestamptz)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_minus_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(ts2cell, tstzset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(ts2cell, tstzset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(ts2cell, tstzspan)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(ts2cell, tstzspan)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(ts2cell, tstzspanset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(ts2cell, tstzspanset)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION beforeTimestamp(ts2cell, timestamptz, strict boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_before_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION afterTimestamp(ts2cell, timestamptz, strict boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_after_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Modification functions
 ******************************************************************************/

CREATE FUNCTION insert(ts2cell, ts2cell, connect boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION update(ts2cell, ts2cell, connect boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(ts2cell, timestamptz, connect boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_delete_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(ts2cell, tstzset, connect boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(ts2cell, tstzspan, connect boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(ts2cell, tstzspanset, connect boolean DEFAULT TRUE)
  RETURNS ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE time_ts2cell AS (
  time timestamptz,
  temp ts2cell
);

CREATE FUNCTION timeSplit(ts2cell, bin_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_ts2cell
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree / hash indexing
 *
 * All ts2cell values share the exact same on-disk layout as every
 * other temporal type, so the generic Temporal_* dispatch in the
 * backend is enough — no ts2cell-specific wrappers required.
 ******************************************************************************/

CREATE FUNCTION lt(ts2cell, ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(ts2cell, ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eq(ts2cell, ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(ts2cell, ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(ts2cell, ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(ts2cell, ts2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(ts2cell, ts2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = ts2cell, RIGHTARG = ts2cell,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = ts2cell, RIGHTARG = ts2cell,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR = (
  LEFTARG = ts2cell, RIGHTARG = ts2cell,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = ts2cell, RIGHTARG = ts2cell,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = ts2cell, RIGHTARG = ts2cell,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR > (
  LEFTARG = ts2cell, RIGHTARG = ts2cell,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE OPERATOR CLASS ts2cell_btree_ops
  DEFAULT FOR TYPE ts2cell USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 cmp(ts2cell, ts2cell);

/******************************************************************************/

CREATE FUNCTION hash(ts2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hashExtended(ts2cell, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Temporal_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS ts2cell_hash_ops
  DEFAULT FOR TYPE ts2cell USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash(ts2cell),
    FUNCTION    2   hashExtended(ts2cell, bigint);

/******************************************************************************/

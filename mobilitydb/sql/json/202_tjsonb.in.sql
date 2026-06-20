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
 * @brief Basic functions for temporal JSONB
 */

CREATE TYPE tjsonb;

/*****************************************************************************
 * Input/Output
 *****************************************************************************/

CREATE FUNCTION tjsonb_in(cstring, oid, integer)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tjsonb)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb_recv(internal, oid, integer)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(tjsonb)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tjsonb (
  internallength = variable,
  input = tjsonb_in,
  output = temporal_out,
  send = temporal_send,
  receive = tjsonb_recv,
  typmod_in = temporal_typmod_in,
  typmod_out = temporal_typmod_out,
  storage = extended,
  alignment = double,
  analyze = temporal_analyze
);

-- Special cast for enforcing the typmod restrictions
CREATE FUNCTION tjsonb(tjsonb, integer)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tjsonb AS tjsonb) WITH FUNCTION tjsonb(tjsonb, integer) AS IMPLICIT;

/*****************************************************************************
 * Input/output from WKT, WKB, HexWKB, and MFJSON representation
 *****************************************************************************/

CREATE FUNCTION tjsonbFromText(text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (text AS tjsonb) WITH FUNCTION tjsonbFromText(text);

CREATE FUNCTION tjsonbFromBinary(bytea)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbFromHexWKB(text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbFromMFJSON(text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION asText(tjsonb)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tjsonb[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Temporalarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE CAST (jsonb AS text) WITH FUNCTION asText(jsonb);
CREATE CAST (tjsonb AS text) WITH FUNCTION asText(tjsonb);

CREATE FUNCTION asMFJSON(temp tjsonb, options int4 DEFAULT 0, flags int4 DEFAULT 0)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tjsonb, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(tjsonb, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Constructors
 *****************************************************************************/

CREATE FUNCTION tjsonb(jsonb, timestamptz)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tinstant_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb(jsonb, tstzset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb(jsonb, tstzspan)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonb(jsonb, tstzspanset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tsequenceset_from_base_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
  
/*****************************************************************************/

CREATE FUNCTION tjsonbSeq(tjsonb[], text DEFAULT 'step',
    lowerInc boolean DEFAULT true, upperInc boolean DEFAULT true)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tsequence_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tjsonbSeqSet(tjsonb[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The function is not strict
CREATE FUNCTION tjsonbSeqSetGaps(tjsonb[], maxt interval DEFAULT NULL)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor_gaps'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * Conversions
 *****************************************************************************/

CREATE FUNCTION timeSpan(tjsonb)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tjsonb AS tstzspan) WITH FUNCTION timeSpan(tjsonb);

CREATE FUNCTION ttext(tjsonb)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Tjsonb_as_ttext'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tjsonb(ttext)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Ttext_as_tjsonb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tjsonb AS ttext) WITH FUNCTION ttext(tjsonb);
CREATE CAST (ttext AS tjsonb) WITH FUNCTION tjsonb(ttext);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

-- Accessors for all temporal types

CREATE FUNCTION tempSubtype(tjsonb)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interp(tjsonb)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(tjsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- value is a reserved word in SQL
CREATE FUNCTION getValue(tjsonb)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Tinstant_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(tjsonb)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tinstant_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- values is a reserved word in SQL
CREATE FUNCTION getValues(tjsonb)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(tjsonb)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(tjsonb)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(tjsonb)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(tjsonb, integer)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(tjsonb, timestamptz)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tjsonb, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lowerInc(tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upperInc(tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(tjsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_start_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_end_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(tjsonb, integer)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_instant_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(tjsonb)
  RETURNS tjsonb[]
  AS 'MODULE_PATHNAME', 'Temporal_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(tjsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(tjsonb)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_start_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(tjsonb)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_end_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(tjsonb, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_timestamptz_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(tjsonb)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Temporal_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(tjsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_start_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_end_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(tjsonb, integer)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_sequence_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(tjsonb)
  RETURNS tjsonb[]
  AS 'MODULE_PATHNAME', 'Temporal_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION segments(tjsonb)
  RETURNS tjsonb[]
  AS 'MODULE_PATHNAME', 'Temporal_segments'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/
  
-- Transformations for all temporal types

CREATE FUNCTION tjsonbInst(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_to_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tjsonbSeq(tjsonb, text DEFAULT NULL)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_to_tsequence'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tjsonbSeqSet(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_to_tsequenceset'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION setInterp(tjsonb, text)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_set_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTime(tjsonb, interval)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scaleTime(tjsonb, interval)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftScaleTime(tjsonb, interval, interval)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tsample(tjsonb, duration interval,
  origin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

CREATE FUNCTION atValues(tjsonb, jsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValues(tjsonb, jsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atValues(tjsonb, jsonbset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValues(tjsonb, jsonbset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atMin(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_min'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusMin(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_min'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atMax(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_max'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusMax(tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_max'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tjsonb, timestamptz)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tjsonb, timestamptz)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tjsonb, tstzset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tjsonb, tstzset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tjsonb, tstzspan)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tjsonb, tstzspan)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tjsonb, tstzspanset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tjsonb, tstzspanset)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION beforeTimestamp(tjsonb, timestamptz, strict bool DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_before_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION afterTimestamp(tjsonb, timestamptz, strict bool DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_after_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Unnest Function
 *****************************************************************************/

CREATE TYPE jsonb_tstzspanset AS (
  value jsonb,
  time  tstzspanset
);

CREATE FUNCTION unnest(tjsonb)
  RETURNS SETOF jsonb_tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Stops Function
 *****************************************************************************/

-- CREATE FUNCTION stops(tjsonb, minduration interval DEFAULT '0 minutes')
  -- RETURNS tjsonb
  -- AS 'MODULE_PATHNAME', 'Temporal_stops'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modification Functions
 *****************************************************************************/

CREATE FUNCTION insert(tjsonb, tjsonb, connect boolean DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION update(tjsonb, tjsonb, connect boolean DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tjsonb, timestamptz, connect boolean DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_delete_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tjsonb, tstzset, connect boolean DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tjsonb, tstzspan, connect boolean DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tjsonb, tstzspanset, connect boolean DEFAULT TRUE)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendInstant(tjsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_append_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendSequence(tjsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_append_tsequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The function is not strict
CREATE FUNCTION merge(tjsonb, tjsonb)
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_merge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION merge(tjsonb[])
  RETURNS tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_merge_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Multidimensional tiling
 *****************************************************************************/

CREATE TYPE time_tjsonb AS (
  time timestamptz,
  temp tjsonb
);

CREATE FUNCTION timeSplit(tjsonb, bin_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tjsonb
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

-- spans()
CREATE FUNCTION spans(tjsonb)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- splitNSpans()
CREATE FUNCTION splitNSpans(tjsonb, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- splitEachNSpans()
CREATE FUNCTION splitEachNSpans(tjsonb, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Comparison functions and B-tree indexing
 *****************************************************************************/

CREATE FUNCTION temporal_lt(tjsonb, tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tjsonb, tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tjsonb, tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tjsonb, tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tjsonb, tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tjsonb, tjsonb)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_cmp(tjsonb, tjsonb)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
  LEFTARG = tjsonb, RIGHTARG = tjsonb,
  PROCEDURE = temporal_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tjsonb_btree_ops
  DEFAULT FOR TYPE tjsonb USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 temporal_cmp(tjsonb, tjsonb);

/*****************************************************************************/

CREATE FUNCTION temporal_hash(tjsonb)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tjsonb_hash_ops
  DEFAULT FOR TYPE tjsonb USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(tjsonb);

/*****************************************************************************/


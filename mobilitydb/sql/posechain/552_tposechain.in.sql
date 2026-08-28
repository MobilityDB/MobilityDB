/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Basic functions for temporal pose chains
 */

-- GENERATED-IO-BEGIN posechain — tools/codegen/inherited/generate.py from templates/comparisons.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/io_families.yaml and re-run.
CREATE TYPE tposechain;

/******************************************************************************
 * Input/output
 ******************************************************************************/

CREATE FUNCTION tposechain_in(cstring, oid, integer)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tposechain_in'
  LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION temporal_out(tposechain)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION tposechain_recv(internal, oid, integer)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION temporal_send(tposechain)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tposechain_typmod_in(cstring[])
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tposechain_typmod_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tposechain (
  internallength = variable,
  input = tposechain_in,
  output = temporal_out,
  send = temporal_send,
  receive = tposechain_recv,
  typmod_in = tposechain_typmod_in,
  typmod_out = tspatial_typmod_out,
  storage = extended,
  alignment = double,
  analyze = tspatial_analyze
);

-- Special cast for enforcing the typmod restrictions
CREATE FUNCTION tposechain(tposechain, integer)
  RETURNS tposechain
  AS 'MODULE_PATHNAME','Tspatial_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tposechain AS tposechain) WITH FUNCTION tposechain(tposechain, integer) AS IMPLICIT;

-- GENERATED-IO-END posechain

  -- GENERATED-REPRESENTATIONS-BEGIN posechain — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/*****************************************************************************
   * Input/output from (E)WKT, (E)WKB, HexEWKB, and MFJSON representation
   *****************************************************************************/

CREATE FUNCTION tposechainFromText(text)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tspatial_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechainFromEWKT(text)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tspatial_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechainFromMFJSON(text)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechainFromGeoPose(text)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tposechain_from_geopose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- A Composite Chain document carries one valid time, so it is written
-- from a single instant, and its frame chain holds at least two frames.
-- maxdecimaldigits: significant digits to keep; -1 = lossless
CREATE FUNCTION asGeoPose(tposechain, maxdecimaldigits integer DEFAULT -1)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tposechain_as_geopose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- A Composite Graph document is a set of pose chains sharing their outermost
-- frame, so it is written from chains read at one and the same instant, and
-- its frame list holds at least two frames.
CREATE FUNCTION asGeoPose(tposechain[], maxdecimaldigits integer DEFAULT -1)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tposechainarr_as_geopose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechainFromBinary(bytea)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechainFromEWKB(bytea)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechainFromHexEWKB(text)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION asText(tposechain, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tposechain[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tposechain, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tposechain[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(tposechain, options integer DEFAULT 0,
    flags integer DEFAULT 0, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tposechain, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(tposechain, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tspatial_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(tposechain, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(tposechain, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tspatial_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-END posechain

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION tposechain(posechain, timestamptz)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tinstant_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechain(posechain, tstzset)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechain(posechain, tstzspan, text DEFAULT 'linear')
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechain(posechain, tstzspanset, text DEFAULT 'linear')
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tsequenceset_from_base_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE FUNCTION tposechainSeq(tposechain[], text DEFAULT 'linear',
    lower_inc boolean DEFAULT true, upper_inc boolean DEFAULT true)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tsequence_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tposechainSeqSet(tposechain[])
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The function is not strict
CREATE FUNCTION tposechainSeqSetGaps(tposechain[], maxt interval DEFAULT NULL,
    maxdist float DEFAULT NULL, text DEFAULT 'step')
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor_gaps'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/******************************************************************************
 * Conversions
 ******************************************************************************/

CREATE FUNCTION tpose(tposechain)
  RETURNS tpose
  AS 'MODULE_PATHNAME', 'Tposechain_to_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tposechain AS tpose) WITH FUNCTION tpose(tposechain);

/******************************************************************************
 * Accessor functions
 ******************************************************************************/
-- Specific accessors for temporal pose chains

CREATE FUNCTION numPoses(tposechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tposechain_num_poses'
  LANGUAGE C IMMUTABLE STRICT;

/******************************************************************************/
-- Accessors for all temporal types

-- GENERATED-ACCESSORS-BEGIN posechain — tools/codegen/inherited/generate.py from templates/accessors.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/accessor_families.yaml and re-run.

CREATE FUNCTION tempSubtype(tposechain)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tempBasetype(tposechain)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_basetype_name'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interp(tposechain)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(tposechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- value is a reserved word in SQL
CREATE FUNCTION getValue(tposechain)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Tinstant_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(tposechain)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tinstant_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- values is a reserved word in SQL
CREATE FUNCTION getValues(tposechain)
  RETURNS posechainset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(tposechain)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timeSpan is the bounding period, the tstzspan extent of the temporal value
CREATE FUNCTION timeSpan(tposechain)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(tposechain)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(tposechain)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(tposechain, int)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(tposechain, timestamptz)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tposechain, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lowerInc(tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upperInc(tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(tposechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_start_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_end_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(tposechain, integer)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_instant_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(tposechain)
  RETURNS tposechain[]
  AS 'MODULE_PATHNAME', 'Temporal_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(tposechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(tposechain)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_start_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(tposechain)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_end_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(tposechain, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_timestamptz_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(tposechain)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Temporal_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(tposechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_start_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_end_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(tposechain, integer)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_sequence_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(tposechain)
  RETURNS tposechain[]
  AS 'MODULE_PATHNAME', 'Temporal_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION segments(tposechain)
  RETURNS tposechain[]
  AS 'MODULE_PATHNAME', 'Temporal_segments'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- GENERATED-ACCESSORS-END posechain

-- The tstzspan cast is backed by the generated timeSpan accessor.
CREATE CAST (tposechain AS tstzspan) WITH FUNCTION timeSpan(tposechain);

/******************************************************************************
 * Transformation functions
 ******************************************************************************/

CREATE FUNCTION tposechainInst(tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_as_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tposechainSeq(tposechain, text DEFAULT NULL)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequence'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tposechainSeqSet(tposechain, text DEFAULT NULL)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequenceset'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION setInterp(tposechain, text)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_set_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION round(tposechain, integer DEFAULT 0)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION round(tposechain[], integer DEFAULT 0)
  RETURNS tposechain[]
  AS 'MODULE_PATHNAME', 'Temporalarr_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tsample(tposechain, duration interval,
  origin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tprecision(tposechain, duration interval,
  origin timestamptz DEFAULT '2000-01-03')
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTime(tposechain, interval)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scaleTime(tposechain, interval)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftScaleTime(tposechain, interval, interval)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

CREATE FUNCTION atValue(tposechain, posechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_at_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValue(tposechain, posechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_minus_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atValues(tposechain, posechainset)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_at_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValues(tposechain, posechainset)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_minus_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tposechain, timestamptz)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tposechain, timestamptz)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_minus_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tposechain, tstzset)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tposechain, tstzset)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tposechain, tstzspan)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tposechain, tstzspan)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tposechain, tstzspanset)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tposechain, tstzspanset)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION beforeTimestamp(tposechain, timestamptz, strict boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_before_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION afterTimestamp(tposechain, timestamptz, strict boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_after_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Unnest Function
 *****************************************************************************/

CREATE TYPE posechain_tstzspanset AS (
  value posechain,
  time tstzspanset
);

CREATE FUNCTION unnest(tposechain)
  RETURNS SETOF posechain_tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Stop Function
 *****************************************************************************/

CREATE FUNCTION stops(tposechain, maxdist float DEFAULT 0.0,
    minduration interval DEFAULT '0 minutes')
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_stops'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modification Functions
 *****************************************************************************/

CREATE FUNCTION insert(tposechain, tposechain, connect boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION update(tposechain, tposechain, connect boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tposechain, timestamptz, connect boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_delete_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tposechain, tstzset, connect boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tposechain, tstzspan, connect boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tposechain, tstzspanset, connect boolean DEFAULT TRUE)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendInstant(tposechain, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_append_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendSequence(tposechain, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_append_tsequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The function is not strict
CREATE FUNCTION merge(tposechain, tposechain)
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_merge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION merge(tposechain[])
  RETURNS tposechain
  AS 'MODULE_PATHNAME', 'Temporal_merge_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Multidimensional tiling
 ******************************************************************************/

CREATE TYPE time_tposechain AS (
  time timestamptz,
  temp tposechain
);

CREATE FUNCTION timeSplit(tposechain, duration interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tposechain
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION lt(tposechain, tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(tposechain, tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eq(tposechain, tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(tposechain, tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(tposechain, tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(tposechain, tposechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(tposechain, tposechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = tposechain, RIGHTARG = tposechain,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = tposechain, RIGHTARG = tposechain,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR = (
  LEFTARG = tposechain, RIGHTARG = tposechain,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tposechain, RIGHTARG = tposechain,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = tposechain, RIGHTARG = tposechain,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR > (
  LEFTARG = tposechain, RIGHTARG = tposechain,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE OPERATOR CLASS tposechain_btree_ops
  DEFAULT FOR TYPE tposechain USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 cmp(tposechain, tposechain);

/******************************************************************************/

CREATE FUNCTION hash(tposechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hashExtended(tposechain, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Temporal_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tposechain_hash_ops
  DEFAULT FOR TYPE tposechain USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash(tposechain),
    FUNCTION    2   hashExtended(tposechain, bigint);

/******************************************************************************/

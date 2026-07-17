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
 * @brief Basic functions for temporal rigid geometries
 */

CREATE TYPE trgeometry;

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION trgeometry_in(cstring, oid, integer)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_in'
  LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION trgeometry_out(trgeometry)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Trgeometry_out'
  LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION trgeometry_recv(internal)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_recv'
  LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION trgeometry_send(trgeometry)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Trgeometry_send'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION trgeo_typmod_in(cstring[])
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Trgeometry_typmod_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE trgeometry (
  internallength = variable,
  input = trgeometry_in,
  output = trgeometry_out,
  receive = trgeometry_recv,
  send = trgeometry_send,
  typmod_in = trgeo_typmod_in,
  typmod_out = tspatial_typmod_out,
  storage = extended,
  alignment = double,
  analyze = tspatial_analyze
);

-- Special cast for enforcing the typmod restrictions
CREATE FUNCTION trgeometry(trgeometry, integer)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME','Tspatial_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (trgeometry AS trgeometry) WITH FUNCTION trgeometry(trgeometry, integer) AS IMPLICIT;

/*****************************************************************************
 * Input/output from (E)WKT, (E)WKB, HexEWKB, and MFJSON representation
 *****************************************************************************/

CREATE FUNCTION trgeometryFromText(text)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trgeometryFromEWKT(text)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trgeometryFromMFJSON(text)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trgeometryFromBinary(bytea)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trgeometryFromEWKB(bytea)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trgeometryFromHexEWKB(text)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE FUNCTION asText(trgeometry, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Trgeometry_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(trgeometry[], maxdecimaldigits int4 DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(trgeometry, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Trgeometry_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(trgeometry[], maxdecimaldigits int4 DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asMFJSON(trgeometry, options int4 DEFAULT 0,
    flags int4 DEFAULT 0, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(trgeometry, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(trgeometry, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tspatial_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(trgeometry, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION trgeometry(geometry, pose, timestamptz)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_inst_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trgeometrySeq(trgeometry[], text DEFAULT 'linear',
    lower_inc boolean DEFAULT true, upper_inc boolean DEFAULT true)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_seq_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trgeometrySeqSet(trgeometry[])
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_seqset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The function is not strict
CREATE FUNCTION trgeometrySeqSetGaps(trgeometry[], maxt interval DEFAULT NULL,
    maxdist float DEFAULT NULL, text DEFAULT 'linear')
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_seqset_constructor_gaps'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION trgeometry(geometry, tpose)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Conversion functions
 ******************************************************************************/

CREATE FUNCTION tpose(trgeometry)
  RETURNS tpose
  AS 'MODULE_PATHNAME', 'Trgeometry_to_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint(trgeometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Trgeometry_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint(trgeometry)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Trgeometry_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeometry(trgeometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_to_tgeometry'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (trgeometry AS tpose) WITH FUNCTION tpose(trgeometry);
CREATE CAST (trgeometry AS tgeompoint) WITH FUNCTION tgeompoint(trgeometry);
CREATE CAST (trgeometry AS tgeogpoint) WITH FUNCTION tgeogpoint(trgeometry);
CREATE CAST (trgeometry AS tgeometry) WITH FUNCTION tgeometry(trgeometry);

CREATE FUNCTION stbox(trgeometry)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tspatial_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (trgeometry AS stbox) WITH FUNCTION stbox(trgeometry);

CREATE FUNCTION expandSpace(trgeometry, float)
  RETURNS stbox
  AS 'SELECT @extschema@.expandSpace($1::stbox, $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION geometry(trgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Trgeometry_to_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (trgeometry AS geometry) WITH FUNCTION geometry(trgeometry);

/******************************************************************************
 * Accessor Functions
 ******************************************************************************/
-- Specific accessors for temporal rigid geometries

CREATE FUNCTION points(trgeometry)
  RETURNS geomset
  AS 'MODULE_PATHNAME', 'Trgeometry_points'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION rotation(trgeometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Trgeometry_rotation'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
-- Accessors for all temporal types

-- GENERATED-ACCESSORS-BEGIN rgeo — tools/codegen/inherited/generate.py from templates/accessors.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.yaml (accessor_families) and re-run.

CREATE FUNCTION tempSubtype(trgeometry)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tempBasetype(trgeometry)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_basetype_name'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interp(trgeometry)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(trgeometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- value is a reserved word in SQL
CREATE FUNCTION getValue(trgeometry)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Tinstant_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(trgeometry)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tinstant_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- values is a reserved word in SQL
CREATE FUNCTION getValues(trgeometry)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(trgeometry)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timeSpan is the bounding period, the tstzspan extent of the temporal value
CREATE FUNCTION timeSpan(trgeometry)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(trgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Trgeometry_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(trgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Trgeometry_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(trgeometry, int)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Trgeometry_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(trgeometry, timestamptz)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Trgeometry_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(trgeometry, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lowerInc(trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upperInc(trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(trgeometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_start_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_end_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(trgeometry, integer)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_instant_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(trgeometry)
  RETURNS trgeometry[]
  AS 'MODULE_PATHNAME', 'Temporal_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(trgeometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(trgeometry)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_start_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(trgeometry)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_end_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(trgeometry, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_timestamptz_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(trgeometry)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Temporal_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(trgeometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_start_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_end_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(trgeometry, integer)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_sequence_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(trgeometry)
  RETURNS trgeometry[]
  AS 'MODULE_PATHNAME', 'Temporal_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION segments(trgeometry)
  RETURNS trgeometry[]
  AS 'MODULE_PATHNAME', 'Temporal_segments'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- GENERATED-ACCESSORS-END rgeo

-- The tstzspan cast is backed by the generated timeSpan accessor.
CREATE CAST (trgeometry AS tstzspan) WITH FUNCTION timeSpan(trgeometry);

/******************************************************************************
 * Transformation functions
 ******************************************************************************/

CREATE FUNCTION trgeometryInst(trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_as_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION trgeometrySeq(trgeometry, text DEFAULT NULL)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_as_tsequence'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION trgeometrySeqSet(trgeometry, text DEFAULT NULL)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Trgeometry_as_tsequenceset'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION setInterp(trgeometry, text)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_set_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION round(trgeometry, integer DEFAULT 0)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION round(trgeometry[], integer DEFAULT 0)
  RETURNS trgeometry[]
  AS 'MODULE_PATHNAME', 'Temporalarr_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTime(trgeometry, interval)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scaleTime(trgeometry, interval)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftScaleTime(trgeometry, interval, interval)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tprecision(trgeometry, duration interval,
  origin timestamptz DEFAULT '2000-01-03')
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_tprecision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tsample(trgeometry, duration interval,
--   origin timestamptz DEFAULT '2000-01-03')
--   RETURNS trgeometry
--   AS 'MODULE_PATHNAME', 'Temporal_tsample'
--   LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

-- CREATE FUNCTION atValues(trgeometry, geometry)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Temporal_at_value'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION minusValues(trgeometry, geometry)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Temporal_minus_value'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION atValues(trgeometry, geomset)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Temporal_at_values'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION minusValues(trgeometry, geomset)
  -- RETURNS trgeometry
  -- AS 'MODULE_PATHNAME', 'Temporal_minus_values'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(trgeometry, timestamptz)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(trgeometry, timestamptz)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_minus_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(trgeometry, tstzset)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(trgeometry, tstzset)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(trgeometry, tstzspan)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(trgeometry, tstzspan)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(trgeometry, tstzspanset)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(trgeometry, tstzspanset)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION beforeTimestamp(trgeometry, timestamptz, strict bool DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_before_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION afterTimestamp(trgeometry, timestamptz, strict bool DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_after_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Unnest Function
 *****************************************************************************/

CREATE FUNCTION unnest(trgeometry)
  RETURNS SETOF geom_tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Stops Function
 *****************************************************************************/

CREATE FUNCTION stops(trgeometry, maxdist float DEFAULT 0.0,
    minduration interval DEFAULT '0 minutes')
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_stops'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modification Functions
 *****************************************************************************/

CREATE FUNCTION insert(trgeometry, trgeometry, connect boolean DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION update(trgeometry, trgeometry, connect boolean DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(trgeometry, timestamptz, connect boolean DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_delete_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(trgeometry, tstzset, connect boolean DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(trgeometry, tstzspan, connect boolean DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(trgeometry, tstzspanset, connect boolean DEFAULT TRUE)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendInstant(trgeometry, trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_append_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendSequence(trgeometry, trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_append_tsequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The function is not strict
CREATE FUNCTION merge(trgeometry, trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_merge'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION merge(trgeometry[])
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_merge_array'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Multidimensional tiling
 ******************************************************************************/

CREATE TYPE time_trgeometry AS (
  time timestamptz,
  temp trgeometry
);

CREATE FUNCTION timeSplit(trgeometry, bucket_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_trgeometry
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION lt(trgeometry, trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(trgeometry, trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eq(trgeometry, trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(trgeometry, trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(trgeometry, trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(trgeometry, trgeometry)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(trgeometry, trgeometry)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS trgeometry_btree_ops
  DEFAULT FOR TYPE trgeometry USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(trgeometry, trgeometry);

/******************************************************************************/

CREATE FUNCTION temporal_hash(trgeometry)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS trgeometry_hash_ops
  DEFAULT FOR TYPE trgeometry USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(trgeometry);

/******************************************************************************/

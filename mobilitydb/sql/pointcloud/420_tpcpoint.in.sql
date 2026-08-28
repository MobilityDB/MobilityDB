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
 * @brief Temporal pgpointcloud point type.
 *
 * `tpcpoint` is a temporal lifting of `pcpoint` — a single moving
 * pgpointcloud point (LiDAR sensor, GPS fix with per-reading metadata,
 * etc.) whose value at each instant is the full schema-typed pcpoint.
 *
 * Bindings delegate almost entirely to the generic `Temporal_*` PG
 * wrappers in `mobilitydb/src/temporal/temporal.c`. The default
 * interpolation is STEP (not LINEAR) because a pcpoint carries
 * heterogeneous dimensions (Intensity, ReturnNumber, Classification, …)
 * that do not interpolate linearly; explicit per-dimension linear
 * interpolation can be layered on top via `getDim(pcpoint, name)`.
 *
 * Type registration, constructors, generic accessors, the schema-aware
 * `pcid(tpcpoint)` getter, per-dimension projections to `tfloat`, and
 * the `tgeompoint(tpcpoint)` XY projection cast are all included.
 */

-- GENERATED-IO-BEGIN pointcloud — tools/codegen/inherited/generate.py from templates/comparisons.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/io_families.yaml and re-run.
CREATE TYPE tpcpoint;

/******************************************************************************
 * Input / output — all via the generic Temporal_* wrappers
 ******************************************************************************/

CREATE FUNCTION tpcpoint_in(cstring, oid, integer)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tpcpoint)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcpoint_recv(internal, oid, integer)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(tpcpoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpc_typmod_in(cstring[])
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tpc_typmod_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpc_typmod_out(integer)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Tpc_typmod_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tpcpoint (
  internallength = variable,
  input = tpcpoint_in,
  output = temporal_out,
  send = temporal_send,
  receive = tpcpoint_recv,
  typmod_in = tpc_typmod_in,
  typmod_out = tpc_typmod_out,
  storage = extended,
  alignment = double,
  analyze = tspatial_analyze
);

-- Special cast for enforcing the typmod restriction on INSERT / cast.
CREATE FUNCTION tpcpoint(tpcpoint, integer)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tpc_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE CAST (tpcpoint AS tpcpoint)
  WITH FUNCTION tpcpoint(tpcpoint, integer) AS IMPLICIT;

-- GENERATED-IO-END pointcloud

-- GENERATED-REPRESENTATIONS-BEGIN pointcloud — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/******************************************************************************
 * WKB / HexWKB helpers
 ******************************************************************************/

CREATE FUNCTION tpcpointFromBinary(bytea)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcpointFromHexWKB(text)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tpcpoint, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tpcpoint, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- asMFJSON is output-only: the JSON form summarises a tpcpoint by
-- (coordinates, datetimes) without carrying the schema (pcid + dim
-- layout) needed to reconstruct the value, so a parse-back path is
-- intentionally not exposed. Use asBinary / tpcpointFromBinary or
-- asHexWKB / tpcpointFromHexWKB for round-trip.
CREATE FUNCTION asMFJSON(tpcpoint, options integer DEFAULT 0,
    flags integer DEFAULT 0, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_mfjson'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- asText takes no maxdecimaldigits argument: a pcpoint prints as the hex-WKB
-- of its serialized form, which has no decimal places to round. This mirrors
-- th3index, the other spatial temporal type whose base value has no
-- coordinate text form.
CREATE FUNCTION asText(tpcpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tpcpoint[])
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Temporalarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tpcpoint AS text) WITH FUNCTION asText(tpcpoint);

-- GENERATED-REPRESENTATIONS-END pointcloud

/******************************************************************************
 * pcpoint constructors
 *
 * pgPointCloud's `pcpoint_in` only accepts hex-WKB, so a coordinate is given
 * to a constructor instead. The schema the pcid names is resolved through the
 * MEOS cache, which reads the pointcloud_schemas and pointcloud_dimensions
 * tables and falls back to the XML document pgPointCloud's own catalog
 * carries, so a schema stated either way builds a value.
 *
 * The number of coordinates must be the number of dimensions the schema
 * states, which the constructor enforces.
 ******************************************************************************/

CREATE FUNCTION pcpoint(pcid integer, x double precision, y double precision)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Pcpoint_make'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pcpoint(pcid integer, x double precision, y double precision,
    z double precision)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Pcpoint_make'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* The coordinates of every dimension of the schema's LAYOUT, inactive
 * dimensions included, which is how a schema of more dimensions than the
 * overloads above reach is given its values. The shape is the one
 * pgPointCloud's own PC_MakePoint takes, and that count is NOT the count of
 * ACTIVE dimensions pointCloudSchemaNDims reports. */
CREATE FUNCTION pcpoint(pcid integer, coordinates double precision[])
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Pcpoint_make_coords'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION tpcpoint(pcpoint, timestamptz)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tinstant_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcpoint(pcpoint, tstzset)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Interpolation default 'step' to match tpcpoint's natural semantics.
CREATE FUNCTION tpcpoint(pcpoint, tstzspan, text DEFAULT 'step')
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tsequence_from_base_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcpoint(pcpoint, tstzspanset, text DEFAULT 'step')
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tsequenceset_from_base_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcpointSeq(tpcpoint[], text DEFAULT 'step',
    lower_inc boolean DEFAULT true, upper_inc boolean DEFAULT true)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tsequence_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcpointSeqSet(tpcpoint[])
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tsequenceset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessors — generic Temporal_* wrappers where possible
 ******************************************************************************/

-- GENERATED-ACCESSORS-BEGIN pointcloud — tools/codegen/inherited/generate.py from templates/accessors.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/accessor_families.yaml and re-run.

CREATE FUNCTION tempSubtype(tpcpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tempBasetype(tpcpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_basetype_name'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interp(tpcpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- value is a reserved word in SQL
CREATE FUNCTION getValue(tpcpoint)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Tinstant_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(tpcpoint)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tinstant_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- values is a reserved word in SQL
CREATE FUNCTION getValues(tpcpoint)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(tpcpoint)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timeSpan is the bounding period, the tstzspan extent of the temporal value
CREATE FUNCTION timeSpan(tpcpoint)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(tpcpoint)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(tpcpoint)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(tpcpoint, int)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(tpcpoint, timestamptz)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tpcpoint, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lowerInc(tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upperInc(tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_start_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_end_instant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(tpcpoint, integer)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_instant_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(tpcpoint)
  RETURNS tpcpoint[]
  AS 'MODULE_PATHNAME', 'Temporal_instants'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(tpcpoint)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_start_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(tpcpoint)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_end_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(tpcpoint, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Temporal_timestamptz_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(tpcpoint)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Temporal_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_start_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_end_sequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(tpcpoint, integer)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_sequence_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(tpcpoint)
  RETURNS tpcpoint[]
  AS 'MODULE_PATHNAME', 'Temporal_sequences'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION segments(tpcpoint)
  RETURNS tpcpoint[]
  AS 'MODULE_PATHNAME', 'Temporal_segments'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- GENERATED-ACCESSORS-END pointcloud

-- Type-specific accessor
CREATE FUNCTION pcid(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tpcpoint_pcid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Per-dimension projection to tfloat; the projection reads the schema cache.
CREATE FUNCTION getX(tpcpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_x'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getY(tpcpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_y'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getZ(tpcpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_z'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getDim(tpcpoint, text)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_dim'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- XY projection to tgeompoint. Step interpolation on the source
-- tpcpoint is promoted to linear in the output: the projected XY
-- trajectory is physically a sensor path, where linear interp
-- between consecutive fixes is the natural default.
CREATE FUNCTION tgeompoint(tpcpoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpcpoint_to_tgeompoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tpcpoint AS tgeompoint) WITH FUNCTION tgeompoint(tpcpoint);

-- The tstzspan cast is backed by the generated timeSpan accessor.
CREATE CAST (tpcpoint AS tstzspan) WITH FUNCTION timeSpan(tpcpoint);

/******************************************************************************
 * Transformation functions
 ******************************************************************************/

CREATE FUNCTION tpcpointInst(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_as_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tpcpointSeq(tpcpoint, text DEFAULT NULL)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequence'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- The function is not strict
CREATE FUNCTION tpcpointSeqSet(tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_as_tsequenceset'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION setInterp(tpcpoint, text)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_set_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTime(tpcpoint, interval)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scaleTime(tpcpoint, interval)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftScaleTime(tpcpoint, interval, interval)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tsample(tpcpoint, duration interval,
  origin timestamptz DEFAULT '2000-01-03', interp text DEFAULT 'discrete')
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_tsample'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Restrictions
 ******************************************************************************/

CREATE FUNCTION atValue(tpcpoint, pcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_at_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValue(tpcpoint, pcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_minus_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atValues(tpcpoint, pcpointset)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_at_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValues(tpcpoint, pcpointset)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_minus_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTime(tpcpoint, timestamptz)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_at_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(tpcpoint, tstzset)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(tpcpoint, tstzspan)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTime(tpcpoint, tstzspanset)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_at_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTime(tpcpoint, timestamptz)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_minus_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(tpcpoint, tstzset)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(tpcpoint, tstzspan)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTime(tpcpoint, tstzspanset)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_minus_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION beforeTimestamp(tpcpoint, timestamptz,
    strict boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_before_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION afterTimestamp(tpcpoint, timestamptz,
    strict boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_after_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * TPCBox-based restrictions
 ******************************************************************************/

CREATE FUNCTION atTpcbox(tpcpoint, tpcbox, border_inc boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tpcpoint_at_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTpcbox(tpcpoint, tpcbox, border_inc boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Tpcpoint_minus_tpcbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Modification functions
 ******************************************************************************/

CREATE FUNCTION insert(tpcpoint, tpcpoint, connect boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_insert'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION update(tpcpoint, tpcpoint, connect boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_update'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION deleteTime(tpcpoint, timestamptz, connect boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_delete_timestamptz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(tpcpoint, tstzset, connect boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(tpcpoint, tstzspan, connect boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION deleteTime(tpcpoint, tstzspanset, connect boolean DEFAULT TRUE)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_delete_tstzspanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendInstant(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_tinstant'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendSequence(tpcpoint, tpcpoint)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_append_tsequence'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Unnest
 ******************************************************************************/

CREATE TYPE pcpoint_tstzspanset AS (
  value pcpoint,
  time tstzspanset
);

CREATE FUNCTION unnest(tpcpoint)
  RETURNS SETOF pcpoint_tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Multidimensional tiling
 ******************************************************************************/

CREATE TYPE time_tpcpoint AS (
  time timestamptz,
  temp tpcpoint
);

CREATE FUNCTION timeSplit(tpcpoint, duration interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION lt(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eq(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(tpcpoint, tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR = (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR > (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE OPERATOR CLASS tpcpoint_btree_ops
  DEFAULT FOR TYPE tpcpoint USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 cmp(tpcpoint, tpcpoint);

/******************************************************************************/

CREATE FUNCTION hash(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hashExtended(tpcpoint, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Temporal_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tpcpoint_hash_ops
  DEFAULT FOR TYPE tpcpoint USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash(tpcpoint),
    FUNCTION    2   hashExtended(tpcpoint, bigint);

/*****************************************************************************/

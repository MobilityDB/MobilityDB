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

CREATE TYPE tpcpoint;

/******************************************************************************
 * Input / output — all via the generic Temporal_* wrappers
 ******************************************************************************/

CREATE FUNCTION tpcpoint_in(cstring, oid, integer)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcpoint_out(tpcpoint)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcpoint_recv(internal, oid, integer)
  RETURNS tpcpoint
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpcpoint_send(tpcpoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tpcpoint (
  internallength = variable,
  input = tpcpoint_in,
  output = tpcpoint_out,
  receive = tpcpoint_recv,
  send = tpcpoint_send,
  storage = extended,
  alignment = double
);

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
CREATE FUNCTION asBinary(tpcpoint, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tpcpoint, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_as_hexwkb'
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

CREATE FUNCTION tempSubtype(tpcpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_subtype'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interp(tpcpoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Temporal_interp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(tpcpoint)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(tpcpoint)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(tpcpoint, integer)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(tpcpoint)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Temporal_valueset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getTime(tpcpoint)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Temporal_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tpcpoint, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Temporal_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timespan(tpcpoint)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Temporal_to_tstzspan'
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

CREATE FUNCTION numTimestamps(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Type-specific accessor
CREATE FUNCTION pcid(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tpcpoint_pcid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Per-dimension projection to tfloat. STABLE because the projection
-- reads the schema cache, which depends on pg_catalog state.
CREATE FUNCTION getX(tpcpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_x'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getY(tpcpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_y'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getZ(tpcpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_z'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getDim(tpcpoint, text)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tpcpoint_get_dim'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

-- XY projection to tgeompoint. Step interpolation on the source
-- tpcpoint is promoted to linear in the output: the projected XY
-- trajectory is physically a sensor path, where linear interp
-- between consecutive fixes is the natural default.
CREATE FUNCTION tgeompoint(tpcpoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Tpcpoint_to_tgeompoint'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE CAST (tpcpoint AS tgeompoint) WITH FUNCTION tgeompoint(tpcpoint);

/******************************************************************************
 * Value-at-timestamp / restriction
 ******************************************************************************/

CREATE FUNCTION valueAtTimestamp(tpcpoint, timestamptz)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Temporal_value_at_timestamptz'
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
 * Ever / always predicates
 ******************************************************************************/

CREATE FUNCTION ever_eq(tpcpoint, pcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tpcpoint, pcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_temporal_base'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tpcpoint, RIGHTARG = pcpoint,
  PROCEDURE = ever_eq
);
CREATE OPERATOR %= (
  LEFTARG = tpcpoint, RIGHTARG = pcpoint,
  PROCEDURE = always_eq
);

/******************************************************************************
 * Comparison / B-tree / hash
 ******************************************************************************/

CREATE FUNCTION temporal_eq(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tpcpoint, tpcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_cmp(tpcpoint, tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, PROCEDURE = temporal_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, PROCEDURE = temporal_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, PROCEDURE = temporal_lt,
  COMMUTATOR = >, NEGATOR = >=
);
CREATE OPERATOR <= (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, PROCEDURE = temporal_le,
  COMMUTATOR = >=, NEGATOR = >
);
CREATE OPERATOR >= (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, PROCEDURE = temporal_ge,
  COMMUTATOR = <=, NEGATOR = <
);
CREATE OPERATOR > (
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, PROCEDURE = temporal_gt,
  COMMUTATOR = <, NEGATOR = <=
);

CREATE OPERATOR CLASS tpcpoint_btree_ops
  DEFAULT FOR TYPE tpcpoint USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  temporal_cmp(tpcpoint, tpcpoint);

CREATE FUNCTION temporal_hash(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tpcpoint_hash_ops
  DEFAULT FOR TYPE tpcpoint USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(tpcpoint);

/*****************************************************************************/

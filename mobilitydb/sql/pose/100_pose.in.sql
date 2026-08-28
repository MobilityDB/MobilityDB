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
 * @brief Functions for the static pose type
 */

CREATE TYPE pose;

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION pose_in(cstring)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_in'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pose_out(pose)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Pose_out'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pose_recv(internal)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_recv'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pose_send(pose)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Pose_send'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE pose (
  internallength = variable,
  input = pose_in,
  output = pose_out,
  receive = pose_recv,
  send = pose_send,
  storage = plain,
  alignment = double
);

-- GENERATED-REPRESENTATIONS-BEGIN pose_base — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/*****************************************************************************
 * Input/output from (E)WKT, (E)WKB, and HexEWKB
 *****************************************************************************/

CREATE FUNCTION poseFromText(text)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION poseFromEWKT(text)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION poseFromBinary(bytea)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION poseFromEWKB(bytea)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION poseFromHexEWKB(text)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * OGC GeoPose JSON I/O — Basic-Quaternion, Basic-YPR and Advanced
 *****************************************************************************/

CREATE FUNCTION poseFromGeoPose(text)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_from_geopose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- conformance: 0 = Basic-Quaternion (default), 1 = Basic-YPR, 2 = Advanced
-- maxdecimaldigits: significant digits to keep; -1 = lossless
CREATE FUNCTION asGeoPose(pose, conformance integer DEFAULT 0,
    maxdecimaldigits integer DEFAULT -1)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Pose_as_geopose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION applyPose(geometry, pose)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Pose_apply_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The composition of two poses: the first one carried into the frame the
-- second one names, that is, P_WS from P_WV and P_VS.
CREATE FUNCTION applyPose(pose, pose)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_apply_pose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION asText(pose, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Pose_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(pose[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(pose, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Pose_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(pose[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(pose, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Pose_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(pose, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Pose_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(pose, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Pose_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(pose, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Pose_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-END pose_base

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION pose(double precision, double precision, double precision,
    srid integer DEFAULT 0)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pose(double precision, double precision, double precision,
  double precision, double precision, double precision, double precision,
    srid integer DEFAULT 0)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pose(geometry, double precision)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_constructor_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pose(geometry, double precision, double precision, 
    double precision, double precision)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_constructor_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The OGC GeoPose Basic-YPR encoding of the same orientation: yaw, pitch
-- and roll in radians, ZYX intrinsic Tait-Bryan.
CREATE FUNCTION pose(geometry, yaw double precision, pitch double precision,
    roll double precision)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_constructor_point_ypr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Conversions
 *****************************************************************************/

CREATE FUNCTION geometry(pose)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Pose_to_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pose AS geometry) WITH FUNCTION geometry(pose);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION point(pose)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Pose_point'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION yaw(pose)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Pose_yaw'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pitch(pose)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Pose_pitch'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION roll(pose)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Pose_roll'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE quaternion AS (
  W float,
  X float,
  Y float,
  Z float
);

CREATE FUNCTION quaternion(pose)
  RETURNS quaternion
  AS 'MODULE_PATHNAME', 'Pose_quaternion'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE ypr AS (
  yaw float,
  pitch float,
  roll float
);

CREATE FUNCTION ypr(pose)
  RETURNS ypr
  AS 'MODULE_PATHNAME', 'Pose_ypr'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

CREATE FUNCTION poseNormalize(pose)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_normalize'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION poseInverse(pose)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_inverse'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION round(pose, integer DEFAULT 0)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
  
/*****************************************************************************
 * SRID functions
 *****************************************************************************/

CREATE FUNCTION SRID(pose)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Pose_srid'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION setSRID(pose, integer)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(pose, integer)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(pose, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION pose_same(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = pose_same,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/******************************************************************************
 * Comparisons
 ******************************************************************************/

CREATE FUNCTION eq(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(pose, pose)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Pose_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = eq,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = ne,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = lt,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = le,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = ge,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
  PROCEDURE = gt,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS pose_btree_ops
  DEFAULT FOR TYPE pose USING btree AS
  OPERATOR  1 < ,
  OPERATOR  2 <= ,
  OPERATOR  3 = ,
  OPERATOR  4 >= ,
  OPERATOR  5 > ,
  FUNCTION  1 cmp(pose, pose);

/******************************************************************************/

CREATE FUNCTION hash(pose)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Pose_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hashExtended(pose, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Pose_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS pose_hash_ops
  DEFAULT FOR TYPE pose USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash(pose),
    FUNCTION    2   hashExtended(pose, bigint);

/******************************************************************************/

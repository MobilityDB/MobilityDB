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
 * @brief Functions for the static pose chain type
 */

CREATE TYPE posechain;

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION posechain_in(cstring)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_in'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION posechain_out(posechain)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Posechain_out'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION posechain_recv(internal)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_recv'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION posechain_send(posechain)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Posechain_send'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE posechain (
  internallength = variable,
  input = posechain_in,
  output = posechain_out,
  receive = posechain_recv,
  send = posechain_send,
  storage = plain,
  alignment = double
);

-- GENERATED-REPRESENTATIONS-BEGIN posechain_base — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/*****************************************************************************
 * Input/output from (E)WKT, (E)WKB, and HexEWKB
 *****************************************************************************/

CREATE FUNCTION posechainFromText(text)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION posechainFromEWKT(text)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION posechainFromBinary(bytea)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION posechainFromEWKB(bytea)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION posechainFromHexEWKB(text)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION asText(posechain, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Posechain_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(posechain[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(posechain, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Posechain_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(posechain[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(posechain, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Posechain_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(posechain, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Posechain_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(posechain, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Posechain_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(posechain, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Posechain_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-END posechain_base

/*****************************************************************************
 * Constructors
 *****************************************************************************/

CREATE FUNCTION posechain(pose[])
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION appendPose(posechain, pose)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_append_pose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Conversions
 *****************************************************************************/

CREATE FUNCTION posechain(pose)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Pose_to_posechain'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pose AS posechain) WITH FUNCTION posechain(pose);

-- The pose of the innermost frame, read in the outer frame of the chain: the
-- composition of every link. This is what gives a chain the surface of a pose.
CREATE FUNCTION pose(posechain)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Posechain_to_pose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (posechain AS pose) WITH FUNCTION pose(posechain);

-- The pose of the frame the first n links define, that is, where the n-th
-- joint of the chain is.
CREATE FUNCTION pose(posechain, integer)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Posechain_prefix_pose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION point(posechain)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Posechain_to_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Accessors
 *****************************************************************************/

CREATE FUNCTION numPoses(posechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Posechain_num_poses'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startPose(posechain)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Posechain_start_pose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endPose(posechain)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Posechain_end_pose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- The link as it is stored, in the frame the link before it defines. The pose
-- of that frame in the outer frame of the chain is pose(posechain, integer).
CREATE FUNCTION poseN(posechain, integer)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Posechain_pose_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION poses(posechain)
  RETURNS pose[]
  AS 'MODULE_PATHNAME', 'Posechain_poses'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

CREATE FUNCTION round(posechain, integer DEFAULT 0)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

CREATE FUNCTION SRID(posechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Posechain_srid'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION setSRID(posechain, integer)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(posechain, integer)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(posechain, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS posechain
  AS 'MODULE_PATHNAME', 'Posechain_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same(posechain, posechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Posechain_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = same,
  LEFTARG = posechain, RIGHTARG = posechain,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Comparisons
 *****************************************************************************/

CREATE FUNCTION eq(posechain, posechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Posechain_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(posechain, posechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Posechain_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(posechain, posechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Posechain_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(posechain, posechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Posechain_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(posechain, posechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Posechain_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(posechain, posechain)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Posechain_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(posechain, posechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Posechain_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = posechain, RIGHTARG = posechain,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel, HASHES
);
CREATE OPERATOR <> (
  LEFTARG = posechain, RIGHTARG = posechain,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = posechain, RIGHTARG = posechain,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = posechain, RIGHTARG = posechain,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
  LEFTARG = posechain, RIGHTARG = posechain,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
  LEFTARG = posechain, RIGHTARG = posechain,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS posechain_btree_ops
  DEFAULT FOR TYPE posechain USING btree AS
  OPERATOR  1 < ,
  OPERATOR  2 <= ,
  OPERATOR  3 = ,
  OPERATOR  4 >= ,
  OPERATOR  5 > ,
  FUNCTION  1 cmp(posechain, posechain);

/******************************************************************************/

CREATE FUNCTION hash(posechain)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Posechain_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hashExtended(posechain, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Posechain_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS posechain_hash_ops
  DEFAULT FOR TYPE posechain USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash(posechain),
    FUNCTION    2   hashExtended(posechain, bigint);

/******************************************************************************/

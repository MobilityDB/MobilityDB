/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * tpose_static.sql
 * Static pose type
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

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION pose(double precision, double precision, double precision)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pose(double precision, double precision, double precision,
  double precision, double precision, double precision, double precision)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Pose_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Conversions
 *****************************************************************************/

CREATE FUNCTION geometry(pose)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Pose_to_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pose AS geometry) WITH FUNCTION geometry(pose);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/


/******************************************************************************
 * Comparisons
 ******************************************************************************/

CREATE FUNCTION pose_eq(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pose_ne(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pose_lt(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pose_le(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pose_ge(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pose_gt(pose, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Pose_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pose_cmp(pose, pose)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Pose_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = pose_eq,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = pose_ne,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = pose_lt,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = pose_le,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = pose_ge,
  LEFTARG = pose, RIGHTARG = pose,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
  PROCEDURE = pose_gt,
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
  FUNCTION  1 pose_cmp(pose, pose);

/******************************************************************************/

/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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

/*
 * tpoint_compops.sql
 * Comparison functions and operators for temporal points.
 */

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION tpoint_teq(geometry(Point), tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeompoint, geometry(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tpoint_teq,
  LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tpoint_teq,
  LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tpoint_teq,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = #=
);

/*****************************************************************************/

CREATE FUNCTION tpoint_teq(geography(Point), tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeogpoint, geography(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tpoint_teq,
  LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tpoint_teq,
  LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tpoint_teq,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION tpoint_ne(geometry(Point), tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeompoint, geometry(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tpoint_ne,
  LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_ne,
  LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_ne,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = #<>
);

/*****************************************************************************/

CREATE FUNCTION tpoint_ne(geography(Point), tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeogpoint, geography(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tpoint_ne,
  LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_ne,
  LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_ne,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = #<>
);

/******************************************************************************/

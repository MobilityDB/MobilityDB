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
 * @file
 * @brief Ever/always and temporal comparison functions and operators for
 * temporal points.
 */

/*****************************************************************************
 * Index Support Function
 *****************************************************************************/

CREATE FUNCTION tpoint_supportfn(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpoint_supportfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Ever/Always Comparison Functions
 *****************************************************************************/

CREATE FUNCTION ever_eq(geometry(Point), tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_point_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(geography(Point), tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_point_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION always_eq(geometry(Point), tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_point_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(geography(Point), tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_point_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION ever_ne(geometry(Point), tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(geography(Point), tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION always_ne(geometry(Point), tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(geography(Point), tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tgeompoint, geometry(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tpoint_point'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tgeogpoint, geography(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tpoint_point'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION always_eq(tgeompoint, geometry(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tpoint_point'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tgeogpoint, geography(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tpoint_point'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION ever_ne(tgeompoint, geometry(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tgeogpoint, geography(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION always_ne(tgeompoint, geometry(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tgeogpoint, geography(Point))
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION always_eq(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION ever_ne(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

CREATE FUNCTION always_ne(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION tpoint_teq(geometry(Point), tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeompoint, geometry(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_tpoint'
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

CREATE FUNCTION tpoint_teq(geometry(Point), tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeompoint, geometry(Point), atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeompoint, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tpoint_teq(geography(Point), tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeogpoint, geography(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_tpoint'
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

CREATE FUNCTION tpoint_teq(geography(Point), tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeogpoint, geography(Point), atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_teq(tgeogpoint, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION tpoint_tne(geometry(Point), tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeompoint, geometry(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tpoint_tne,
  LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_tne,
  LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_tne,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = #<>
);

CREATE FUNCTION tpoint_tne(geometry(Point), tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeompoint, geometry(Point), atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeompoint, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tpoint_tne(geography(Point), tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeogpoint, geography(Point))
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tpoint_tne,
  LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_tne,
  LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tpoint_tne,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = #<>
);

CREATE FUNCTION tpoint_tne(geography(Point), tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_point_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeogpoint, geography(Point), atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_tne(tgeogpoint, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

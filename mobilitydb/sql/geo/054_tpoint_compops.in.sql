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
 * @brief Ever/always and temporal comparison functions and operators for
 * temporal geometry/geography points
 */

/*****************************************************************************
 * Ever/Always Comparison Functions
 *****************************************************************************/

CREATE FUNCTION ever_eq(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_eq(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_ne(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_eq(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_ne(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_eq(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_ne(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION tgeo_teq(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tgeo_teq,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tgeo_teq,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tgeo_teq,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = #=
);

CREATE FUNCTION tgeo_teq(geometry, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeompoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeompoint, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tgeo_teq(geography, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeogpoint, geography)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = tgeo_teq,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tgeo_teq,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = tgeo_teq,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = #=
);

CREATE FUNCTION tgeo_teq(geography, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeogpoint, geography, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_teq(tgeogpoint, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION tgeo_tne(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tgeo_tne,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tgeo_tne,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tgeo_tne,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = #<>
);

CREATE FUNCTION tgeo_tne(geometry, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeompoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeompoint, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tgeo_tne(geography, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeogpoint, geography)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = tgeo_tne,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tgeo_tne,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = tgeo_tne,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = #<>
);

CREATE FUNCTION tgeo_tne(geography, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeogpoint, geography, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeo_tne(tgeogpoint, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

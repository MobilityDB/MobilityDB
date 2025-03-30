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
 * documentation for any purgeometry, without fee, and without a written
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
 * temporal geometrys
 */

/*****************************************************************************
 * Ever/Always Comparison Functions
 *****************************************************************************/

CREATE FUNCTION ever_eq(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = geometry, RIGHTARG = trgeometry,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = geometry, RIGHTARG = trgeometry,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = geometry, RIGHTARG = trgeometry,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = geometry, RIGHTARG = trgeometry,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = trgeometry, RIGHTARG = geometry,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = trgeometry, RIGHTARG = geometry,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = trgeometry, RIGHTARG = geometry,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = trgeometry, RIGHTARG = geometry,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION temporal_teq(geometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(trgeometry, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_teq(trgeometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = geometry, RIGHTARG = trgeometry,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = trgeometry, RIGHTARG = geometry,
  COMMUTATOR = #=
);
CREATE OPERATOR #= (
  PROCEDURE = temporal_teq,
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION temporal_tne(geometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(trgeometry, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_tne(trgeometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = geometry, RIGHTARG = trgeometry,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = trgeometry, RIGHTARG = geometry,
  COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
  PROCEDURE = temporal_tne,
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  COMMUTATOR = #<>
);

/******************************************************************************/

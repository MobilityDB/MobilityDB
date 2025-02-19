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
 * temporal geos.
 */

/*****************************************************************************
 * Ever/Always Comparison Functions
 *****************************************************************************/

CREATE FUNCTION ever_eq(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = geometry, RIGHTARG = tgeometry,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = geography, RIGHTARG = tgeography,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_eq(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = geography, RIGHTARG = tgeography,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = geometry, RIGHTARG = tgeometry,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = geometry, RIGHTARG = tgeometry,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = geography, RIGHTARG = tgeography,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_ne(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = geometry, RIGHTARG = tgeometry,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = geography, RIGHTARG = tgeography,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tgeometry, RIGHTARG = geometry,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tgeography, RIGHTARG = geography,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_eq(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tgeography, RIGHTARG = geography,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tgeometry, RIGHTARG = geometry,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tgeometry, RIGHTARG = geometry,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tgeography, RIGHTARG = geography,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_ne(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tgeometry, RIGHTARG = geometry,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tgeography, RIGHTARG = geography,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************/

CREATE FUNCTION ever_eq(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?= (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = ever_eq,
  NEGATOR = %<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_eq(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_eq_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %= (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = always_eq,
  NEGATOR = ?<>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION ever_ne(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ?<> (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = ever_ne,
  NEGATOR = %=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

CREATE FUNCTION always_ne(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Always_ne_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR %<> (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = always_ne,
  NEGATOR = ?=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

-- CREATE FUNCTION tgeo_teq(geometry, tgeometry)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeometry, geometry)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeometry, tgeometry)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE OPERATOR #= (
  -- PROCEDURE = tgeo_teq,
  -- LEFTARG = geometry, RIGHTARG = tgeometry,
  -- COMMUTATOR = #=
-- );
-- CREATE OPERATOR #= (
  -- PROCEDURE = tgeo_teq,
  -- LEFTARG = tgeometry, RIGHTARG = geometry,
  -- COMMUTATOR = #=
-- );
-- CREATE OPERATOR #= (
  -- PROCEDURE = tgeo_teq,
  -- LEFTARG = tgeometry, RIGHTARG = tgeometry,
  -- COMMUTATOR = #=
-- );

-- CREATE FUNCTION tgeo_teq(geometry, tgeometry, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeometry, geometry, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeometry, tgeometry, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- CREATE FUNCTION tgeo_teq(geography, tgeography)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeography, geography)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeography, tgeography)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE OPERATOR #= (
  -- PROCEDURE = tgeo_teq,
  -- LEFTARG = geography, RIGHTARG = tgeography,
  -- COMMUTATOR = #=
-- );
-- CREATE OPERATOR #= (
  -- PROCEDURE = tgeo_teq,
  -- LEFTARG = tgeography, RIGHTARG = geography,
  -- COMMUTATOR = #=
-- );
-- CREATE OPERATOR #= (
  -- PROCEDURE = tgeo_teq,
  -- LEFTARG = tgeography, RIGHTARG = tgeography,
  -- COMMUTATOR = #=
-- );

-- CREATE FUNCTION tgeo_teq(geography, tgeography, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeography, geography, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_teq(tgeography, tgeography, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Teq_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

-- CREATE FUNCTION tgeo_tne(geometry, tgeometry)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeometry, geometry)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeometry, tgeometry)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE OPERATOR #<> (
  -- PROCEDURE = tgeo_tne,
  -- LEFTARG = geometry, RIGHTARG = tgeometry,
  -- COMMUTATOR = #<>
-- );
-- CREATE OPERATOR #<> (
  -- PROCEDURE = tgeo_tne,
  -- LEFTARG = tgeometry, RIGHTARG = geometry,
  -- COMMUTATOR = #<>
-- );
-- CREATE OPERATOR #<> (
  -- PROCEDURE = tgeo_tne,
  -- LEFTARG = tgeometry, RIGHTARG = tgeometry,
  -- COMMUTATOR = #<>
-- );

-- CREATE FUNCTION tgeo_tne(geometry, tgeometry, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeometry, geometry, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeometry, tgeometry, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- CREATE FUNCTION tgeo_tne(geography, tgeography)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeography, geography)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeography, tgeography)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE OPERATOR #<> (
  -- PROCEDURE = tgeo_tne,
  -- LEFTARG = geography, RIGHTARG = tgeography,
  -- COMMUTATOR = #<>
-- );
-- CREATE OPERATOR #<> (
  -- PROCEDURE = tgeo_tne,
  -- LEFTARG = tgeography, RIGHTARG = geography,
  -- COMMUTATOR = #<>
-- );
-- CREATE OPERATOR #<> (
  -- PROCEDURE = tgeo_tne,
  -- LEFTARG = tgeography, RIGHTARG = tgeography,
  -- COMMUTATOR = #<>
-- );

-- CREATE FUNCTION tgeo_tne(geography, tgeography, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_geo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeography, geography, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION tgeo_tne(tgeography, tgeography, atvalue bool)
  -- RETURNS tbool
  -- AS 'MODULE_PATHNAME', 'Tne_tgeo_tgeo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

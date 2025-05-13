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
 * @brief Spatial relationships for temporal geometries/geographies
 * @note Index support for these functions is enabled
 */

/*****************************************************************************
 * eContains, aContains
 *****************************************************************************/

CREATE FUNCTION eContains(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eContains(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eContains(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aContains(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aContains(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aContains(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eCovers, aCovers
 *****************************************************************************/

CREATE FUNCTION eCovers(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ecovers_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eCovers(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ecovers_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eCovers(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ecovers_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aCovers(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acovers_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aCovers(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acovers_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aCovers(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acovers_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDisjoint, aDisjoint
 *****************************************************************************/

-- TODO implement the index support in the tspatial_supportfn

CREATE FUNCTION _edisjoint(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(geometry, tgeometry)
  RETURNS boolean
  AS 'SELECT NOT(stbox($1) OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tgeometry, geometry)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) stbox($2)) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tgeometry, tgeometry)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

-- CREATE FUNCTION eDisjoint(geometry, tgeometry)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_geo_tgeo'
  -- SUPPORT tspatial_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeometry, geometry)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_geo'
  -- SUPPORT tspatial_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeometry, tgeometry)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_tgeo'
  -- SUPPORT tspatial_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- TODO implement the index support in the tspatial_supportfn

CREATE FUNCTION _edisjoint(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(geography, tgeography)
  RETURNS boolean
  AS 'SELECT NOT(stbox($1) OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tgeography, geography)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) stbox($2)) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tgeography, tgeography)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

-- CREATE FUNCTION eDisjoint(geography, tgeography)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_geo_tgeo'
  -- SUPPORT tspatial_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeography, geography)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeography, tgeography)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tgeo_tgeo'
  -- SUPPORT tspatial_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aDisjoint(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDisjoint(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eIntersects, aIntersects
 *****************************************************************************/

CREATE FUNCTION eIntersects(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eIntersects(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aIntersects(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aIntersects(geography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeography, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeography, tgeography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eTouches, aTouches
 *****************************************************************************/

CREATE FUNCTION eTouches(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aTouches(geometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(tgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(tgeometry, tgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDwithin, aDwithin
 *****************************************************************************/

CREATE FUNCTION eDwithin(geometry, tgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeometry, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeometry, tgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eDwithin(geography, tgeography, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeography, geography, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeography, tgeography, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- NOTE: aDWithin for geograhies is not provided since it is based on the
-- PostGIS ST_Buffer() function which is performed by GEOS

CREATE FUNCTION aDwithin(geometry, tgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_geo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tgeometry, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tgeometry, tgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDwithin(tgeography, tgeography, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tgeo_tgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

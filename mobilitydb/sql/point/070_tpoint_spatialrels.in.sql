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
 * @brief Spatial relationships for temporal points.
 * @note Index support for these functions is enabled
 */

/*****************************************************************************
 * eContains, aContains
 *****************************************************************************/

CREATE FUNCTION eContains(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aContains(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDisjoint, aDisjoint
 *****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

CREATE FUNCTION _edisjoint(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT NOT(stbox($1) OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) stbox($2)) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

-- CREATE FUNCTION eDisjoint(geometry, tgeompoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeompoint, geometry)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_geo'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeompoint, tgeompoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDisjoint(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

CREATE FUNCTION _edisjoint(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(geography, tgeogpoint)
  RETURNS boolean
  AS 'SELECT NOT(stbox($1) OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tgeogpoint, geography)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) stbox($2)) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

-- CREATE FUNCTION eDisjoint(geography, tgeogpoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeogpoint, geography)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION eDisjoint(tgeogpoint, tgeogpoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDisjoint(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eIntersects, aIntersects
 *****************************************************************************/

CREATE FUNCTION eIntersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aIntersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION eIntersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aIntersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eTouches, aTouches
 *****************************************************************************/

CREATE FUNCTION eTouches(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aTouches(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDwithin, aDwithin
 *****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

CREATE FUNCTION eDwithin(geometry, tgeompoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeompoint, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeompoint, tgeompoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDwithin(geometry, tgeompoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tgeompoint, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tgeompoint, tgeompoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

-- NOTE: aDWithin for geograhies is not provided since it is based on the
-- PostGIS ST_Buffer() function which is performed by GEOS

CREATE FUNCTION eDwithin(geography, tgeogpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeogpoint, geography, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tgeogpoint, tgeogpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDwithin(tgeogpoint, tgeogpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

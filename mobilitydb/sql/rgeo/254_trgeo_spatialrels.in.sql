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
 * @brief Spatial relationships for temporal rigid geometries
 * @note Index support for these functions is enabled
 */

/*****************************************************************************
 * eContains, aContains
 *****************************************************************************/

CREATE FUNCTION eContains(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eContains(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eContains(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aContains(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aContains(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aContains(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eCovers, aCovers
 *****************************************************************************/

CREATE FUNCTION eCovers(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ecovers_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eCovers(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ecovers_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eCovers(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ecovers_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aCovers(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acovers_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aCovers(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acovers_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aCovers(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acovers_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDisjoint, aDisjoint
 *****************************************************************************/

CREATE FUNCTION eDisjoint(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aDisjoint(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eIntersects, aIntersects
 *****************************************************************************/

CREATE FUNCTION eIntersects(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aIntersects(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eTouches, aTouches
 *****************************************************************************/

CREATE FUNCTION eTouches(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aTouches(geometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(trgeometry, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(trgeometry, trgeometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDwithin, aDwithin
 *****************************************************************************/

CREATE FUNCTION eDwithin(geometry, trgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(trgeometry, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(trgeometry, trgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aDwithin(geometry, trgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_geo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(trgeometry, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_trgeo_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(trgeometry, trgeometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_trgeo_trgeo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

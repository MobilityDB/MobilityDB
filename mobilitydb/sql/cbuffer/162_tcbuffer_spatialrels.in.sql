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
 * @brief Spatial relationships for temporal circular buffers
 * @details These relationships are generalized to the temporal dimension with
 * the "ever" and "always" semantics, and return a Boolean.
 *
 * The following relationships are supported:
 *    `eContains`, `aContains`, `eDisjoint`, `aDisjoint`, `eIntersects`,
 *    `aIntersects`, `eTouches`, `aTouches`, `eDwithin`, and `aDwithin`
 * All these relationships, excepted `eDisjoint`, will automatically perform
 * a bounding box comparison that will make use of any spatial, temporal, or
 * spatiotemporal indexes that are available.
 */

/*****************************************************************************
 * eContains, aContains
 *****************************************************************************/

CREATE FUNCTION eContains(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eContains(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eContains(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eContains(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eContains(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aContains(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aContains(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aContains(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aContains(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aContains(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDisjoint, aDisjoint
 *****************************************************************************/

CREATE FUNCTION eDisjoint(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eDisjoint(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eDisjoint(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aDisjoint(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_cbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDisjoint(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tcbuffer_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDisjoint(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tcbuffer_tcbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eIntersects, aIntersects
 *****************************************************************************/

CREATE FUNCTION eintersects(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eIntersects(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eIntersects(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aIntersects(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aIntersects(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aIntersects(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eTouches, aTouches
 *****************************************************************************/

CREATE FUNCTION eTouches(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(tcbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eTouches(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eTouches(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aTouches(cbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(tcbuffer, cbuffer)
  RETURNS boolean
  SUPPORT tspatial_supportfn
  AS 'MODULE_PATHNAME', 'Atouches_tcbuffer_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aTouches(geometry, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(tcbuffer, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aTouches(tcbuffer, tcbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDwithin, aDwithin
 *****************************************************************************/

CREATE FUNCTION eDwithin(cbuffer, tcbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tcbuffer, cbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eDwithin(geometry, tcbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tcbuffer, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eDwithin(tcbuffer, tcbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aDwithin(cbuffer, tcbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_cbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tcbuffer, cbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tcbuffer_cbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDwithin(geometry, tcbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_geo_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tcbuffer, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tcbuffer_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDwithin(tcbuffer, tcbuffer, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tcbuffer_tcbuffer'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

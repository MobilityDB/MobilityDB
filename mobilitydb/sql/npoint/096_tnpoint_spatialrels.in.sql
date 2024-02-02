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
 * @brief Spatial relationships for temporal network points
 *
 * These relationships are generalized to the temporal dimension with the
 * "at any instant" semantics, that is, the traditional operator is applied to
 * the union of all values taken by the temporal npoint and returns a Boolean.
 * The following relationships are supported:
 *    eContains, aContains, eDisjoint, aDisjoint, eIntersects, aIntersects,
 *    eTouches, aTouches, eDwithin, and aDwithin
 * All these relationships, excepted eDisjoint, will automatically
 * include a bounding box comparison that will make use of any spatial,
 * temporal, or spatiotemporal indexes that are available.
 */

/*****************************************************************************
 * eContains, aContains
 *****************************************************************************/

CREATE FUNCTION eContains(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aContains(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Acontains_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDisjoint, aDisjoint
 *****************************************************************************/

CREATE FUNCTION eDisjoint(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDisjoint(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eIntersects, aIntersects
 *****************************************************************************/

CREATE FUNCTION _eintersects(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(npoint, tnpoint)
  RETURNS boolean
  AS 'SELECT stbox($1) OPERATOR(@extschema@.&&) $2 AND @extschema@._eintersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aIntersects(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION _eintersects(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tnpoint, npoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) stbox($2) AND @extschema@._eintersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aIntersects(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eIntersects(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tnpoint_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aIntersects(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aIntersects(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tnpoint_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eTouches, aTouches
 *****************************************************************************/

CREATE FUNCTION _eTouches(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(npoint, tnpoint)
  RETURNS boolean
  AS 'SELECT stbox($1) OPERATOR(@extschema@.&&) $2 AND @extschema@._etouches($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aTouches(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION _eTouches(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(tnpoint, npoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) stbox($2) AND @extschema@._etouches($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aTouches(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eTouches(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eTouches(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aTouches(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aTouches(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Atouches_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eDwithin, aDwithin
 *****************************************************************************/

CREATE FUNCTION _eDwithin(npoint, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(npoint, tnpoint, dist float)
  RETURNS boolean
  AS 'SELECT @extschema@.expandSpace(stbox($1),$3) OPERATOR(@extschema@.&&) $2 AND @extschema@._edwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aDwithin(npoint, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION _eDwithin(tnpoint, npoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tnpoint, npoint, dist float)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.expandSpace(stbox($2),$3) AND @extschema@._edwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aDwithin(tnpoint, npoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION eDwithin(geometry, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tnpoint, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tnpoint, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tnpoint_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION aDwithin(geometry, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tnpoint, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION aDwithin(tnpoint, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tnpoint_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * tnpoint_spatialrels.sql
 * Spatial relationships for temporal network points.
 *
 * These relationships are generalized to the temporal dimension with the
 * "at any instant" semantics, that is, the traditional operator is applied to
 * the union of all values taken by the temporal npoint and returns a Boolean.
 * The following relationships are supported:
 *    econtains, edisjoint, eintersects, etouches, and edwithin
 * All these relationships, excepted edisjoint, will automatically
 * include a bounding box comparison that will make use of any spatial,
 * temporal, or spatiotemporal indexes that are available.
 */

/*****************************************************************************
 * econtains
 *****************************************************************************/

CREATE FUNCTION econtains(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * edisjoint
 *****************************************************************************/

CREATE FUNCTION edisjoint(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eintersects
 *****************************************************************************/

CREATE FUNCTION _eintersects(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(npoint, tnpoint)
  RETURNS boolean
  AS 'SELECT stbox($1) OPERATOR(@extschema@.&&) $2 AND @extschema@._eintersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _eintersects(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tnpoint, npoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) stbox($2) AND @extschema@._eintersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION eintersects(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tnpoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tnpoint_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * etouches
 *****************************************************************************/

CREATE FUNCTION _etouches(npoint, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION etouches(npoint, tnpoint)
  RETURNS boolean
  AS 'SELECT stbox($1) OPERATOR(@extschema@.&&) $2 AND @extschema@._etouches($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _etouches(tnpoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION etouches(tnpoint, npoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) stbox($2) AND @extschema@._etouches($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION etouches(geometry, tnpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION etouches(tnpoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Edwithin
 *****************************************************************************/

CREATE FUNCTION _edwithin(npoint, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(npoint, tnpoint, dist float)
  RETURNS boolean
  AS 'SELECT @extschema@.expandSpace(stbox($1),$3) OPERATOR(@extschema@.&&) $2 AND @extschema@._edwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edwithin(tnpoint, npoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(tnpoint, npoint, dist float)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.expandSpace(stbox($2),$3) AND @extschema@._edwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION Edwithin(geometry, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(tnpoint, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tnpoint_geo'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(tnpoint, tnpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tnpoint_tnpoint'
  SUPPORT tnpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

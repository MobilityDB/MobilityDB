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

/*
 * tpoint_spatialrels.sql
 * Spatial relationships for temporal points.
 * Depending on PostgreSQL version, index support for these functions is
 * enabled with rewriting (version < 12) or support functions (version >= 12)
 */

/*****************************************************************************
 * econtains
 *****************************************************************************/

CREATE FUNCTION econtains(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Econtains_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * edisjoint
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

-- CREATE FUNCTION edisjoint(geometry, tgeompoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION edisjoint(tgeompoint, geometry)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_geo'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION edisjoint(tgeompoint, tgeompoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

CREATE FUNCTION _edisjoint(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(geography, tgeogpoint)
  RETURNS boolean
  AS 'SELECT NOT(stbox($1) OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tgeogpoint, geography)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) stbox($2)) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _edisjoint(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION edisjoint(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._edisjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

-- CREATE FUNCTION edisjoint(geography, tgeogpoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION edisjoint(tgeogpoint, geography)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION edisjoint(tgeogpoint, tgeogpoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Edisjoint_tpoint_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * eintersects
 *****************************************************************************/

CREATE FUNCTION eintersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION eintersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eintersects(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * etouches
 *****************************************************************************/

CREATE FUNCTION etouches(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION etouches(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Etouches_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Edwithin
 *****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

CREATE FUNCTION Edwithin(geometry, tgeompoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(tgeompoint, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(tgeompoint, tgeompoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

CREATE FUNCTION Edwithin(geography, tgeogpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(tgeogpoint, geography, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Edwithin(tgeogpoint, tgeogpoint, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

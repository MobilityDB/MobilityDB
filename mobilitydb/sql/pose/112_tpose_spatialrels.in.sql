/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Spatial relationships for temporal poses
 */

/*****************************************************************************
 * eContains, aContains
 *****************************************************************************/

CREATE FUNCTION eContains(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.eContains($1, $2::@extschema@.tgeompoint) $$;

/*****************************************************************************/

CREATE FUNCTION aContains(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aContains($1, $2::@extschema@.tgeompoint) $$;

/*****************************************************************************
 * eCovers, aCovers
 *****************************************************************************/

CREATE FUNCTION eCovers(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.eCovers($1, $2::@extschema@.tgeompoint) $$;

/*****************************************************************************/

CREATE FUNCTION aCovers(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aCovers($1, $2::@extschema@.tgeompoint) $$;

/*****************************************************************************
 * eDisjoint, aDisjoint
 *****************************************************************************/

CREATE FUNCTION eDisjoint(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.eDisjoint($1, $2::@extschema@.tgeompoint) $$;
CREATE FUNCTION eDisjoint(tpose, geometry)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.eDisjoint($1::@extschema@.tgeompoint, $2) $$;
CREATE FUNCTION eDisjoint(tpose, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.eDisjoint($1::@extschema@.tgeompoint, $2::@extschema@.tgeompoint) $$;

/*****************************************************************************/

CREATE FUNCTION aDisjoint(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aDisjoint($1, $2::@extschema@.tgeompoint) $$;
CREATE FUNCTION aDisjoint(tpose, geometry)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aDisjoint($1::@extschema@.tgeompoint, $2) $$;
CREATE FUNCTION aDisjoint(tpose, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aDisjoint($1::@extschema@.tgeompoint, $2::@extschema@.tgeompoint) $$;

/*****************************************************************************
 * eIntersects, aIntersects
 *****************************************************************************/

CREATE FUNCTION eIntersects(geometry, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tpose'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tpose, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpose_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eIntersects(tpose, tpose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpose_tpose'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aIntersects(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aIntersects($1, $2::@extschema@.tgeompoint) $$;
CREATE FUNCTION aIntersects(tpose, geometry)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aIntersects($1::@extschema@.tgeompoint, $2) $$;
CREATE FUNCTION aIntersects(tpose, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aIntersects($1::@extschema@.tgeompoint, $2::@extschema@.tgeompoint) $$;

/*****************************************************************************
 * eTouches, aTouches
 *****************************************************************************/

CREATE FUNCTION eTouches(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.eTouches($1, $2::@extschema@.tgeompoint) $$;
CREATE FUNCTION eTouches(tpose, geometry)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.eTouches($1::@extschema@.tgeompoint, $2) $$;

/*****************************************************************************/

CREATE FUNCTION aTouches(geometry, tpose)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aTouches($1, $2::@extschema@.tgeompoint) $$;
CREATE FUNCTION aTouches(tpose, geometry)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aTouches($1::@extschema@.tgeompoint, $2) $$;

/*****************************************************************************
 * eDwithin, aDwithin
 *****************************************************************************/

CREATE FUNCTION eDwithin(geometry, tpose, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tpose'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tpose, geometry, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpose_geo'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION eDwithin(tpose, tpose, dist float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpose_tpose'
  SUPPORT tspatial_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION aDwithin(geometry, tpose, dist float)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aDwithin($1, $2::@extschema@.tgeompoint, $3) $$;
CREATE FUNCTION aDwithin(tpose, geometry, dist float)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aDwithin($1::@extschema@.tgeompoint, $2, $3) $$;
CREATE FUNCTION aDwithin(tpose, tpose, dist float)
  RETURNS boolean
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.aDwithin($1::@extschema@.tgeompoint, $2::@extschema@.tgeompoint, $3) $$;

/*****************************************************************************/

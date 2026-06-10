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
 * @brief Temporal spatial relationships for temporal poses
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

CREATE FUNCTION tContains(geometry, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tContains($1, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;
CREATE FUNCTION tContains(tpose, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tContains($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2) $$;
CREATE FUNCTION tContains(tpose, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tContains($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;

/*****************************************************************************
 * tCovers
 *****************************************************************************/

CREATE FUNCTION tCovers(geometry, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tCovers($1, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;
CREATE FUNCTION tCovers(tpose, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tCovers($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2) $$;
CREATE FUNCTION tCovers(tpose, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tCovers($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

CREATE FUNCTION tDisjoint(geometry, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tDisjoint($1, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;
CREATE FUNCTION tDisjoint(tpose, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tDisjoint($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2) $$;
-- Alias for temporal not equals, that is, tpose_tne or #<>
CREATE FUNCTION tDisjoint(tpose, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tDisjoint($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

CREATE FUNCTION tIntersects(geometry, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tIntersects($1, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;
CREATE FUNCTION tIntersects(tpose, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tIntersects($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2) $$;
-- Alias for temporal equals, that is, tpose_teq or #=
CREATE FUNCTION tIntersects(tpose, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tIntersects($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

CREATE FUNCTION tTouches(geometry, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tTouches($1, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;
CREATE FUNCTION tTouches(tpose, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tTouches($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2) $$;
CREATE FUNCTION tTouches(tpose, tpose)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tTouches($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2::@extschema@.tgeompoint::@extschema@.tgeometry) $$;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

CREATE FUNCTION tDwithin(geometry, tpose, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tDwithin($1, $2::@extschema@.tgeompoint::@extschema@.tgeometry, $3) $$;
CREATE FUNCTION tDwithin(tpose, geometry, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE AS
  $$ SELECT @extschema@.tDwithin($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2, $3) $$;
CREATE FUNCTION tDwithin(tpose, tpose, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS
  $$ SELECT @extschema@.tDwithin($1::@extschema@.tgeompoint::@extschema@.tgeometry, $2::@extschema@.tgeompoint::@extschema@.tgeometry, $3) $$;

/*****************************************************************************/

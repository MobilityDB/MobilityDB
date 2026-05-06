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
 * @brief Temporal spatial relationship functions for tpcpoint
 *
 * All functions delegate to the tgeompoint equivalents by casting
 * tpcpoint → tgeompoint via the XY-projection cast.
 */

/******************************************************************************
 * tIntersects
 ******************************************************************************/

CREATE FUNCTION tIntersects(geometry, tpcpoint)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tIntersects($1, $2::@extschema@.tgeompoint)
  $$;
CREATE FUNCTION tIntersects(tpcpoint, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tIntersects($1::@extschema@.tgeompoint, $2)
  $$;

/******************************************************************************
 * tDisjoint
 ******************************************************************************/

CREATE FUNCTION tDisjoint(geometry, tpcpoint)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tDisjoint($1, $2::@extschema@.tgeompoint)
  $$;
CREATE FUNCTION tDisjoint(tpcpoint, geometry)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tDisjoint($1::@extschema@.tgeompoint, $2)
  $$;

/******************************************************************************
 * tDwithin
 ******************************************************************************/

CREATE FUNCTION tDwithin(geometry, tpcpoint, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tDwithin($1, $2::@extschema@.tgeompoint, $3)
  $$;
CREATE FUNCTION tDwithin(tpcpoint, geometry, dist float)
  RETURNS tbool
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tDwithin($1::@extschema@.tgeompoint, $2, $3)
  $$;

/*****************************************************************************/

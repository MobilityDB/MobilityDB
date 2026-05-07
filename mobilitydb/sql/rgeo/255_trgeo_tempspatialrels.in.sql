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
 * @brief Temporal spatial relationships for temporal rigid geometries
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

CREATE FUNCTION tContains(geometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tContains(trgeometry, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tContains(trgeometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tCovers
 *****************************************************************************/

CREATE FUNCTION tCovers(geometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tCovers(trgeometry, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tCovers(trgeometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

CREATE FUNCTION tDisjoint(geometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(trgeometry, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(trgeometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

CREATE FUNCTION tIntersects(geometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(trgeometry, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(trgeometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

CREATE FUNCTION tTouches(geometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tTouches(trgeometry, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tTouches(trgeometry, trgeometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

CREATE FUNCTION tDwithin(geometry, trgeometry, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDwithin(trgeometry, geometry, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDwithin(trgeometry, trgeometry, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

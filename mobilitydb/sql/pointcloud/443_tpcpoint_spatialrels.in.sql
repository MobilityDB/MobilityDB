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
 *****************************************************************************/

/**
 * @file
 * @brief Spatial relationship functions for tpcpoint — eIntersects /
 *   aIntersects / eDisjoint / aDisjoint / eDwithin / aDwithin against
 *   a geometry or another tpcpoint. Implemented by projecting the
 *   tpcpoint to a tgeompoint via the schema cache and delegating to
 *   the corresponding tgeompoint relation. tpcpatch is not in scope:
 *   patch-level intersections would require per-point decompression.
 */

/******************************************************************************
 * eIntersects / aIntersects
 ******************************************************************************/

CREATE FUNCTION eIntersects(geometry, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_geo_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION eIntersects(tpcpoint, geometry) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpcpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION eIntersects(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Eintersects_tpcpoint_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aIntersects(geometry, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_geo_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION aIntersects(tpcpoint, geometry) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tpcpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION aIntersects(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Aintersects_tpcpoint_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/******************************************************************************
 * eDisjoint / aDisjoint
 ******************************************************************************/

CREATE FUNCTION eDisjoint(geometry, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_geo_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tpcpoint, geometry) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpcpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION eDisjoint(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edisjoint_tpcpoint_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aDisjoint(geometry, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_geo_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tpcpoint, geometry) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tpcpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION aDisjoint(tpcpoint, tpcpoint) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adisjoint_tpcpoint_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/******************************************************************************
 * eDwithin / aDwithin
 ******************************************************************************/

CREATE FUNCTION eDwithin(geometry, tpcpoint, dist float) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_geo_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION eDwithin(tpcpoint, geometry, dist float) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpcpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION eDwithin(tpcpoint, tpcpoint, dist float) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Edwithin_tpcpoint_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION aDwithin(geometry, tpcpoint, dist float) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_geo_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION aDwithin(tpcpoint, geometry, dist float) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tpcpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION aDwithin(tpcpoint, tpcpoint, dist float) RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adwithin_tpcpoint_tpcpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************/

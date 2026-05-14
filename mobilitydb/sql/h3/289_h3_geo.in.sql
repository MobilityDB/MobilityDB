/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Static-geometry → H3 cell / cell set SQL surface.
 *
 * Covers POINT, LINESTRING, POLYGON, and MULTI* / GEOMETRYCOLLECTION
 * combinations.  The cell-set output (`geoToH3IndexSet`) plus the
 * Set ever-intersects predicate (`everIntersectsH3IndexSet_Th3Index`)
 * are the cross-platform spatial prefilter consumed by the portable
 * BerlinMOD SQL.
 *
 * C wrappers in `mobilitydb/src/h3/h3_geo.c`; MEOS kernel in
 * `meos/src/h3/h3_geo.c`.
 */

/******************************************************************************
 * Static geometry → H3 cell (POINT only) / cell set (any geometry)
 ******************************************************************************/

CREATE FUNCTION geoToH3Cell(geometry, integer)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Geo_gs_point_to_h3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geoToH3IndexSet(geometry, integer)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Geo_to_h3indexset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3indexset × th3index — ever-intersects predicate (spatial prefilter)
 ******************************************************************************/

CREATE FUNCTION everIntersectsH3IndexSet_Th3Index(h3indexset, th3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Ever_eq_anyof_h3indexset_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

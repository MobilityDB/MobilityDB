/*****************************************************************************
 *
 * tpoint_analytics.sql
 *    Analytic functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************/
-- There are two versions of the functions since the single-argument version
-- is required for defining the casting

CREATE FUNCTION asGeometry(tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'tpoint_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asGeometry(tgeompoint, boolean DEFAULT FALSE)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'tpoint_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS geometry) WITH FUNCTION asGeometry(tgeompoint);

CREATE FUNCTION asGeography(tgeogpoint)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'tpoint_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asGeography(tgeogpoint, boolean DEFAULT FALSE)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'tpoint_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeogpoint AS geography) WITH FUNCTION asGeography(tgeogpoint);

CREATE FUNCTION tgeompoint(geometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'geo_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geometry AS tgeompoint) WITH FUNCTION tgeompoint(geometry);

CREATE FUNCTION tgeogpoint(geography)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'geo_to_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geography AS tgeogpoint) WITH FUNCTION tgeogpoint(geography);

/*****************************************************************************/

CREATE FUNCTION geoMeasure(tgeompoint, tfloat, boolean DEFAULT FALSE)
RETURNS geometry
AS 'MODULE_PATHNAME', 'tpoint_to_geo_measure'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geoMeasure(tgeogpoint, tfloat, boolean DEFAULT FALSE)
RETURNS geography
AS 'MODULE_PATHNAME', 'tpoint_to_geo_measure'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE OR REPLACE FUNCTION simplify(tfloat, float8)
RETURNS tfloat
AS 'MODULE_PATHNAME', 'tfloat_simplify'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION simplify(tgeompoint, float8, float8 DEFAULT -1.0)
RETURNS tgeompoint
AS 'MODULE_PATHNAME', 'tpoint_simplify'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

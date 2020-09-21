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

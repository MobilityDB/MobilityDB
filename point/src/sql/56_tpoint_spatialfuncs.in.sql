/*****************************************************************************
 *
 * tpoint_spatialfuncs.sql
 *    Spatial functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION srid(stbox)
  RETURNS int
  AS 'MODULE_PATHNAME', 'stbox_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setSRID(stbox, srid integer)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'stbox_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION transform(stbox, srid integer)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'stbox_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setPrecision(stbox, int)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'stbox_set_precision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION SRID(tgeompoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'tpoint_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION SRID(tgeogpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'tpoint_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tgeompoint, srid integer)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setSRID(tgeogpoint, srid integer)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(tgeompoint, srid integer)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

----- Gauss Kruger transformation
CREATE FUNCTION transform_gk(tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tgeompoint_transform_gk'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform_gk(geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'geometry_transform_gk'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-------

CREATE FUNCTION tgeogpoint(tgeompoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tgeompoint_to_tgeogpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint(tgeogpoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tgeogpoint_to_tgeompoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS tgeogpoint) WITH FUNCTION tgeogpoint(tgeompoint);
CREATE CAST (tgeogpoint AS tgeompoint) WITH FUNCTION tgeompoint(tgeogpoint);

CREATE FUNCTION setprecision(tgeompoint, int)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_set_precision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setprecision(tgeogpoint, int)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_set_precision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trajectory(tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'tpoint_trajectory'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trajectory(tgeogpoint)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'tpoint_trajectory'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION length(tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tpoint_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION length(tgeogpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'tpoint_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cumulativeLength(tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tpoint_cumulative_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cumulativeLength(tgeogpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tpoint_cumulative_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION speed(tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tpoint_speed'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION speed(tgeogpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tpoint_speed'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION twcentroid(tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'tgeompoint_twcentroid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION azimuth(tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tpoint_azimuth'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION azimuth(tgeogpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'tpoint_azimuth'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION atGeometry(tgeompoint, geometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_at_geometry'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusGeometry(tgeompoint, geometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_minus_geometry'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atStbox(tgeompoint, stbox)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_at_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atStbox(tgeogpoint, stbox)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_at_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusStbox(tgeompoint, stbox)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'tpoint_minus_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusStbox(tgeogpoint, stbox)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'tpoint_minus_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

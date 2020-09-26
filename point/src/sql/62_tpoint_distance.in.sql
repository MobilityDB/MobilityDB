/*****************************************************************************
 *
 * tpoint_distance.sql
 *    Distance functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

CREATE FUNCTION distance(geometry, tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeompoint, geometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeompoint, tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = <->
);

/*****************************************************************************/

CREATE FUNCTION distance(geography, tgeogpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeogpoint, geography)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeogpoint, tgeogpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'distance_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach instant/distance and shortest line functions
 *****************************************************************************/

CREATE FUNCTION NearestApproachInstant(geometry, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'NAI_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, geometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'NAI_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'NAI_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION NearestApproachInstant(geography, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'NAI_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeogpoint, geography)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'NAI_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeogpoint, tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'NAI_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geometry, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geography, tgeogpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, geography)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tgeogpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, tgeogpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

CREATE OPERATOR |=| (
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

CREATE FUNCTION shortestLine(geometry, tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'shortestline_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'shortestline_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'shortestline_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shortestLine(geography, tgeogpoint)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'shortestline_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, geography)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'shortestline_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, tgeogpoint)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'shortestline_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

/*****************************************************************************
 *
 * geo_constructors.sql
 *    SQL constructors for geometric types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION point(float, float)
  RETURNS point
  AS 'MODULE_PATHNAME', 'point_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
  
CREATE FUNCTION line(float, float, float)
  RETURNS line
  AS 'MODULE_PATHNAME', 'line_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
  
CREATE FUNCTION lseg(point, point)
  RETURNS lseg
  AS 'MODULE_PATHNAME', 'lseg_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION box(point, point)
  RETURNS box
  AS 'MODULE_PATHNAME', 'box_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION circle(point, float)
  RETURNS circle
  AS 'MODULE_PATHNAME', 'circle_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION path(point[])
  RETURNS path
  AS 'MODULE_PATHNAME', 'path_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION polygon(point[])
  RETURNS polygon
  AS 'MODULE_PATHNAME', 'poly_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

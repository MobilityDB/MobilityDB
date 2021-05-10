/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

/*
 * random_tpoint.sql
 * Basic synthetic data generator functions for geometry/geography types
 * and temporal point types.
 *
 * These functions use lower and upper bounds for the generated values:
 * lowx/lowy/lowz and highx/highy/highz for coordinates, lowtime and hightime
 * for timestamps. When generating series of values, the maxdelta argument
 * states the maximum difference between two consecutive coordinate values,
 * while maxminutes states the maximum number of minutes between two
 * consecutive timestamps as well as the maximum number of minutes for time
 * gaps between two consecutive components of temporal instant/sequence sets.
 */

-------------------------------------------------------------------------------
-- STBox Type
-------------------------------------------------------------------------------

/**
 * Generate a random 2D stbox 
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value between the bounds for the x,y,z coordinates
 * @param[in] maxminutes Maximum number of minutes between the bounds
 */
DROP FUNCTION IF EXISTS random_stbox;
CREATE FUNCTION random_stbox(lowx float, highx float, lowy float, highy float,
  lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int)
  RETURNS stbox AS $$
DECLARE
  xmin float;
  ymin float;
  tmin timestamptz;
BEGIN
  IF lowx > highx - maxdelta THEN
    RAISE EXCEPTION 'lowx must be less than or equal to highx - maxdelta: %, %, %',
      lowx, highx, maxdelta;
  END IF;
  IF lowy > highy - maxdelta THEN
    RAISE EXCEPTION 'lowy must be less than or equal to highy - maxdelta: %, %, %',
      lowy, highy, maxdelta;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes minutes: %, %, %',
      lowtime, hightime, maxminutes;
  END IF;
  xmin = random_float(lowx, highx - maxdelta);
  ymin = random_float(lowy, highy - maxdelta);
  tmin = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes);
  RETURN stbox_t(xmin, ymin, tmin, xmin + random_float(1, maxdelta),
    ymin + random_float(1, maxdelta), tmin + random_minutes(1, maxminutes));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_stbox(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D stbox 
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value between the bounds for the x,y,z coordinates
 * @param[in] maxminutes Maximum number of minutes between the bounds
 * @param[in] geodetic True if the stbox is geodetic
 * @param[in] geodZ True if the stbox is geodetic and has Z coordinates
 */
DROP FUNCTION IF EXISTS random_stbox3D;
CREATE FUNCTION random_stbox3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int,
  geodetic boolean DEFAULT false, geodZ boolean DEFAULT false)
  RETURNS stbox AS $$
DECLARE
  xmin float;
  ymin float;
  zmin float;
  tmin timestamptz;
BEGIN
  IF lowx > highx - maxdelta THEN
    RAISE EXCEPTION 'lowx must be less than or equal to highx - maxdelta: %, %, %',
      lowx, highx, maxdelta;
  END IF;
  IF lowy > highy - maxdelta THEN
    RAISE EXCEPTION 'lowy must be less than or equal to highy - maxdelta: %, %, %',
      lowy, highy, maxdelta;
  END IF;
  IF lowz > highz - maxdelta THEN
    RAISE EXCEPTION 'lowz must be less than or equal to highz - maxdelta: %, %, %',
      lowz, highz, maxdelta;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes minutes: %, %, %',
      lowtime, hightime, maxminutes;
  END IF;
  xmin = random_float(lowx, highx - maxdelta);
  ymin = random_float(lowy, highy - maxdelta);
  zmin = random_float(lowz, highz - maxdelta);
  tmin = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes);
  IF geodetic THEN
    IF geodZ THEN
      RETURN geodstbox_zt(xmin, ymin, zmin, tmin, xmin + random_float(1, maxdelta),
        ymin + random_float(1, maxdelta), zmin + random_float(1, maxdelta),
        tmin + random_minutes(1, maxminutes));
    ELSE
      RETURN geodstbox_t(xmin, ymin, zmin, tmin, xmin + random_float(1, maxdelta),
        ymin + random_float(1, maxdelta), zmin + random_float(1, maxdelta),
        tmin + random_minutes(1, maxminutes));
    END IF;
  ELSE
    RETURN stbox_zt(xmin, ymin, zmin, tmin, xmin + random_float(1, maxdelta),
      ymin + random_float(1, maxdelta), zmin + random_float(1, maxdelta),
      tmin + random_minutes(1, maxminutes));
  END IF;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_stbox3D(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geodetic stbox 
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value between the bounds for the x,y,z coordinates
 * @param[in] maxminutes Maximum number of minutes between the bounds
 */
DROP FUNCTION IF EXISTS random_geodstbox;
CREATE FUNCTION random_geodstbox(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int)
  RETURNS stbox AS $$
BEGIN
  RETURN random_stbox3D(lowx, highx, lowy, highy, lowz, highz, lowtime,
    hightime, maxdelta, maxminutes, true, false);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geodstbox(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geodetic stbox 
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value between the bounds for the x,y,z coordinates
 * @param[in] maxminutes Maximum number of minutes between the bounds
 */
DROP FUNCTION IF EXISTS random_geodstbox3D;
CREATE FUNCTION random_geodstbox3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int)
  RETURNS stbox AS $$
BEGIN
  RETURN random_stbox3D(lowx, highx, lowy, highy, lowz, highz, lowtime,
    hightime, maxdelta, maxminutes, true, true);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geodstbox3D(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Geometry/Geography
-------------------------------------------------------------------------------
/**
 * Generate a random 2D geometric point
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 */
DROP FUNCTION IF EXISTS random_geom_point;
CREATE FUNCTION random_geom_point(lowx float, highx float, lowy float,
  highy float)
  RETURNS geometry AS $$
BEGIN
  IF lowx > highx THEN
    RAISE EXCEPTION 'lowx must be less than or equal to highx: %, %',
      lowx, highx;
  END IF;
  IF lowy > highy THEN
    RAISE EXCEPTION 'lowy must be less than or equal to highy: %, %',
      lowy, highy;
  END IF;
  RETURN st_point(random_float(lowx, highx), random_float(lowy, highy));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_point(-100, 100, -100, 100))
FROM generate_series(1,10) k;

SELECT k, random_geom_point(-100, 100, -100, 100) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geometric point
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 */
DROP FUNCTION IF EXISTS random_geom_point3D;
CREATE FUNCTION random_geom_point3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float)
  RETURNS geometry AS $$
BEGIN
  IF lowx > highx THEN
    RAISE EXCEPTION 'lowx must be less than or equal to highx: %, %',
      lowx, highx;
  END IF;
  IF lowy > highy THEN
    RAISE EXCEPTION 'lowy must be less than or equal to highy: %, %',
      lowy, highy;
  END IF;
  IF lowz > highz THEN
    RAISE EXCEPTION 'lowz must be less than or equal to highz: %, %',
      lowz, highz;
  END IF;
  RETURN st_makepoint(random_float(lowx, highx), random_float(lowy, highy),
    random_float(lowz, highz));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_point3D(-100, 100, -100, 100, 0, 100))
FROM generate_series(1,10) k;

SELECT k, random_geom_point3D(-100, 100, -100, 100, 0, 100) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geographic point
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 */
DROP FUNCTION IF EXISTS random_geog_point;
CREATE FUNCTION random_geog_point(lowx float, highx float,
  lowy float, highy float)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_point(lowx, highx, lowy, highy)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_point(-180, 180, 90, 90))
FROM generate_series(1,10) k;

SELECT k, random_geog_point(-180, 180, 90, 90) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geographic point
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 */
DROP FUNCTION IF EXISTS random_geog_point3D;
CREATE FUNCTION random_geog_point3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_point3D(lowx, highx, lowy, highy, lowz, highz)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_point3D(0, 90, 0, 90, 0, 90))
FROM generate_series(1,10) k;

SELECT k, random_geog_point3D(0, 90, 0, 90, 0, 90) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random 2D geometric points
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_geom_point_array;
CREATE FUNCTION random_geom_point_array(lowx float, highx float, lowy float,
  highy float, maxdelta float, mincard int, maxcard int)
  RETURNS geometry[] AS $$
DECLARE
  result geometry[];
  card int;
  x float;
  y float;
  delta float;
  p geometry;
BEGIN
  IF lowx > highx THEN
    RAISE EXCEPTION 'lowx must be less than or equal to highx: %, %',
      lowx, highx;
  END IF;
  IF lowy > highy THEN
    RAISE EXCEPTION 'lowy must be less than or equal to highy: %, %',
      lowy, highy;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  p = random_geom_point(lowx, highx, lowy, highy);
  FOR i IN 1..card
  LOOP
    result[i] = p;
    IF i = card THEN EXIT; END IF;
    x = st_x(p);
    y = st_y(p);
    delta = random_float(-1 * maxdelta, maxdelta);
    /* If neither of these conditions is satisfied the same value is kept */
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSIF (x - delta >= lowx AND x - delta <= highx) THEN
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSIF (y - delta >= lowy AND y - delta <= highy) THEN
      y = y - delta;
    END IF;
    p = st_makepoint(x, y);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, astext(random_geom_point_array(-100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_point_array(-100, 100, -100, 100, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random 3D geometric points
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_geom_point3D_array;
CREATE FUNCTION random_geom_point3D_array(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, mincard int, maxcard int)
  RETURNS geometry[] AS $$
DECLARE
  result geometry[];
  card int;
  x float;
  y float;
  z float;
  delta float;
  p geometry;
BEGIN
  IF lowx > highx THEN
    RAISE EXCEPTION 'lowx must be less than or equal to highx: %, %',
      lowx, highx;
  END IF;
  IF lowy > highy THEN
    RAISE EXCEPTION 'lowy must be less than or equal to highy: %, %',
      lowy, highy;
  END IF;
  IF lowz > highz THEN
    RAISE EXCEPTION 'lowz must be less than or equal to highz: %, %',
      lowz, highz;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  p = random_geom_point3D(lowx, highx, lowy, highy, lowz, highz);
  FOR i IN 1..card
  LOOP
    result[i] = p;
    IF i = card THEN EXIT; END IF;
    x = st_x(p);
    y = st_y(p);
    z = st_z(p);
    delta = random_float(-1 * maxdelta, maxdelta);
    /* If neither of these conditions is satisfied the same value is kept */
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSIF (x - delta >= lowx AND x - delta <= highx) THEN
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSIF (y - delta >= lowy AND y - delta <= highy) THEN
      y = y - delta;
    END IF;
    IF (z + delta >= lowz and z + delta <= highz) THEN
      z = z + delta;
    ELSIF (z - delta >= lowz AND z - delta <= highz) THEN
      z = z - delta;
    END IF;
    p = st_makepoint(x, y, z);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, astext(random_geom_point3D_array(-100, 100, -100, 100, 0, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_point3D_array(-100, 100, -100, 100, 0, 100, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random 2D geographic points
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_geog_point_array;
CREATE FUNCTION random_geog_point_array(lowx float, highx float, lowy float,
  highy float, maxdelta float, mincard int, maxcard int)
  RETURNS geography[] AS $$
DECLARE
  pointarr geometry[];
  result geography[];
  card int;
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  SELECT random_geom_point_array(lowx, highx, lowy, highy, maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  FOR i in 1..card
  LOOP
    result[i] = pointarr[i]::geography;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asewkt(random_geom_point_array(-180, 180, -90, 90, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_point_array(-180, 180, -90, 90, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random 3D geographic points
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_geog_point3D_array;
CREATE FUNCTION random_geog_point3D_array(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, mincard int, maxcard int)
  RETURNS geography[] AS $$
DECLARE
  pointarr geometry[];
  result geography[];
  card int;
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  SELECT random_geom_point3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  FOR i in 1..card
  LOOP
    result[i] = pointarr[i]::geography;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asewkt(random_geog_point3D_array(-180, 180, -90, 90, 0, 10000, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_point3D_array(-180, 180, -90, 90, 0, 10000, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geometric linestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geom_linestring;
CREATE FUNCTION random_geom_linestring(lowx float, highx float, lowy float,
  highy float, maxdelta float, minvertices int, maxvertices int)
  RETURNS geometry AS $$
BEGIN
  RETURN st_makeline(random_geom_point_array(lowx, highx, lowy, highy, maxdelta,
    minvertices, maxvertices));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_linestring(-100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT distinct st_issimple(random_geom_linestring(-100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geom_linestring(-100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_linestring(-100, 100, -100, 100, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geometric linestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geom_linestring3D;
CREATE FUNCTION random_geom_linestring3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, minvertices int,
  maxvertices int)
  RETURNS geometry AS $$
BEGIN
  RETURN st_makeline(random_geom_point3D_array(lowx, highx, lowy, highy, lowz,
    highz, maxdelta, minvertices, maxvertices));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_linestring3D(-100, 100, -100, 100, 0, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT distinct st_issimple(random_geom_linestring3D(-100, 100, -100, 100, 0, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geom_linestring3D(-100, 100, -100, 100, 0, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_linestring3D(-100, 100, -100, 100, 0, 100, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geographic linestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geog_linestring;
CREATE FUNCTION random_geog_linestring(lowx float, highx float, lowy float,
  highy float, maxdelta float, minvertices int, maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_linestring(lowx, highx, lowy, highy, maxdelta,
    minvertices, maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_linestring(0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geog_linestring(0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_linestring(0, 80, 0, 80, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geographic linestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geog_linestring3D;
CREATE FUNCTION random_geog_linestring3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxdelta float,
    minvertices int, maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_linestring3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, minvertices, maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_linestring3D(0, 80, 0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geog_linestring3D(0, 80, 0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_linestring3D(0, 80, 0, 0, 80, 80, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geometric polygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geom_polygon;
CREATE FUNCTION random_geom_polygon(lowx float, highx float, lowy float,
  highy float, maxdelta float, minvertices int, maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
  noVertices int;
BEGIN
  IF minvertices < 3 THEN
    raise exception 'A polygon requires at least 3 vertices';
  END IF;
  IF minvertices > maxvertices THEN
    raise exception 'minvertices must be greater than or equal to maxvertices';
  END IF;
  SELECT random_geom_point_array(lowx, highx, lowy, highy, maxdelta, minvertices,
    maxvertices) INTO pointarr;
  noVertices = array_length(pointarr, 1);
  pointarr[noVertices + 1] = pointarr[1];
  RETURN st_makepolygon(st_makeline(pointarr));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_polygon(-100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geom_polygon(-100, 100, -100, 100, 10, 5, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geom_polygon(-100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geometric polygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geom_polygon3D;
CREATE FUNCTION random_geom_polygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, minvertices int,
  maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
  noVertices int;
BEGIN
  IF minvertices < 3 THEN
    raise exception 'A polygon requires at least 3 vertices';
  END IF;
  IF minvertices > maxvertices THEN
    raise exception 'minvertices must be greater than or equal to maxvertices';
  END IF;
  SELECT random_geom_point3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, minvertices, maxvertices) INTO pointarr;
  noVertices = array_length(pointarr, 1);
  pointarr[noVertices + 1] = pointarr[1];
  RETURN st_makepolygon(st_makeline(pointarr));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_polygon3D(-100, 100, -100, 100, 0, 100, 10, 5, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geom_polygon3D(-100, 100, -100, 100, 0, 100, 10, 5, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geom_polygon3D(-100, 100, -100, 100, 0, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geographic polygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geog_polygon;
CREATE FUNCTION random_geog_polygon(lowx float, highx float, lowy float,
  highy float, maxdelta float, minvertices int, maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_polygon(lowx, highx, lowy, highy, maxdelta, minvertices,
    maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_polygon(0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geog_polygon(0, 80, 0, 80, 10, 5, 10) AS g
FROM generate_series(1,10) k;

SELECT k, st_area(random_geog_polygon(0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geographic polygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 */
DROP FUNCTION IF EXISTS random_geog_polygon3D;
CREATE FUNCTION random_geog_polygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, minvertices int,
  maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_polygon3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, minvertices, maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_polygon3D(0, 80, 0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geog_polygon3D(0, 80, 0, 80, 0, 80, 10, 5, 10) AS g
FROM generate_series(1,10) k;

SELECT k, st_area(random_geog_polygon3D(0, 80, 0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geometric multipoint
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the number of points
 */
DROP FUNCTION IF EXISTS random_geom_multipoint;
CREATE FUNCTION random_geom_multipoint(lowx float, highx float, lowy float,
  highy float, maxdelta float, mincard int, maxcard int)
  RETURNS geometry AS $$
BEGIN
  RETURN st_collect(random_geom_point_array(lowx, highx, lowy, highy, maxdelta,
    mincard, maxcard));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_multipoint(-100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_multipoint(-100, 100, -100, 100, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geometric multipoint
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the number of points
 */
DROP FUNCTION IF EXISTS random_geom_multipoint3D;
CREATE FUNCTION random_geom_multipoint3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, mincard int, maxcard int)
  RETURNS geometry AS $$
BEGIN
  RETURN st_collect(random_geom_point3D_array(lowx, highx, lowy, highy, lowz,
    highz, maxdelta, mincard, maxcard));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_multipoint3D(-100, 100, -100, 100, -100, 100, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_multipoint3D(-100, 100, -100, 100, -100, 100, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geographic multipoint
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the number of points
 */
DROP FUNCTION IF EXISTS random_geog_multipoint;
CREATE FUNCTION random_geog_multipoint(lowx float, highx float, lowy float,
  highy float, maxdelta float, mincard int, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_multipoint(lowx, highx, lowy, highy, maxdelta, mincard,
    maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_multipoint(0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_multipoint(0, 80, 0, 80, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geographic multipoint
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] mincard, maxcard Inclusive bounds of the number of points
 */
DROP FUNCTION IF EXISTS random_geog_multipoint3D;
CREATE FUNCTION random_geog_multipoint3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, mincard int, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_multipoint3D(lowx, highx, lowy, highy, lowz, highz, maxdelta, mincard,
    maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_multipoint3D(0, 80, 0, 80, 0, 80, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_multipoint3D(0, 80, 0, 80, 0, 80, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geometric multilinestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of linestrings
 */
DROP FUNCTION IF EXISTS random_geom_multilinestring;
CREATE FUNCTION random_geom_multilinestring(lowx float, highx float, lowy float,
  highy float, maxdelta float, minvertices int, maxvertices int, mincard int,
  maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geom_linestring(lowx, highx, lowy, highy, maxdelta,
    minvertices, maxvertices)) INTO result
  FROM generate_series(mincard, random_int(mincard, maxcard)) AS x;
  RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_multilinestring(-100, 100, -100, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geom_multilinestring(-100, 100, -100, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_multilinestring(-100, 100, -100, 100, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geometric multilinestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of linestrings
 */
DROP FUNCTION IF EXISTS random_geom_multilinestring3D;
CREATE FUNCTION random_geom_multilinestring3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  minvertices int, maxvertices int, mincard int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geom_linestring3D(lowx, highx, lowy, highy, lowz,
    highz, maxdelta, minvertices, maxvertices)) INTO result
  FROM generate_series(mincard, random_int(mincard, maxcard)) AS x;
  RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_multilinestring3D(-100, 100, -100, 100, 0, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geom_multilinestring3D(-100, 100, -100, 100, 0, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_multilinestring3D(-100, 100, -100, 100, 0, 100, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geographic multilinestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of linestrings
 */
DROP FUNCTION IF EXISTS random_geog_multilinestring;
CREATE FUNCTION random_geog_multilinestring(lowx float, highx float, lowy float,
    highy float, maxdelta float, minvertices int, maxvertices int, mincard int,
    maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_multilinestring(lowx, highx, lowy, highy, maxdelta,
    minvertices, maxvertices, mincard, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_multilinestring(0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geog_multilinestring(0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_multilinestring(0, 80, 0, 80, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geographic multilinestring
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of linestrings
 */
DROP FUNCTION IF EXISTS random_geog_multilinestring3D;
CREATE FUNCTION random_geog_multilinestring3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  minvertices int, maxvertices int, mincard int, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_multilinestring3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, minvertices, maxvertices, mincard, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_multilinestring3D(0, 80, 0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_length(random_geog_multilinestring3D(0, 80, 0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_multilinestring3D(0, 80, 0, 80, 0, 80, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geometric multipolygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_geom_multipolygon;
CREATE FUNCTION random_geom_multipolygon(lowx float, highx float, lowy float,
    highy float, maxdelta float, minvertices int, maxvertices int, mincard int,
    maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geom_polygon(lowx, highx, lowy, highy, maxdelta,
    minvertices, maxvertices)) INTO result
  FROM generate_series(mincard, random_int(mincard, maxcard)) AS x;
  RETURN st_collect(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_multipolygon(-100, 100, -100, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_area(random_geom_multipolygon(-100, 100, -100, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_multipolygon(-100, 100, -100, 100, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geometric multipolygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_geom_multipolygon3D;
CREATE FUNCTION random_geom_multipolygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, minvertices int,
  maxvertices int, mincard int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geom_polygon3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, minvertices, maxvertices)) INTO result
  FROM generate_series(mincard, random_int(mincard, maxcard)) AS x;
  RETURN st_collect(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geom_multipolygon3D(-100, 100, -100, 100, 0, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_area(random_geom_multipolygon3D(-100, 100, -100, 100, 0, 100, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geom_multipolygon3D(-100, 100, -100, 100, 0, 100, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D geographic multipolygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_geog_multipolygon;
CREATE FUNCTION random_geog_multipolygon(lowx float, highx float, lowy float,
  highy float, maxdelta float, minvertices int, maxvertices int, mincard int,
  maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_multipolygon(lowx, highx, lowy, highy, maxdelta,
    minvertices, maxvertices, mincard, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_multipolygon(0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_area(random_geog_multipolygon(0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_multipolygon(0, 80, 0, 80, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D geographic multipolygon without holes
 * 
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] maxdelta Maximum difference between two consecutive coordinate values
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of vertices
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_geog_multipolygon3D;
CREATE FUNCTION random_geog_multipolygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, minvertices int,
  maxvertices int, mincard int, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geom_multipolygon3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, minvertices, maxvertices, mincard, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geog_multipolygon3D(0, 80, 0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, st_area(random_geog_multipolygon3D(0, 80, 0, 80, 0, 80, 10, 5, 10, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_geog_multipolygon3D(0, 80, 0, 80, 0, 80, 10, 5, 10, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Instant
-------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeompoint instant
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 */
DROP FUNCTION IF EXISTS random_tgeompoint_inst;
CREATE FUNCTION random_tgeompoint_inst(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeompoint AS $$
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeompointinst(random_geom_point(lowx, highx, lowy, highy),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint_inst(-100, 100, -100, 100, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeompoint instant
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 */
DROP FUNCTION IF EXISTS random_tgeompoint3D_inst;
CREATE FUNCTION random_tgeompoint3D_inst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz)
  RETURNS tgeompoint AS $$
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeompointinst(random_geom_point3D(lowx, highx, lowy, highy, lowz, highz),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint3D_inst(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeogpoint instant
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 */
DROP FUNCTION IF EXISTS random_tgeogpoint_inst;
CREATE FUNCTION random_tgeogpoint_inst(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeogpoint AS $$
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeogpointinst(random_geog_point(lowx, highx, lowy, highy),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint_inst(0, 80, 0, 80, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeogpoint instant
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 */
DROP FUNCTION IF EXISTS random_tgeogpoint3D_inst;
CREATE FUNCTION random_tgeogpoint3D_inst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz)
  RETURNS tgeogpoint AS $$
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeogpointinst(random_geog_point3D(lowx, highx, lowy, highy, lowz, highz),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint3D_inst(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Instant Set
-------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeompoint instant set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeompoint_instset;
CREATE FUNCTION random_tgeompoint_instset(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, mincard int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  pointarr geometry[];
  tsarr timestamptz[];
  result tgeompoint[];
  card int;
BEGIN
  SELECT random_geom_point_array(lowx, highx, lowy, highy, maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tgeompointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeompointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint_instset(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10))
FROM generate_series(1,10) k;

SELECT k, random_tgeompoint_instset(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeompoint instant set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeompoint3D_instset;
CREATE FUNCTION random_tgeompoint3D_instset(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, mincard int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  pointarr geometry[];
  tsarr timestamptz[];
  result tgeompoint[];
  card int;
BEGIN
  SELECT random_geom_point3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tgeompointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeompointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint3D_instset(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10)) AS ti
FROM generate_series(1,10) k;

SELECT k, random_tgeompoint3D_instset(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10)
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeogpoint instant set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeogpoint_instset;
CREATE FUNCTION random_tgeogpoint_instset(lowx float, highx float,
  lowy float, highy float, lowtime timestamptz, hightime timestamptz,
  maxdelta float, maxminutes int, mincard int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
BEGIN
  SELECT random_geog_point_array(lowx, highx, lowy, highy, maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tgeogpointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeogpointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint_instset(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10))
FROM generate_series(1,10) k;

SELECT k, random_tgeogpoint_instset(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeogpoint instant set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeogpoint3D_instset;
CREATE FUNCTION random_tgeogpoint3D_instset(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, mincard int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
BEGIN
  SELECT random_geog_point3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tgeogpointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeogpointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint3D_instset(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10))
FROM generate_series(1,10) k;

SELECT k, random_tgeogpoint3D_instset(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Sequence
-------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeompoint sequence
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tgeompoint_seq;
CREATE FUNCTION random_tgeompoint_seq(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, mincard int, maxcard int, fixstart bool DEFAULT false)
  RETURNS tgeompoint AS $$
DECLARE
  pointarr geometry[];
  tsarr timestamptz[];
  result tgeompoint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geom_point_array(lowx, highx, lowy, highy, maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card,
    fixstart) INTO tsarr;
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card
  LOOP
    result[i] = tgeompointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeompointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint_seq(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeompoint_seq(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeompoint sequence
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tgeompoint3D_seq;
CREATE FUNCTION random_tgeompoint3D_seq(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, mincard int, maxcard int,
  fixstart bool DEFAULT false)
  RETURNS tgeompoint AS $$
DECLARE
  pointarr geometry[];
  tsarr timestamptz[];
  result tgeompoint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geom_point3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card,
    fixstart) INTO tsarr;
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card
  LOOP
    result[i] = tgeompointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeompointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint3D_seq(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeompoint3D_seq(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeogpoint sequence
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tgeogpoint_seq;
CREATE FUNCTION random_tgeogpoint_seq(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, mincard int, maxcard int, fixstart bool DEFAULT false)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geog_point_array(lowx, highx, lowy, highy, maxdelta, mincard, maxcard)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card,
    fixstart) INTO tsarr;
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card
  LOOP
    result[i] = tgeogpointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeogpointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint_seq(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeogpoint_seq(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeogpoint sequence
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tgeogpoint3D_seq;
CREATE FUNCTION random_tgeogpoint3D_seq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, mincard int,
  maxcard int, fixstart bool DEFAULT false)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geog_point3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, mincard, maxcard) INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card,
    fixstart) INTO tsarr;
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card
  LOOP
    result[i] = tgeogpointinst(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeogpointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint3D_seq(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeogpoint3D_seq(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeompoint sequence set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeompoint_seqset;
CREATE FUNCTION random_tgeompoint_seqset(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  seq tgeompoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM valid_duration_tsequenceset(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeompoint_seq(lowx, highx, lowy, highy, t1, t2, maxdelta,
      maxminutes, mincardseq, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint_seqset(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10)) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeompoint_seqset(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeompoint sequence set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeompoint3D_seqset;
CREATE FUNCTION random_tgeompoint3D_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, mincardseq int,
  maxcardseq int, mincard int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  seq tgeompoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM valid_duration_tsequenceset(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeompoint3D_seq(lowx, highx, lowy, highy, lowz, highz,
      t1, t2, maxdelta, maxminutes, mincardseq, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint3D_seqset(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10)) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeompoint3D_seqset(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, numSequences(random_tgeompoint3D_seqset(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, asText(endSequence(random_tgeompoint3D_seqset(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10))) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 2D tgeogpoint sequence set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeogpoint_seqset;
CREATE FUNCTION random_tgeogpoint_seqset(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  seq tgeogpoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM valid_duration_tsequenceset(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeogpoint_seq(lowx, highx, lowy, highy, t1, t2, maxdelta,
      maxminutes, mincardseq, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint_seqset(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10)) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeogpoint_seqset(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random 3D tgeogpoint sequence set
 *
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the period
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of polygons
 */
DROP FUNCTION IF EXISTS random_tgeogpoint3D_seqset;
CREATE FUNCTION random_tgeogpoint3D_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, mincardseq int,
  maxcardseq int, mincard int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  seq tgeogpoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM valid_duration_tsequenceset(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeogpoint3D_seq(lowx, highx, lowy, highy, lowz, highz,
      t1, t2, maxdelta, maxminutes, mincardseq, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10)) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeogpoint3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, numSequences(random_tgeogpoint3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, asText(endSequence(random_tgeogpoint3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10))) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/*****************************************************************************
 *
 * random_tpoint.sql
 *    Basic synthetic data generator functions for geometry/geography types
 *    and temporal point types.
 *
 * These functions use lower and upper bounds for the generated values:
 * lowx/lowy/lowz and highx/highy/highz for coordinates, lowtime and hightime
 * for timestamps. When generating series of values, the maxdelta argument
 * states the maximum difference between two consecutive coordinate values,
 * while maxminutes states the maximum number of minutes between two
 * consecutive timestamps as well as the maximum number of minutes for time
 * gaps between two consecutive components of temporal instant/sequence sets.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

-------------------------------------------------------------------------------
-- STBox Type
-------------------------------------------------------------------------------

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

DROP FUNCTION IF EXISTS random_geompoint;
CREATE FUNCTION random_geompoint(lowx float, highx float, lowy float,
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
SELECT k, st_astext(random_geompoint(-100, 100, -100, 100))
FROM generate_series(1,10) k;

SELECT k, random_geompoint(-100, 100, -100, 100) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geompoint3D;
CREATE FUNCTION random_geompoint3D(lowx float, highx float, lowy float,
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
SELECT k, st_astext(random_geompoint3D(-100, 100, -100, 100, 0, 100))
FROM generate_series(1,10) k;

SELECT k, random_geompoint3D(-100, 100, -100, 100, 0, 100) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogpoint;
CREATE FUNCTION random_geogpoint(lowx float, highx float,
  lowy float, highy float)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geompoint(lowx, highx, lowy, highy)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogpoint(-180, 180, 90, 90))
FROM generate_series(1,10) k;

SELECT k, random_geogpoint(-180, 180, 90, 90) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogpoint3D;
CREATE FUNCTION random_geogpoint3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geompoint3D(lowx, highx, lowy, highy, lowz, highz)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogpoint3D(0, 90, 0, 90, 0, 90))
FROM generate_series(1,10) k;

SELECT k, random_geogpoint3D(0, 90, 0, 90, 0, 90) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geompoint_array;
CREATE FUNCTION random_geompoint_array(lowx float, highx float, lowy float,
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
  card = random_int(mincard, maxcard);
  p = random_geompoint(lowx, highx, lowy, highy);
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
SELECT k, astext(random_geompoint_array(-100, 100, -100, 100, 10, 1, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geompoint_array(-100, 100, -100, 100, 10, 1, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geompoint3D_array;
CREATE FUNCTION random_geompoint3D_array(lowx float, highx float, lowy float,
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
  card = random_int(mincard, maxcard);
  p = random_geompoint3D(lowx, highx, lowy, highy, lowz, highz);
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
SELECT k, astext(random_geompoint3D_array(-100, 100, -100, 100, 0, 100, 10, 1, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geompoint3D_array(-100, 100, -100, 100, 0, 100, 10, 1, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogpoint_array;
CREATE FUNCTION random_geogpoint_array(lowx float, highx float, lowy float,
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
  SELECT random_geompoint_array(lowx, highx, lowy, highy, maxdelta, mincard, maxcard)
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
SELECT k, asewkt(random_geompoint_array(-180, 180, -90, 90, 10, 1, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geompoint_array(-180, 180, -90, 90, 10, 1, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogpoint3D_array;
CREATE FUNCTION random_geogpoint3D_array(lowx float, highx float, lowy float,
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
  SELECT random_geompoint3D_array(lowx, highx, lowy, highy, lowz, highz,
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
SELECT k, asewkt(random_geogpoint3D_array(-180, 180, -90, 90, 0, 10000, 10, 1, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogpoint3D_array(-180, 180, -90, 90, 0, 10000, 10, 1, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geomlinestring;
CREATE FUNCTION random_geomlinestring(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxvertices int)
  RETURNS geometry AS $$
BEGIN
  RETURN st_makeline(random_geompoint_array(lowx, highx, lowy, highy, maxdelta,
    1, maxvertices));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geomlinestring(-100, 100, -100, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT distinct st_issimple(random_geomlinestring(-100, 100, -100, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geomlinestring(-100, 100, -100, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geomlinestring(-100, 100, -100, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geomlinestring3D;
CREATE FUNCTION random_geomlinestring3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, maxvertices int)
  RETURNS geometry AS $$
BEGIN
  RETURN st_makeline(random_geompoint3D_array(lowx, highx, lowy, highy, lowz,
    highz, maxdelta, 1, maxvertices));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geomlinestring3D(-100, 100, -100, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT distinct st_issimple(random_geomlinestring3D(-100, 100, -100, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geomlinestring3D(-100, 100, -100, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geomlinestring3D(-100, 100, -100, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geoglinestring;
CREATE FUNCTION random_geoglinestring(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geomlinestring(lowx, highx, lowy, highy, maxdelta,
    maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geoglinestring(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geoglinestring(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geoglinestring(0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geoglinestring3D;
CREATE FUNCTION random_geoglinestring3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxdelta float,
    maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geomlinestring3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geoglinestring3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geoglinestring3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geoglinestring3D(0, 80, 0, 0, 80, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geompolygon;
CREATE FUNCTION random_geompolygon(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
  noVertices int;
BEGIN
  IF maxvertices < 3 THEN
    raise exception 'A polygon requires at least 3 vertices';
  END IF;
  SELECT random_geompoint_array(lowx, highx, lowy, highy, maxdelta, 3,
    maxvertices) INTO pointarr;
  noVertices = array_length(pointarr, 1);
  pointarr[noVertices + 1] = pointarr[1];
  RETURN st_makepolygon(st_makeline(pointarr));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geompolygon(-100, 100, -100, 100, 10, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geompolygon(-100, 100, -100, 100, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geompolygon(-100, 100, -100, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geompolygon3D;
CREATE FUNCTION random_geompolygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
  noVertices int;
BEGIN
  IF maxvertices < 3 THEN
    raise exception 'A polygon requires at least 3 vertices';
  END IF;
  SELECT random_geompoint3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, 3, maxvertices) INTO pointarr;
  noVertices = array_length(pointarr, 1);
  pointarr[noVertices + 1] = pointarr[1];
  RETURN st_makepolygon(st_makeline(pointarr));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geompolygon3D(-100, 100, -100, 100, 0, 100, 10, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geompolygon3D(-100, 100, -100, 100, 0, 100, 10, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geompolygon3D(-100, 100, -100, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogpolygon;
CREATE FUNCTION random_geogpolygon(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geompolygon(lowx, highx, lowy, highy, maxdelta,
    maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogpolygon(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geogpolygon(0, 80, 0, 80, 10, 10) AS g
FROM generate_series(1,10) k;

SELECT k, st_area(random_geogpolygon(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogpolygon3D;
CREATE FUNCTION random_geogpolygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, maxvertices int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geompolygon3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, maxvertices)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogpolygon3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series(1,10) k;

SELECT k, random_geogpolygon3D(0, 80, 0, 80, 0, 80, 10, 10) AS g
FROM generate_series(1,10) k;

SELECT k, st_area(random_geogpolygon3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geommultipoint;
CREATE FUNCTION random_geommultipoint(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxcard int)
  RETURNS geometry AS $$
BEGIN
  RETURN st_collect(random_geompoint_array(lowx, highx, lowy, highy, maxdelta,
    1, maxcard));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipoint(-100, 100, -100, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipoint(-100, 100, -100, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogmultipoint;
CREATE FUNCTION random_geogmultipoint(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geommultipoint(lowx, highx, lowy, highy, maxdelta,
    maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogmultipoint(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipoint(0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogmultipoint3D;
CREATE FUNCTION random_geogmultipoint3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geommultipoint(lowx, highx, lowy, highy, maxdelta,
    maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogmultipoint3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipoint3D(0, 80, 0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geommultilinestring;
CREATE FUNCTION random_geommultilinestring(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxvertices int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geomlinestring(lowx, highx, lowy, highy, maxdelta,
    maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultilinestring(-100, 100, -100, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geommultilinestring(-100, 100, -100, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultilinestring(-100, 100, -100, 100, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geommultilinestring3D;
CREATE FUNCTION random_geommultilinestring3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  maxvertices int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geomlinestring3D(lowx, highx, lowy, highy, lowz,
    highz, maxdelta, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultilinestring3D(-100, 100, -100, 100, 0, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geommultilinestring3D(-100, 100, -100, 100, 0, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultilinestring3D(-100, 100, -100, 100, 0, 100, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogmultilinestring;
CREATE FUNCTION random_geogmultilinestring(lowx float, highx float,
    lowy float, highy float, maxdelta float, maxvertices int, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geommultilinestring(lowx, highx, lowy, highy, maxdelta,
    maxvertices, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogmultilinestring(0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geogmultilinestring(0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultilinestring(0, 80, 0, 80, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogmultilinestring3D;
CREATE FUNCTION random_geogmultilinestring3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  maxvertices int, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geommultilinestring3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, maxvertices, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geommultipolygon;
CREATE FUNCTION random_geommultipolygon(lowx float, highx float, lowy float,
    highy float, maxdelta float, maxvertices int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geompolygon(lowx, highx, lowy, highy, maxdelta,
    maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_collect(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipolygon(-100, 100, -100, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geommultipolygon(-100, 100, -100, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipolygon(-100, 100, -100, 100, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geommultipolygon3D;
CREATE FUNCTION random_geommultipolygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, maxvertices int,
  maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geompolygon3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_collect(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipolygon3D(-100, 100, -100, 100, 0, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geommultipolygon3D(-100, 100, -100, 100, 0, 100, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipolygon3D(-100, 100, -100, 100, 0, 100, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogmultipolygon;
CREATE FUNCTION random_geogmultipolygon(lowx float, highx float, lowy float,
  highy float, maxdelta float, maxvertices int, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geommultipolygon(lowx, highx, lowy, highy, maxdelta,
    maxvertices, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogmultipolygon(0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geogmultipolygon(0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipolygon(0, 80, 0, 80, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_geogmultipolygon3D;
CREATE FUNCTION random_geogmultipolygon3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, maxdelta float, maxvertices int,
  maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geommultipolygon3D(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, maxvertices, maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Instant
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompointinst;
CREATE FUNCTION random_tgeompointinst(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeompoint AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeompointinst(random_geompoint(lowx, highx, lowy, highy),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompointinst(-100, 100, -100, 100, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompoint3Dinst;
CREATE FUNCTION random_tgeompoint3Dinst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz)
  RETURNS tgeompoint AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeompointinst(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint3Dinst(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpointinst;
CREATE FUNCTION random_tgeogpointinst(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeogpoint AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpointinst(0, 80, 0, 80, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpoint3Dinst;
CREATE FUNCTION random_tgeogpoint3Dinst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz)
  RETURNS tgeogpoint AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint3Dinst(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Instant Set
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompointi;
CREATE FUNCTION random_tgeompointi(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  pointarr geometry[];
  tsarr timestamptz[];
  result tgeompoint[];
  card int;
BEGIN
  SELECT random_geompoint_array(lowx, highx, lowy, highy, maxdelta, 1, maxcard)
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
SELECT k, asText(random_tgeompointi(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 10))
FROM generate_series(1,10) k;

SELECT k, random_tgeompointi(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompoint3Di;
CREATE FUNCTION random_tgeompoint3Di(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  pointarr geometry[];
  tsarr timestamptz[];
  result tgeompoint[];
  card int;
BEGIN
  SELECT random_geompoint3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, 1, maxcard)
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
SELECT k, asText(random_tgeompoint3Di(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10)) AS ti
FROM generate_series(1,10) k;

SELECT k, random_tgeompoint3Di(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10)
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpointi;
CREATE FUNCTION random_tgeogpointi(lowx float, highx float,
  lowy float, highy float, lowtime timestamptz, hightime timestamptz,
  maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
BEGIN
  SELECT random_geogpoint_array(lowx, highx, lowy, highy, maxdelta, 1, maxcard)
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
SELECT k, asEwkt(random_tgeogpointi(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10))
FROM generate_series(1,10) k;

SELECT k, random_tgeogpointi(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpoint3Di;
CREATE FUNCTION random_tgeogpoint3Di(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
BEGIN
  SELECT random_geogpoint3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, 1, maxcard)
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
SELECT k, asEwkt(random_tgeogpoint3Di(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10))
FROM generate_series(1,10) k;

SELECT k, random_tgeogpoint3Di(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Sequence
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompointseq;
CREATE FUNCTION random_tgeompointseq(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, maxcard int, fixstart bool DEFAULT false)
  RETURNS tgeompoint AS $$
DECLARE
  pointarr geometry[];
  tsarr timestamptz[];
  result tgeompoint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geompoint_array(lowx, highx, lowy, highy, maxdelta, 1, maxcard)
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
SELECT k, asText(random_tgeompointseq(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeompointseq(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompoint3Dseq;
CREATE FUNCTION random_tgeompoint3Dseq(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcard int,
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
  SELECT random_geompoint3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, 1, maxcard)
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
SELECT k, asText(random_tgeompoint3Dseq(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeompoint3Dseq(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpointseq;
CREATE FUNCTION random_tgeogpointseq(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, maxcard int, fixstart bool DEFAULT false)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geogpoint_array(lowx, highx, lowy, highy, maxdelta, 1, maxcard)
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
SELECT k, asEwkt(random_tgeogpointseq(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeogpointseq(0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpoint3Dseq;
CREATE FUNCTION random_tgeogpoint3Dseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcard int,
  fixstart bool DEFAULT false)
  RETURNS tgeogpoint AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeogpoint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geogpoint3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, 1, maxcard) INTO pointarr;
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
SELECT k, asEwkt(random_tgeogpoint3Dseq(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeogpoint3Dseq(0, 80, 0, 80, 0, 80, '2001-01-01', '2002-01-01', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompoints;
CREATE FUNCTION random_tgeompoints(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, maxcardseq int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  seq tgeompoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeompointseq(lowx, highx, lowy, highy, t1, t2, maxdelta,
      maxminutes, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoints(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeompoints(-100, 100, -100, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeompoint3Ds;
CREATE FUNCTION random_tgeompoint3Ds(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcardseq int,
  maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  seq tgeompoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeompoint3Dseq(lowx, highx, lowy, highy, lowz, highz,
      t1, t2, maxdelta, maxminutes, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint3Ds(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeompoint3Ds(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, numSequences(random_tgeompoint3Ds(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, asText(endSequence(random_tgeompoint3Ds(-100, 100, -100, 100, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10))) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpoints;
CREATE FUNCTION random_tgeogpoints(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxdelta float,
  maxminutes int, maxcardseq int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  seq tgeogpoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeogpointseq(lowx, highx, lowy, highy, t1, t2, maxdelta,
      maxminutes, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoints(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeogpoints(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

DROP FUNCTION IF EXISTS random_tgeogpoint3Ds;
CREATE FUNCTION random_tgeogpoint3Ds(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcardseq int,
  maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  seq tgeogpoint;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      '( (maxminutes * maxcardseq * maxcard) + ((maxcard - 1) * maxminutes) ) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, maxcardseq, maxcard;
  END IF;
  card = random_int(1, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * maxcardseq * (maxcard - 1)) + ((maxcard - 1) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tgeogpoint3Dseq(lowx, highx, lowy, highy, lowz, highz,
      t1, t2, maxdelta, maxminutes, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq);
  END LOOP;
  RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asEwkt(random_tgeogpoint3Ds(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, random_tgeogpoint3Ds(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, numSequences(random_tgeogpoint3Ds(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, asText(endSequence(random_tgeogpoint3Ds(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 10, 10, 10))) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

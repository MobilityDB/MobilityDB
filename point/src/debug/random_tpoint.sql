-------------------------------------------------------------------------------
-- STBox Type
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_stbox(lowx float, highx float, lowy float,
  highy float, lowt timestamptz, hight timestamptz, maxsize float,
  maxminutes int)
  RETURNS stbox AS $$
DECLARE
  xmin float;
  ymin float;
  tmin timestamptz;
BEGIN
  xmin = random_float(lowx, highx);
  ymin = random_float(lowy, highy);
  tmin = random_timestamptz(lowt, hight);
  RETURN stbox_t(xmin, ymin, tmin, xmin + random_float(1, maxsize),
    ymin + random_float(1, maxsize), tmin + random_minutes(1, maxminutes));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_stbox(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_stbox3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowt timestamptz, hight timestamptz,
  maxsize float, maxminutes int)
  RETURNS stbox AS $$
DECLARE
  xmin float;
  ymin float;
  zmin float;
  tmin timestamptz;
BEGIN
  xmin = random_float(lowx, highx);
  ymin = random_float(lowy, highy);
  zmin = random_float(lowz, highz);
  tmin = random_timestamptz(lowt, hight);
  RETURN stbox_zt(xmin, ymin, zmin, tmin, xmin + random_float(1, maxsize),
    ymin + random_float(1, maxsize), zmin + random_float(1, maxsize),
    tmin + random_minutes(1, maxminutes));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_stbox3D(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geodstbox(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowt timestamptz,
  hight timestamptz, maxsize float, maxminutes int)
  RETURNS stbox AS $$
DECLARE
  xmin float;
  ymin float;
  zmin float;
  tmin timestamptz;
BEGIN
  xmin = random_float(lowx, highx);
  ymin = random_float(lowy, highy);
  zmin = random_float(lowz, highz);
  tmin = random_timestamptz(lowt, hight);
  RETURN geodstbox_t(xmin, ymin, zmin, tmin, xmin + random_float(1, maxsize),
    ymin + random_float(1, maxsize), zmin + random_float(1, maxsize),
    tmin + random_minutes(1, maxminutes));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geodstbox(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geodstbox3D(lowx float, highx float, 
  lowy float, highy float, lowz float, highz float, lowt timestamptz, 
  hight timestamptz, maxsize float, maxminutes int)
  RETURNS stbox AS $$
DECLARE
  xmin float;
  ymin float;
  zmin float;
  tmin timestamptz;
BEGIN
  xmin = random_float(lowx, highx);
  ymin = random_float(lowy, highy);
  zmin = random_float(lowz, highz);
  tmin = random_timestamptz(lowt, hight);
  RETURN geodstbox_zt(xmin, ymin, zmin, tmin, xmin + random_float(1, maxsize),
    ymin + random_float(1, maxsize), zmin + random_float(1, maxsize),
    tmin + random_minutes(1, maxminutes));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geodstbox3D(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Geometry/Geography
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompoint(lowx float, highx float,
  lowy float, highy float)
  RETURNS geometry AS $$
BEGIN
  RETURN st_point(random_float(lowx, highx), random_float(lowy, highy));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompoint(0, 100, 0, 100) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geompoint(0, 100, 0, 100))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompoint3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float)
  RETURNS geometry AS $$
BEGIN
  RETURN st_makepoint(random_float(lowx, highx), random_float(lowy, highy),
    random_float(lowz, highz));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompoint3D(0, 100, 0, 100, 0, 100) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geompoint3D(0, 100, 0, 100, 0, 100))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpoint(lowx float, highx float,
  lowy float, highy float)
  RETURNS geography AS $$
BEGIN
  RETURN random_geompoint(lowx, highx, lowy, highy)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geogpoint(0, 90, 0, 90) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geogpoint(0, 90, 0, 90))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpoint3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float)
  RETURNS geography AS $$
BEGIN
  RETURN random_geompoint3D(lowx, highx, lowy, highy, lowz, highz)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geogpoint3D(0, 90, 0, 90, 0, 90) AS g
FROM generate_series(1,10) k;

SELECT k, st_astext(random_geogpoint3D(0, 90, 0, 90, 0, 90))
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompoint_array(lowx float, highx float,
  lowy float, highy float, maxdelta float, maxcard int)
  RETURNS geometry[] AS $$
DECLARE
  result geometry[];
  card int;
  x float;
  y float;
  delta float;
  p geometry;
BEGIN
  card = random_int(1, maxcard);
  p = random_geompoint(lowx, highx, lowy, highy);
  FOR i IN 1..card
  LOOP
    result[i] = p;
    IF i = card THEN EXIT; END IF;
    x = st_x(p);
    y = st_y(p);
    delta = random_float(-1 * maxdelta, maxdelta);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    p = st_makepoint(x, y);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, astext(random_geompoint_array(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geompoint_array(0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompoint3D_array(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  maxcard int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
  card int;
  x float;
  y float;
  z float;
  delta float;
  p geometry;
BEGIN
  card = random_int(1, maxcard);
  p = random_geompoint3D(lowx, highx, lowy, highy, lowz, highz);
  FOR i IN 1..card
  LOOP
    pointarr[i] = p;
    IF i = card THEN EXIT; END IF;
    x = st_x(p);
    y = st_y(p);
    z = st_z(p);
    delta = random_float(-1 * maxdelta, maxdelta);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    IF (z + delta >= lowz and z + delta <= highz) THEN
      z = z + delta;
    ELSE
      z = z - delta;
    END IF;
    p = st_makepoint(x, y, z);
  END LOOP;
  RETURN st_collect(pointarr);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geompoint3D_array(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geompoint3D_array(0, 100, 0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpoint_array(lowx float, highx float,
  lowy float, highy float, maxdelta float, maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN 
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geompoint_array(lowx, highx, lowy, highy, maxdelta,
    maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geompoint_array(-180, 180, -90, 90, 0, 10000, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geompoint_array(-180, 180, -90, 90, 0, 10000, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpoint3D_array(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  maxcard int)
  RETURNS geography AS $$
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN 
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geommultipoint3D(lowx, highx, lowy, highy, lowz, highz, maxdelta,
    maxcard)::geography;
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_asewkt(random_geogmultipoint3D(-180, 180, -90, 90, 0, 10000, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipoint3D(-180, 180, -90, 90, 0, 10000, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geomlinestring(lowx float, highx float,
  lowy float, highy float, maxdelta float, maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
BEGIN
  SELECT random_geompoint_array(lowx, highx, lowy, highy, maxdelta, maxvertices) 
  INTO pointarr;
  RETURN st_makeline(pointarr);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geomlinestring(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT distinct st_issimple(random_geomlinestring(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geomlinestring(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geomlinestring(0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geomlinestring3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
BEGIN
  SELECT random_geompoint3D_array(lowx, highx, lowy, highy, lowz, highz,
    maxdelta, maxvertices) 
  INTO pointarr;
  RETURN st_makeline(pointarr);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT distinct st_issimple(random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geomlinestring3D(0, 100, 0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geoglinestring(lowx float, highx float,
    lowy float, highy float, maxdelta float, maxvertices int)
  RETURNS geography AS $$
BEGIN
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

CREATE OR REPLACE FUNCTION random_geoglinestring3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxdelta float,
    maxvertices int)
  RETURNS geography AS $$
BEGIN
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

CREATE OR REPLACE FUNCTION random_geompolygon(lowx float, highx float,
  lowy float, highy float, maxdelta float, maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
  noVertices int;
  x float;
  y float;
  delta float;
  p geometry;
BEGIN
  IF maxvertices < 3 THEN 
    raise exception 'A polygon requires at least 3 vertices';
  END IF;
  noVertices = random_int(3, maxvertices);
  p = random_geompoint(lowx, highx, lowy, highy);
  FOR i IN 1..noVertices
  LOOP
    pointarr[i] = p;
    IF i = noVertices THEN EXIT; END IF;
    x = st_x(p);
    y = st_y(p);
    delta = random_float(-1 * maxdelta, maxdelta);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    p = st_point(x, y);
  END LOOP;
  pointarr[noVertices + 1] = pointarr[1];
  RETURN st_makepolygon(st_makeline(pointarr));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompolygon(0, 100, 0, 100, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geompolygon(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_astext(random_geompolygon(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geompolygon3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  maxvertices int)
  RETURNS geometry AS $$
DECLARE
  pointarr geometry[];
  noVertices int;
  x float;
  y float;
  z float;
  delta float;
  p geometry;
BEGIN
  IF maxvertices < 3 THEN 
    raise exception 'A polygon requires at least 3 vertices';
  END IF;
  noVertices = random_int(3, maxvertices);
  p = random_geompoint3D(lowx, highx, lowy, highy, lowz, highz);
  FOR i IN 1..noVertices
  LOOP
    pointarr[i] = p;
    IF i = noVertices THEN EXIT; END IF;
    x = st_x(p);
    y = st_y(p);
    z = st_z(p);
    delta = random_float(-1 * maxdelta, maxdelta);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    IF (z + delta >= lowz and z + delta <= highz) THEN
      z = z + delta;
    ELSE
      z = z - delta;
    END IF;
    p = st_makepoint(x, y, z);
  END LOOP;
  pointarr[noVertices+1] = pointarr[1];
  RETURN st_makepolygon(st_makeline(pointarr));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_geompolygon3D(0, 100, 0, 100, 0, 100, 10, 10) AS g
FROM generate_series(1,10) k;

SELECT distinct st_isvalid(random_geompolygon3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_astext(random_geompolygon3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogpolygon(lowx float, highx float,
  lowy float, highy float, maxdelta float, maxvertices int)
  RETURNS geography AS $$
BEGIN
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

CREATE OR REPLACE FUNCTION random_geogpolygon3D(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, maxdelta float,
  maxvertices int)
  RETURNS geography AS $$
BEGIN
  RETURN random_geompolygon3d(lowx, highx, lowy, highy, lowz, highz,
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

CREATE OR REPLACE FUNCTION random_geommultipoint(lowx float, highx float,
    lowy float, highy float, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geompoint(lowx, highx, lowy, highy)) INTO result
  FROM generate_series (1, random_int(1, maxcard)) AS x;
  RETURN st_collect(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipoint(0, 100, 0, 100, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipoint(0, 100, 0, 100, 10) AS g
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipoint(lowx float, highx float,
    lowy float, highy float, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN 
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  SELECT array_agg(random_geompoint(lowx, highx, lowy, highy)) INTO result
  FROM generate_series (1, random_int(1, maxcard)) AS x;
  RETURN st_setsrid(st_collect(result),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipoint(0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipoint(0, 80, 0, 80, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipoint3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN 
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  SELECT array_agg(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz)) INTO result
  FROM generate_series (1, random_int(1, maxcard)) AS x;
  RETURN st_setsrid(st_collect(result),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipoint3D(0, 80, 0, 80, 0, 80, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipoint3D(0, 80, 0, 80, 0, 80, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultilinestring(lowx float, highx float,
    lowy float, highy float, maxvertices int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geomlinestring(lowx, highx, lowy, highy, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultilinestring(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geommultilinestring(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultilinestring(0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultilinestring3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geomlinestring3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_unaryunion(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultilinestring3D(0, 100, 0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultilinestring(lowx float, highx float,
    lowy float, highy float, maxvertices int, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geomlinestring(lowx, highx, lowy, highy, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_setsrid(st_unaryunion(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultilinestring(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geogmultilinestring(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultilinestring(0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultilinestring3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geomlinestring3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_setsrid(st_unaryunion(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_length(random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultilinestring3D(0, 80, 0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultipolygon(lowx float, highx float,
    lowy float, highy float, maxvertices int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geompolygon(lowx, highx, lowy, highy, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_makevalid(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipolygon(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geommultipolygon(0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipolygon(0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geommultipolygon3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int)
  RETURNS geometry AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geompolygon3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_makevalid(st_collect(result));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geommultipolygon3D(0, 100, 0, 100, 0, 100, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipolygon(lowx float, highx float,
    lowy float, highy float, maxvertices int, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geompolygon(lowx, highx, lowy, highy, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_setsrid(st_makevalid(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipolygon(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geogmultipolygon(0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipolygon(0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_geogmultipolygon3D(lowx float, highx float,
    lowy float, highy float, lowz float, highz float, maxvertices int, maxcard int)
  RETURNS geography AS $$
DECLARE
  result geometry[];
BEGIN
  SELECT array_agg(random_geompolygon3D(lowx, highx, lowy, highy, lowz, highz, maxvertices)) INTO result
  FROM generate_series (1, random_int(2, maxcard)) AS x;
  RETURN st_setsrid(st_makevalid(st_collect(result)),4326);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, st_astext(random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, st_area(random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10)) AS g
FROM generate_series (1, 15) AS k;

SELECT k, random_geogmultipolygon3D(0, 80, 0, 80, 0, 80, 10, 10) AS g
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
-- Temporal Instant
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompointinst(lowx float, highx float,
  lowy float, highy float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeompoint AS $$
BEGIN
  RETURN tgeompointinst(random_geompoint(lowx, highx, lowy, highy), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompointinst(0, 100, 0, 100, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Dinst(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeompoint AS $$
BEGIN
  RETURN tgeompointinst(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeompoint3Dinst(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpointinst(lowx float, highx float,
  lowy float, highy float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeogpoint AS $$
BEGIN
  RETURN tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeogpointinst(0, 80, 0, 80, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Dinst(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz, hightime timestamptz)
  RETURNS tgeogpoint AS $$
BEGIN
  RETURN tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, asText(random_tgeogpoint3Dinst(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Instant Set
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompointi(lowx float, highx float,
  lowy float, highy float, lowtime timestamptz, hightime timestamptz, 
  maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  delta float;
  x float;
  y float;
  p geometry;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  p = random_geompoint(lowx, highx, lowy, highy);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    delta = random_float(-1 * maxdelta, maxdelta);
    x = st_x(p);
    y = st_y(p);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    p = st_point(x, y);
    result[i] = tgeompointinst(p, t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeompointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, asText(random_tgeompointi(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Di(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  x float;
  y float;
  z float;
  delta float;
  p geometry;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  p = random_geompoint3D(lowx, highx, lowy, highy, lowz, highz);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    delta = random_float(-1 * maxdelta, maxdelta);
    x = st_x(p);
    y = st_y(p);
    z = st_z(p);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    IF (z + delta >= lowz and z + delta <= highz) THEN
      z = z + delta;
    ELSE
      z = z - delta;
    END IF;
    p = st_makepoint(x, y, z);
    result[i] = tgeompointinst(p, t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeompointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoint3Di(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series(1,10) k;

SELECT k, asText(random_tgeompoint3Di(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) AS ti
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpointi(lowx float, highx float,
  lowy float, highy float, lowtime timestamptz, hightime timestamptz,
  maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  x float;
  y float;
  delta float;
  p geometry;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  p = random_geogpoint(lowx, highx, lowy, highy);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tgeogpointinst(p, t);
    IF i = card THEN EXIT; END IF; 
    delta = random_float(-1 * maxdelta, maxdelta);
    x = st_x(p);
    y = st_y(p);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    p = st_makepoint(x, y);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeogpointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpointi(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, asText(random_tgeogpointi(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10, 10))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Di(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  x float;
  y float;
  z float;
  delta float;
  p geometry;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  p = random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tgeogpointinst(p, t);
    IF i = card THEN EXIT; END IF; 
    delta = random_float(-1 * maxdelta, maxdelta);
    x = st_x(p);
    y = st_y(p);
    z = st_z(p);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    IF (z + delta >= lowz and z + delta <= highz) THEN
      z = z + delta;
    ELSE
      z = z - delta;
    END IF;
    p = st_makepoint(x, y, z);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeogpointi(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoint3Di(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, asText(random_tgeogpoint3Di(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10, 10))
FROM generate_series(1,10) k;
*/
-------------------------------------------------------------------------------
-- Temporal Sequence
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompointseq(lowx float, highx float, 
  lowy float, highy float, lowtime timestamptz, hightime timestamptz, 
  maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  t timestamptz;
  x float;
  y float;
  delta float;
  p geometry;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  p = random_geompoint(lowx, highx, lowy, highy);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    delta = random_float(-1 * maxdelta, maxdelta);
    x = st_x(p);
    y = st_y(p);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    p = st_point(x, y);
    t = t + random_minutes(1, maxminutes);
    result[i] = tgeompointinst(p, t);
  END LOOP;
  RETURN tgeompointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, asText(random_tgeompointseq(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Dseq(lowx float, highx float, 
  lowy float, highy float, lowz float, highz float, lowtime timestamptz, 
  hightime timestamptz, maxdelta float, maxminutes int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  card int;
  x float;
  y float;
  z float;
  delta float;
  t timestamptz;
  p geometry;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  p = random_geompoint3D(lowx, highx, lowy, highy, lowz, highz);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    delta = random_float(-1 * maxdelta, maxdelta);
    x = st_x(p);
    y = st_y(p);
    z = st_z(p);
    IF (x + delta >= lowx and x + delta <= highx) THEN
      x = x + delta;
    ELSE
      x = x - delta;
    END IF;
    IF (y + delta >= lowy and y + delta <= highy) THEN
      y = y + delta;
    ELSE
      y = y - delta;
    END IF;
    IF (z + delta >= lowz and z + delta <= highz) THEN
      z = z + delta;
    ELSE
      z = z - delta;
    END IF;
    p = st_makepoint(x, y, z);
    t = t + random_minutes(1, maxminutes);
    result[i] = tgeompointinst(p, t);
  END LOOP;
  RETURN tgeompointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, asText(random_tgeompoint3Dseq(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpointseq(lowx float, highx float, lowy float, highy float,
  lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    t = t + random_minutes(1, maxminutes);
    result[i] = tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy), t);
  END LOOP;
  RETURN tgeogpointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpointseq(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, asText(random_tgeogpointseq(0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Dseq(lowx float, highx float, lowy float, highy float,
  lowz float, highz float, lowtime timestamptz, hightime timestamptz, maxminutes int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    t = t + random_minutes(1, maxminutes);
    result[i] = tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz), t);
  END LOOP;
  RETURN tgeogpointseq(result, lower_inc, upper_inc);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoint3Dseq(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10) AS seq
FROM generate_series (1, 15) AS k;

SELECT k, asText(random_tgeogpoint3Dseq(0, 80, 0, 80, 0, 80, '2001-01-01', '2001-12-31', 10, 10))
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoints(lowx float, highx float, lowy float, highy float,
  lowtime timestamptz, hightime timestamptz,
  maxminutes int, maxcardseq int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  instants tgeompoint[];
  cardseq int;
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    cardseq = random_int(1, maxcardseq);
    IF cardseq = 1 THEN
      lower_inc = true;
      upper_inc = true;
    ELSE
      lower_inc = random() > 0.5;
      upper_inc = random() > 0.5;
    END IF;
    FOR j IN 1..cardseq
    LOOP
      t = t + random_minutes(1, maxminutes);
      instants[j] = tgeompointinst(random_geompoint(lowx, highx, lowy, highy), t);
    END LOOP;
    result[i] = tgeompointseq(instants, lower_inc, upper_inc);
    instants = NULL;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, asText(random_tgeompoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeompoint3Ds(lowx float, highx float,
  lowy float, highy float, lowz float, highz float,
  lowtime timestamptz, hightime timestamptz, maxminutes int, maxcardseq int, maxcard int)
  RETURNS tgeompoint AS $$
DECLARE
  result tgeompoint[];
  instants tgeompoint[];
  cardseq int;
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    cardseq = random_int(1, maxcardseq);
    IF cardseq = 1 THEN
      lower_inc = true;
      upper_inc = true;
    ELSE
      lower_inc = random() > 0.5;
      upper_inc = random() > 0.5;
    END IF;
    FOR j IN 1..cardseq
    LOOP
      t = t + random_minutes(1, maxminutes);
      instants[j] = tgeompointinst(random_geompoint3D(lowx, highx, lowy, highy, lowz, highz), t);
    END LOOP;
    result[i] = tgeompointseq(instants, lower_inc, upper_inc);
    instants = NULL;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeompoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, asText(random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, numSequences(random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, asText(endSequence(random_tgeompoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoints(lowx float, highx float, lowy float, highy float,
  lowtime timestamptz, hightime timestamptz,
  maxminutes int, maxcardseq int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  instants tgeogpoint[];
  cardseq int;
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    cardseq = random_int(1, maxcardseq);
    IF cardseq = 1 THEN
      lower_inc = true;
      upper_inc = true;
    ELSE
      lower_inc = random() > 0.5;
      upper_inc = random() > 0.5;
    END IF;
    FOR j IN 1..cardseq
    LOOP
      t = t + random_minutes(1, maxminutes);
      instants[j] = tgeogpointinst(random_geogpoint(lowx, highx, lowy, highy), t);
    END LOOP;
    result[i] = tgeogpointseq(instants, lower_inc, upper_inc);
    instants = NULL;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoints(0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;
*/
-------------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION random_tgeogpoint3Ds(lowx float, highx float,
  lowy float, highy float, lowz float, highz float,
  lowtime timestamptz, hightime timestamptz, maxminutes int, maxcardseq int, maxcard int)
  RETURNS tgeogpoint AS $$
DECLARE
  result tgeogpoint[];
  instants tgeogpoint[];
  cardseq int;
  card int;
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    cardseq = random_int(1, maxcardseq);
    IF cardseq = 1 THEN
      lower_inc = true;
      upper_inc = true;
    ELSE
      lower_inc = random() > 0.5;
      upper_inc = random() > 0.5;
    END IF;
    FOR j IN 1..cardseq
    LOOP
      t = t + random_minutes(1, maxminutes);
      instants[j] = tgeogpointinst(random_geogpoint3D(lowx, highx, lowy, highy, lowz, highz), t);
    END LOOP;
    result[i] = tgeogpointseq(instants, lower_inc, upper_inc);
    instants = NULL;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeogpoints(result);
END;
$$ LANGUAGE 'plpgsql' STRICT;

/*
SELECT k, random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, asText(random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, numSequences(random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))
FROM generate_series (1, 15) AS k;

SELECT k, asText(endSequence(random_tgeogpoint3Ds(0, 100, 0, 100, 0, 100, '2001-01-01', '2001-12-31', 10, 10, 10))) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

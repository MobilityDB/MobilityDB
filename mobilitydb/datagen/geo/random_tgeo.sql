/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation FOR any purpose, without fee, and without a written
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
 * Basic synthetic data generator functions for temporal geo types
 */

-------------------------------------------------------------------------------

/**
 * @brief Generate a random geometry which can be (currently) a point, a 
 * linestring or a polygon
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geometry;
CREATE FUNCTION random_geometry(lowx float, highx float, lowy float,
  highy float, srid int DEFAULT 0)
  RETURNS geometry AS $$
DECLARE
  i int;
BEGIN
  i = random_int(1, 3);
  IF i = 1 THEN
    RETURN random_geom_point(lowx, highx, lowy, highy, srid);
  ELSIF i = 2 THEN
    RETURN random_geom_linestring(lowx, highx, lowy, highy, 10, 2, 2, srid);
  ELSE -- i = 0 
    RETURN random_geom_multipoint(lowx, highx, lowy, highy, 10, 2, 2, srid);
  END IF;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, ST_AsText(random_geometry(-100, 100, -100, 100)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, ST_AsEWKT(random_geometry(-100, 100, -100, 100, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random geometry which can be (currently) a point, a 
 * linestring or a polygon
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geometry3D;
CREATE FUNCTION random_geometry3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, srid int DEFAULT 0)
  RETURNS geometry AS $$
DECLARE
  i int;
BEGIN
  i = random_int(1, 3);
  IF i = 1 THEN
    RETURN random_geom_point3D(lowx, highx, lowy, highy, lowz, highz, srid);
  ELSIF i = 2 THEN
    RETURN random_geom_linestring3D(lowx, highx, lowy, highy, lowz, highz,
      10, 2, 2, srid);
  ELSE -- i = 0 
    RETURN random_geom_multipoint3D(lowx, highx, lowy, highy, lowz, highz,
      10, 2, 2, srid);
  END IF;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, ST_AsText(random_geometry3D(-100, 100, -100, 100, -100, 100)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, ST_AsEWKT(random_geometry3D(-100, 100, -100, 100, -100, 100, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random geography which can be (currently) a point, a 
 * linestring or a polygon
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geography;
CREATE FUNCTION random_geography(lowx float, highx float, lowy float,
  highy float, srid int DEFAULT 4326)
  RETURNS geography AS $$
DECLARE
  i int;
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geometry(lowx, highx, lowy, highy, srid)::geography;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, ST_AsText(random_geography(-180, 180, -90, 90)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, ST_AsEWKT(random_geography(-180, 180, -90, 90, 7844)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random geography which can be (currently) a point, a 
 * linestring or a polygon
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geography3D;
CREATE FUNCTION random_geography3D(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, srid int DEFAULT 4326)
  RETURNS geography AS $$
DECLARE
  i int;
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  RETURN random_geometry3D(lowx, highx, lowy, highy, lowz, highz, srid)::geography;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, ST_AsEWKT(random_geography3D(-180, 180, -90, 90, 0, 100)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, ST_AsEWKT(random_geography3D(-180, 180, -90, 90, 0, 100, 7844)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random geometries
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geometry_array;
CREATE FUNCTION random_geometry_array(lowx float, highx float, lowy float,
  highy float, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS geometry[] AS $$
DECLARE
  result geometry[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    IF mod(i, 3) = 1 THEN
      result[i] = random_geom_point(lowx, highx, lowy, highy, srid);
    ELSIF mod(i, 3) = 2 THEN
      result[i] = random_geom_linestring(lowx, highx, lowy, highy, 10, 2, 2, srid);
    ELSE -- mod(i, 3) = 0 
      result[i] = random_geom_multipoint(lowx, highx, lowy, highy, 10, 2, 2, srid);
    END IF;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_geometry_array(-100, 100, -100, 100, 1, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asText(random_geometry_array(-100, 100, -100, 100, 1, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random geometries
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geometry3D_array;
CREATE FUNCTION random_geometry3D_array(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS geometry[] AS $$
DECLARE
  result geometry[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    IF mod(i, 3) = 1 THEN
      result[i] = random_geom_point3D(lowx, highx, lowy, highy, lowz, highz, srid);
    ELSIF mod(i, 3) = 2 THEN
      result[i] = random_geom_linestring3D(lowx, highx, lowy, highy, 
        lowz, highz, 10, 2, 2, srid);
    ELSE -- mod(i, 3) = 0 
      result[i] = random_geom_multipoint3D(lowx, highx, lowy, highy, 
        lowz, highz, 10, 2, 2, srid);
    END IF;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_geometry3D_array(-100, 100, -100, 100, 0, 100, 1, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asText(random_geometry3D_array(-100, 100, -100, 100, 0, 100, 1, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random geographies
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geography_array;
CREATE FUNCTION random_geography_array(lowx float, highx float, lowy float,
  highy float, mincard int, maxcard int, srid int DEFAULT 4326)
  RETURNS geography[] AS $$
DECLARE
  geomresult geometry[];
  result geometry[];
  i int;
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  geomresult = random_geometry_array(lowx, highx, lowy, highy, mincard, 
    maxcard, srid);
  FOR i IN 1..array_length(geomresult, 1)
  LOOP
    result[i] = geomresult[i]::geography;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_geography_array(-180, 180, -90, 90, 1, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asText(random_geography_array(-180, 180, -90, 90, 1, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random geographies
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geography3D_array;
CREATE FUNCTION random_geography3D_array(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, mincard int, maxcard int,
  srid int DEFAULT 4326)
  RETURNS geography[] AS $$
DECLARE
  geomresult geometry[];
  result geometry[];
  i int;
BEGIN
  IF lowx < -180 OR highx > 180 OR lowy < -90 OR highy > 90 THEN
    RAISE EXCEPTION 'Geography coordinates must be in the range [-180 -90, 180 90]';
  END IF;
  geomresult = random_geometry3D_array(lowx, highx, lowy, highy, lowz, highz, 
    mincard, maxcard, srid);
  FOR i IN 1..array_length(geomresult, 1)
  LOOP
    result[i] = geomresult[i]::geography;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_geography3D_array(-180, 180, -90, 90, 0, 100, 1, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asText(random_geography3D_array(-180, 180, -90, 90, 0, 100, 1, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a set of random geometries
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geometry_set;
CREATE FUNCTION random_geometry_set(lowx float, highx float, lowy float,
  highy float, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS geomset AS $$
BEGIN
  RETURN set(random_geometry_array(lowx, highx, lowy, highy,
      mincard, maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_geometry_set(1, 100, 1, 100, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_geometry_set(1, 100, 1, 100, 5, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a set of random geometries
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geometry3D_set;
CREATE FUNCTION random_geometry3D_set(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS geomset AS $$
BEGIN
  RETURN set(random_geometry3D_array(lowx, highx, lowy, highy, lowz, highz,
      mincard, maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_geometry3D_set(1, 100, 1, 100, 1, 100, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_geometry3D_set(1, 100, 1, 100, 1, 100, 5, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a set of random geographies
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geography_set;
CREATE FUNCTION random_geography_set(lowx float, highx float, lowy float,
  highy float, mincard int, maxcard int, srid int DEFAULT 4326)
  RETURNS geogset AS $$
BEGIN
  RETURN set(random_geography_array(lowx, highx, lowy, highy,
    mincard, maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_geography_set(-180, 180, -90, 90, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_geography_set(-180, 180, -90, 90, 5, 10, 7844)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a set of random geographies
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_geography3D_set;
CREATE FUNCTION random_geography3D_set(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, mincard int, maxcard int,
  srid int DEFAULT 4326)
  RETURNS geogset AS $$
BEGIN
  RETURN set(random_geography3D_array(lowx, highx, lowy, highy, lowz, highz,
    mincard, maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_geography3D_set(-180, 180, -90, 90, 0, 100, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_geography3D_set(-180, 180, -90, 90, 0, 100, 5, 10, 7844)) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Instant
------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geometry of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_tgeometry_inst;
CREATE FUNCTION random_tgeometry_inst(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, srid int DEFAULT 0)
  RETURNS tgeometry AS $$
BEGIN
  RETURN tgeometry(random_geometry(lowx, highx, lowy, highy, srid), 
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_tgeometry_inst(1, 100, 1, 100, '2001-01-01',
  '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geometry of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_tgeometry3D_inst;
CREATE FUNCTION random_tgeometry3D_inst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz, 
  hightime timestamptz, srid int DEFAULT 0)
  RETURNS tgeometry AS $$
BEGIN
  RETURN tgeometry(random_geometry3D(lowx, highx, lowy, highy, lowz, highz, srid), 
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_tgeometry3D_inst(1, 100, 1, 100, 1, 100, '2001-01-01',
  '2001-12-31')) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * @brief Generate a random 2D tgeography instant
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid SRID of the coordinates
 */
DROP FUNCTION IF EXISTS random_tgeography_inst;
CREATE FUNCTION random_tgeography_inst(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, srid int DEFAULT 4326)
  RETURNS tgeography AS $$
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeography(random_geography(lowx, highx, lowy, highy, srid),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography_inst(-180, 180, -90, 90, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;

SELECT k, asEWKT(random_tgeography_inst(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 7844)) AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * @brief Generate a random 3D tgeography instant
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid SRID of the coordinates
 */
DROP FUNCTION IF EXISTS random_tgeography3D_inst;
CREATE FUNCTION random_tgeography3D_inst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, srid int DEFAULT 4326)
  RETURNS tgeography AS $$
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tgeography(random_geography3D(lowx, highx, lowy, highy, lowz,
    highz, srid), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography3D_inst(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01')) AS inst
FROM generate_series(1,10) k;

SELECT k, asEWKT(random_tgeography3D_inst(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 7844)) AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Discrete Sequence
-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geo of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_tgeometry_discseq;
CREATE FUNCTION random_tgeometry_discseq(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxminutes int, 
  mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS tgeometry AS $$
DECLARE
  result tgeometry[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tgeometry(random_geometry(lowx, highx, lowy, highy,
      srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeometrySeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_tgeometry_discseq(1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10)) AS ti
FROM generate_series(1,10) k;

SELECT k, asEWKT(random_tgeometry_discseq(1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 3812)) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geo of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_tgeometry3D_discseq;
CREATE FUNCTION random_tgeometry3D_discseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincard int, maxcard int, 
  srid int DEFAULT 0)
  RETURNS tgeometry AS $$
DECLARE
  result tgeometry[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tgeometry(random_geometry3D(lowx, highx, lowy, highy, lowz, 
      highz, srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tgeometrySeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_tgeometry3D_discseq(1, 100, 1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10)) AS ti
FROM generate_series(1,10) k;

SELECT k, asEWKT(random_tgeometry3D_discseq(1, 100, 1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 3812)) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random 2D tgeography discrete sequence
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality sequence
 * @param[in] srid SRID of the coordinates
 */
DROP FUNCTION IF EXISTS random_tgeography_discseq;
CREATE FUNCTION random_tgeography_discseq(lowx float, highx float,
  lowy float, highy float, lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int, srid int DEFAULT 4326)
  RETURNS tgeography AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeography[];
  card int;
BEGIN
  SELECT random_geography_array(lowx, highx, lowy, highy, mincard,
    maxcard, srid)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tgeography(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeographySeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography_discseq(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 5, 10))
FROM generate_series(1,10) k;

SELECT k, asEWKT(random_tgeography_discseq(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 5, 10, 7844))
FROM generate_series(1,10) k;

SELECT k, random_tgeography_discseq(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random 3D tgeography discrete sequence
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality sequence
 * @param[in] srid SRID of the coordinates
 */
DROP FUNCTION IF EXISTS random_tgeography3D_discseq;
CREATE FUNCTION random_tgeography3D_discseq(lowx float, highx float,
  lowy float, highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int,
  srid int DEFAULT 4326)
  RETURNS tgeography AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeography[];
  card int;
BEGIN
  SELECT random_geography3D_array(lowx, highx, lowy, highy, lowz, highz,
    mincard, maxcard, srid)
  INTO pointarr;
  card = array_length(pointarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tgeography(pointarr[i], tsarr[i]);
  END LOOP;
  RETURN tgeographySeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography3D_discseq(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10))
FROM generate_series(1,10) k;

SELECT k, asEWKT(random_tgeography3D_discseq(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10, 7844))
FROM generate_series(1,10) k;

SELECT k, random_tgeography3D_discseq(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Step Sequence
-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geo of sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tgeometry_stepseq;
CREATE FUNCTION random_tgeometry_stepseq(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxminutes int, 
  mincard int, maxcard int, srid int DEFAULT 0, fixstart bool DEFAULT false)
  RETURNS tgeometry AS $$
DECLARE
  tsarr timestamptz[];
  result tgeometry[];
  card int;
  t1 timestamptz;
  interp text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard,
    maxcard, fixstart) INTO tsarr;
  card = array_length(tsarr, 1);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card - 1
  LOOP
    result[i] = tgeometry(random_geometry(lowx, highx, lowy, highy, srid), 
      tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = tgeometry(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tgeometry(random_geometry(lowx, highx, lowy, highy,
      srid), tsarr[card]);
  END IF;
  RETURN tgeometrySeq(result, 'Step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_tgeometry_stepseq(1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 10, 10)) AS seq
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tgeometry_stepseq(1, 100, 1, 100,
    '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

WITH temp AS (
  SELECT k, random_tgeometry_stepseq(1, 100, 1, 100,
    '2001-01-01', '2001-12-31', 10, 10, 10, 3812) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geo of sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 * @note Type tgeometry does not accept linear interpolation
 */
DROP FUNCTION IF EXISTS random_tgeometry3D_stepseq;
CREATE FUNCTION random_tgeometry3D_stepseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincard int, maxcard int, 
  srid int DEFAULT 0, fixstart bool DEFAULT false)
  RETURNS tgeometry AS $$
DECLARE
  tsarr timestamptz[];
  result tgeometry[];
  card int;
  t1 timestamptz;
  interp text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard,
    maxcard, fixstart) INTO tsarr;
  card = array_length(tsarr, 1);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card - 1
  LOOP
    result[i] = tgeometry(random_geometry3D(lowx, highx, lowy, highy, lowz, 
      highz, srid), tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = tgeometry(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tgeometry(random_geometry3D(lowx, highx, lowy, highy, 
      lowz, highz, srid), tsarr[card]);
  END IF;
  RETURN tgeometrySeq(result, 'Step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_tgeometry3D_stepseq(1, 100, 1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 10, 10)) AS seq
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tgeometry3D_stepseq(1, 100, 1, 100, 1, 100,
    '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

WITH temp AS (
  SELECT k, random_tgeometry3D_stepseq(1, 100, 1, 100, 1, 100,
    '2001-01-01', '2001-12-31', 10, 10, 10, 3812) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random 2D tgeography sequence
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 * @param[in] srid SRID of the coordinates
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tgeography_stepseq;
CREATE FUNCTION random_tgeography_stepseq(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxminutes int, 
  mincard int, maxcard int, srid int DEFAULT 4326, 
  fixstart bool DEFAULT false)
  RETURNS tgeography AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeography[];
  card int;
  interp text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geography_array(lowx, highx, lowy, highy, mincard,
    maxcard, srid)
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
  FOR i IN 1..card - 1
  LOOP
    result[i] = tgeography(pointarr[i], tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = tgeography(pointarr[card - 1], tsarr[card]);
  ELSE
    result[card] = tgeography(pointarr[card], tsarr[card]);
  END IF;
  RETURN tgeographySeq(result, 'Step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography_stepseq(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_tgeography_stepseq(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 5, 10, 7844))
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random 3D tgeography sequence
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 * @param[in] srid SRID of the coordinates
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tgeography3D_stepseq;
CREATE FUNCTION random_tgeography3D_stepseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int, 
  srid int DEFAULT 4326, fixstart bool DEFAULT false)
  RETURNS tgeography AS $$
DECLARE
  pointarr geography[];
  tsarr timestamptz[];
  result tgeography[];
  card int;
  interp text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_geography3D_array(lowx, highx, lowy, highy, lowz, highz,
    mincard, maxcard, srid) INTO pointarr;
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
  FOR i IN 1..card - 1
  LOOP
    result[i] = tgeography(pointarr[i], tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = tgeography(pointarr[card - 1], tsarr[card]);
  ELSE
    result[card] = tgeography(pointarr[card], tsarr[card]);
  END IF;
  RETURN tgeographySeq(result, 'Step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography3D_stepseq(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_tgeography3D_stepseq(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10, 7844))
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geo of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants 
 *   in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_tgeometry_seqset;
CREATE FUNCTION random_tgeometry_seqset(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxminutes int, 
  mincardseq int, maxcardseq int, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS tgeometry AS $$
DECLARE
  result tgeometry[];
  card int;
  seq tgeometry;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i > 1
    SELECT random_tgeometry_stepseq(lowx, highx, lowy, highy,
      t1, t2, maxminutes, mincardseq, maxcardseq, srid, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeometrySeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_tgeometry_seqset(1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10)) AS seqset
FROM generate_series (1, 15) AS k;

SELECT k, asEWKT(random_tgeometry_seqset(1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 3812)) AS seqset
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tgeometry_seqset(1, 100, 1, 100,
    '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 3812) AS seqset
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seqset) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal geo of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants 
 *   in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID
 */
DROP FUNCTION IF EXISTS random_tgeometry3D_seqset;
CREATE FUNCTION random_tgeometry3D_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincardseq int, maxcardseq int, 
  mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS tgeometry AS $$
DECLARE
  result tgeometry[];
  card int;
  seq tgeometry;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i > 1
    SELECT random_tgeometry3D_stepseq(lowx, highx, lowy, highy, lowz, highz,
      t1, t2, maxminutes, mincardseq, maxcardseq, srid, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeometrySeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asText(random_geometry3D_seqset(1, 100, 1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10)) AS seqset
FROM generate_series (1, 15) AS k;

SELECT k, asEWKT(random_geometry3D_seqset(1, 100, 1, 100, 1, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 3812)) AS seqset
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_geometry3D_seqset(1, 100, 1, 100, 1, 100,
    '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 3812) AS seqset
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seqset) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random 2D tgeography sequence set
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 * @param[in] srid SRID of the coordinates
 */
DROP FUNCTION IF EXISTS random_tgeography_seqset;
CREATE FUNCTION random_tgeography_seqset(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, maxminutes int, 
  mincardseq int, maxcardseq int, mincard int, maxcard int, 
  srid int DEFAULT 4326)
  RETURNS tgeography AS $$
DECLARE
  result tgeography[];
  card int;
  seq tgeography;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i > 1
    SELECT random_tgeography_stepseq(lowx, highx, lowy, highy, t1, t2, 
      maxminutes, mincardseq, maxcardseq, srid, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeographySeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography_seqset(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 5, 10, 5, 10)) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_tgeography_seqset(-180, 180, -90, 90, '2001-01-01', '2002-01-01', 10, 5, 10, 5, 10, 7844)) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random 3D tgeography sequence set
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 * @param[in] srid SRID of the coordinates
 */
DROP FUNCTION IF EXISTS random_tgeography3D_seqset;
CREATE FUNCTION random_tgeography3D_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincardseq int, maxcardseq int, 
  mincard int, maxcard int, srid int DEFAULT 4326)
  RETURNS tgeography AS $$
DECLARE
  result tgeography[];
  card int;
  seq tgeography;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i > 1
    SELECT random_tgeography3D_stepseq(lowx, highx, lowy, highy, lowz, highz,
      t1, t2, maxminutes, mincardseq, maxcardseq, srid, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tgeographySeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_tgeography3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10, 5, 10)) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_tgeography3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10, 5, 10, 7844)) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, random_tgeography3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;

SELECT k, numSequences(random_tgeography3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10, 5, 10))
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(endSequence(random_tgeography3D_seqset(-180, 180, -90, 90, 0, 100, '2001-01-01', '2002-01-01', 10, 5, 10, 5, 10))) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

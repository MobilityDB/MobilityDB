/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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

/**
 * @file
 * @brief Basic synthetic data generator functions for temporal rigid
 * geometries
 */

------------------------------------------------------------------------------
-- Temporal rigid geometry
------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 * rotation
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid Optional SRID for the coordinates
 */
DROP FUNCTION IF EXISTS random_trgeom2d_inst;
CREATE FUNCTION random_trgeom2d_inst(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, maxdelta float,
  minvertices int, maxvertices int, lowtime timestamptz, hightime timestamptz,
  srid int DEFAULT 0)
  RETURNS trgeometry AS $$
BEGIN
  RETURN trgeometry(
    random_geom_polygon(lowx, highx, lowy, highy, maxdelta, minvertices,
      maxvertices, srid),
    random_tpose2d_inst(lowx, highx, lowy, highy, lowrotation,
      highrotation, lowtime, hightime, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom2d_inst(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the W coordinates
 * of the quaternion
 * @param[in] low_X, high_X Inclusive bounds of the range for the X coordinates
 * of the quaternion
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Y coordinates
 * of the quaternion
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Z coordinates
 * of the quaternion
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid Optional SRID for the coordinates
 */
DROP FUNCTION IF EXISTS random_trgeom3d_inst;
CREATE FUNCTION random_trgeom3d_inst(lowx float, highx float, lowy float,
    highy float, lowz float, highz float, low_W float, high_W float, low_X float,
    high_X float, low_Y float, high_Y float, low_Z float, high_Z float,
    maxdelta float, minvertices int, maxvertices int, lowtime timestamptz,
    hightime timestamptz, srid int DEFAULT 0)
  RETURNS trgeometry AS $$
BEGIN
  RETURN trgeometry(
    random_geom_polygon3d(lowx, highx, lowy, highy, lowz, highz, maxdelta,
      minvertices, maxvertices, srid),
    random_tpose3d_inst(lowx, highx, lowy, highy, lowz, highz, low_W, high_W,
       low_X, high_X, low_Y, high_Y, low_Z, high_Z, lowtime, hightime, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom3d_inst(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 * rotation
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the coordinates
 */
DROP FUNCTION IF EXISTS random_trgeom2d_discseq;
CREATE FUNCTION random_trgeom2d_discseq(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, maxdelta float,
  minvertices int, maxvertices int, lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS trgeometry AS $$
DECLARE
  result tpose[];
  geo geometry;
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  geo = random_geom_polygon(lowx, highx, lowy, highy, maxdelta, minvertices,
      maxvertices, srid);
  FOR i IN 1..card
  LOOP
    result[i] = tpose(random_pose2d(lowx, highx, lowy,
      highy, lowrotation, highrotation, srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN trgeometry(geo, tposeSeq(result, 'Discrete'));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom2d_discseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, random_trgeom2d_discseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 5676) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the W coordinates
 * of the quaternion
 * @param[in] low_X, high_X Inclusive bounds of the range for the X coordinates
 * of the quaternion
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Y coordinates
 * of the quaternion
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Z coordinates
 * of the quaternion
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the coordinates
 */
DROP FUNCTION IF EXISTS random_trgeom3d_discseq;
CREATE FUNCTION random_trgeom3d_discseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float,
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float, 
  maxdelta float, minvertices int, maxvertices int, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS trgeometry AS $$
DECLARE
  result tpose[];
  geo geometry;
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  geo = random_geom_polygon3d(lowx, highx, lowy, highy, lowz, highz, maxdelta, 
    minvertices, maxvertices, srid);
  FOR i IN 1..card
  LOOP
    result[i] = tpose(random_pose3d(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN trgeometry(geo, tposeSeq(result, 'Discrete'));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom3d_discseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 1, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, random_trgeom3d_discseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 1, 10, 5676) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 * rotation
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the coordinates
 * @param[in] linear True when the sequence has linear interpolation
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_trgeom2d_contseq;
CREATE FUNCTION random_trgeom2d_contseq(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, maxdelta float,
  minvertices int, maxvertices int, lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int, srid int DEFAULT 0,
  linear bool DEFAULT true, fixstart bool DEFAULT false)
  RETURNS trgeometry AS $$
DECLARE
  tsarr timestamptz[];
  result tpose[];
  geo geometry;
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
  geo = random_geom_polygon(lowx, highx, lowy, highy, maxdelta, minvertices,
      maxvertices, srid);
  FOR i IN 1..card - 1
  LOOP
    result[i] = tpose(random_pose2d(lowx, highx, lowy, highy, lowrotation,
      highrotation, srid), tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = tpose(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tpose(random_pose2d(lowx, highx, lowy, highy,
      lowrotation, highrotation, srid), tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN trgeometry(geo, tposeSeq(result, interp, lower_inc, upper_inc));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom2d_contseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_trgeom2d_contseq(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

WITH temp AS (
  SELECT k, random_trgeom2d_contseq(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10, 5676) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

SELECT k, random_trgeom2d_contseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10, 0, false)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_trgeom2d_contseq(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10, 5676,
    false) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the W coordinates
 * of the quaternion
 * @param[in] low_X, high_X Inclusive bounds of the range for the X coordinates
 * of the quaternion
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Y coordinates
 * of the quaternion
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Z coordinates
 * of the quaternion
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the coordinates
 * @param[in] linear True when the sequence has linear interpolation
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_trgeom3d_contseq;
CREATE FUNCTION random_trgeom3d_contseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float,
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float, 
  maxdelta float, minvertices int, maxvertices int, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int,
  srid int DEFAULT 0, linear bool DEFAULT true, fixstart bool DEFAULT false)
  RETURNS trgeometry AS $$
DECLARE
  tsarr timestamptz[];
  result tpose[];
  geo geometry;
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
  geo = random_geom_polygon3d(lowx, highx, lowy, highy, lowz, highz, maxdelta, 
    minvertices, maxvertices, srid);
  FOR i IN 1..card - 1
  LOOP
    result[i] = tpose(random_pose3d(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid),
      tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = trgeometry(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tpose(random_pose3d(lowx, highx, lowy, highy, lowz,
      highz, low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid),
      tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN trgeometry(geo, tposeSeq(result, interp, lower_inc, upper_inc));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_trgeom3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
    0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seq) FROM temp;

WITH temp AS (
  SELECT k, random_trgeom3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
    0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10, 5676) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seq) FROM temp;

SELECT k, random_trgeom3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
  0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10, 0, false)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_trgeom3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
    0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 10, 10, 5676, false) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seq) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 * rotation
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants 
 *   in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID for the coordinates
 * @param[in] linear True when the sequence has linear interpolation
 */
DROP FUNCTION IF EXISTS random_trgeom2d_seqset;
CREATE FUNCTION random_trgeom2d_seqset(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, maxdelta float,
  minvertices int, maxvertices int, lowtime timestamptz, hightime timestamptz, 
  maxminutes int, mincardseq int, maxcardseq int, mincard int, maxcard int,
  srid int DEFAULT 0, linear bool DEFAULT true)
  RETURNS trgeometry AS $$
DECLARE
  result tpose[];
  seq tpose;
  geo geometry;
  card int;
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
  geo = random_geom_polygon(lowx, highx, lowy, highy, maxdelta, minvertices,
      maxvertices, srid);
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tpose2d_contseq(lowx, highx, lowy, highy, lowrotation,
      highrotation, t1, t2, maxminutes, mincardseq, maxcardseq, srid, linear,
      i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN trgeometry(geo, tposeSeqSet(result));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom2d_seqset(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), 10, 5, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS seqset
FROM generate_series (1, 15) AS k;

SELECT k, random_trgeom2d_seqset(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false)
  AS seqset
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_trgeom2d_seqset(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false)
    AS seqset
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seqset) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the W coordinates
 * of the quaternion
 * @param[in] low_X, high_X Inclusive bounds of the range for the X coordinates
 * of the quaternion
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Y coordinates
 * of the quaternion
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Z coordinates
 * of the quaternion
 * @param[in] maxdelta Maximum difference between two consecutive coordinate
 * values in the polygon
 * @param[in] minvertices, maxvertices Inclusive bounds of the number of
 * vertices in the polygon
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants 
 *   in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID for the coordinates
 * @param[in] linear True when the sequence has linear interpolation
 */
DROP FUNCTION IF EXISTS random_trgeom3d_seqset;
CREATE FUNCTION random_trgeom3d_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float,
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float, 
  maxdelta float, minvertices int, maxvertices int, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincardseq int, maxcardseq int,
  mincard int, maxcard int, srid int DEFAULT 0, linear bool DEFAULT true)
  RETURNS trgeometry AS $$
DECLARE
  result tpose[];
  seq tpose;
  geo geometry;
  card int;
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
  geo = random_geom_polygon3d(lowx, highx, lowy, highy, lowz, highz, maxdelta, 
    minvertices, maxvertices, srid);
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tpose3d_contseq(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, t1, t2, 
      maxminutes, mincardseq, maxcardseq, srid, linear, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN trgeometry(geo, tposeSeqSet(result));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_trgeom3d_seqset(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS seqset
FROM generate_series (1, 15) AS k;

SELECT k, random_trgeom3d_seqset(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false) 
    AS seqset
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_trgeom3d_seqset(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, 10, 5, 10, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false)
  AS seqset
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seqset) FROM temp;
*/

-------------------------------------------------------------------------------


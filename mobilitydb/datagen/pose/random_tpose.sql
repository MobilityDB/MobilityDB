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
 * @brief Basic synthetic data generator functions for pose and
 * temporal pose types
 */

------------------------------------------------------------------------------
-- Static pose type
------------------------------------------------------------------------------

/**
 * @brief Generate a random pose
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 */
DROP FUNCTION IF EXISTS random_pose2d;
CREATE FUNCTION random_pose2d(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, srid int DEFAULT 0)
  RETURNS pose AS $$
BEGIN
  RETURN pose(random_geom_point(lowx, highx, lowy, highy, srid),
    random_float(lowrotation, highrotation));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_pose2d(-100, 100, -100, 100, radians(-pi()), radians(pi())) AS g
FROM generate_series(1,10) k;

SELECT k, random_pose2d(-100, 100, -100, 100, radians(-pi()), radians(pi()), 5676) AS g
FROM generate_series(1,10) k;

-- Errors
SELECT k, random_pose2d(100, -100, 100, -100, 1, 10) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose2d(-100, 100, 100, -100, 1, 10) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose2d(-100, 100, -100, 100, 10, 1) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose2d(-100, 100, -100, 100, 10, 20) AS g
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * @brief Generate a random pose
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 * @note The computation of the unit norm from the generated values is taken 
 * from 
 * https://math.stackexchange.com/questions/1494740/finding-the-unit-quaternion
 */
DROP FUNCTION IF EXISTS random_pose3d;
CREATE FUNCTION random_pose3d(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float, 
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float,
  srid int DEFAULT 0)
  RETURNS pose AS $$
DECLARE
  X float;
  Y float;
  Z float;
  Q_W float;
  Q_X float;
  Q_Y float;
  Q_Z float;
  norm float;
  point geometry;
BEGIN
  X = random_float(lowx, highx);
  Y = random_float(lowy, highy);
  Z = random_float(lowz, highz);
  Q_W = random_float(low_W, high_W);
  Q_X = random_float(low_X, high_X);
  Q_Y = random_float(low_Y, high_Y);
  Q_Z = random_float(low_Z, high_Z);
  norm = sqrt(Q_W * Q_W + Q_X * Q_X + Q_Y * Q_Y + Q_Z * Q_Z);
  X = X / norm;
  Y = X / norm;
  Z = X / norm;
  Q_W = Q_W / norm;
  Q_X = Q_X / norm;
  Q_Y = Q_Y / norm;
  Q_Z = Q_Z / norm;
  point = ST_PointZ(X, Y, Z, srid);
  RETURN pose(point, Q_W, Q_X, Q_Y, Q_Z);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_pose3d(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 0, 1, 0, 1) AS g
FROM generate_series(1,10) k;

SELECT k, random_pose3d(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 0, 1, 0, 1, 5676) AS g
FROM generate_series(1,10) k;

-- Errors
SELECT k, random_pose3d(100, -100, 100, -100, 100, -100, 0, 1, 0, 1, 0, 1, 0, 1) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose3d(-100, 100, 100, -100, 100, -100, 0, 1, 0, 1, 0, 1, 0, 1) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose3d(-100, 100, -100, 100, 100, -100, 0, 1, 0, 1, 0, 1, 0, 1) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose3d(-100, 100, -100, 100, -100, 100, 1, 0, 0, 1, 0, 1, 0, 1) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose3d(-100, 100, -100, 100, -100, 100, 0, 1, 1, 0, 0, 1, 0, 1) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose3d(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 1, 0, 0, 1) AS g
FROM generate_series(1,10) k;
SELECT k, random_pose3d(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 0, 1, 1, 0) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random poses
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_pose2d_array;
CREATE FUNCTION random_pose2d_array(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS pose[] AS $$
DECLARE
  result pose[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    result[i] = random_pose2d(lowx, highx, lowy, highy, lowrotation,
      highrotation, srid);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_pose2d_array(-100, 100, -100, 100, radians(-pi()), radians(pi()), 1, 10) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_pose2d_array(-100, 100, -100, 100, radians(-pi()), radians(pi()), 1, 10, 5676) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random poses
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the Q_W coordinates
 * @param[in] low_X, high_X Inclusive bounds of the range for the Q_X coordinates
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Q_Y coordinates
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Q_Z coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_pose3d_array;
CREATE FUNCTION random_pose3d_array(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float, 
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float,
  mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS pose[] AS $$
DECLARE
  result pose[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    result[i] = random_pose3d(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_pose3d_array(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 0, 1, 0, 1, 1, 10) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_pose3d_array(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, 1, 10, 5676) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a set of random poses
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_pose2d_set;
CREATE FUNCTION random_pose2d_set(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS poseset AS $$
DECLARE
  nparr pose[];
BEGIN
  RETURN set(random_pose2d_array(lowx, highx, lowy, highy, lowrotation,
      highrotation, mincard, maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_pose2d_set(-100, 100, -100, 100, radians(-pi()), 
  radians(pi()), 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_pose2d_set(-100, 100, -100, 100, radians(-pi()), 
  radians(pi()), 5, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_pose2d_set(-100, 100, -100, 100, radians(-pi()), 
  radians(pi()), 5, 10, 5676) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a set of random poses
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the Q_W coordinates
 * @param[in] low_X, high_X Inclusive bounds of the range for the Q_X coordinates
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Q_Y coordinates
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Q_Z coordinates
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_pose3d_set;
CREATE FUNCTION random_pose3d_set(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float,
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float, 
  mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS poseset AS $$
DECLARE
  nparr pose[];
BEGIN
  RETURN set(random_pose3d_array(lowx, highx, lowy, highy, lowz, highz, 
    low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, mincard, 
    maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_pose3d_set(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
  0, 1, 0, 1, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_pose3d_set(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
  0, 1, 0, 1, 5, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_pose3d_set(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 0, 1,
  0, 1, 5, 10, 5676) AS g
FROM generate_series(1, 15) AS k;
*/

------------------------------------------------------------------------------
-- Temporal pose
------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_tpose2d_inst;
CREATE FUNCTION random_tpose2d_inst(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, lowtime timestamptz, 
  hightime timestamptz, srid int DEFAULT 0)
  RETURNS tpose AS $$
BEGIN
  RETURN tpose(random_pose2d(lowx, highx, lowy, highy, lowrotation,
      highrotation, srid), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose2d_inst(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the Q_W coordinates
 * @param[in] low_X, high_X Inclusive bounds of the range for the Q_X coordinates
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Q_Y coordinates
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Q_Z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_tpose3d_inst;
CREATE FUNCTION random_tpose3d_inst(lowx float, highx float, lowy float,
    highy float, lowz float, highz float, low_W float, high_W float, low_X float,
    high_X float, low_Y float, high_Y float, low_Z float, high_Z float,
    lowtime timestamptz, hightime timestamptz, srid int DEFAULT 0)
  RETURNS tpose AS $$
BEGIN
  RETURN tpose(random_pose3d(lowx, highx, lowy, highy, lowz, highz, 
    low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose3d_inst(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_tpose2d_discseq;
CREATE FUNCTION random_tpose2d_discseq(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS tpose AS $$
DECLARE
  result tpose[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tpose(random_pose2d(lowx, highx, lowy, highy, lowrotation,
      highrotation, srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tposeSeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose2d_discseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, random_tpose2d_discseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 10, 5676) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the Q_W coordinates
 * @param[in] low_X, high_X Inclusive bounds of the range for the Q_X coordinates
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Q_Y coordinates
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Q_Z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the point of the pose
 */
DROP FUNCTION IF EXISTS random_tpose3d_discseq;
CREATE FUNCTION random_tpose3d_discseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float,
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float, 
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int, 
  maxcard int, srid int DEFAULT 0)
  RETURNS tpose AS $$
DECLARE
  result tpose[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tpose(random_pose3d(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tposeSeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose3d_discseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 1, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, random_tpose3d_discseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 1, 10, 5676) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the point of the pose
 * @param[in] linear True when the sequence has linear interpolation
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tpose2d_contseq;
CREATE FUNCTION random_tpose2d_contseq(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincard int, maxcard int, 
  srid int DEFAULT 0, linear bool DEFAULT true, fixstart bool DEFAULT false)
  RETURNS tpose AS $$
DECLARE
  tsarr timestamptz[];
  result tpose[];
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
    result[i] = tpose(random_pose2d(lowx, highx, lowy, highy, lowrotation,
      highrotation, srid), tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = tpose(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tpose(random_pose2d(lowx, highx, lowy, highy, lowrotation,
      highrotation, srid), tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN tposeSeq(result, interp, lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose2d_contseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tpose2d_contseq(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

WITH temp AS (
  SELECT k, random_tpose2d_contseq(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), '2001-01-01', '2001-12-31', 10, 10, 10, 5676) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

SELECT k, random_tpose2d_contseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 10, 10, 0, false)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tpose2d_contseq(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), '2001-01-01', '2001-12-31', 10, 10, 10, 5676, false) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W Inclusive bounds of the range for the Q_W coordinates
 * @param[in] low_X, high_X Inclusive bounds of the range for the Q_X coordinates
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Q_Y coordinates
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Q_Z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the point of the pose
 * @param[in] linear True when the sequence has linear interpolation
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tpose3d_contseq;
CREATE FUNCTION random_tpose3d_contseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float,
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float, 
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int, 
  maxcard int, srid int DEFAULT 0, linear bool DEFAULT true, 
  fixstart bool DEFAULT false)
  RETURNS tpose AS $$
DECLARE
  tsarr timestamptz[];
  result tpose[];
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
    result[i] = tpose(random_pose3d(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid), 
      tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = tpose(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tpose(random_pose3d(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid), 
      tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN tposeSeq(result, interp, lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tpose3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
    0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seq) FROM temp;

WITH temp AS (
  SELECT k, random_tpose3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
    0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 10, 10, 5676) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seq) FROM temp;

SELECT k, random_tpose3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 10, 10, 0, false)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tpose3d_contseq(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1,
    0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 10, 10, 5676, false) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seq) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the 
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants 
 *   in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID for the point of the pose
 * @param[in] linear True when the sequence has linear interpolation
 */
DROP FUNCTION IF EXISTS random_tpose2d_seqset;
CREATE FUNCTION random_tpose2d_seqset(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincardseq int, maxcardseq int,
  mincard int, maxcard int, srid int DEFAULT 0, linear bool DEFAULT true)
  RETURNS tpose AS $$
DECLARE
  result tpose[];
  card int;
  seq tpose;
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
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tpose2d_contseq(lowx, highx, lowy, highy, lowrotation,
      highrotation, t1, t2, maxminutes, mincardseq, maxcardseq, srid, linear, 
      i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tposeSeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose2d_seqset(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS seqset
FROM generate_series (1, 15) AS k;

SELECT k, random_tpose2d_seqset(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false)
  AS seqset
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tpose2d_seqset(-100, 100, -100, 100, radians(-pi()),
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
 * @param[in] low_W, high_W Inclusive bounds of the range for the Q_W coordinates
 * @param[in] low_X, high_X Inclusive bounds of the range for the Q_X coordinates
 * @param[in] low_Y, high_Y Inclusive bounds of the range for the Q_Y coordinates
 * @param[in] low_Z, high_Z Inclusive bounds of the range for the Q_Z coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants 
 *   in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID for the point of the pose
 * @param[in] linear True when the sequence has linear interpolation
 */
DROP FUNCTION IF EXISTS random_tpose3d_seqset;
CREATE FUNCTION random_tpose3d_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float, low_X float,
  high_X float, low_Y float, high_Y float, low_Z float, high_Z float, 
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincardseq int, 
  maxcardseq int, mincard int, maxcard int, srid int DEFAULT 0, 
  linear bool DEFAULT true)
  RETURNS tpose AS $$
DECLARE
  result tpose[];
  card int;
  seq tpose;
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
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tpose3d_contseq(lowx, highx, lowy, highy, lowz, highz, 
      low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, t1, t2, 
      maxminutes, mincardseq, maxcardseq, srid, linear, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tposeSeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tpose3d_seqset(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS seqset
FROM generate_series (1, 15) AS k;

SELECT k, random_tpose3d_seqset(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false) 
    AS seqset
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tpose3d_seqset(-100, 100, -100, 100, -100, 100, 0, 1, 0, 1, 
  0, 1, 0, 1, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false)
  AS seqset
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT SRID(seqset) FROM temp;
*/

-------------------------------------------------------------------------------


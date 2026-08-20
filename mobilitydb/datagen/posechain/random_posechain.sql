/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Basic synthetic data generator functions for pose chain types
 */

------------------------------------------------------------------------------
-- Static pose chain type
------------------------------------------------------------------------------

/**
 * @brief Generate a random 2D pose chain
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the
 *   rotation
 * @param[in] mincard, maxcard Inclusive bounds of the number of links
 * @param[in] srid SRID of the outer frame of the chain
 * @details The outer link is expressed in the frame the SRID names, and every
 * later link is a rigid transform read in the axes of the link before it, so
 * only the outer link carries the SRID
 */
CREATE FUNCTION random_posechain2d(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS posechain AS $$
DECLARE
  result pose[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  result[1] = random_pose2d(lowx, highx, lowy, highy, lowrotation,
    highrotation, srid);
  FOR i IN 2..card
  LOOP
    result[i] = random_pose2d(lowx, highx, lowy, highy, lowrotation,
      highrotation);
  END LOOP;
  RETURN posechain(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_posechain2d(-100, 100, -100, 100, radians(-pi()), radians(pi()),
  1, 5) AS g
FROM generate_series(1,10) k;

SELECT k, random_posechain2d(-100, 100, -100, 100, radians(-pi()), radians(pi()),
  1, 5, 3812) AS g
FROM generate_series(1,10) k;

-- Errors
SELECT k, random_posechain2d(-100, 100, -100, 100, radians(-pi()), radians(pi()),
  5, 1) AS g
FROM generate_series(1,10) k;
*/

/**
 * @brief Generate a random 3D pose chain
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowz, highz Inclusive bounds of the range for the z coordinates
 * @param[in] low_W, high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z
 *   Inclusive bounds of the ranges for the components of the quaternion
 * @param[in] mincard, maxcard Inclusive bounds of the number of links
 * @param[in] srid SRID of the outer frame of the chain
 * @details The outer link is expressed in the frame the SRID names, and every
 * later link is a rigid transform read in the axes of the link before it, so
 * only the outer link carries the SRID
 */
CREATE FUNCTION random_posechain3d(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, low_W float, high_W float,
  low_X float, high_X float, low_Y float, high_Y float, low_Z float,
  high_Z float, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS posechain AS $$
DECLARE
  result pose[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  result[1] = random_pose3d(lowx, highx, lowy, highy, lowz, highz, low_W,
    high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z, srid);
  FOR i IN 2..card
  LOOP
    result[i] = random_pose3d(lowx, highx, lowy, highy, lowz, highz, low_W,
      high_W, low_X, high_X, low_Y, high_Y, low_Z, high_Z);
  END LOOP;
  RETURN posechain(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_posechain3d(-100, 100, -100, 100, -100, 100, -1, 1, -1, 1,
  -1, 1, -1, 1, 1, 5) AS g
FROM generate_series(1,10) k;

SELECT k, random_posechain3d(-100, 100, -100, 100, -100, 100, -1, 1, -1, 1,
  -1, 1, -1, 1, 1, 5, 3812) AS g
FROM generate_series(1,10) k;
*/

------------------------------------------------------------------------------

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
/**
 * @brief Generate a random temporal pose chain of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] mincard, maxcard Inclusive bounds of the number of links
 * @param[in] srid SRID of the outer frame of the chain
 */
CREATE FUNCTION random_tposechain2d_inst(lowx float, highx float, lowy float,
  highy float, lowrotation float, highrotation float, lowtime timestamptz,
  hightime timestamptz, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS tposechain AS $$
BEGIN
  RETURN tposechain(random_posechain2d(lowx, highx, lowy, highy, lowrotation,
    highrotation, mincard, maxcard, srid),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tposechain2d_inst(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 1, 5) AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose chain of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] minlinks, maxlinks Inclusive bounds of the number of links
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid SRID of the outer frame of the chain
 * @details The number of links is drawn once and every instant is generated
 * with it, since all the values of a temporal pose chain hold the same number
 * of links
 */
CREATE FUNCTION random_tposechain2d_discseq(lowx float, highx float,
  lowy float, highy float, lowrotation float, highrotation float,
  lowtime timestamptz, hightime timestamptz, maxminutes int, minlinks int,
  maxlinks int, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS tposechain AS $$
DECLARE
  result tposechain[];
  card int;
  links int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  links = random_int(minlinks, maxlinks);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tposechain(random_posechain2d(lowx, highx, lowy, highy,
      lowrotation, highrotation, links, links, srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tposechainSeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tposechain2d_discseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 5, 1, 10) AS ti
FROM generate_series(1,10) k;

WITH temp AS (
  SELECT k, random_tposechain2d_discseq(-100, 100, -100, 100, radians(-pi()),
    radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 5, 1, 10) AS ti
  FROM generate_series(1,10) k )
SELECT DISTINCT numPoses(ti) IS NOT NULL FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose chain of continuous sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] minlinks, maxlinks Inclusive bounds of the number of links
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid SRID of the outer frame of the chain
 * @param[in] linear True for linear interpolation, false for step
 * @param[in] fixstart True when the sequence starts at @p lowtime
 * @details The number of links is drawn once and every instant is generated
 * with it, since all the values of a temporal pose chain hold the same number
 * of links
 */
CREATE FUNCTION random_tposechain2d_contseq(lowx float, highx float,
  lowy float, highy float, lowrotation float, highrotation float,
  lowtime timestamptz, hightime timestamptz, maxminutes int, minlinks int,
  maxlinks int, mincard int, maxcard int, srid int DEFAULT 0,
  linear bool DEFAULT true, fixstart bool DEFAULT false)
  RETURNS tposechain AS $$
DECLARE
  tsarr timestamptz[];
  result tposechain[];
  card int;
  links int;
  interp text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard,
    maxcard, fixstart) INTO tsarr;
  card = array_length(tsarr, 1);
  links = random_int(minlinks, maxlinks);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card - 1
  LOOP
    result[i] = tposechain(random_posechain2d(lowx, highx, lowy, highy,
      lowrotation, highrotation, links, links, srid), tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = tposechain(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tposechain(random_posechain2d(lowx, highx, lowy, highy,
      lowrotation, highrotation, links, links, srid), tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN tposechainSeq(result, interp, lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tposechain2d_contseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 5, 10, 10)
FROM generate_series (1, 15) AS k;

SELECT k, random_tposechain2d_contseq(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 5, 10, 10, 5676, false)
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal pose chain of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowrotation, highrotation Inclusive bounds of the range for the
 *   rotation
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] minlinks, maxlinks Inclusive bounds of the number of links
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants
 *   of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid SRID of the outer frame of the chain
 * @param[in] linear True for linear interpolation, false for step
 * @details The number of links is drawn once for the whole value, since every
 * sequence of a sequence set holds values of one temporal pose chain
 */
CREATE FUNCTION random_tposechain2d_seqset(lowx float, highx float,
  lowy float, highy float, lowrotation float, highrotation float,
  lowtime timestamptz, hightime timestamptz, maxminutes int, minlinks int,
  maxlinks int, mincardseq int, maxcardseq int, mincard int, maxcard int,
  srid int DEFAULT 0, linear bool DEFAULT true)
  RETURNS tposechain AS $$
DECLARE
  result tposechain[];
  card int;
  links int;
  seq tposechain;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  links = random_int(minlinks, maxlinks);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1, and the
    -- number of links is the one drawn above, since every sequence of the
    -- result holds values of one temporal pose chain
    SELECT random_tposechain2d_contseq(lowx, highx, lowy, highy, lowrotation,
      highrotation, t1, t2, maxminutes, links, links, mincardseq, maxcardseq,
      srid, linear, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tposechainSeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tposechain2d_seqset(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), '2001-01-01', '2001-12-31', 10, 1, 5, 5, 10, 1, 10) AS seqset
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

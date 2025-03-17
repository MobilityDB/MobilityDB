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

/*
 * random_tcbuffer.sql
 * Basic synthetic data generator functions for circular buffer and
 * temporal circular buffer types
 */

------------------------------------------------------------------------------
-- Static circular buffer type
------------------------------------------------------------------------------

/**
 * @brief Generate a random circular buffer
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowradius, highradius Inclusive bounds of the range for the 
 *   radius
 */
DROP FUNCTION IF EXISTS random_cbuffer;
CREATE FUNCTION random_cbuffer(lowx float, highx float, lowy float,
  highy float, lowradius float, highradius float, srid int DEFAULT 0)
  RETURNS cbuffer AS $$
BEGIN
  RETURN cbuffer(random_geom_point(lowx, highx, lowy, highy, srid), 
    random_float(lowradius, highradius));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_cbuffer(-100, 100, -100, 100, 1, 10) AS g
FROM generate_series(1,10) k;

SELECT k, random_cbuffer(-100, 100, -100, 100, 1, 10, 5676) AS g
FROM generate_series(1,10) k;

-- Errors
SELECT k, random_cbuffer(100, -100, 100, -100, 1, 10) AS g
FROM generate_series(1,10) k;
SELECT k, random_cbuffer(-100, 100, 100, -100, 1, 10) AS g
FROM generate_series(1,10) k;
SELECT k, random_cbuffer(-100, 100, -100, 100, 10, 1) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random circular buffers
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowradius, highradius Inclusive bounds of the range for the 
 *   radius
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID for the point of the cbuffer
 */
DROP FUNCTION IF EXISTS random_cbuffer_array;
CREATE FUNCTION random_cbuffer_array(lowx float, highx float, lowy float,
  highy float, lowradius float, highradius float, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS cbuffer[] AS $$
DECLARE
  result cbuffer[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    result[i] = random_cbuffer(lowx, highx, lowy, highy, lowradius,
      highradius, srid);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_cbuffer_array(-100, 100, -100, 100, 1, 10, 1, 10) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_cbuffer_array(-100, 100, -100, 100, 1, 10, 1, 10, 5676) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a set of random circular buffers
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowradius, highradius Inclusive bounds of the range for the 
 *   radius
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID for the point of the cbuffer
 */
DROP FUNCTION IF EXISTS random_cbuffer_set;
CREATE FUNCTION random_cbuffer_set(lowx float, highx float, lowy float,
  highy float, lowradius float, highradius float, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS cbufferset AS $$
DECLARE
  nparr cbuffer[];
BEGIN
  RETURN set(random_cbuffer_array(lowx, highx, lowy, highy, lowradius,
      highradius, mincard, maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asEWKT(random_cbuffer_set(1, 100, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asEWKT(random_cbuffer_set(1, 100, 5, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_cbuffer_set(1, 100, 1, 100, 1, 100, 5, 10, 5676) AS g
FROM generate_series(1, 15) AS k;
*/

------------------------------------------------------------------------------
-- Temporal circular buffer
------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal circular buffer of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowradius, highradius Inclusive bounds of the range for the 
 *   radius
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] srid Optional SRID for the point of the cbuffer
 */
DROP FUNCTION IF EXISTS random_tcbuffer_inst;
CREATE FUNCTION random_tcbuffer_inst(lowx float, highx float, lowy float,
  highy float, lowradius float, highradius float, lowtime timestamptz, 
  hightime timestamptz, srid int DEFAULT 0)
  RETURNS tcbuffer AS $$
BEGIN
  RETURN tcbuffer(random_cbuffer(lowx, highx, lowy, highy, lowradius,
      highradius, srid), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tcbuffer_inst(1, 100, 1, 100, 1, 100, 
  '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal circular buffer of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowradius, highradius Inclusive bounds of the range for the 
 *   radius
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the point of the cbuffer
 */
DROP FUNCTION IF EXISTS random_tcbuffer_discseq;
CREATE FUNCTION random_tcbuffer_discseq(lowx float, highx float, lowy float,
  highy float, lowradius float, highradius float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincard int, maxcard int, 
  srid int DEFAULT 0)
  RETURNS tcbuffer AS $$
DECLARE
  result tcbuffer[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tcbuffer(random_cbuffer(lowx, highx, lowy, highy, lowradius,
      highradius, srid), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tcbufferSeq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tcbuffer_discseq(1, 100, 1, 100, 1, 100, 
  '2001-01-01', '2001-12-31', 10, 1, 10) AS ti
FROM generate_series(1,10) k;

SELECT k, random_tcbuffer_discseq(1, 100, 1, 100, 1, 100, 
  '2001-01-01', '2001-12-31', 10, 1, 10, 5676) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal circular buffer of sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowradius, highradius Inclusive bounds of the range for the 
 *   radius
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID for the point of the cbuffer
 * @param[in] linear True when the sequence has linear interpolation
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tcbuffer_contseq;
CREATE FUNCTION random_tcbuffer_contseq(lowx float, highx float, lowy float,
  highy float, lowradius float, highradius float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincard int, maxcard int, 
  srid int DEFAULT 0, linear bool DEFAULT true, fixstart bool DEFAULT false)
  RETURNS tcbuffer AS $$
DECLARE
  tsarr timestamptz[];
  result tcbuffer[];
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
    result[i] = tcbuffer(random_cbuffer(lowx, highx, lowy, highy, lowradius,
      highradius, srid), tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = tcbuffer(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tcbuffer(random_cbuffer(lowx, highx, lowy, highy, lowradius,
      highradius, srid), tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN tcbufferSeq(result, interp, lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tcbuffer_contseq(1, 100, 1, 100, 1, 100, 
  '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tcbuffer_contseq(1, 100, 1, 100, 1, 100, 
    '2001-01-01', '2001-12-31', 10, 10, 10) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

WITH temp AS (
  SELECT k, random_tcbuffer_contseq(1, 100, 1, 100, 1, 100, 
    '2001-01-01', '2001-12-31', 10, 10, 10, 5676) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;

SELECT k, random_tcbuffer_contseq(1, 100, 1, 100, 1, 100, 
  '2001-01-01', '2001-12-31', 10, 10, 10, 0, false)
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tcbuffer_contseq(1, 100, 1, 100, 1, 100, 
    '2001-01-01', '2001-12-31', 10, 10, 10, 5676, false) AS seq
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seq) FROM temp;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal circular buffer of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowradius, highradius Inclusive bounds of the range for the 
 *   radius
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants 
 *   in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID for the point of the cbuffer
 * @param[in] linear True when the sequence has linear interpolation
 */
DROP FUNCTION IF EXISTS random_tcbuffer_seqset;
CREATE FUNCTION random_tcbuffer_seqset(lowx float, highx float, lowy float,
  highy float, lowradius float, highradius float, lowtime timestamptz, 
  hightime timestamptz, maxminutes int, mincardseq int, maxcardseq int,
  mincard int, maxcard int, srid int DEFAULT 0, linear bool DEFAULT true)
  RETURNS tcbuffer AS $$
DECLARE
  result tcbuffer[];
  card int;
  seq tcbuffer;
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
    SELECT random_tcbuffer_contseq(lowx, highx, lowy, highy, lowradius,
      highradius, t1, t2, maxminutes, mincardseq, maxcardseq, srid, linear, 
      i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tcbufferSeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tcbuffer_seqset(1, 100, 1, 100, 1, 100, 
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS seqset
FROM generate_series (1, 15) AS k;

SELECT k, random_tcbuffer_seqset(1, 100, 1, 100, 1, 100, 
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false) AS seqset
FROM generate_series (1, 15) AS k;

WITH temp AS (
  SELECT k, random_tcbuffer_seqset(1, 100, 1, 100, 1, 100, 
    '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 5676, false) AS seqset
  FROM generate_series (1, 15) AS k )
SELECT DISTINCT srid(seqset) FROM temp;
*/

-------------------------------------------------------------------------------
